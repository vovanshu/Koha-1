package t::lib::Koha::BackgroundJob::BatchTest;

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
use JSON qw( decode_json encode_json );

use Koha::BackgroundJobs;
use Koha::DateUtils qw( dt_from_string );

use base 'Koha::BackgroundJob';

sub job_type {
    return 'batch_test';
}

sub process {
    my ( $self, $args ) = @_;

    my $job = Koha::BackgroundJobs->find( $args->{job_id} );

    if ( !exists $args->{job_id} || !$job || $job->status eq 'cancelled' ) {
        return;
    }

    my $job_progress = 0;
    $job->started_on(dt_from_string)
        ->progress($job_progress)
        ->status('started')
        ->store;

    my $report = {
        total_records => $job->size,
        total_success => 0,
    };
    my @messages;
    for my $i ( 0 .. $job->size - 1 ) {

        last if $job->get_from_storage->status eq 'cancelled';

        push @messages, {
            type => 'success',
            i => $i,
        };
        $report->{total_success}++;
        $job->progress( ++$job_progress )->store;
    }

    my $job_data = decode_json $job->data;
    $job_data->{messages} = \@messages;
    $job_data->{report} = $report;

    $job->ended_on(dt_from_string)
        ->data(encode_json $job_data);
    $job->status('finished') if $job->status ne 'cancelled';
    $job->store;
}

sub enqueue {
    my ( $self, $args) = @_;

    $self->SUPER::enqueue({
        job_size => $args->{size},
        job_args => {a => $args->{a}, b => $args->{b}}
    });
}

1;
