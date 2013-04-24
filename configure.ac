AC_INIT([eir], [0.0.1])

AC_PREFIX_DEFAULT([~/eir])

AC_SUBST([PACKAGE_VERSION])

AC_MSG_CHECKING([whether to modify moduledir])
AC_ARG_WITH(moduledir, 
AC_HELP_STRING([--with-moduledir=DIR],
   [Directory to install modules.]),
   [ MODDIR=`echo $withval | sed 's/\/$//'`
     AC_MSG_RESULT(yes)
     AC_DEFINE(MODDIR, MODDIR, [Prefix where modules are installed.])
     AC_SUBST([MODDIR]) ],
   [ MODDIR='${prefix}/modules'
     AC_MSG_RESULT(no)
     AC_DEFINE(MODDIR, MODDIR, [Prefix where modules are installed.])
     AC_SUBST([MODDIR])]
)


AC_MSG_CHECKING([whether to modify confdir])
AC_ARG_WITH(confdir, 
AC_HELP_STRING([--with-confdir=DIR],
   [Location of configuration files.]),
   [ ETCDIR=`echo $withval | sed 's/\/$//'`
     AC_MSG_RESULT(yes)
     AC_DEFINE(ETCDIR, ETCDIR, [Location of configuration files.])
     AC_SUBST([ETCDIR]) ],
   [ ETCDIR='${prefix}/etc'
     AC_MSG_RESULT(no)
     AC_DEFINE(ETCDIR, ETCDIR, [Location of configuration files.])
     AC_SUBST([ETCDIR])]
)


AC_MSG_CHECKING([whether to modify datadir])
AC_ARG_WITH(datadir, 
AC_HELP_STRING([--with-datadir=DIR],
   [Location of data files.]),
   [ DATADIR=`echo $withval | sed 's/\/$//'`
     AC_MSG_RESULT(yes)
     AC_DEFINE(DATADIR, DATADIR, [Location of data files.])
     AC_SUBST([DATADIR]) ],
   [ DATADIR='${prefix}/data'
     AC_MSG_RESULT(no)
     AC_DEFINE(DATADIR, DATADIR, [Location of data files.])
     AC_SUBST([DATADIR])]
)

AC_ARG_ENABLE([warnings],
        [AS_HELP_STRING([--enable-warnings],[Enable warnings.])],
        [enable_warnings=$enableval],
        [enable_warnings="no"])

AS_IF([test "x$enable_warnings" != "xno"],
[WARNINGS_CFLAGS="-Wall -Werror -Wextra"])
AC_SUBST(WARNINGS_CFLAGS)

AC_ARG_ENABLE([perl],
        [AS_HELP_STRING([--disable-perl],[Disable building perl scripting module.])],
        [enable_perl=$enableval],
        [enable_perl="yes"])

AS_IF([test "x$enable_perl" != "xno"],
[
  AC_PATH_PROG(perlpath, perl)
  if test -n "$perlpath"; then
      ENABLE_PERL=perl
      PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
      PERL_LIBS=`perl -MExtUtils::Embed -e ldopts`
  fi
])

AS_IF([test "x$enable_perl" = "xyes" && test "x$PERL_CFLAGS" = "x"],
[AC_MSG_ERROR([Perl support was requested but Perl could not be found.])])

AC_SUBST(ENABLE_PERL)
AC_SUBST(PERL_CFLAGS)
AC_SUBST(PERL_LIBS)

AC_OUTPUT([settings.mk])
