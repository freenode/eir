PERL_INCLUDES := $(shell perl -MExtUtils::Embed -e ccopts) -Imodules/perl/
PERL_LDFLAGS := $(shell perl -MExtUtils::Embed -e ldopts)

APIDIR=modules/perl/api

# First, add the right compiler flags to build the main perl.so module

EXTRA_CXXFLAGS_perl= $(PERL_INCLUDES) $(PERL_LDFLAGS) \
		     -Wno-unused-variable -Wno-write-strings \
		     -DPERL_GCC_PEDANTIC

# The perl module also require the generated definition of xs_init...
modules/perl/perl.so: modules/perl/perlxsi.c

modules/perl/perlxsi.c:
	perl -MExtUtils::Embed -e xsinit -- -o $@

# Then, define how to build the api library.

modules/perl/perl.so: | $(APIDIR)/Eir.so

PERLAPI_SOURCES = api_XS.cpp HashWrappers.cpp perl_helpers.cpp value_wrapper.cpp

.PRECIOUS: $(APIDIR)/%_XS.cpp

# The perl substitution here is necessary as xsubpp seems to have a stupid bug in its
# -except handling (using '\0' in a heredoc), causing it to put NUL characters into the
# generated source file. GCC does not like this.
$(APIDIR)/%_XS.cpp: $(APIDIR)/%.xs $(APIDIR)/typemap $(wildcard $(APIDIR)/*.xs)
	xsubpp -csuffix _XS.cpp -typemap typemap -except $< | perl -pe 's/\0/\\0/g' >$@ ; [[ $$PIPESTATUS -eq 0 ]] || rm -f $@

PERLAPI_OBJS = $(addprefix $(APIDIR)/,$(addsuffix .o,$(basename $(PERLAPI_SOURCES))))

$(APIDIR)/%.o: $(APIDIR)/%.cpp
	g++ $(CXXFLAGS) $(mod_INCLUDES) $(PERL_INCLUDES) \
	    -Wno-unused-variable -Wno-write-strings -Wno-unused \
	    -DPERL_GCC_PEDANTIC \
	    -fPIC -c -o$@ $<

$(APIDIR)/Eir.so: $(PERLAPI_OBJS)
	g++ -shared -o $@ $^


$(APIDIR)/.depend: $(addprefix $(APIDIR)/,$(PERLAPI_SOURCES))
	g++ -MM -MG $(CXXFLAGS) $(PERL_INCLUDES) $(mod_INCLUDES) $^ 2>/dev/null | sed 's!\(^[^ ]*\.o\):!$(APIDIR)/\1:!' >$@


-include $(APIDIR)/.depend
