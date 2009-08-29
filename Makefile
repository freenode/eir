
### Global settings first

CXXFLAGS = -O0 -fno-inline-functions -ggdb3 -I. -Wall -Werror -Wextra -pedantic -std=gnu++0x
LDFLAGS = -Wl,-export-dynamic

mod_INCLUDES = -I. -Isrc


### Find source/target lists

eir_SOURCES = $(wildcard src/*.cpp)
eir_HEADERS = $(wildcard src/*.h)
eir_OBJS = $(subst .cpp,.o,$(eir_SOURCES))

mod_SOURCES = $(wildcard modules/*/*.cpp modules/*.cpp)
mod_MODULES = $(subst .cpp,.so,$(mod_SOURCES))

libpaludisutil_SOURCES = $(wildcard paludis/util/*.cc)
libpaludisutil_HEADERS = $(wildcard paludis/util/*.hh)
libpaludisutil_OBJS = $(subst .cc,.o,$(libpaludisutil_SOURCES))

libjson_SOURCES = $(wildcard libjson/*.cpp)
libjson_OBJS = $(subst .cpp,.o,$(libjson_SOURCES))
libjson.so: CXXFLAGS += -Ilibjson -Wno-error


### Rules, specific then generic

.PHONY: all clean
all: eir modules libpaludisutil.so libjson.so

eir: libpaludisutil.so

eir: $(eir_OBJS)
	g++ $(LDFLAGS) -o $@ $(eir_OBJS) -ldl -lpaludisutil -L. -Wl,--rpath `pwd`

.PHONY: modules

modules: $(mod_MODULES)

modules/%.so: modules/%.cpp
	g++ $(CXXFLAGS) $(EXTRA_CXXFLAGS_$(dir $@)) $(mod_INCLUDES) -fPIC -shared -o $@.tmp $<
	mv $@.tmp $@

paludis/util/%.o: paludis/util/%.cc
	g++ $(CXXFLAGS) -fPIC -c -o $@ $<

libpaludisutil.so: $(libpaludisutil_OBJS)
	g++ $(CXXFLAGS) -fPIC -shared -o $@ $^

libjson/%.o: libjson/%.cpp
	g++ $(CXXFLAGS) -fPIC -c -o $@ $<

libjson.so: $(libjson_OBJS)
	g++ $(CXXFLAGS) -fPIC -shared -o $@ $^

clean:
	rm -f $(eir_OBJS) eir $(mod_MODULES) libpaludisutil.so $(libpaludisutil_OBJS)


### Some magic to allow extra compiler flags for particular modules

makefile_includes=$(wildcard modules/*/extra.mk)
subdir=$(dir $(inc))
subdir_sources=$(wildcard $(subdir)*.cpp)
subdir_modules=$(subst .cpp,.so,$(subdir_sources))

$(foreach inc,$(makefile_includes), \
    $(eval include $(inc)) \
    $(eval SUBDIR_CXXFLAGS_$(subdir)=$(EXTRA_CXXFLAGS)) \
    $(foreach mod,$(subdir_modules), \
        $(eval MODULE_CXXFLAGS_$(mod)=$(EXTRA_CXXFLAGS_$(notdir $(basename $(mod)))))))

CXXFLAGS += $(SUBDIR_CXXFLAGS_$(dir $@)) $(MODULE_CXXFLAGS_$@)


### Dependency stuff below here

# While eir includes no generated headers, we use -MG nonetheless so that individual modules can
# use extra include paths without triggering errors during dependency generation
src/.depend: $(eir_SOURCES) $(eir_HEADERS)
	g++ -MM -MG $(CXXFLAGS) $(eir_SOURCES) | sed 's!\(^[^ ]*.o:\)!src/\1!' >$@

modules/.depend: $(mod_SOURCES) $(eir_HEADERS)
	g++ -MM -MG $(CXXFLAGS) $(mod_INCLUDES) $(mod_SOURCES) | sed 's!\(^[^ ]*\)\.o:!modules/\1.so:!' >$@

paludis/util/.depend: $(paludisutil_SOURCES) $(paludisutil_HEADERS)
	g++ -MM -MG $(CXXFLAGS) $(paludisutil_CXXFLAGS) $(paludisutil_SOURCES) | sed 's!\(^[^ ]*.o:\)!paludis/util/\1!' >$@

-include src/.depend modules/.depend paludis/util/.depend
