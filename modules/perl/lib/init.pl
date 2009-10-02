package Eir::Init;

=head1 init.pl -- perl initialisation for eir.

This file is processed on initialising the perl interpreter in C<perl.so>. It is
mainly concerned with setting up the Perl environment, maintaining the list of
loaded perl scripts, and providing the means to load and unload them.

Much of the code herein has been copied almost verbatim from the examples in
perlembed(1), with minor changes to reflect their purpose in the eir
environment.

=cut

use strict;
use warnings;

use FindBin;
use Symbol qw(delete_package);

use lib "$FindBin::Bin";
use lib "$FindBin::Bin/../api";

our %Scripts;

sub package_name_ify {
    my ($filename) = @_;

    $filename =~ s/([^A-Za-z0-9\/])/sprintf("_%2x",unpack("C",$1))/eg;
    # second pass only for words starting with a digit
    $filename =~ s|/(\d)|sprintf("/_%2x",unpack("C",$1))|eg;

    # Dress it up as a real package name
    $filename =~ s|/|::|g;
    return "Embed" . $filename;
}

sub load_script {
    call_wrapper(\&do_load_script, @_);
}

sub do_load_script {
    my ($filename) = @_;
    my $packagename = package_name_ify($filename);

    if (defined $Scripts{$packagename})
    {
        # This will always be called with G_EVAL, so this die can be translated
        # into a C++ exception.
        die "$filename is already loaded.";
    }

    local *FH;
    open FH, $filename or die "Couldn't open $filename: $!";
    local ($/) = undef;
    my $script_text = <FH>;
    close FH;

    my $eval = qq{package $packagename; $script_text };
    {
        my ($filename, $packagename, $script_text);
        eval $eval;
    }
    if ($@) {
        delete_package $packagename;
        die $@;
    }

    $Scripts{$packagename} = 1;
}

sub unload_script {
    my ($filename, $message) = @_;
    my $packagename = package_name_ify($filename);

    if (!defined $Scripts{$packagename})
    {
        die "$filename is not loaded";
    }

    eval "${packagename}::unload(\$message)";

    delete_package($packagename);
    delete $Scripts{$packagename};
}

sub do_eval {
    my ($text, $message) = @_;
    my $sub = sub {
            use Eir;
            sub reply { $message->reply(shift); }
            sub bot { $message->bot; }
            sub client { $message->source->{"client"} }
            eval "$text";
            die $@ if $@;
        };
    call_wrapper($sub);
}

sub call_wrapper {
    my $sub = shift;
    my $saved_alarm=$SIG{ALRM};
    $SIG{ALRM} = sub { die "Script used too much running time"; };
    eval {
        alarm 1;
        &$sub(@_);
    };
    alarm 0;
    $SIG{ALRM} = $saved_alarm;
    die $@ if $@;
}
