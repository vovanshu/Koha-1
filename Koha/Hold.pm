package Koha::Hold;

# Copyright ByWater Solutions 2014
# Copyright 2017 Koha Development team
#
# This file is part of Koha.
#
# Koha is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Koha is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Koha; if not, see <http://www.gnu.org/licenses>.

use Modern::Perl;

use Data::Dumper qw( Dumper );
use List::MoreUtils qw( any );

use C4::Context qw(preference);
use C4::Letters qw( GetPreparedLetter EnqueueLetter );
use C4::Log qw( logaction );

use Koha::AuthorisedValues;
use Koha::DateUtils qw( dt_from_string );
use Koha::Patrons;
use Koha::Biblios;
use Koha::Items;
use Koha::Libraries;
use Koha::Old::Holds;
use Koha::Calendar;

use Koha::Exceptions::Hold;

use base qw(Koha::Object);

=head1 NAME

Koha::Hold - Koha Hold object class

=head1 API

=head2 Class methods

=cut

=head3 age

returns the number of days since a hold was placed, optionally
using the calendar

my $age = $hold->age( $use_calendar );

=cut

sub age {
    my ( $self, $use_calendar ) = @_;

    my $today = dt_from_string;
    my $age;

    if ( $use_calendar ) {
        my $calendar = Koha::Calendar->new( branchcode => $self->branchcode );
        $age = $calendar->days_between( dt_from_string( $self->reservedate ), $today );
    }
    else {
        $age = $today->delta_days( dt_from_string( $self->reservedate ) );
    }

    $age = $age->in_units( 'days' );

    return $age;
}

=head3 suspend_hold

my $hold = $hold->suspend_hold( $suspend_until_dt );

=cut

sub suspend_hold {
    my ( $self, $dt ) = @_;

    my $date = $dt ? $dt->clone()->truncate( to => 'day' )->datetime : undef;

    if ( $self->is_found ) {    # We can't suspend found holds
        if ( $self->is_waiting ) {
            Koha::Exceptions::Hold::CannotSuspendFound->throw( status => 'W' );
        }
        elsif ( $self->is_in_transit ) {
            Koha::Exceptions::Hold::CannotSuspendFound->throw( status => 'T' );
        }
        elsif ( $self->is_in_processing ) {
            Koha::Exceptions::Hold::CannotSuspendFound->throw( status => 'P' );
        }
        else {
            Koha::Exceptions::Hold::CannotSuspendFound->throw(
                      'Unhandled data exception on found hold (id='
                    . $self->id
                    . ', found='
                    . $self->found
                    . ')' );
        }
    }

    $self->suspend(1);
    $self->suspend_until($date);
    $self->store();

    logaction( 'HOLDS', 'SUSPEND', $self->reserve_id, Dumper( $self->unblessed ) )
        if C4::Context->preference('HoldsLog');

    return $self;
}

=head3 resume

my $hold = $hold->resume();

=cut

sub resume {
    my ( $self ) = @_;

    $self->suspend(0);
    $self->suspend_until( undef );

    $self->store();

    logaction( 'HOLDS', 'RESUME', $self->reserve_id, Dumper($self->unblessed) )
        if C4::Context->preference('HoldsLog');

    return $self;
}

=head3 delete

$hold->delete();

=cut

sub delete {
    my ( $self ) = @_;

    my $deleted = $self->SUPER::delete($self);

    logaction( 'HOLDS', 'DELETE', $self->reserve_id, Dumper($self->unblessed) )
        if C4::Context->preference('HoldsLog');

    return $deleted;
}

=head3 set_transfer

=cut

sub set_transfer {
    my ( $self ) = @_;

    $self->priority(0);
    $self->found('T');
    $self->store();

    return $self;
}

=head3 set_waiting

=cut

sub set_waiting {
    my ( $self, $desk_id ) = @_;

    $self->priority(0);

    my $today = dt_from_string();
    my $values = {
        found => 'W',
        waitingdate => $today->ymd,
        desk_id => $desk_id,
    };

    my $requested_expiration;
    if ($self->expirationdate) {
        $requested_expiration = dt_from_string($self->expirationdate);
    }

    my $max_pickup_delay = C4::Context->preference("ReservesMaxPickUpDelay");
    my $cancel_on_holidays = C4::Context->preference('ExpireReservesOnHolidays');

    my $expirationdate = $today->clone;
    $expirationdate->add(days => $max_pickup_delay);

    if ( C4::Context->preference("ExcludeHolidaysFromMaxPickUpDelay") ) {
        my $itemtype = $self->item ? $self->item->effective_itemtype : $self->biblio->itemtype;
        my $daysmode = Koha::CirculationRules->get_effective_daysmode(
            {
                categorycode => $self->borrower->categorycode,
                itemtype     => $itemtype,
                branchcode   => $self->branchcode,
            }
        );
        my $calendar = Koha::Calendar->new( branchcode => $self->branchcode, days_mode => $daysmode );

        $expirationdate = $calendar->days_forward( dt_from_string(), $max_pickup_delay );
    }

    # If patron's requested expiration date is prior to the
    # calculated one, we keep the patron's one.
    my $cmp = $requested_expiration ? DateTime->compare($requested_expiration, $expirationdate) : 0;
    $values->{expirationdate} = $cmp == -1 ? $requested_expiration->ymd : $expirationdate->ymd;

    $self->set($values)->store();

    return $self;
}

=head3 is_pickup_location_valid

    if ($hold->is_pickup_location_valid({ library_id => $library->id }) ) {
        ...
    }

Returns a I<boolean> representing if the passed pickup location is valid for the hold.
It throws a I<Koha::Exceptions::_MissingParameter> if the library_id parameter is not
passed.

=cut

sub is_pickup_location_valid {
    my ( $self, $params ) = @_;

    Koha::Exceptions::MissingParameter->throw('The library_id parameter is mandatory')
        unless $params->{library_id};

    my @pickup_locations;

    if ( $self->itemnumber ) { # item-level
        @pickup_locations = $self->item->pickup_locations({ patron => $self->patron });
    }
    else { # biblio-level
        @pickup_locations = $self->biblio->pickup_locations({ patron => $self->patron });
    }

    return any { $_->branchcode eq $params->{library_id} } @pickup_locations;
}

=head3 set_pickup_location

    $hold->set_pickup_location(
        {
            library_id => $library->id,
          [ force   => 0|1 ]
        }
    );

Updates the hold pickup location. It throws a I<Koha::Exceptions::Hold::InvalidPickupLocation> if
the passed pickup location is not valid.

Note: It is up to the caller to verify if I<AllowHoldPolicyOverride> is set when setting the
B<force> parameter.

=cut

sub set_pickup_location {
    my ( $self, $params ) = @_;

    Koha::Exceptions::MissingParameter->throw('The library_id parameter is mandatory')
        unless $params->{library_id};

    if (
        $params->{force}
        || $self->is_pickup_location_valid(
            { library_id => $params->{library_id} }
        )
      )
    {
        # all good, set the new pickup location
        $self->branchcode( $params->{library_id} )->store;
    }
    else {
        Koha::Exceptions::Hold::InvalidPickupLocation->throw;
    }

    return $self;
}

=head3 set_processing

$hold->set_processing;

Mark the hold as in processing.

=cut

sub set_processing {
    my ( $self ) = @_;

    $self->priority(0);
    $self->found('P');
    $self->store();

    return $self;
}

=head3 is_found

Returns true if hold is waiting, in transit or in processing

=cut

sub is_found {
    my ($self) = @_;

    return 0 unless $self->found();
    return 1 if $self->found() eq 'W';
    return 1 if $self->found() eq 'T';
    return 1 if $self->found() eq 'P';
}

=head3 is_waiting

Returns true if hold is a waiting hold

=cut

sub is_waiting {
    my ($self) = @_;

    my $found = $self->found;
    return $found && $found eq 'W';
}

=head3 is_in_transit

Returns true if hold is a in_transit hold

=cut

sub is_in_transit {
    my ($self) = @_;

    return 0 unless $self->found();
    return $self->found() eq 'T';
}

=head3 is_in_processing

Returns true if hold is a in_processing hold

=cut

sub is_in_processing {
    my ($self) = @_;

    return 0 unless $self->found();
    return $self->found() eq 'P';
}

=head3 is_cancelable_from_opac

Returns true if hold is a cancelable hold

Holds may be only canceled if they are not found.

This is used from the OPAC.

=cut

sub is_cancelable_from_opac {
    my ($self) = @_;

    return 1 unless $self->is_found();
    return 0; # if ->is_in_transit or if ->is_waiting or ->is_in_processing
}

=head3 is_at_destination

Returns true if hold is waiting
and the hold's pickup branch matches
the hold item's holding branch

=cut

sub is_at_destination {
    my ($self) = @_;

    return $self->is_waiting() && ( $self->branchcode() eq $self->item()->holdingbranch() );
}

=head3 biblio

Returns the related Koha::Biblio object for this hold

=cut

sub biblio {
    my ($self) = @_;

    $self->{_biblio} ||= Koha::Biblios->find( $self->biblionumber() );

    return $self->{_biblio};
}

=head3 patron

Returns the related Koha::Patron object for this hold

=cut

sub patron {
    my ($self) = @_;

    my $patron_rs = $self->_result->patron;
    return Koha::Patron->_new_from_dbic($patron_rs);
}

=head3 item

Returns the related Koha::Item object for this Hold

=cut

sub item {
    my ($self) = @_;

    $self->{_item} ||= Koha::Items->find( $self->itemnumber() );

    return $self->{_item};
}

=head3 branch

Returns the related Koha::Library object for this Hold

=cut

sub branch {
    my ($self) = @_;

    $self->{_branch} ||= Koha::Libraries->find( $self->branchcode() );

    return $self->{_branch};
}

=head3 desk

Returns the related Koha::Desk object for this Hold

=cut

sub desk {
    my $self = shift;
    my $desk_rs = $self->_result->desk;
    return unless $desk_rs;
    return Koha::Desk->_new_from_dbic($desk_rs);
}

=head3 borrower

Returns the related Koha::Patron object for this Hold

=cut

# FIXME Should be renamed with ->patron
sub borrower {
    my ($self) = @_;

    $self->{_borrower} ||= Koha::Patrons->find( $self->borrowernumber() );

    return $self->{_borrower};
}

=head3 is_suspended

my $bool = $hold->is_suspended();

=cut

sub is_suspended {
    my ( $self ) = @_;

    return $self->suspend();
}


=head3 cancel

my $cancel_hold = $hold->cancel(
    {
        [ charge_cancel_fee => 1||0, ]
        [ cancellation_reason => $cancellation_reason, ]
    }
);

Cancel a hold:
- The hold will be moved to the old_reserves table with a priority=0
- The priority of other holds will be updated
- The patron will be charge (see ExpireReservesMaxPickUpDelayCharge) if the charge_cancel_fee parameter is set
- The canceled hold will have the cancellation reason added to old_reserves.cancellation_reason if one is passed in
- a CANCEL HOLDS log will be done if the pref HoldsLog is on

=cut

sub cancel {
    my ( $self, $params ) = @_;
    $self->_result->result_source->schema->txn_do(
        sub {
            $self->cancellationdate( dt_from_string->strftime( '%Y-%m-%d %H:%M:%S' ) );
            $self->priority(0);
            $self->cancellation_reason( $params->{cancellation_reason} );
            $self->store();

            if ( $params->{cancellation_reason} ) {
                my $letter = C4::Letters::GetPreparedLetter(
                    module                 => 'reserves',
                    letter_code            => 'HOLD_CANCELLATION',
                    message_transport_type => 'email',
                    branchcode             => $self->borrower->branchcode,
                    lang                   => $self->borrower->lang,
                    tables => {
                        branches    => $self->borrower->branchcode,
                        borrowers   => $self->borrowernumber,
                        items       => $self->itemnumber,
                        biblio      => $self->biblionumber,
                        biblioitems => $self->biblionumber,
                        reserves    => $self->unblessed,
                    }
                );

                if ($letter) {
                    C4::Letters::EnqueueLetter(
                        {
                            letter                   => $letter,
                            borrowernumber         => $self->borrowernumber,
                            message_transport_type => 'email',
                        }
                    );
                }
            }

            $self->_move_to_old;
            $self->SUPER::delete(); # Do not add a DELETE log

            # now fix the priority on the others....
            C4::Reserves::_FixPriority({ biblionumber => $self->biblionumber });

            # and, if desired, charge a cancel fee
            my $charge = C4::Context->preference("ExpireReservesMaxPickUpDelayCharge");
            if ( $charge && $params->{'charge_cancel_fee'} ) {
                my $account =
                  Koha::Account->new( { patron_id => $self->borrowernumber } );
                $account->add_debit(
                    {
                        amount     => $charge,
                        user_id    => C4::Context->userenv ? C4::Context->userenv->{'number'} : undef,
                        interface  => C4::Context->interface,
                        library_id => C4::Context->userenv ? C4::Context->userenv->{'branch'} : undef,
                        type       => 'RESERVE_EXPIRED',
                        item_id    => $self->itemnumber
                    }
                );
            }

            C4::Log::logaction( 'HOLDS', 'CANCEL', $self->reserve_id, Dumper($self->unblessed) )
                if C4::Context->preference('HoldsLog');
        }
    );
    return $self;
}

=head3 store

Override base store method to set default
expirationdate for holds.

=cut

sub store {
    my ($self) = @_;

    if ( !$self->in_storage ) {
        if (
            C4::Context->preference('DefaultHoldExpirationdate')
            and ( not defined $self->expirationdate
                or $self->expirationdate eq '' )
          )
        {
            $self->_set_default_expirationdate;
        }
    }
    else {

        my %updated_columns = $self->_result->get_dirty_columns;
        return $self->SUPER::store unless %updated_columns;

        if ( exists $updated_columns{reservedate} ) {
            if (
                C4::Context->preference('DefaultHoldExpirationdate')
                and ( not exists $updated_columns{expirationdate}
                    or exists $updated_columns{expirationdate}
                    and $updated_columns{expirationdate} eq '' )
              )
            {
                $self->_set_default_expirationdate;
            }
        }
    }

    $self = $self->SUPER::store;
}

sub _set_default_expirationdate {
    my $self = shift;

    my $period = C4::Context->preference('DefaultHoldExpirationdatePeriod') || 0;
    my $timeunit =
      C4::Context->preference('DefaultHoldExpirationdateUnitOfTime') || 'days';

    $self->expirationdate(
        dt_from_string( $self->reservedate )->add( $timeunit => $period ) );
}

=head3 _move_to_old

my $is_moved = $hold->_move_to_old;

Move a hold to the old_reserve table following the same pattern as Koha::Patron->move_to_deleted

=cut

sub _move_to_old {
    my ($self) = @_;
    my $hold_infos = $self->unblessed;
    return Koha::Old::Hold->new( $hold_infos )->store;
}

=head3 to_api_mapping

This method returns the mapping for representing a Koha::Hold object
on the API.

=cut

sub to_api_mapping {
    return {
        reserve_id       => 'hold_id',
        borrowernumber   => 'patron_id',
        reservedate      => 'hold_date',
        biblionumber     => 'biblio_id',
        branchcode       => 'pickup_library_id',
        notificationdate => undef,
        reminderdate     => undef,
        cancellationdate => 'cancellation_date',
        reservenotes     => 'notes',
        found            => 'status',
        itemnumber       => 'item_id',
        waitingdate      => 'waiting_date',
        expirationdate   => 'expiration_date',
        lowestPriority   => 'lowest_priority',
        suspend          => 'suspended',
        suspend_until    => 'suspended_until',
        itemtype         => 'item_type',
        item_level_hold  => 'item_level',
    };
}

=head2 Internal methods

=head3 _type

=cut

sub _type {
    return 'Reserve';
}

=head1 AUTHORS

Kyle M Hall <kyle@bywatersolutions.com>
Jonathan Druart <jonathan.druart@bugs.koha-community.org>
Martin Renvoize <martin.renvoize@ptfs-europe.com>

=cut

1;
