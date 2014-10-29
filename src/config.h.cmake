#ifndef __CONFIG_H__
#define __CONFIG_H__

// Defines if paths are absolute or relative to the executable directory.
constexpr bool PATHS_ARE_ABSOLUTE = @WL_PATHS_ARE_ABSOLUTE@;

// This is the install path prefix;
// Absolute path if PATHS_ARE_ABSOLUTE, otherwise relative to the directory
// where the executable is in.
// For portable installation, this needs to be "." and PATHS_ARE_ABSOLUTE must be false.
// For a standard Linux installation, this should be "/usr/local" and PATHS_ARE_ABSOLUTE should be true
#define INSTALL_PREFIX "@WL_INSTALL_PREFIX@"

// This is the path where the executable binary is located;
// Path is always relative to INSTALL_PREFIX, ignoring PATHS_ARE_ABSOLUTE.
// For portable installation, this needs to be ".".
// For a standard Linux installation, this should be "games"
#define INSTALL_BINDIR "@WL_INSTALL_BINDIR@"

// This is the path where the data files are located;
// Path is always relative to INSTALL_PREFIX, ignoring PATHS_ARE_ABSOLUTE.
// For portable installation, this needs to be ".".
// For a standard Linux installation, this should be "share/games/widelands"
#define INSTALL_DATADIR "@WL_INSTALL_DATADIR@"

// This is the locale directory, usually located within the data directory;
// Absolute path if PATHS_ARE_ABSOLUTE, otherwise relative to the directory
// where the executable is in.
// For portable installation, this needs to be "locale" and PATHS_ARE_ABSOLUTE
// must be false.
// For a standard Linux installation, this should be
// "/usr/local/share/games/widelands/locale" and PATHS_ARE_ABSOLUTE should be
// true.
#define INSTALL_LOCALEDIR "@WL_INSTALL_LOCALEDIR@"

// don't know if this causes problems on Windows but it solves the problems
// finding a user's home directory on Linux
#define HAS_GETENV

#endif /* __CONFIG_H__ */
