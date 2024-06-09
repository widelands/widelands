# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libmd], [m4_esyscmd([./get-version])], [libbsd@lists.freedesktop.org])
AC_CONFIG_SRCDIR([src/sha2.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(
  [1.11]
  [-Wall]
  [foreign]
  [nostdinc]
  [no-dist-gzip dist-xz]
)
AM_SILENT_RULES([yes])

SOVERSION_MAJOR=0
SOVERSION_MINOR=1
SOVERSION_PATCH=0

SOVERSION="$SOVERSION_MAJOR:$SOVERSION_MINOR:$SOVERSION_PATCH"
AC_SUBST([SOVERSION])

# Check and store if we got user supplied variables.
user_CFLAGS=${CFLAGS-unset}

# Checks for operating system services and capabilities.
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_PROG_AR
LT_INIT
LIBMD_LINKER_VERSION_SCRIPT

# Checks for programs.
AC_PROG_SED
AC_PROG_CC

# Checks for the build machinery.
AS_IF([test "$user_CFLAGS" = unset], [
  LIBMD_CHECK_COMPILER_FLAG([-Wall])
  LIBMD_CHECK_COMPILER_FLAG([-Wextra])

  LIBMD_CHECK_COMPILER_FLAG([-Wbad-function-cast])
  LIBMD_CHECK_COMPILER_FLAG([-Wc99-c11-compat])
  LIBMD_CHECK_COMPILER_FLAG([-Wcast-align])
  LIBMD_CHECK_COMPILER_FLAG([-Wdeclaration-after-statement])
  LIBMD_CHECK_COMPILER_FLAG([-Wdocumentation])
  LIBMD_CHECK_COMPILER_FLAG([-Wduplicated-branches])
  LIBMD_CHECK_COMPILER_FLAG([-Wduplicated-cond])
  LIBMD_CHECK_COMPILER_FLAG([-Wformat -Wformat-security])
  LIBMD_CHECK_COMPILER_FLAG([-Wformat=2])
  LIBMD_CHECK_COMPILER_FLAG([-Winit-self])
  LIBMD_CHECK_COMPILER_FLAG([-Wlogical-not-parentheses])
  LIBMD_CHECK_COMPILER_FLAG([-Wlogical-op])
  LIBMD_CHECK_COMPILER_FLAG([-Wmissing-declarations])
  LIBMD_CHECK_COMPILER_FLAG([-Wmissing-format-attribute])
  LIBMD_CHECK_COMPILER_FLAG([-Wmissing-prototypes])
  LIBMD_CHECK_COMPILER_FLAG([-Wnested-externs])
  LIBMD_CHECK_COMPILER_FLAG([-Wno-tautological-constant-out-of-range-compare])
  LIBMD_CHECK_COMPILER_FLAG([-Wno-unused-parameter])
  LIBMD_CHECK_COMPILER_FLAG([-Wnull-dereference])
  LIBMD_CHECK_COMPILER_FLAG([-Wold-style-definition])
  LIBMD_CHECK_COMPILER_FLAG([-Wpointer-arith])
  LIBMD_CHECK_COMPILER_FLAG([-Wredundant-decls])
  LIBMD_CHECK_COMPILER_FLAG([-Wregister])
  LIBMD_CHECK_COMPILER_FLAG([-Wrestrict])
  LIBMD_CHECK_COMPILER_FLAG([-Wshadow])
  LIBMD_CHECK_COMPILER_FLAG([-Wshift-negative-value])
  LIBMD_CHECK_COMPILER_FLAG([-Wsizeof-array-argument])
  LIBMD_CHECK_COMPILER_FLAG([-Wstrict-prototypes])
  LIBMD_CHECK_COMPILER_FLAG([-Wswitch-bool])
  LIBMD_CHECK_COMPILER_FLAG([-Wvla])
  LIBMD_CHECK_COMPILER_FLAG([-Wwrite-strings])

  CFLAGS="$LIBMD_COMPILER_FLAGS $CFLAGS"

  AC_ARG_ENABLE([sanitize],
    [AS_HELP_STRING([--enable-sanitize], [enable compiler sanitizer support])],
  [
    LIBMD_COMPILER_FLAGS=''

    LIBMD_CHECK_COMPILER_FLAG([-fsanitize=address])
    LIBMD_CHECK_COMPILER_FLAG([-fsanitize=leak])
    LIBMD_CHECK_COMPILER_FLAG([-fsanitize=undefined])

    CFLAGS="$LIBMD_COMPILER_FLAGS $CFLAGS"
    LDFLAGS="$LIBMD_COMPILER_FLAGS $LDFLAGS"
  ])
])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  man/Makefile
  src/Makefile
  src/libmd.pc
  test/Makefile
])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
