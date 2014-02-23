#ifndef BUILD_ID_H
#define BUILD_ID_H
#define WLCR "(C) 2002-2014 "

#include <string>

///\return the build id which is automagically created from the checkout's
///revision number or the VERSION file
const std::string & build_id();

///\return the build type, which is set during compile time (either manually
///or to a default value)
const std::string & build_type();

#endif

