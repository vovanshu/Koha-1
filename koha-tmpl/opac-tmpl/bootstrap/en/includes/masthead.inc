[% USE raw %]
[% USE Koha %]
[% USE Branches %]
[% USE Categories %]
[% USE KohaNews %]
[% PROCESS 'html_helpers.inc' %]
[% SET OpacLangSelectorMode = Koha.Preference('OpacLangSelectorMode') %]
[% SET OpacHeader = KohaNews.get( location => "opacheader", lang => lang, library => branchcode, blocktitle => 0 ) %]
[% SET OpacCustomSearch = KohaNews.get( location => "OpacCustomSearch", lang => lang, library => branchcode, blocktitle => 0 ) %]
[% SET OpacLoginInstructions = KohaNews.get( location => "OpacLoginInstructions", lang => lang, library => branchcode ) %]

<div id="wrapper">
    <div id="header-region" class="noprint">
        <nav class="navbar navbar-expand">
            <button id="scrolltocontent">Skip to main content</button>
            <div id="logo">
            <a class="navbar-brand" href="/cgi-bin/koha/opac-main.pl">
                [% IF ( LibraryNameTitle ) %]
                    [% LibraryNameTitle | html %]
                [% ELSE %]
                    Koha online
                [% END %]
            </a>
            </div>
            [% IF ( Koha.Preference( 'opacbookbag' ) == 1 ) %]
                <div id="cartDetails" class="cart-message">Your cart is empty.</div>
            [% END %]
            <ul id="cart-list-nav" class="navbar-nav">
                [% IF ( Koha.Preference( 'opacbookbag' ) == 1 ) %]
                    <li class="nav-item js-show">
                        <a href="#" class="nav-link" title="Collect items you are interested in" id="cartmenulink" role="button">
                            <i id="carticon" class="fa fa-shopping-cart fa-icon-black" aria-hidden="true"></i> <span class="cartlabel">Cart</span> <span id="basketcount"></span>
                        </a>
                    </li>
                [% END %]
                <li class="divider-vertical"></li>
                [% IF ( Koha.Preference( 'virtualshelves' ) == 1 ) %]
                    <li class="nav-item dropdown">
                        <a href="/cgi-bin/koha/opac-shelves.pl" title="Show lists" class="nav-link dropdown-toggle" id="listsmenu" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"
                            ><i class="fa fa-list fa-icon-black" aria-hidden="true"></i> <span class="listslabel">Lists</span>
                        </a>
                        <div aria-labelledby="listsmenu" role="menu" class="dropdown-menu">
                            [% IF some_public_shelves.count %]
                                <a class="dropdown-item" href="/cgi-bin/koha/opac-shelves.pl?op=list&amp;category=2" tabindex="-1" role="menuitem"><strong>Public lists</strong></a>
                                [% SET number_of_public_shelves = 0 %]
                                [% FOREACH s IN some_public_shelves %]
                                    <a class="dropdown-item" href="/cgi-bin/koha/opac-shelves.pl?op=view&amp;shelfnumber=[% s.shelfnumber | uri %]&amp;sortfield=[% s.sortfield | uri %]" tabindex="-1" role="menuitem">[% s.shelfname | html %]</a>
                                    [% SET number_of_public_shelves = number_of_public_shelves + 1 %]
                                    [% IF number_of_public_shelves >= 10 %][% LAST %][% END %]
                                [% END %]
                                [% IF some_public_shelves > 10 %]
                                    <a class="dropdown-item listmenulink" href="/cgi-bin/koha/opac-shelves.pl?op=list&amp;category=2" tabindex="-1" role="menuitem">View All</a>
                                [% END %]
                            [% END %]
                            [% IF some_public_shelves.count > 0 %]
                                <div class="dropdown-divider"></div>
                            [% END %]
                            [% IF Koha.Preference( 'opacuserlogin' ) == 1 %]
                                <a class="dropdown-item" href="/cgi-bin/koha/opac-shelves.pl?op=list&amp;category=1" tabindex="-1" role="menuitem"><strong>Your lists</strong></a>
                                [% IF loggedinusername %]
                                    [% IF some_private_shelves.count %]
                                        [% SET number_of_private_shelves = 0 %]
                                        [% FOREACH s IN some_private_shelves %]
                                            <a class="dropdown-item" href="/cgi-bin/koha/opac-shelves.pl?op=view&amp;shelfnumber=[% s.shelfnumber | uri %]&amp;sortfield=[% s.sortfield | uri %]" tabindex="-1" role="menuitem">[% s.shelfname | html %]</a>
                                            [% SET number_of_private_shelves = number_of_private_shelves + 1 %]
                                            [% IF number_of_private_shelves >= 10 %][% LAST %][% END %]
                                        [% END %]
                                        [% IF some_private_shelves > 10 %]
                                            <a class="dropdown-item listmenulink" href="/cgi-bin/koha/opac-shelves.pl?op=list&amp;category=1" tabindex="-1" role="menuitem">View All</a>
                                        [% END %]
                                    [% ELSE %]
                                        <a class="dropdown-item disabled" href="#" tabindex="-1" role="menuitem">No private lists</a>
                                    [% END %]
                                        <a class="dropdown-item listmenulink" href="/cgi-bin/koha/opac-shelves.pl?op=add_form" tabindex="-1" role="menuitem">New list</a>
                                [% ELSE %]
                                    <a class="dropdown-item" href="/cgi-bin/koha/opac-shelves.pl?op=add_form" tabindex="-1" role="menuitem">Log in to create your own lists</a>
                                [% END # / IF loggedinusername %]
                            [% END # / IF opacuserlogin %]
                        </div> <!-- / .dropdown-menu -->
                    </li> <!-- / .nav-item.dropdown -->
                [% END # / IF virtualshelves %]
            </ul> <!-- / .navbar-nav -->

            [% IF Koha.Preference( 'opacuserlogin' ) == 1 ||  Koha.Preference( 'EnableOpacSearchHistory') || Koha.Preference( 'opaclanguagesdisplay' ) %]
                [% IF ( Koha.Preference( 'opaclanguagesdisplay' ) ) %]
                    [% INCLUDE 'masthead-langmenu.inc' %]
                [% END %]
                <ul id="members" class="navbar-nav">
                    <li class="nav-item dropdown">
                        [% IF ( Koha.Preference( 'opacuserlogin' ) == 1 ) %]
                            [% IF ( loggedinusername ) %]
                                <a href="/cgi-bin/koha/opac-user.pl" class="dropdown-toggle" id="user-menu" data-toggle="dropdown" role="button">
                                    <i class="fa fa-user fa-icon-black fa-fw" aria-hidden="true"></i>
                                    <span class="userlabel">Welcome, [% INCLUDE 'patron-title.inc' patron = logged_in_user no_html = 1 %]</span>
                                </a>
                            [% ELSE %]
                                [% IF Koha.Preference('casAuthentication') %]
                                    [%# CAS authentication is too complicated for modal window %]
                                        <a class="nav-link login-link" href="/cgi-bin/koha/opac-user.pl"><i class="fa fa-user fa-icon-black fa-fw" aria-hidden="true"></i> <span class="userlabel">Log in to your account</span></a>
                                [% ELSIF ( Koha.Preference('GoogleOpenIDConnect') == 1 ) %]
                                    <a class="nav-link login-link" href="/cgi-bin/koha/svc/auth/googleopenidconnect" role="button" data-toggle="modal"><i class="fa fa-user fa-icon-black fa-fw" aria-hidden="true"></i> <span class="userlabel">Log in to your account</span></a>
                                [% ELSE %]
                                    <a href="/cgi-bin/koha/opac-user.pl" class="nav-link login-link loginModal-trigger"><i class="fa fa-user fa-icon-black fa-fw" aria-hidden="true"></i> <span class="userlabel">Log in to your account</span></a>
                                [% END %]
                            [% END %]
                        [% END %]

                        [% IF ( Koha.Preference( 'opacuserlogin' ) == 1 && loggedinusername ) %]
                            <div aria-labelledby="user-menu" role="menu" class="dropdown-menu dropdown-menu-right">
                                <div id="loggedinuser-menu">
                                    <p>
                                        <a class="login-link" href="/cgi-bin/koha/opac-user.pl"><span class="loggedinusername" data-borrowernumber="[% logged_in_user.borrowernumber | html %]" data-branchcode="[% logged_in_user.branchcode | html %]" data-categorycode="[% logged_in_user.categorycode | html %]">Your account</span></a>
                                    </p>
                                    [% IF Koha.Preference( 'EnableOpacSearchHistory' ) %]
                                        <p>
                                            <a tabindex="-1" role="menuitem" class="login-link" href="/cgi-bin/koha/opac-search-history.pl" title="View your search history">Search history</a>
                                            <span class="divider-vertical"></span>
                                            <a class="clearsh" href="/cgi-bin/koha/opac-search-history.pl?action=delete" title="Delete your search history"><i class="fa fa-trash" aria-hidden="true"></i> Clear</a>
                                        </p>
                                    [% END %]
                                    <a tabindex="-1" role="menuitem" class="logout" id="logout" href="/cgi-bin/koha/opac-main.pl?logout.x=1">Log out</a>
                                </div>
                            </div>
                        [% END %]
                    </li>
                    [% IF ( !Koha.Preference( 'opacuserlogin') || !loggedinusername ) %]
                        [% IF Koha.Preference( 'EnableOpacSearchHistory' ) %]
                            <li class="nav-item search_history">
                                <a class="nav-link login-link" href="/cgi-bin/koha/opac-search-history.pl" title="View your search history">Search history</a>
                            </li>
                            <li class="divider-vertical"></li>
                            <li class="nav-item">
                                <a class="nav-link logout clearsh" href="/cgi-bin/koha/opac-search-history.pl?action=delete" title="Delete your search history"><i class="fa fa-trash" aria-hidden="true"></i> Clear</a>
                            </li>
                        [% END %]
                    [% END %]
                </ul>
            [% END # IF opacuserlogin || EnableOpacSearchHistory || opaclanguagesdisplay %]
        </nav> <!-- /navbar -->

        [% IF ( OpacHeader ) %]
            <div class="container-fluid">
                <div class="row">
                    <div class="col">
                        [% PROCESS koha_news_block news => OpacHeader %]
                    </div>
                </div>
            </div>
        [% END %]

    </div> <!-- / header-region -->

    [% IF ( OpacPublic ) %]
        <div class="container-fluid">
            [% UNLESS ( advsearch ) %]
                <div class="row">
                    <div class="col">
                            <div id="opac-main-search">
                                [% IF ( OpacCustomSearch ) %]
                                    [% PROCESS koha_news_block news => OpacCustomSearch %]
                                [% ELSE %]
                                    <form name="searchform" method="get" action="/cgi-bin/koha/opac-search.pl" id="searchform">
                                        <div class="form-row align-items-center">
                                            <div class="col-sm-auto order-1 order-sm-1">
                                                <label for="masthead_search"> Search
                                                    [% UNLESS ( Koha.Preference( 'OpacAddMastheadLibraryPulldown' ) == 1 ) %]
                                                        [% IF ( mylibraryfirst ) %]
                                                            (in [% Branches.GetName( mylibraryfirst ) | html %] only)
                                                        [% END %]
                                                    [% END %]
                                                </label>
                                            </div>
                                            <div class="col-sm-auto order-2 order-sm-2">
                                                <select name="idx" id="masthead_search" class="form-control">
                                                    [% IF ( ms_kw ) %]
                                                        <option selected="selected" value="">Library catalog</option>
                                                    [% ELSE %]
                                                        <option value="">Library catalog</option>
                                                    [% END # /ms_kw %]
                                                    [% IF ( ms_ti ) %]
                                                        <option selected="selected" value="ti">Title</option>
                                                    [% ELSE %]
                                                        <option value="ti">Title</option>
                                                    [% END # /ms_ti %]
                                                    [% IF ( ms_au ) %]
                                                        <option selected="selected" value="au">Author</option>
                                                    [% ELSE %]
                                                        <option value="au">Author</option>
                                                    [% END # /ms_au%]
                                                    [% IF ( ms_su ) %]
                                                        <option selected="selected" value="su">Subject</option>
                                                    [% ELSE %]
                                                        <option value="su">Subject</option>
                                                    [% END # /ms_su %]
                                                    [% IF ( ms_nb ) %]
                                                        <option selected="selected" value="nb">ISBN</option>
                                                    [% ELSE %]
                                                        <option value="nb">ISBN</option>
                                                    [% END # /ms_nb%]
                                                    [% IF ( ms_ns ) %]
                                                        <option selected="selected" value="ns">ISSN</option>
                                                    [% ELSE %]
                                                        <option value="ns">ISSN</option>
                                                    [% END # /ms_ns%]
                                                    [% IF ( ms_se ) %]
                                                        <option selected="selected" value="se">Series</option>
                                                    [% ELSE %]
                                                        <option value="se">Series</option>
                                                    [% END # /ms_se %]
                                                    [% IF ( Koha.Preference('OPACNumbersPreferPhrase') ) %]
                                                        [% IF ( ms_callnumcommaphr ) %]
                                                            <option selected="selected" value="callnum,phr">Call number</option>
                                                        [% ELSE %]
                                                            <option value="callnum,phr">Call number</option>
                                                        [% END #/ms_callnumcommaphr %]
                                                    [% ELSE %]
                                                        [% IF ( ms_callnum ) %]
                                                            <option selected="selected" value="callnum">Call number</option>
                                                        [% ELSE %]
                                                            <option value="callnum">Call number</option>
                                                        [% END # /ms_callnum %]
                                                    [% END # /IF OPACNumbersPreferPhrase %]
                                                </select>
                                            </div> <!-- /.col-sm-auto -->

                                            <div class="col order-4 order-sm-3">
                                                [% IF ( ms_value ) %]
                                                    <input type="text" title="Type search term" class="transl1 form-control" id="translControl1" name="q" value="[% ms_value | html %]" /><span id="translControl"></span>
                                                [% ELSE %]
                                                    <input type="text" title="Type search term" class="transl1 form-control" id="translControl1" name="q" /><span id="translControl"></span>
                                                [% END # /ms_value %]
                                            </div> <!-- /.col -->

                                            [% IF ( Koha.Preference( 'OpacAddMastheadLibraryPulldown' ) == 1 ) %]
                                                <div class="col-sm col-md-3 col-lg-2 order-3 order-sm-4">
                                                    <select name="branch_group_limit" id="select_library" class="form-control">
                                                        <option value="">All libraries</option>

                                                        [% IF LibrarySearchGroups %]<optgroup label="Libraries">[% END %]

                                                        [% FOREACH library IN Branches.all( selected => opac_name ) %]
                                                            [% IF library.selected %]
                                                                <option selected="selected" value="branch:[% library.branchcode | html %]">[% library.branchname | html %]</option>
                                                            [% ELSE %]
                                                                <option value="branch:[% library.branchcode | html %]">[% library.branchname | html %]</option>
                                                            [% END %]
                                                        [% END %]

                                                        [% IF LibrarySearchGroups %]
                                                            </optgroup>
                                                            <optgroup label="Groups">
                                                                [% FOREACH lsg IN LibrarySearchGroups %]
                                                                    [% IF lsg.id == opac_name %]
                                                                        <option selected="selected" value="multibranchlimit-[% lsg.id | html %]">[% lsg.title | html %]</option>
                                                                    [% ELSE %]
                                                                        <option value="multibranchlimit-[% lsg.id | html %]">[% lsg.title | html %]</option>
                                                                    [% END # / bc.selected %]
                                                                [% END %]
                                                            </optgroup>
                                                        [% END # / BranchCategoriesLoop %]
                                                    </select> <!-- /#select_library -->
                                                </div> <!-- /.col-sm-auto -->
                                                <div class="col-sm-auto order-5">
                                                    <button type="submit" id="searchsubmit" class="btn btn-primary">Go</button>
                                                </div>
                                            [% ELSE %]
                                                [% IF ( opac_limit_override ) %]
                                                    [% IF ( opac_search_limit ) %]
                                                        [% IF ( multibranchlimit = opac_search_limit.match( 'branch:(multibranchlimit-\d+)' ) ) %]
                                                            <input name="branch_group_limit" value="[% multibranchlimit.0 | html %]" type="hidden" />
                                                        [% ELSE %]
                                                            <input name="limit" value="[% opac_search_limit | html %]" type="hidden" />
                                                        [% END %]
                                                    [% END %]
                                                [% ELSE %]
                                                    [% IF ( mylibraryfirst ) %]
                                                        <input name="limit" value="branch:[% mylibraryfirst | html %]" type="hidden" />
                                                    [% END %]
                                                [% END # / opac_limit_override %]
                                                <div class="order-5 col-sm-auto">
                                                    <button type="submit" id="searchsubmit" class="btn btn-primary">Go</button>
                                                </div>
                                            [% END # / OpacAddMastheadLibraryPulldown %]
                                        </div> <!-- /.form-row -->
                                        <input type="hidden" name="weight_search" value="1">
                                    </form>
                                [% END # / ( OpacCustomSearch ) %]
                            </div><!-- /#opac-main-search -->
                    </div> <!-- /.col -->
                </div> <!-- /.row -->
            [% END # / UNLESS advsearch %]

            <div class="row">
                <ul class="nav" id="moresearches">
                    <li class="nav-item">
                        <a href="/cgi-bin/koha/opac-search.pl">Advanced search</a>
                    </li>
                    [% IF ( Koha.Preference( 'UseCourseReserves' ) == 1 ) %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-course-reserves.pl">Course reserves</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'OpacBrowser' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-browser.pl">Browse by hierarchy</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'OpacAuthorities' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-authorities-home.pl">Authority search</a>
                        </li>
                    [% END %]
                    [% IF ( ( Koha.Preference( 'opacuserlogin' ) == 1 ) && ( Koha.Preference( 'OPACComments' ) == 1 ) && ( Koha.Preference( 'OpacShowRecentComments' ) == 1 ) ) %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-showreviews.pl">Recent comments</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'TagsEnabled' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-tags.pl">Tag cloud</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'OpacCloud' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-tags_subject.pl">Subject cloud</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'OpacTopissue' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-topissues.pl">Most popular</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference('SearchEngine') == 'Elasticsearch' && Koha.Preference( 'OpacBrowseSearch' ) == 1 %]
                        <li class="nav-item">
                            <a href="/cgi-bin/koha/opac-browse.pl">Browse search</a>
                        </li>
                    [% END %]
                    [% IF Koha.Preference( 'suggestion' ) == 1 %]
                        [% IF Koha.Preference( 'AnonSuggestions' ) == 1 || Koha.Preference( 'OPACViewOthersSuggestions' ) == 1 %]
                            <li class="nav-item">
                                <a href="/cgi-bin/koha/opac-suggestions.pl?suggested_by_anyone=1">Purchase suggestions</a>
                            </li>
                        [% END %]
                    [% END %]
                    <li class="nav-item">
                        <a id="library_page" href="/cgi-bin/koha/opac-library.pl">
                            [% IF ( singleBranchMode ) %]
                                Library
                            [% ELSE %]
                                Libraries
                            [% END %]
                        </a>
                    </li>
                    [% Koha.Preference('OpacMoreSearches') | $raw %]
                </ul> <!-- /.nav#moresearches -->
            </div> <!-- /.row -->
        </div> <!-- /.container-fluid -->
    [% END # / OpacPublic %]

    <!-- Login form hidden by default, used for modal window -->
    <div id="loginModal" class="modal" tabindex="-1" role="dialog" aria-labelledby="modalLoginLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <h2 class="modal-title" id="modalLoginLabel">Log in to your account</h2>
                    <button type="button" class="closebtn" data-dismiss="modal" aria-label="Close">
                        <span aria-hidden="true">&times;</span>
                    </button>
                </div>
                <form action="/cgi-bin/koha/opac-user.pl" method="post" name="auth" id="modalAuth">
                    <input type="hidden" name="has-search-query" id="has-search-query" value="" />
                    <div class="modal-body">
                        [% IF ( shibbolethAuthentication ) %]
                            [% IF ( invalidShibLogin ) %]
                                <!-- This is what is displayed if shibboleth login has failed to match a koha user -->
                                <div class="alert alert-info">
                                    <p>Sorry, your Shibboleth identity does not match a valid library identity. If you have a local login, you may use that below.</p>
                                </div>
                            [% ELSE %]
                                <h3>Shibboleth login</h3>
                                <p><a href="[% shibbolethLoginUrl | $raw %]">Log in using a Shibboleth account</a>.</p>
                                [% UNLESS Koha.Preference('opacShibOnly') %]
                                    <h3>Local Login</h3>
                                [% END %]
                            [% END %]
                        [% END %]
                        [% UNLESS Koha.Preference('opacShibOnly') %]
                            <input type="hidden" name="koha_login_context" value="opac" />
                            <fieldset class="brief">
                                <label for="muserid">Login:</label><input type="text" id="muserid" name="userid" />
                                <label for="mpassword">Password:</label><input type="password" id="mpassword" name="password" />
                                [% IF OpacLoginInstructions %]
                                    <div id="nologininstructions-modal" class="nologininstructions">
                                        [% PROCESS koha_news_block news => OpacLoginInstructions %]
                                    </div>
                                [% END %]
                                [% IF Koha.Preference('OpacPasswordChange') && Categories.can_any_reset_password %]
                                    <div id="forgotpassword-modal" class="forgotpassword">
                                        <a href="/cgi-bin/koha/opac-password-recovery.pl">Forgot your password?</a>
                                    </div>
                                [% END %]
                                [% IF PatronSelfRegistration && PatronSelfRegistrationDefaultCategory %]
                                    <div id="patronregistration-modal" class="patronregistration">
                                        <p>Don't have an account? <a href="/cgi-bin/koha/opac-memberentry.pl">Register here.</a></p>
                                    </div>
                                [% END %]
                            </fieldset>
                        [% END %]
                    </div>
                    [% UNLESS Koha.Preference('opacShibOnly') %]
                        <div class="modal-footer">
                            <input type="submit" class="btn btn-primary" value="Log in" />
                        </div>
                    [% END %]
                </form> <!-- /#auth -->
            </div> <!-- /.modal-content -->
        </div> <!-- /.modal-dialog -->
    </div>  <!-- /#modalAuth  -->
