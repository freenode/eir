CXXFLAGS = -O0 -fno-inline-functions -ggdb3 -I. -Wall -Werror -Wextra -pedantic
LDFLAGS = -Wl,-export-dynamic

mod_INCLUDES = -I. -Isrc

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

.PHONY: all clean
all: eir modules libpaludisutil.so libjson.so

eir: libpaludisutil.so

eir: $(eir_OBJS)
	g++ $(LDFLAGS) -o $@ $(eir_OBJS) -ldl -lpaludisutil -L. -Wl,--rpath `pwd`

.PHONY: modules

modules: $(mod_MODULES)

modules/%.so: modules/%.cpp
	g++ $(CXXFLAGS) $(mod_INCLUDES) -fPIC -shared -o $@.tmp $<
	mv $@.tmp $@

paludis/util/%.o: paludis/util/%.cc
	g++ -c $(CXXFLAGS) -fPIC -o $@ $<

libpaludisutil.so: $(libpaludisutil_OBJS)
	g++ $(CXXFLAGS) -fPIC -shared -o $@ $^

libjson/%.o: libjson/%.cpp
	g++ $(CXXFLAGS) -fPIC -c -o $@ $<

libjson.so: $(libjson_OBJS)
	g++ $(CXXFLAGS) -fPIC -shared -o $@ $^

clean:
	rm -f $(eir_OBJS) eir $(mod_MODULES) libpaludisutil.so $(libpaludisutil_OBJS)

src/.depend: $(eir_SOURCES) $(eir_HEADERS)
	g++ -MM $(CXXFLAGS) $(eir_SOURCES) | sed 's!\(^[^ ]*.o:\)!src/\1!' >$@

modules/.depend: $(mod_SOURCES) $(eir_HEADERS)
	g++ -MM $(CXXFLAGS) $(mod_INCLUDES) $(mod_SOURCES) | sed 's!\(^[^ ]*\)\.o:!modules/\1.so:!' >$@

paludis/util/.depend: $(paludisutil_SOURCES) $(paludisutil_HEADERS)
	g++ -MM $(CXXFLAGS) $(paludisutil_CXXFLAGS) $(paludisutil_SOURCES) | sed 's!\(^[^ ]*.o:\)!paludis/util/\1!' >$@

-include src/.depend modules/.depend paludis/util/.depend
