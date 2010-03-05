#ifndef __CONFIG_H__
#define __CONFIG_H__

#define INSTALL_PREFIX "@WL_INSTALL_PREFIX@"
#define INSTALL_BINDIR "@WL_INSTALL_BINDIR@"
#define INSTALL_DATADIR "@WL_INSTALL_DATADIR@"
#define INSTALL_LOCALEDIR "@WL_INSTALL_LOCALEDIR@"

//don't know if this causes problems on Windows
//but it solves the problems finding a user's home directory on Linux
#define HAS_GETENV

// GCC supports variable length arrays, but MSVC2008 not.
#cmakedefine HAVE_VARARRAY

#endif /* __CONFIG_H__ */
