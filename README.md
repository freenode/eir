Eir
===

Eir is an IRC client bot framework, written primarily for freenode utility
functions. It is, however, highly extensible using C++ or Perl, and is used by
other projects as well.

Installation
============

If you just want to know how to build and install it, do the following

```
git submodule init
git submodule update
autoconf
./configure --prefix=SOMEWHERE
make
make install
```

To install on FreeBSD, you will need `g++` and `gmake`, so something like this:
```
# if not already available do the following:
sudo pkg install gcc48 gmake
git submodule init
git submodule update
autoconf
./configure --prefix=SOMEWHERE
CXX=g++48 gmake
gmake install

```

Then go to SOMEWHERE/etc, and write eir.conf. There is an example config file
installed.

Scripts
=======

Eir comes with a few perl script modules, including Bantracker.pl, and a module
for sasl-auth login. You will want to copy these to your install location
