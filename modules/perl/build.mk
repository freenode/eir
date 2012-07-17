MODULES = perl
perl_SOURCES = perl.cpp perlxsi.c

$(GENERATED_SOURCE_DIR)/perlxsi.c:
	perl -MExtUtils::Embed -e xsinit -- -o $@

DSOS = api

api_FILENAME = $(MODDIR)/perl/api/Eir.so

api_SOURCES = HashWrappers.cpp perl_helpers.cpp value_wrapper.cpp api.xs
api_SRCDIRS = api
api_CXXFLAGS = -Imodules/perl/api

CPPFLAGS = -fPIC -Isrc -Imodules/perl $(PERL_CFLAGS) \
	   -Wno-unused-parameter -Wno-unused-but-set-variable \
	   -Wno-unused-variable -Wno-write-strings -Wno-format-security -DPERL_GCC_PEDANTIC

LDFLAGS = $(PERL_LIBS)

XSFLAGS = -except -typemap typemap

DATADIR = $(MODDIR)/perl
DATA = lib
