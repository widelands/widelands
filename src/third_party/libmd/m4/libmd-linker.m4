# Copyright © 2014 Guillem Jover <guillem@hadrons.org>

# LIBMD_LINKER_VERSION_SCRIPT
# --------------------------
AC_DEFUN([LIBMD_LINKER_VERSION_SCRIPT], [
  AC_CACHE_CHECK([for --version-script linker flag], [libmd_cv_version_script], [
    echo "{ global: symbol; local: *; };" >conftest.map
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.map"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
extern int symbol(void);
int symbol(void) { return 0; }
]], [[
]])
    ], [
      libmd_cv_version_script=yes
    ], [
      libmd_cv_version_script=no
    ])
    LDFLAGS="$save_LDFLAGS"
    rm -f conftest.map
  ])
  AM_CONDITIONAL([HAVE_LINKER_VERSION_SCRIPT],
    [test "x$libmd_cv_version_script" = "xyes"])
])
