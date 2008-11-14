CXXFLAGS = -O0 -fno-inline-functions -ggdb3 -I. -Wall -Werror -Wextra -pedantic
LDFLAGS = -Wl,-export-dynamic

mod_INCLUDES = -I. -Isrc

eir_SOURCES = $(wildcard src/*.cpp)
eir_HEADERS = $(wildcard src/*.h)
eir_OBJS = $(subst .cpp,.o,$(eir_SOURCES))

mod_SOURCES = $(wildcard modules/*.cpp)
mod_MODULES = $(subst .cpp,.so,$(mod_SOURCES))

.PHONY: all
all: eir modules

eir: $(eir_OBJS)
	g++ $(LDFLAGS) -o $@ $^ -lboost_system -ldl

.PHONY: modules

modules: $(mod_MODULES)

modules/%.so: modules/%.cpp
	g++ $(CXXFLAGS) $(mod_INCLUDES) -fPIC -shared -o $@ $<

clean:
	rm -f $(eir_OBJS) eir $(mod_MODULES)

src/.depend: $(eir_SOURCES) $(eir_HEADERS)
	g++ -MM $(CXXFLAGS) $(eir_SOURCES) | sed 's!\(^[^ ]*.o:\)!src/\1!' >$@

modules/.depend: $(mod_SOURCES) $(eir_HEADERS)
	g++ -MM $(CXXFLAGS) $(mod_INCLUDES) $(mod_SOURCES) | sed 's!\(^[^ ]*.o:\)!modules/\1!' >$@

-include src/.depend modules/.depend
