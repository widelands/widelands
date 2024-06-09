# Copyright © 2021 Guillem Jover <guillem@hadrons.org>

# LIBMD_CHECK_COMPILER_FLAG
# -------------------------
AC_DEFUN([LIBMD_CHECK_COMPILER_FLAG], [
  AS_VAR_PUSHDEF([libmd_varname_cache], [libmd_cv_cflags_$1])
  AC_CACHE_CHECK([whether $CC accepts $1], [libmd_varname_cache], [
    m4_define([libmd_check_flag], m4_bpatsubst([$1], [^-Wno-], [-W]))
    AS_VAR_COPY([libmd_save_CFLAGS], [CFLAGS])
    AS_VAR_SET([CFLAGS], ["-Werror libmd_check_flag"])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[]])
    ], [
      AS_VAR_SET([libmd_varname_cache], [yes])
    ], [
      AS_VAR_SET([libmd_varname_cache], [no])
    ])
    AS_VAR_COPY([CFLAGS], [libmd_save_CFLAGS])
  ])
  AS_VAR_IF([libmd_varname_cache], [yes], [
    AS_VAR_APPEND([LIBMD_COMPILER_FLAGS], [" $1"])
  ])
  AS_VAR_POPDEF([libmd_varname_cache])
])
