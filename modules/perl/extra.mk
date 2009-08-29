PERL_INCLUDES := $(shell perl -MExtUtils::Embed -e ccopts) -Imodules/perl/
PERL_LDFLAGS := $(shell perl -MExtUtils::Embed -e ldopts)

APIDIR=modules/perl/api

# First, add the right compiler flags to build the main perl.so module

EXTRA_CXXFLAGS_perl= $(PERL_INCLUDES) $(PERL_LDFLAGS) -Wno-error

# The perl module also require the generated definition of xs_init...
modules/perl/perl.so: modules/perl/perlxsi.c

modules/perl/perlxsi.c:
	perl -MExtUtils::Embed -e xsinit -- -o $@

# Then, define how to build the api library.

modules/perl/perl.so: | $(APIDIR)/perlapi.so

PERLAPI_SOURCES = api_XS.cpp

.PRECIOUS: $(APIDIR)/%_XS.cpp

$(APIDIR)/%_XS.cpp: $(APIDIR)/%.xs $(APIDIR)/typemap
	xsubpp -csuffix _XS.cpp -typemap typemap $< >$@.tmp
	mv $@.tmp $@

PERLAPI_OBJS = $(addprefix $(APIDIR)/,$(addsuffix .o,$(basename $(PERLAPI_SOURCES))))

$(APIDIR)/%.o: $(APIDIR)/%.cpp
	g++ $(CXXFLAGS) $(mod_INCLUDES) $(PERL_INCLUDES) -Wno-error -fPIC -c -o$@ $<

$(APIDIR)/perlapi.so: $(PERLAPI_OBJS)
	g++ -shared -o $@ $^


$(APIDIR)/.depend: $(addprefix $(APIDIR)/,$(PERLAPI_SOURCES))
	g++ -MM -MG $(CXXFLAGS) $(PERL_INCLUDES) $(mod_INCLUDES) $^ 2>/dev/null | sed 's!\(^[^ ]*\.o\):!$(APIDIR)/\1:!' >$@


-include $(APIDIR)/.depend
