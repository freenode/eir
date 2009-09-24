package Eir;

require Exporter;
require DynaLoader;
@ISA = qw(Exporter DynaLoader);
@EXPORT = qw( find_bot );

bootstrap Eir;

package Eir::Bot;

sub Clients {
    my ($self) = @_;
    my $ret = {};
    tie %$ret, 'Eir::Internal::BotClientHash', $self;
    return $ret;
}

sub Channels {
    my ($self) = @_;
    my $ret = {};
    tie %$ret, 'Eir::Internal::BotChannelHash', $self;
    return $ret;
}

sub Settings {
    my ($self) = @_;
    my $ret = {};
    tie %$ret, 'Eir::Internal::BotSettingsHash', $self;
    return $ret;
}

package Eir::Client;

sub Channels {
    my ($self) = @_;
    my $ret = {};
    tie %$ret, 'Eir::Internal::ClientMembershipHash', $self;
    return $ret;
}

package Eir::Channel;

sub Members {
    my ($self) = @_;
    my $ret = {};
    tie %$ret, 'Eir::Internal::ChannelMembershipHash', $self;
    return $ret;
}

package Eir::Filter;

sub new {
    my ($class, $args) = @_;
    $class = ref $class || $class;
    my $self = $class->do_new;

    foreach my $key (keys %$args) {
        if ($key eq 'command') {
            $self->is_command($args->{$key});
        } elsif ($key eq 'type') {
            $self->source_type($args->{$key});
        } elsif ($key eq 'source_name' ) {
            $self->source_named($args->{$key});
        } elsif ($key eq 'bot' ) {
            $self->from_bot($args->{$key});
        } elsif ($key eq 'private') {
            $self->in_private if $args->{$key};
        } elsif ($key eq 'channel') {
            $self->in_channel($args->{$key});
        } elsif ($key eq 'privilege') {
            $self->requires_privilege($args->{$key});
        } elsif ($key eq 'config') {
            $self->or_config if $args->{$key};
        }
    }
    return $self;
}


1;
