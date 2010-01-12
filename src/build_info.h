#ifndef BUILD_ID_H
#define BUILD_ID_H
#define WLCR "(C) 2002-2010 "

#include <string>

///\return the build id, which is either specified manually when calling scons
///or is automagically created from the checkout's revision number
std::string build_id();

///\return the build type, which is set during the scons call (either manually
///or to a default value)
std::string build_type();

#endif

