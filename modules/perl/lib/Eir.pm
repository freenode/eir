package Eir;

require Exporter;
require DynaLoader;
@ISA = qw(Exporter DynaLoader);
@EXPORT = qw( find_bot );

bootstrap Eir;

package Eir::Bot;

sub Clients {
    my ($self) = @_;
    tie %client_hash, 'Eir::Internal::BotClientHash', $self;
    return \%client_hash;
}

1;
