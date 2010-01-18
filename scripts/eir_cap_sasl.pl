#!/usr/bin/perl
#
# eir_cap_sasl.pl
# Copyright 2010 Mike Quin
#
# Implements PLAIN and DH-BLOWFISH SASL authentication mechanisms for use with charybdis ircds, and enables CAP MULTI-PREFIX
# Incoprorates code from cap_sasl.pl by Michael Tharp and Jilles Tjoelker
# Licensed under the GNU General Public License
#
# to configure this script, define the following in eir.conf:
#
# set sasl_user myusername
# set sasl_password mypassword
# set sasl_mechanism DH-BLOWFISH

use strict;
use Eir;
use MIME::Base64;

our $bot=Eir::find_bot('eir');

our %sasl_auth;
our %mech = ();

# IRC event handlers
our $timeout;
our @handlers = (
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'on_connect',type => Eir::Source::Internal}),\&server_connected),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'CAP',type => Eir::Source::RawIrc}),\&event_cap),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'AUTHENTICATE',type => Eir::Source::RawIrc}),\&event_authenticate),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '903',type => Eir::Source::RawIrc}),\&event_saslend),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '904',type => Eir::Source::RawIrc}),\&event_saslend),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '905',type => Eir::Source::RawIrc}),\&event_saslend),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '906',type => Eir::Source::RawIrc}),\&event_saslend),
    Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '907',type => Eir::Source::RawIrc}),\&event_saslend)
    );

sub server_connected {
    our $bot;
    $bot->send('CAP LS');
}

sub event_cap {
    our %bot;
    our %sasl_auth;
    our $timeout;
    my ($message) = @_;
    my @args=@{$message->args};
    my ($subcmd, $caps, $tosend);

    $tosend = '';
    $subcmd = $args[0];
    $caps = ' '.$args[1].' ';
    if ($subcmd eq 'LS') {
       $tosend .= ' multi-prefix' if $caps =~ / multi-prefix /i;
       $tosend .= ' sasl' if $caps =~ / sasl /i;
       $tosend =~ s/^ //;
       if ($tosend eq '') {
           $bot->send("CAP END");
       } else {
           $bot->send("CAP REQ :$tosend");
       }
    } elsif ($subcmd eq 'ACK') {
       if ($caps =~ / sasl /i) {
           $sasl_auth{buffer} = '';
           $sasl_auth{user}=$bot->Settings->{'sasl_user'};
           $sasl_auth{password}=$bot->Settings->{'sasl_password'};
           $sasl_auth{mech}=$bot->Settings->{'sasl_mechanism'};

           if($mech{$sasl_auth{mech}}) {
               $bot->send("AUTHENTICATE " . $sasl_auth{mech});
               $timeout=Eir::EventManager::add_event(time()+10, \&timeout);
           }else{
               print 'SASL: attempted to start unknown mechanism "' . $sasl_auth{mech} . '"' . "\n";
           }
       }
    } elsif ($subcmd eq 'NAK') {
       print "CLICAP: refused:$caps\n";
       $bot->send("CAP END");
    } elsif ($subcmd eq 'LIST') {
       print "CLICAP: currently enabled:$caps\n";
       return;
    }
}

sub event_authenticate {
    our %bot;
    our %sasl_auth;
    my ($message) = @_;
    my $raw=$message->raw;
    my $sasl = \%sasl_auth;
    my $args;

    if ($raw=~/^AUTHENTICATE (.*)/) {
           $args=$1;
    } else {
           print "SASL: Received AUTHENTICATE with no parameters, aborting\n";
           timeout();
    }

    $sasl->{buffer} .= $args;
    return if length($args) == 400;

    my $data = $sasl->{buffer} eq '+' ? '' : decode_base64($sasl->{buffer});
    my $out = $mech{$sasl->{mech}}($sasl, $data);
    $out = '' unless defined $out;
    $out = $out eq '' ? '+' : encode_base64($out, '');

    while(length $out >= 400) {
       my $subout = substr($out, 0, 400, '');
       $bot->send("AUTHENTICATE $subout");
    }
    if(length $out) {
       $bot->send("AUTHENTICATE $out");
    }else{ # Last piece was exactly 400 bytes, we have to send some padding to indicate we're done
       $bot->send("AUTHENTICATE +");
    }
    $sasl->{buffer} = '';
}

sub event_saslend {
    our %bot;
    $bot->send("CAP END");
    undef $timeout;
}

sub timeout {
    our $bot;
    print "SASL: authentication timed out\n";
    $bot->send("CAP END");
}

$mech{PLAIN} = sub {
    my($sasl, $data) = @_;
    my $u = $sasl->{user};
    my $p = $sasl->{password};

    join("\0", $u, $u, $p);
};

eval {
    use Crypt::OpenSSL::Bignum;
    use Crypt::DH;
    use Crypt::Blowfish;
    use Math::BigInt;
    sub bin2bi { return Crypt::OpenSSL::Bignum->new_from_bin(shift)->to_decimal } # binary to BigInt
    sub bi2bin { return Crypt::OpenSSL::Bignum->new_from_decimal((shift)->bstr)->to_bin } # BigInt to binary
    $mech{'DH-BLOWFISH'} = sub {
       my($sasl, $data) = @_;
       my $u = $sasl->{user};
       my $pass = $sasl->{password};

       # Generate private key and compute secret key
       my($p, $g, $y) = unpack("(n/a*)3", $data);
       my $dh = Crypt::DH->new(p => bin2bi($p), g => bin2bi($g));
       $dh->generate_keys;

       my $secret = bi2bin($dh->compute_secret(bin2bi($y)));
       my $pubkey = bi2bin($dh->pub_key);

       # Pad the password to the nearest multiple of blocksize and encrypt
       $pass .= "\0";
       $pass .= chr(rand(256)) while length($pass) % 8;

       my $cipher = Crypt::Blowfish->new($secret);
       my $crypted = '';
       while(length $pass) {
           my $clear = substr($pass, 0, 8, '');
           $crypted .= $cipher->encrypt($clear);
       }

       pack("n/a*Z*a*", $pubkey, $u, $crypted);
    };
};
