dnl $Id$
dnl config.m4 for extension odus

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(odus, for odus support,
dnl Make sure that the comment is aligned:
dnl [  --with-odus             Include odus support])

dnl Otherwise use enable:

 PHP_ARG_ENABLE(odus, whether to enable odus support,
 Make sure that the comment is aligned:
 [  --enable-odus           Enable odus support])

if test "$PHP_ODUS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-odus -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/odus.h"  # you most likely want to change this
  dnl if test -r $PHP_ODUS/$SEARCH_FOR; then # path given as parameter
  dnl   ODUS_DIR=$PHP_ODUS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for odus files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       ODUS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$ODUS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the odus distribution])
  dnl fi

  dnl # --with-odus -> add include path
  dnl PHP_ADD_INCLUDE($ODUS_DIR/include)

  dnl # --with-odus -> check for lib and symbol presence
  dnl LIBNAME=odus # you may want to change this
  dnl LIBSYMBOL=odus # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ODUS_DIR/lib, ODUS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ODUSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong odus lib version or lib not found])
  dnl ],[
  dnl   -L$ODUS_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(ODUS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(odus, odus.c odwrapper.c od_hash.c hash_si.c hash_function.c od_igbinary.c od_debug.c, $ext_shared)
fi
