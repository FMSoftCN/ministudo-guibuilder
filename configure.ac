#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([guibuilder], [1.0.8], -)
AC_CONFIG_SRCDIR([main.cpp])

guibuilder_MAJOR_VERSION=1
guibuilder_MINOR_VERSION=0
guibuilder_MICRO_VERSION=8
guibuilder_INTERFACE_AGE=0
guibuilder_BINARY_AGE=0
guibuilder_VERSION=$guibuilder_MAJOR_VERSION.$guibuilder_MINOR_VERSION.$guibuilder_MICRO_VERSION

AC_SUBST(guibuilder_MAJOR_VERSION)
AC_SUBST(guibuilder_MINOR_VERSION)
AC_SUBST(guibuilder_MICRO_VERSION)
AC_SUBST(guibuilder_INTERFACE_AGE)
AC_SUBST(guibuilder_BINARY_AGE)
AC_SUBST(guibuilder_VERSION)

AC_DEFINE_UNQUOTED(GUIBUILD_MAJOR_VERSION, $guibuilder_MAJOR_VERSION,
            [Major version of guibuilder])
AC_DEFINE_UNQUOTED(GUIBUILD_MINOR_VERSION, $guibuilder_MINOR_VERSION,
            [Minor version of guibuilder])
AC_DEFINE_UNQUOTED(GUIBUILD_MICRO_VERSION, $guibuilder_MICRO_VERSION,
            [Micro version of guibuilder])
AC_DEFINE_UNQUOTED(GUIBUILD_INTERFACE_AGE, $guibuilder_INTERFACE_AGE,
            [Interface age of guibuilder])
AC_DEFINE_UNQUOTED(GUIBUILD_BINARY_AGE, $guibuilder_BINARY_AGE,
            [Binary age of guibuilder])


# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AM_INIT_AUTOMAKE

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/socket.h syslog.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_FORK
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit bzero gethostbyname memset mkdir mkfifo select setenv socket strcasecmp strchr strdup strerror strncasecmp strndup strrchr strstr strtol strtoul])

AM_CONFIG_HEADER(gbconfig.h)

dnl ========================================================================
dnl check for run-time mode of MiniGUI
dnl ========================================================================

have_libminigui="no"
AC_CHECK_HEADERS(minigui/common.h, have_libminigui=yes, foo=bar)

threads_version="no"
AC_CHECK_DECLS(_MGRM_THREADS, threads_version="yes", foo=bar, [#include <minigui/common.h>])

if test "x$have_libminigui" != "xyes"; then
	AC_MSG_ERROR([
		Need MiniGUI Ver 3.0.2 or later
		])
fi

if test "x$threads_version" != "xyes"; then
	AC_MSG_ERROR([
		Need MiniGUI thread mode 
	])
fi


AC_CHECK_HEADERS([mgutils/mgutils.h mgplus/mgplus.h mgncs/mgncs.h])

LIBS="$LIBS -lmgutils_msd -lmgplus_msd -lmgncs_msd"

AC_CHECK_HEADERS([curl/curl.h])
LIBS="$LIBS -lcurl"

CFLAGS="$CFLAGS `xml2-config --cflags`"
CXXFLAGS="$CFLAGS"
CPPFLAGS="$CFLAGS"

LIBS="$LIBS `xml2-config --libs`"

LIBS="$LIBS -lminigui_msd -lpthread -lpng -ljpeg -lfreetype"
AC_CHECK_HEADERS([lua.h])

LIBS="$LIBS -llua5.1 -ldl"

dnl macro definition
build_official_release="no"
build_locale="no"

AC_ARG_ENABLE(release,
[  --enable-release include softdog and limitation function <default=no>],
build_official_release=$enableval)

if test "x$build_official_release" = "xyes"; then
    AC_DEFINE(_MSTUDIO_OFFICIAL_RELEASE, 1,
            [Define if include softdog and limitation function])

    AC_ARG_WITH(maxnum_limitwnds,
        [ --with-maxnum-limitwnds=NUM   Define the support for maximum window numbers in official release],
        AC_DEFINE_UNQUOTED([_MSTUDIO_MAXNUM_LIMITWNDS], $withval,
                    [Define the support for maximum window numbers in official release]),
        AC_DEFINE_UNQUOTED([_MSTUDIO_MAXNUM_LIMITWNDS], 1,
                    [Define the support for maximum window numbers in official release]))

	LIBS="$LIBS -lsense4 -lusb"
fi

AC_ARG_ENABLE(locale,
[  --enable-locale include muli-language suppport <default=no>],
build_locale=$enableval)

if test "x$build_locale" = "xyes"; then
	AC_DEFINE(_MSTUDIO_LOCALE, 1,
			[Define if include locale function])

#	LIBS="$LIBS -lintl"
fi

AC_ARG_WITH(customversion,
  [  --with-customversion=STR   Configure special package version string in about dialog.],
  [ case "$withval" in
    "")
        AC_MSG_WARN([  --with-customversion called without argument - will use default version number])
        ;;
    *)
        AC_DEFINE_UNQUOTED(_MSTUDIO_PACKAGE_STRING, "$withval", [Define special package version string])
        ;;
    esac
  ]
  )

LDFLAGS="$LDFLAGS $LIBS"

AC_OUTPUT([
	Makefile
	texteditor/Makefile
	texteditor/json/Makefile
	resideditor/Makefile
	uieditor/Makefile
	include/Makefile
	etc/Makefile
	etc/icon/Makefile
	etc/icon/uieditor/Makefile
	etc/icon/uieditor/defines/Makefile
	etc/msd/Makefile
	etc/uieditor/Makefile
	etc/uieditor/templates/Makefile
	etc/uieditor/defines/Makefile
	etc/uieditor/trans/Makefile
	etc/Emulator/Makefile
	etc/renderer/Makefile
	etc/renderer/classic/Makefile
	etc/renderer/flat/Makefile
	etc/renderer/skin/Makefile
	etc/renderer/fashion/Makefile
	etc/user-templates/Makefile
	etc/lang/Makefile
	etc/imgeditor/Makefile
	imgeditor/Makefile
	mgfc/Makefile
	rdreditor/Makefile
	mainframe/Makefile
	tools/Makefile
	tools/ncs2ics/Makefile
	tools/ncs2ics/trans/Makefile
	trans/Makefile
])

