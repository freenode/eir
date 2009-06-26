EXTRA_CXXFLAGS_json=-Ilibjson -L. -ljson -Wl,--rpath `pwd`

modules/storage/json.so: libjson.so
