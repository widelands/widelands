#ifndef __CONFIG_H__
#define __CONFIG_H__

// Defines if paths are absolute or relative to the executable directory.
constexpr bool PATHS_ARE_ABSOLUTE = @WL_PATHS_ARE_ABSOLUTE@;

// This is the path where the data files are located;
// It ignores PATHS_ARE_ABSOLUTE.
// For portable installation, this needs to be ".".
// For a standard Linux installation, this should be "share/games/widelands"
#define INSTALL_DATADIR "@WL_INSTALL_DATADIR@"

// don't know if this causes problems on Windows but it solves the problems
// finding a user's home directory on Linux
#define HAS_GETENV

#endif /* __CONFIG_H__ */
