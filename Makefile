CXXFLAGS = -O1 -fno-inline-functions -ggdb3 -Wall -Werror -Wextra -pedantic

OBJS = $(subst .cpp,.o,$(wildcard *.cpp))

eir: $(OBJS)
	g++ $(CXXFLAGS) -o $@ $^ -lboost_system

clean:
	rm -f $(OBJS) eir
