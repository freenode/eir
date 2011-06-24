DSOS = json

json_SOURCES = json_reader.cpp json_writer.cpp json_value.cpp

CXXFLAGS = -Ilibjson -Wno-error -fPIC
