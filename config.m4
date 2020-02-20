dnl config.m4 for extension study_php_extension

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([study_php_extension],
dnl   [for study_php_extension support],
dnl   [AS_HELP_STRING([--with-study_php_extension],
dnl     [Include study_php_extension support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([study_php_extension],
  [whether to enable study_php_extension support],
  [AS_HELP_STRING([--enable-study_php_extension],
    [Enable study_php_extension support])],
  [no])

if test "$PHP_STUDY_PHP_EXTENSION" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, STUDY_PHP_EXTENSION_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-study_php_extension -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/study_php_extension.h"  # you most likely want to change this
  dnl if test -r $PHP_STUDY_PHP_EXTENSION/$SEARCH_FOR; then # path given as parameter
  dnl   STUDY_PHP_EXTENSION_DIR=$PHP_STUDY_PHP_EXTENSION
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for study_php_extension files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       STUDY_PHP_EXTENSION_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$STUDY_PHP_EXTENSION_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the study_php_extension distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-study_php_extension -> add include path
  dnl PHP_ADD_INCLUDE($STUDY_PHP_EXTENSION_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-study_php_extension -> check for lib and symbol presence
  dnl LIBNAME=STUDY_PHP_EXTENSION # you may want to change this
  dnl LIBSYMBOL=STUDY_PHP_EXTENSION # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_STUDY_PHP_EXTENSION_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your study_php_extension library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $STUDY_PHP_EXTENSION_DIR/$PHP_LIBDIR, STUDY_PHP_EXTENSION_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_STUDY_PHP_EXTENSION_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your study_php_extension library.])
  dnl ],[
  dnl   -L$STUDY_PHP_EXTENSION_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(STUDY_PHP_EXTENSION_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_STUDY_PHP_EXTENSION, 1, [ Have study_php_extension support ])

  PHP_NEW_EXTENSION(study_php_extension, study_php_extension.c, $ext_shared)
fi
