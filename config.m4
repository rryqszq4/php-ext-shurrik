dnl $Id$
dnl config.m4 for extension shurrik

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(shurrik, for shurrik support,
dnl Make sure that the comment is aligned:
dnl [  --with-shurrik             Include shurrik support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(shurrik, whether to enable shurrik support,
dnl Make sure that the comment is aligned:
dnl [  --enable-shurrik           Enable shurrik support])

if test "$PHP_SHURRIK" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-shurrik -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/shurrik.h"  # you most likely want to change this
  dnl if test -r $PHP_SHURRIK/$SEARCH_FOR; then # path given as parameter
  dnl   SHURRIK_DIR=$PHP_SHURRIK
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for shurrik files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SHURRIK_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SHURRIK_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the shurrik distribution])
  dnl fi

  dnl # --with-shurrik -> add include path
  dnl PHP_ADD_INCLUDE($SHURRIK_DIR/include)

  dnl # --with-shurrik -> check for lib and symbol presence
  dnl LIBNAME=shurrik # you may want to change this
  dnl LIBSYMBOL=shurrik # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SHURRIK_DIR/lib, SHURRIK_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SHURRIKLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong shurrik lib version or lib not found])
  dnl ],[
  dnl   -L$SHURRIK_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(SHURRIK_SHARED_LIBADD)

  PHP_NEW_EXTENSION(shurrik, shurrik.c, $ext_shared)
fi
