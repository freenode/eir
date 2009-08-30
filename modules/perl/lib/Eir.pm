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

1;
