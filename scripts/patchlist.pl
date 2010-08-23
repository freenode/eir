#!/usr/bin/perl

# eircchi.pl - Hacchi replacement written for Eir
# Copyright 2010 Anders Ossowicki <and@vmn.dk>
# Copyright 2010 Stephen Bennett <spb@exherbo.org>
# Released under the GNU General Public License v2

use strict;
use warnings;

use Eir;
use Time::Duration;
use LWP::UserAgent;
use Data::Dumper;

our @handlers = (
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "pq" }), \&patchqueue ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "patchqueue" }), \&patchqueue ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "pl" }), \&patchlist ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "patchlist" }), \&patchlist ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "pd" }), \&patchdone ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "patchdone" }), \&patchdone ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "pad", privilege => "patchadmin" }), \&patchalldone ),
    Eir::CommandRegistry::add_handler( Eir::Filter->new({ command => "patchalldone", privilege => "patchadmin" }), \&patchalldone ),
    Eir::EventManager::add_recurring_event(3600, \&slacker_alert)
);

my ($patches, $done_patches);
eval {
    $patches = Eir::Storage::Load("patch_list");
    $done_patches = Eir::Storage::Load("done_patch_list");
};
if ($@) {
    $patches = Eir::Settings::Add("patch_list", [ ]);
    $done_patches = Eir::Settings::Add("done_patch_list", [ ]);
} else {
    $patches = Eir::Settings::Add("patch_list", $patches);
    $done_patches = Eir::Settings::Add("done_patch_list", $done_patches);
}

Eir::Storage::auto_save($patches, "patch_list");
Eir::Storage::auto_save($done_patches, "done_patch_list");

my $bot = Eir::find_bot('eir');

sub patchlist {
    my $message = shift;
    my $match = $message->args->[0];

    $message->reply(pluralise_patch(scalar @$patches) . " in queue:");

    foreach my $patch (@$patches) {
        if (!$match || match_patch($patch, $match)) {
            send_patch($message, $patch);
        }
    }
}

sub patchqueue {
    my $message = shift;
    my $nick = $message->source->{name};
    my $summary = "";
    my @args = @{$message->args};
    my ($url, $saneurl, $repo, $comment);

    if ($args[0] =~ /^http:/) {
        $url = shift @args;
    }
    if ($args[0] =~ /^::/) {
        $repo = shift @args;
    }
    $comment = join " ", @args;

    if (my ( $id ) = ($url =~ qr|^http://dpaste.com/(\d+)(?:/(?:plain(?:/)?)?)?(\s+.*)?$|)) {
        $saneurl = "http://dpaste.com/$id/plain";
    }
    elsif (( $id ) = ($url =~ qr|^http://ix.io/(\w+)|)) {
        $saneurl = "http://ix.io/$id";
    }
    elsif (( $id ) = ($url =~ qr|^http://paste.pocoo.org/(?:show\|raw)/(\d+)|)) {
        $saneurl = "http://paste.pocoo.org/raw/$id";
    }

    if ($saneurl) {
        my $ua = LWP::UserAgent->new;
        my $response = $ua->get($saneurl);
        if ($response->is_success) {
            my $subject = "";
            my @lines = split(/\n/, $response->decoded_content);
            foreach my $line (@lines[0..9]) {
                last unless $line;
                last if ( $subject ) = ($line =~ /^Subject: (.+)$/);
            }
            $summary = $subject if $subject;
        }
        else {
            $message->reply("Got HTTP " . $response->code() . " for $saneurl");
        }
    }

    push @$patches, {
                nick => $nick,
                time => time,
                url => $url,
                summary => $summary,
                repo => $repo,
                comment => $comment,
            };
    $summary = "Queued '" . $summary . "' in $repo. " if $summary;
    $message->reply($summary . "Now " . pluralise_patch(scalar @$patches)
                  . " in queue" );
}

sub patchdone {
    my $message = shift;
    my $match = $message->args->[0];

    if (!$match) {
        $message->reply("Tell me what to mark done.");
        return;
    }

    my @tmp;
    my $deleted = 0;
    foreach my $i (0 .. scalar(@$patches)-1) {
        if (match_patch($patches->[$i], $match))
        {
            $deleted++;
            push @$done_patches, { 
                        %{$patches->[$i]},
                        deleted_time => time,
                        deleted_by => $message->source->{name}
                    };
            delete $patches->[$i];
        }
    }
    if (!$deleted) {
        $message->reply("No patches found matching $match");
        return;
    }
    $message->reply("Marked " . pluralise_patch($deleted) . " done.");
}

sub patchalldone {
    my $message = shift;
    push @$done_patches, @$patches;
    while (scalar @$patches) {
        delete $patches->[0];
    }
    $message->reply("All patches marked done.");
}

sub slacker_alert {
    return unless (@$patches);

    my @slackerchannels = split ',', $bot->Settings->{'slackerchannels'};

    my %repo;
    foreach my $patch (@$patches) {
        ++$repo{$patch->{repo}};
    }

    my $msg = pluralise_patch(scalar @$patches) . " in queue ... slackers!";

    if (scalar keys %repo) {
        $msg .= " (" . (join ", ", map {$repo{$_} . " in $_"} keys %repo) . ")";
    }

    foreach my $channel (@slackerchannels) {
        $bot->send("PRIVMSG $channel :$msg");
    }
}

sub pluralise_patch {
    my $num = shift;
    if ( $num == 1 ) {
        return '1 patch';
    }
    return "$num patches";
}

sub match_patch {
    my ($patch, $match) = @_;
    return ( $patch->{nick}    =~ /\Q$match\E/
          or $patch->{url}     =~ /\Q$match\E/
          or $patch->{repo}    =~ /\Q$match\E/
          or $patch->{comment} =~ /\Q$match\E/
          or $patch->{summary} =~ /\Q$match\E/ );
}

sub send_patch {
    my ($message, $patch) = @_;
    my $age = time - $patch->{time};
    $message->reply($patch->{url} . " " . $patch->{repo} . " " . $patch->{comment} .
                    " (submitted by " . $patch->{nick} .
                    " " . ago($age) . "): " . $patch->{summary});
}


# vim: set tw=120 sw=4 sts=4 et :
