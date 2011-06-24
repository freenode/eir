MODULES = perl

DSOS = api

api_FILENAME = lib/eir/modules/perl/api/Eir.so

api_SOURCES = HashWrappers.cpp perl_helpers.cpp value_wrapper.cpp api.xs
api_SRCDIRS = api
api_CXXFLAGS = -Imodules/perl/api

PERL_INCLUDES := $(shell perl -MExtUtils::Embed -e ccopts) -Imodules/perl/ -Wno-format-security
PERL_LDFLAGS := $(shell perl -MExtUtils::Embed -e ldopts)

CXXFLAGS = -fPIC -Isrc $(PERL_INCLUDES) $(PERL_LDFLAGS) \
	   -Wno-unused-variable -Wno-write-strings -DPERL_GCC_PEDANTIC

XSFLAGS = -except -typemap typemap
