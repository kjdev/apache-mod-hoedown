#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(mod_hoedown, 0.2.0,[])

AC_CONFIG_SRCDIR([mod_hoedown.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
# AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL

# Option for hoedown url support
AC_ARG_ENABLE(hoedown-url-support,
  AC_HELP_STRING([--enable-hoedown-url-support],
    [Enable hoedown url support [default=no]]),
  [ENABLED_HOEDOWN_URL_SUPPORT="${enableval:-yes}"],
  [ENABLED_HOEDOWN_URL_SUPPORT=no]
)
AS_IF([test "x${ENABLED_HOEDOWN_URL_SUPPORT}" = xyes],
    [AC_DEFINE([HOEDOWN_URL_SUPPORT], [1], [Enable hoedown url support])]
)

# Checks for apxs.
AC_ARG_WITH(apxs,
  [AC_HELP_STRING([--with-apxs=PATH], [apxs path [default=yes]])],
  [APXS_PATH="$withval"],
  [APXS_PATH="/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"]
)

AC_PATH_PROG(APXS, apxs, no, ${APXS_PATH})

AS_IF([test "x${APXS}" = "xno"], AC_MSG_ERROR([apxs not found]),
  [test "x${APXS}" = "x"], AC_MSG_ERROR([apxs not found])
)

APXS_CFLAGS=`${APXS} -q CFLAGS 2> /dev/null`
APXS_CPPFLAGS=`${APXS} -q CPPFLAGS 2> /dev/null`
APXS_LDFLAGS=`${APXS} -q LDFLAGS 2> /dev/null`
APXS_LIBS=`${APXS} -q LIBS 2> /dev/null`
APXS_LIBEXECDIR=`${APXS} -q LIBEXECDIR 2> /dev/null`

APXS_INCLUDEDIR=`${APXS} -q INCLUDEDIR 2> /dev/null`
APXS_INCLUDES="-I${APXS_INCLUDEDIR}"

# Checks for apr.
AC_ARG_WITH(apr,
  [AC_HELP_STRING([--with-apr=PATH], [apr config path [default=yes]])],
  [APR_CONFIG="$withval"],
  [with_apr=yes]
)

AC_MSG_CHECKING([whether apr])
AS_IF([test "x$with_apr" != "xno"],
  [
    if test ! -x "${APR_CONFIG}" -o -z "${APR_CONFIG}"; then
      APR_PATH=`${APXS} -q APR_BINDIR 2> /dev/null`
      APR_CONFIG="${APR_PATH}/apr-1-config"
      if test ! -x "${APR_CONFIG}"; then
        APR_CONFIG="${APR_PATH}/apr-config"
      fi
    fi
    APR_INCLUDES=`${APR_CONFIG} --includes 2> /dev/null`
    APR_CFLAGS=`${APR_CONFIG} --cflags 2> /dev/null`
    APR_CPPFLAGS=`${APR_CONFIG} --cppflags 2> /dev/null`
    APR_LDFLAGS=`${APR_CONFIG} --ldflags 2> /dev/null`
    APR_LIBS=`${APR_CONFIG} --libs 2> /dev/null`
    AC_MSG_RESULT(yes)
  ],
  AC_MSG_ERROR(apr not found)
)

# Checks for apreq2.
AC_ARG_WITH(apreq2,
  [AC_HELP_STRING([--with-apreq2=PATH], [apreq2 config path [default=yes]])],
  [APREQ2_CONFIG="$withval"],
  [with_apreq2=yes]
)

AC_MSG_CHECKING([whether apreq2])
AS_IF([test "x$with_apreq2" != "xno"],
  [
    if test ! -x "${APREQ2_CONFIG}" -o -z "${APREQ2_CONFIG}"; then
      SEARCH_PATH="/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"
      AC_PATH_PROG(APREQ2_CONFIG, apreq2-config, no, ${SEARCH_PATH})
      AS_IF(
        [test "x${APREQ2_CONFIG}" = "xno"], AC_MSG_ERROR([apreq2-config not found]),
        [test "x${APREQ2_CONFIG}" = "x"], AC_MSG_ERROR([apreq2-config not found])
      )
    fi
    APREQ2_INCLUDES=`${APREQ2_CONFIG} --includes 2> /dev/null`
    APREQ2_LDFLAGS=`${APREQ2_CONFIG} --ldflags 2> /dev/null`
    APREQ2_LIBS=`${APREQ2_CONFIG} --libs 2> /dev/null`
    AC_MSG_RESULT(yes)
  ],
  AC_MSG_ERROR(apreq2 not found)
)

# Apache libraries.
APACHE_MODULEDIR="${APXS_LIBEXECDIR}"
APACHE_INCLUDES="${APXS_INCLUDES} ${APR_INCLUDES} ${APREQ2_INCLUDES}"
APACHE_CFLAGS="${APXS_CFLAGS} ${APR_CFLAGS}"
APACHE_CPPFLAGS="${APXS_CPPFLAGS} ${APR_CPPFLAGS}"
APACHE_LDFLAGS="${APXS_LDFLAGS} ${APR_LDFLAGS} ${APREQ2_LDFLAGS}"
APACHE_LIBS="${APXS_LIBS} ${APR_LIBS} ${APREQ2_LIBS}"

AC_SUBST(APACHE_MODULEDIR)
AC_SUBST(APACHE_INCLUDES)
AC_SUBST(APACHE_CFLAGS)
AC_SUBST(APACHE_CPPFLAGS)
AC_SUBST(APACHE_LDFLAGS)
AC_SUBST(APACHE_LIBS)


AS_IF([test "x${ENABLED_HOEDOWN_URL_SUPPORT}" = xyes],
  [
    # Checks for curl.
    AC_ARG_WITH(curl,
      [AC_HELP_STRING([--with-curl=PATH], [curl path [default=/usr/include]])],
      [CURL_PATH="$withval"],
      [CURL_PATH="/usr/include"]
    )

    AC_CHECK_FILE(
      "${CURL_PATH}/curl/curl.h",
      [CURL_INCLUDEDIR=${CURL_PATH}],
      [AC_MSG_ERROR([Missing required curl header.])]
    )

    AC_ARG_WITH(curl-lib,
      [AC_HELP_STRING([--with-curl-lib=PATH], [curl library path [default=no]])],
      [CURL_LIBPATH="$withval"],
      [CURL_LIBPATH="no"]
    )

    AS_IF([test "x${CURL_LIBPATH}" != "xno"],
      CURL_LDFLAGS="-L${CURL_LIBPATH}"
    )

    AC_MSG_CHECKING([for curl library])
    SAVED_CFLAGS=$CFLAGS
    SAVED_LDFLAGS=$LDFLAGS
    CFLAGS="$CFLAGS -I ${CURL_INCLUDEDIR}"
    LDFLAGS="$LDFLAGS ${CURL_LDFLAGS} -lcurl"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
        [#include "curl/curl.h"],
        [CURL *curl])],
      [CURL_LIBS="-lcurl"],
      [AC_MSG_ERROR([Missing required curl library.])]
    )
    AC_MSG_RESULT(yes)
    CFLAGS=$SAVED_CFLAGS
    LDFLAGS=$SAVED_LDFLAGS

    # curl libraries.
    CURL_CFLAGS="-I ${CURL_INCLUDES}"
    CURL_CPPFLAGS="${CURL_CFLAGS}"
    CURL_LDFLAGS="${CURL_LDFLAGS} ${CURL_LIBS}"
  ]
)

AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_CPPFLAGS)
AC_SUBST(CURL_LDFLAGS)
AC_SUBST(CURLLIBS)


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
