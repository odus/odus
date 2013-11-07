dnl $Id$
dnl config.m4 for extension odus

 PHP_ARG_ENABLE(odus, whether to enable odus support,
 Make sure that the comment is aligned:
 [  --enable-odus           Enable odus support])

if test "$PHP_ODUS" != "no"; then
  PHP_NEW_EXTENSION(odus, odus.c odwrapper.c od_hash.c hash_si.c hash_function.c od_igbinary.c od_debug.c, $ext_shared)
fi
