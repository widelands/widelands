#ifndef __CONFIG_H__
#define __CONFIG_H__

//This is the install path prefix;
//Absolute path
//For portable installation, this needs to be "."
//For a standard Linux installation, this should be "/usr/local"
#define INSTALL_PREFIX "@WL_INSTALL_PREFIX@"

//This is the path where the executable binary is located;
//Path relative to INSTALL_PREFIX
//For portable installation, this needs to be "."
//For a standard Linux installation, this should be "games"
#define INSTALL_BINDIR "@WL_INSTALL_BINDIR@"

//This is the path where the data files are located;
//Path relative to INSTALL_PREFIX
//For portable installation, this needs to be "."
//For a standard Linux installation, this should be "share/games/widelands"
#define INSTALL_DATADIR "@WL_INSTALL_DATADIR@"

//This is the locale directory, usually located within the data directory;
//Either path relative to working directory of the executable or absolute path
//For portable installation, this needs to be "locale" (relative path)
//For a standard Linux installation, this should be "/usr/local/share/games/widelands/locale" (absolute path)
#define INSTALL_LOCALEDIR "@WL_INSTALL_LOCALEDIR@"

//don't know if this causes problems on Windows
//but it solves the problems finding a user's home directory on Linux
#define HAS_GETENV

#endif /* __CONFIG_H__ */
