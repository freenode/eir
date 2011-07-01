CXXFLAGS = -O0 -fno-inline-functions -ggdb3 -I. -Wall -Werror -Wextra -pedantic -std=c++0x

SUBDIRS = \
	  paludis/util \
	  src \
	  libjson \
	  modules

include settings.mk

include bsextra/target_module.mk
include bs/bs.mk

