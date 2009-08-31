#include "stupid_perl_workarounds.h"
#include <EXTERN.h>
#include <perl.h>

#include "eir.h"
#include "perl_helpers.hh"

using namespace eir::perl;

PerlHolder::~PerlHolder()
{
}
