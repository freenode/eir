#!/usr/bin/perl -w

# eir.conf settings:
# bantracker_dbtype
# bantracker_dsn
# bantracker_dbuser
# bantracker_dbpass
# bantracker_debug
# bantracker_enable_logging
#
# To make configuration changes the privilege 'bantracker' is required

use strict;
use warnings;
use Eir;
use DBI;
use Data::Dumper;
use Time::Local;

our %heap;
our $bot=Eir::find_bot('eir');

$heap{'chanmodes'}   = {
                       'b' => 'ban',
                       'q' => 'quiet',
                       'r' => 'restricted',
                       'm' => 'moderated',
                       'z' => 'reduced moderation',
                       'n' => 'no external send',
                       't' => 'protect topic',
                       's' => 'secret',
                       'Q' => 'block forwarded users',
                       'i' => 'invite only',
                       'c' => 'filter colours',
                       'C' => 'block ctcp',
                       'o' => 'operator',
                       'v' => 'voice'
                      };

#
# load saved settings
#
eval {
  my $settings=Eir::Storage::Load('bantracker');
  $heap{'settings'}={ %$settings };
};
if ($@) {
  $heap{'settings'}=();
}

#
# Date functions are implementation specific - select the right ones here
#
my ($currtime, $unixtime);
if ($bot->Settings->{'bantracker_dbtype'} eq 'sqlite') {
  $currtime="datetime('now')";
  $unixtime="datetime(?,'unixepoch')";
} elsif ($bot->Settings->{'bantracker_dbtype'} eq 'mysql') {
  $currtime="now()";
  $unixtime="from_unixtime(?)";
} elsif ($bot->Settings->{'bantracker_dbtype'} eq 'postgres') {
  $currtime="'now'";
  $unixtime="to_timestamp(?)::timestamp"
} else {
  print "bantracker Error: Unrecognised database type " . $bot->Settings->{'bantracker_dbtype'} . ", aborting\n";
  exit;
}



#
# Define our SQL queries
#
$heap{'query'}       = {
                       btinfo_channel       => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE unbandate is null and channel LIKE ?',
                       btinfo_ban           => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE i = ?',
                       btinfo_nick          => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE setter LIKE ? AND unbanDate IS NULL',
                       btcheck_channel      => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE channel LIKE ? AND mask LIKE ? and unbanDate is NULL',
                       btpending_channel    => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE channel LIKE ? AND reason IS NULL AND unbanDate IS NULL',
                       btpending_nick       => 'SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type FROM bans WHERE setter LIKE ? AND reason IS NULL AND unbanDate IS NULL',
                       update_reason        => 'UPDATE bans SET reason = ? WHERE i =?',
                       update_action        => 'UPDATE bans SET action = ? WHERE i =?',
                       update_nagged        => 'UPDATE bans SET nagged=? WHERE i=?',
                       prepare_remove_ban   => 'SELECT  i, mask, isSet, channel, setter, setDate, reason, affected, action,actionDate, type FROM bans WHERE channel=? AND mask=? AND type=? AND unbanDate IS NULL',
                       btexpired_channel    => "SELECT i, mask, isSet, channel, setter, setDate, reason, affected, action, actionDate, type, nagged FROM bans WHERE channel LIKE ? AND actionDate < $currtime AND unbanDate IS NULL",
                       autoremove_channel    => "SELECT mask, type FROM bans WHERE channel LIKE ? AND action > 0  AND actionDate < $currtime AND unbanDate IS NULL",
                       insert_ban           => "INSERT INTO bans (channel, setter, mask, type, action, isSet, setDate) values (?, ?, ?, ?, 'true', $currtime)",
                       update_time_reason   => "UPDATE bans SET actionDate = $unixtime, reason = ? WHERE i =?",
                       update_time          => "UPDATE bans SET actionDate = $unixtime WHERE i =?",
                       remove_ban           => "UPDATE bans SET unbanner=?, unbanDate=$currtime, isSet='false'  WHERE i=?",
                       add_ban              => "INSERT INTO bans (channel, setter, mask, type, action, isSet, setDate, actionDate) VALUES (?, ?, ?, ?, ?, 'true', $currtime, $unixtime)",
                       log_entry            => "INSERT INTO log (channel, sender, command, data, date) values (?, ?, ?, ?, $currtime)"
                      };


#
# Connect to the database
#
our $dbh;

eval {
    my $alarm = alarm 0;
    $dbh=DBI->connect($bot->Settings->{'bantracker_dsn'},
                     $bot->Settings->{'bantracker_dbuser'},
                     $bot->Settings->{'bantracker_dbpass'} ) or die $!;
    $dbh->{mysql_auto_reconnect} = 1;
    alarm $alarm;
};


#
# Register our event handlers
#
our @handlers;
@handlers = (
            # IRC event handlers
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '367',type => Eir::Source::RawIrc}),\&irc_367,1),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '368', type => Eir::Source::RawIrc}),\&irc_368,1),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'JOIN',type => Eir::Source::RawIrc}),\&irc_join,1),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'mode_change',type => Eir::Source::Internal}),\&irc_mode,1),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'PRIVMSG', type => Eir::Source::RawIrc}),\&cmd_btcomment),
            # Command handlers
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'help', type => Eir::Source::IrcCommand}),\&cmd_help),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btinfo', type => Eir::Source::IrcCommand}),\&cmd_btquery),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btpending',type => Eir::Source::IrcCommand}),\&cmd_btquery),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btexpired',type => Eir::Source::IrcCommand}),\&cmd_btquery),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btcheck',type => Eir::Source::IrcCommand}),\&cmd_btquery),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btconfig',type => Eir::Source::IrcCommand}),\&cmd_btconfig),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btset',type => Eir::Source::IrcCommand}),\&cmd_btset),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'dumpheap',type => Eir::Source::IrcCommand}),\&cmd_dumpheap),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btsync',type => Eir::Source::IrcCommand}),\&cmd_btsync),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btcomment',type => Eir::Source::IrcCommand}),\&cmd_btcomment),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btsaveconfig',type => Eir::Source::IrcCommand}),\&cmd_btsaveconfig),
            Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'btloadconfig',type => Eir::Source::IrcCommand}),\&cmd_btloadconfig),
            # Recurring Events
            Eir::EventManager::add_recurring_event(60, \&nag_expired)
           );

# The following are only required if logging is in use, so we'll only register them if logging has been enabled globally
if ($bot->Settings->{'bantracker_enable_logging'}) {
  push (@handlers, (
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'PRIVMSG', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'NOTICE', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'JOIN', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'PART', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'QUIT', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'MODE', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => 'TOPIC', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '332', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '333', type => Eir::Source::RawIrc}),\&irc_log),
                   Eir::CommandRegistry::add_handler(Eir::Filter->new({command => '353', type => Eir::Source::RawIrc}),\&irc_log),
                  ));
}

#
# Call btsync() to populate per-channel information if the script is being reloaded.
#
btsync();

#
# IRC event handlers
#
sub irc_join {
  our %heap;
  our $bot;
  my ($message) = @_;
  my $source=$message->source();
  return unless ($source->{'name'} eq $bot->nick);
  my $channel=irclc($source->{'destination'});
  if ($heap{'settings'}{$channel}{'frequency'}) {
    $heap{$channel}{'nag_after'}=time()+$heap{'settings'}{$channel}{'frequency'};
  }
  $bot->send("MODE $channel qb");
}


sub irc_367 {
  our %heap;
  my ($message) = @_;
  my @args=@{$message->args};
  push (@{$heap{irclc($args[0])}{'temp_list'}}, $args[1]);
}

sub irc_log {
  our %heap;
  my ($message) = @_;
  my $raw=$message->raw;
  my ($sender, $command, $target, $data) = split /\ /,$raw,4;
  if ($target=~/^:?(#.+)$/) {
    my $channel=irclc($1);
    return unless $heap{'settings'}{$channel}{'logging'};
    db_insert($heap{query}{log_entry}, [ $channel, $sender, $command, $data ]);
  } elsif (($command eq '332' || $command eq '333') && $data=~/^(#.+?) (.*)$/) {
    my $channel=$1;
    return unless $heap{'settings'}{$channel}{'logging'};
    $data=$2;
    db_insert($heap{query}{log_entry}, [ $channel, '*SERVER*', $command, $data ]);
  }
}

sub irc_368 {
  our %heap;
  my ($message) = @_;
  my @args=@{$message->args};
  my $channel=irclc($args[0]);
  if ($args[1]=~/End of Channel (Ban|Quiet) List/ && defined @{$heap{$channel}{'temp_list'}}) {
    my $access_list=lc $1 . '_list';
    @{$heap{$channel}{$access_list}} = @{$heap{$channel}{'temp_list'}};
    undef @{$heap{$channel}{'temp_list'}};
    print Dumper(@{$heap{$channel}{$access_list}});
  }
}

sub irc_728 {
  our %heap;
  my ($message) = @_;
  my @args=@{$message->args};
  push (@{$heap{irclc($args[0])}{'temp_quiet_list'}}, $args[2]);
}

sub irc_729 {
  our %heap;
  my ($message) = @_;
  my @args=@{$message->args};
  my $channel=irclc($args[0]);
  @{$heap{$channel}{quiet_list}} = @{$heap{$channel}{'temp_quiet_list'}};
  undef @{$heap{$channel}{'temp_quiet_list'}};
}

sub irc_mode {
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  my $sender=irclc($source->{'raw'});
  my $nick=irclc($source->{'name'});
  my $destination=irclc($source->{'destination'});
  my @args=@{$message->args};
  my $target=$heap{'settings'}{$destination}{'report'} || undef;
  my ($bantime, $action,$type);

  return unless $heap{'settings'}{$destination}{'enabled'};
  return unless $destination=~/^#/ && $heap{'settings'}{$destination}{'trackmodes'};
  return if $sender =~ $heap{'settings'}{$destination}{'ignore'};
  my $re='^([' . $heap{'settings'}{$destination}{'trackmodes'} . ']$)';
  if ($args[1] =~ $re && $heap{'chanmodes'}{$args[1]}) {
    $type=$heap{'chanmodes'}{$args[1]};
    if ($args[0] eq 'add') {
      if ($heap{'settings'}{$destination}{'bantime'}) {
       $bantime=time() + calc_time($heap{'settings'}{$destination}{'bantime'});
      } else {
       $bantime=time() + 86400;
      }

      if ($heap{'settings'}{$destination}{'action'}) {
       $action=$heap{'settings'}{$destination}{'action'};
      } else {
       $action = 0;
      }

      push (@{$heap{$destination}{$type .'_list'}}, $args[2] || undef);
      my $banid=db_insert($heap{query}{add_ban}, [ $destination, $sender, $args[2] || '', $type, $action, $bantime ]);
      push(@{$heap{'to_comment'}{$sender}}, $banid);
      if (scalar @{$heap{'to_comment'}{$sender}} == 1 ) {
       request_ban_comment($banid,$nick);
      }
      ;
      my $text="(\0034NEW\003) \002$type\002\[$banid\] was set";
      if ($args[2]) {
       $text .= " on \002$args[2]\002";
      }
      $text .= " in \002$destination\002 by \002$sender\002";
      if ($target && $heap{'settings'}{$destination}{'reporton'}=~/new/i) {
       $message->bot()->send("NOTICE $target :$text");
      }
    } elsif ($args[0] eq 'remove' ) {
      if ($args[2]) {
       @{$heap{$destination}{$type . '_list'}} = grep { $_ ne $args[2] } @{$heap{$destination}{$type . '_list'}};
      }
      for my $row (db_query($heap{query}{prepare_remove_ban}, [ $destination,  $args[2] || '', $type ])) {
       my $text="(\0039REM\003) ". describe_ban(@$row) . " It was removed by \002$sender\002 on \002" . scalar gmtime() . "\002";
       my $setter=${$row}[4];
       my $i=${$row}[0];
       db_update($heap{query}{remove_ban}, [ $sender, $i ]);
       # Make sure the ban isn't in the setter's to_comment queue
       if ($heap{'to_comment'}{$setter} && scalar @{$heap{'to_comment'}{$setter}} > 0) {
         @{$heap{'to_comment'}{$setter}}= grep {$_ ne $i} @{$heap{'to_comment'}{$setter}};
       }
       if ($target &&  $heap{'settings'}{$destination}{'reporton'}=~/rem/i) {
         $message->bot()->send("NOTICE $target :$text");
       }
      }
    }
  } elsif ($args[0] eq 'add' && $args[1] eq 'o' && $args[2] eq $bot->nick()) {
    auto_remove_expired($destination);
  }
}


#
# Bot command handlers
#
sub cmd_btquery {
  # combined function for the btinfo, btexpired, btcheck, and btpending commands
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  my $command=$message->command();
  my $sender=$source->{'raw'};
  my $destination=$source->{'destination'};
  my $client=$message->source()->{'client'};
  my @args = @{$message->args};
  my $query;
  my $activeonly=1;
  my @placeholders;

  return unless $args[0];
  if ($args[0] =~ /^(#+\S+)/) {
    $query=$heap{query}{$command .'_channel'};
    @placeholders= ( irclc($1) );
    if ($command eq 'btcheck') {
      if ($args[1]=~/@/) {
        push (@placeholders, irclc($args[1]) );
      } else {
        push (@placeholders, irclc($args[1]) . "!%")
      }
    }
  } elsif ($args[0] =~ /^(\d+)/ && $command eq 'btinfo') {
    $query=$heap{query}{$command. '_ban'};
    @placeholders= ( $1 );
    $activeonly=0;
  } elsif ($args[0] =~ /^(\D\S+)/ && $command ne 'btexpired') {
    $query=$heap{query}{$command . '_nick'};
    @placeholders= (irclc($1) . "!%");
  } else {
    return;
  }
  if ($query && @placeholders) {
    my $count=0;
    my $state='inactive';
    my @results=db_query($query, \@placeholders);
    for my $row (@results) {
      next unless ($sender eq ${$row}[4] || $sender =~ $heap{'settings'}{lc(${$row}[3])}{'admins'} || $sender =~ $heap{'settings'}{lc(${$row}[3])}{'query'} || $client->has_privilege('bantracker') );
      my $mask=${$row}[1];
      my $channel=${$row}[3];
      my $type=${$row}[10];
      my $text=describe_ban(@$row);
      for my $ban (@{$heap{$channel}{$type .'_list'}}) {
         if ($ban eq $mask) {
               $state='active';
             }
      }
      next if ($state eq 'inactive' && $activeonly==1);
      $text .= " It is currently $state.";
    $message->reply($text);
    $count++;
    }
    if ($count > 1) {
      $message->reply("End of results");
    } elsif ($count == 0) {
      $message->reply("No results");
    }
  }
}

sub cmd_btset {
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  my $sender=$source->{'raw'};
  my $destination=$source->{'destination'};
  my @args = @{$message->args};
  if (scalar @args < 2) {
    $message->reply('Usage: btset banid [timespec] comment');
    return;
  }
  if ($heap{'to_comment'}{$sender} &&  scalar @{$heap{'to_comment'}{$sender}} > 0) {
    @{$heap{'to_comment'}{$sender}} = grep { $_ ne $args[0] } @{$heap{'to_comment'}{$sender}};
  }

  my $res=update_ban($sender,$destination, @args);
  if ($res) {
    $message->reply($res);
  }
}

sub cmd_btconfig {
  our %heap;
  our $bot;
  my ($message) = @_;
  my @args = @{$message->args};
  my $source=$message->source();
  my $client=$message->source()->{'client'};

  unless ($client->has_privilege('bantracker')) {
    $message->reply('You are not authorised to configure the bantracker');
    return;
  }
  if (scalar @args < 1) {
    $message->reply('Usage: btconfig <#channel> [setting] [value]');
    return;
  }
  my ($channel,$setting);

  if ($args[0] =~ /^(#+.+)/) {
    $channel=irclc($1);
    my @options=qw(enabled report frequency admins bantime admins ops trackmodes action logging reporton query urlprefix ignore);
    if ( grep {$_ eq $args[1] } @options ) {
      $setting=$args[1];
    } elsif (! defined $args[1]) {
      my $count=0;
      for $setting (keys %{$heap{'settings'}{$channel}}) {
       $message->reply("$channel: $setting = " . $heap{'settings'}{$channel}{$setting}) || '';
       $count++;
      }
      if ($count > 1) {
       $message->reply("End of results.");
      }
      return;
   } else {
      $message->reply("Unknown setting: $args[1] (available settings: (" . join(", ",@options) . ")");
      return;
    }
  }
  if ($args[2] eq 'CLEAR') {
    $heap{'settings'}{$channel}{$setting}=undef;
    $message->reply("Cleared $setting for $channel");
  } elsif (defined $args[2]) {
    $heap{'settings'}{$channel}{$setting}=$args[2];
    $message->reply("Set $setting to $args[2] for $channel");
  } else {
    $message->reply("$channel: $setting = " . $heap{'settings'}{$channel}{$setting}) || '';
  }
}


sub cmd_btcomment {
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  # bail out if we've been called by the PRIVMSG hook and the destination is a channel
  return if $source->{'type'} & Eir::Source::RawIrc &&  substr($source->{destination},0,1) eq '#';

  my $sender=$source->{'raw'};
  my $nick=$source->{'name'};
  my @args = @{$message->args};
  if ($source->{'type'} & Eir::Source::RawIrc) {
    @args=split /\s+/,$args[0];
  }
  if ($heap{'to_comment'}{$sender} &&  scalar @{$heap{'to_comment'}{$sender}} > 0) {
    my $banid=shift @{$heap{'to_comment'}{$sender}};
    update_ban($sender,undef,$banid, @args );
    if (scalar @{$heap{'to_comment'}{$sender}} > 0) {
      request_ban_comment(${$heap{'to_comment'}{$sender}}[0] , $nick);
    } else {
      $message->reply("All your bans are now commented.");
    }
  } elsif ($source->{'type'} & Eir::Source::IrcCommand) {
    $message->reply("You have no bans to comment.");
  }
}

sub cmd_dumpheap {
  our %heap;
  our $bot;
  my ($message) = @_;

  return unless $bot->Settings->{'bantracker_debug'};
  $message->reply('Dumping heap to console');
  print Dumper(%heap)
}

sub cmd_btsync {
  my ($message) = @_;
  $message->reply('Resyncing Channels');
  btsync();
}

sub btsync {
  our $bot;
  our %heap;
  my $channels=$bot->Channels();
  for my $channel (keys %$channels) {
    next unless $heap{'settings'}{$channel}{'enabled'};
    undef %{$heap{$channel}};
    if ($heap{'settings'}{$channel}{'frequency'}) {
      $heap{$channel}{'nag_after'}= time() + $heap{'settings'}{$channel}{'frequency'} || 0;
    }
    $bot->send("MODE $channel qb");
  }
}

sub cmd_btsaveconfig {
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  my $client=$message->source()->{'client'};
  unless ($client->has_privilege('bantracker')) {
    $message->reply('You are not authorised to configure the bantracker');
    return;
  }
  eval {
    Eir::Storage::Save(\%{$heap{'settings'}}, 'bantracker');
  };
  if ($@) {
    $message->reply('Unable to save channel configuration');
  } else {
    $message->reply('Channel configuration saved');
  }
}

sub cmd_btloadconfig {
  our %heap;
  my ($message) = @_;
  my $source=$message->source();
  my $client=$message->source()->{'client'};
  unless ($client->has_privilege('bantracker')) {
    $message->reply('You are not authorised to configure the bantracker');
    return;
  }
  eval {
    my $settings=Eir::Storage::Load('bantracker');
    $heap{'settings'}={ %$settings };
  };
  if ($@) {
    $message->reply('Unable to load channel configuration');
  } else {
    $message->reply('Channel configuration reloaded');
  }
}

sub cmd_help {
  my ($message) = @_;
  my @args = @{$message->args};
  return if (scalar @args > 0);
  $message->reply('For bantracker documentation see http://freenode.net/eir.shtml');
}

#
# Timed event handlers
#
sub nag_expired {
  our %heap;
  our $bot;
  my ($now)=@_;
  my %modechar=reverse %{$heap{'chanmodes'}};
  my $channels=$bot->Channels();
  my $target;
  for my $channel (keys %$channels) {
    my $chanserv=0;
    my @modes;
    next unless ($heap{'settings'}{$channel}{'enabled'} && $heap{'settings'}{$channel}{'frequency'});
    if ( $heap{'settings'}{$channel}{'report'}) {
      $target=$heap{'settings'}{$channel}{'report'};
    }
    next unless (($heap{$channel}{'nag_after'} && time() > $heap{$channel}{'nag_after'}) || $now );
    $heap{$channel}{'nag_after'}=time()+$heap{'settings'}{$channel}{'frequency'};
    my @results=db_query($heap{query}{btexpired_channel}, [ $channel ]);

    for my $row (@results) {
      my ($i,$mask,$action,$type,$nagged)=@$row[0,1,8,10,11];
      my $state='inactive';
      for my $ban (@{$heap{$channel}{$type .'_list'}}) {
       if (irclc($ban) eq $mask) {
         $state='active';
       }
      }
      if ($state eq 'inactive' && $heap{'settings'}{$channel}{'reporton'}=~/rem/i) {
       for my $row (db_query($heap{query}{prepare_remove_ban}, [ $channel,  $mask || '', $type ])) {
         my $text="(\0039REM\003) ". describe_ban(@$row) . " Ban is no longer set on channel";
         my $setter=${$row}[4];
         my $i=${$row}[0];
         db_update($heap{query}{remove_ban}, [ $bot->nick(), $i ]);
         # Make sure the ban isn't in the setter's to_comment queue
         if ($heap{'to_comment'}{$setter} && scalar @{$heap{'to_comment'}{$setter}} > 0) {
           @{$heap{'to_comment'}{$setter}}= grep {$_ ne $i} @{$heap{'to_comment'}{$setter}};
         }
         if ($target) {
           $bot->send("NOTICE $target :$text");
         }
       }
      }
      if ($action == 2 && ! $bot->Channels->{$channel}->Members->{$bot->nick()}->has_mode('o')) {
       unless ($chanserv) {
         $bot->send("cs op $channel");
         $chanserv=1;
       }
       next;
      }
      if ($action > 0 && $bot->Channels->{$channel}->Members->{$bot->nick()}->has_mode('o')) {
       push @modes, [ $modechar{$type}, $mask];
       next;
      } else {
       if ($target && $heap{'settings'}{$channel}{'reporton'}=~/exp/i  ) {
         my $message="(\00310EXP\003) " . describe_ban(@$row);
          if ($nagged) {
           $message .= " This has been nagged $nagged times.";
         }
         $bot->send("NOTICE $target :$message");
       }
       db_update($heap{query}{update_nagged}, [ ++$nagged, $i ]);
      }
    }
    if (scalar @modes > 0) {
      dispatch_modes($channel,@modes);
    }
  }
}

#
# Non-event subroutines
#
sub auto_remove_expired {
  our %heap;
  our $bot;
  my ($channel)=@_;

  my %modechar=reverse %{$heap{'chanmodes'}};
  my @results=db_query($heap{query}{autoremove_channel}, [ $channel ]);
  my @modes;
  for my $row (@results) {
    my ($mask,$type)=@$row;
    # $bot->send("MODE $channel -" . $modechar{$type} . " $mask");
    push @modes, [ $modechar{$type}, $mask ];
  }
  unless ($heap{'settings'}{$channel}{'ops'} && $heap{'settings'}{$channel}{'ops'} eq 'yes') {
    push @modes, [ 'o', $bot->nick() ];
  }
  if ( scalar @modes > 0 ) {
    dispatch_modes($channel,@modes);
  }
}

sub dispatch_modes {
  our $bot;
  my $channel=shift;
  my @modes;
  my @args;
  for my $aref  (@_) {
    push @modes, $aref->[0];
    push @args, $aref->[1];
    if (scalar @modes == 4 || $aref eq $_[$#_]) {
      $bot->send("MODE $channel -" . join('',@modes) . ' ' . join(' ',@args));
      undef(@modes);
      undef(@args);
    }
  }
}

sub update_ban {
  our %heap;
  my ($banid, $newtime, $newreason);
  my @data=@_;
  my $action='';
  if ($data[2]=~/^(\d+)/) {
    $banid=$1;
    if ($data[3]=~/^([\#\@\%]?)~(.*)/) {
      # got an expiry time;
      $newtime=time() + calc_time($2);
      $newreason=join ' ',@data[4..$#data];
      if ($1 eq '#') {
       $action=0;
      } elsif ($1 eq '@' ) {
       $action=1;
      } elsif ($1 eq '%' ) {
       $action=2;
      }
    } else {
      $newreason=join ' ', @data[3..$#data];
    }
    my $query;
    my @placeholders;
    my @results=db_query($heap{query}{btinfo_ban}, [ $banid ]);
    if (scalar @results == 0) {
      return 'No such record.';
    }
    my $row=shift @results;
    if ($data[0] eq ${$row}[4] || $data[0] =~ $heap{'settings'}{${$row}[3]}{'admins'}) {
      my $i=${$row}[0];
      if ($newtime && $newreason) {
       $query=$heap{query}{update_time_reason};
       @placeholders= ( $newtime, $newreason, $i);
      } elsif ($newtime) {
       $query=$heap{query}{update_time};
       @placeholders= ( $newtime, $i );
      } elsif ($newreason) {
       $query=$heap{query}{update_reason};
       @placeholders= ( $newreason, $i);
      }
      db_update($query,\@placeholders);
      # update action field if set
      if ($action ne '') {
       $query=$heap{query}{update_action};
       @placeholders=($action,$i);
       db_update($query,\@placeholders);
      }
    } else {
      return 'You do not have permission to alter that record.';
    }
  }
  return 'Done.'
}

sub request_ban_comment {
  our %heap;
  our $bot;
  my ($banid,$nick)=@_;

  my @results=db_query($heap{query}{btinfo_ban}, [ $banid ]);
  my $row = shift @results;
  return if ($nick=~/serv$/i || $nick eq $bot->nick()); # don't spam services, or ourselves
  $bot->send("PRIVMSG $nick :Please comment on the following: " . describe_ban(@$row));
}

sub calc_time {
  # take a dircbot style time specification
  # and return a time in seconds
  my $spec=shift @_;
  my $time=0;
  while ($spec =~ /^~?(\d+)([dhmsw]?)/) {
    if ($2 eq 'm') {
      $time+=$1*60;
    } elsif ($2 eq 'h') {
      $time+=$1*3600;
    } elsif ($2 eq 'd') {
      $time+=$1*86400;
    } elsif ($2 eq 'w') {
      $time+=$1*604800;
    } elsif ($2 eq 's') {
      $time+=$1;
    } else {
      $time+=$1*60;
    }
    $spec=$';
  }
  return $time;
}

sub describe_ban {
  # columns in brackets should always be set, the others are optional
  # ($i), $mask, $isSet, ($channel), ($setter), ($setDate), $reason, $affected, ($action), ($actionDate), ($type)
  my   ($i, $mask, $isSet, $channel, $setter, $setDate, $reason, $affected, $action, $actionDate, $type)=@_;
  my $text="\002$type\002\[$i]";
  # strip microseconds if present
  if ($setDate=~/(\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d).(\d+)/){
    $setDate=$1;
  }
  my ($year,$mon,$mday,$hour,$min,$sec)=split /[-:\ ]/,$setDate;
  $mon--;
  my $unixSetDate=timegm($sec,$min,$hour,$mday,$mon,$year);
  if ($mask) {
    $text .= " \002$mask\002";
  }
  $text .= " was set on \002$channel\002 by \002$setter\002 on \002$setDate\002";
  if ($reason) {
    $text .= " with reason \"\002$reason\002\"";
  }
  if ($affected) {
    $text .= " It affected $affected and"
  } else {
    $text .= ' and';
  }
  $text .= " had an expiry date of \002$actionDate.\002";
  if ($heap{'settings'}{$channel}{'urlprefix'}) {
    my $uChannel=$channel;
    $uChannel =~ s/([^A-Za-z0-9])/sprintf("%%%02X", ord($1))/seg;
    $text .= ' Log: ' . $heap{'settings'}{$channel}{'urlprefix'} . '?i=' . $i . '&ts=' . $unixSetDate . '&c=' . $uChannel;
  }
  return $text;
}

sub db_query {
  our $dbh;
  my $query=$_[0];
  my @placeholders=@{$_[1]};
  my @results;
#  print $query . '(' . join(',',@placeholders) . ")\n";
  my $sth = $dbh->prepare( $query ) or die $!;
  $sth->execute( @placeholders ) or die $!;
  while ( my @row = $sth->fetchrow_array() ) {
    push( @results,\@row );
  }
  return @results;
  $sth->finish();
}

sub db_insert {
  our $dbh;
  my $query=$_[0];
  my @placeholders=@{$_[1]};
  my $table;
  if ($query=~/INSERT INTO (.+?) /) {
   $table=$1;
  } else {
   $table=undef;
  }
  my $sth = $dbh->prepare( $query ) or die $!;
  $sth->execute( @placeholders ) or die $!;
  return $dbh->last_insert_id(undef,undef,$table,undef);
}

sub db_update {
  our $dbh;
  my $query=$_[0];
  my @placeholders=@{$_[1]};
  my $sth = $dbh->prepare( $query ) or die $!;
  $sth->execute( @placeholders ) or die $!;
}

sub irclc {
  # converts a string to lower case, using rfc1459 casemapping
  my $s=shift;
  $s=~tr/A-Z[]\^/a-z{}|~/;
  return $s;
}
