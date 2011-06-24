MODULES = perl

DSOS = api

api_FILENAME = $(MODDIR)/perl/api/Eir.so

api_SOURCES = HashWrappers.cpp perl_helpers.cpp value_wrapper.cpp api.xs
api_SRCDIRS = api
api_CXXFLAGS = -Imodules/perl/api

CXXFLAGS = -fPIC -Isrc -Imodules/perl $(PERL_CFLAGS) $(PERL_LIBS) \
	   -Wno-unused-variable -Wno-write-strings -DPERL_GCC_PEDANTIC

XSFLAGS = -except -typemap typemap
