DSOS = paludisutil

paludisutil_SOURCES = destringify.cc \
		      exception.cc \
		      mutex.cc \
		      tokeniser.cc

CXXFLAGS += -fPIC
