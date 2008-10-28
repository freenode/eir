CXXFLAGS = -O1 -fno-inline-functions -ggdb3 -Wall -Werror -Wextra -pedantic

eir_SOURCES = $(wildcard src/*.cpp)
eir_HEADERS = $(wildcard src/*.h)
eir_OBJS = $(subst .cpp,.o,$(eir_SOURCES))

eir: $(eir_OBJS)
	g++ $(CXXFLAGS) -o $@ $^ -lboost_system

clean:
	rm -f $(OBJS) eir

src/.depend: $(eir_SOURCES) $(eir_HEADERS)
	g++ -MM $(CXXFLAGS) $(eir_SOURCES) | sed 's!\(^[^ ]*.o:\)!src/\1!' >$@

-include src/.depend
