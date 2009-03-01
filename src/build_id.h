#ifndef BUILD_ID_H
#define BUILD_ID_H
#define WLCR "(C) 2002-2009 "

#include <string>

///\return the build id, which is either specified manually when calling scons
///or is automagically created from the checkout's revision number
std::string build_id();

#endif

