#include "build_info.h"

std::string build_id()
{
	return "@WL_VERSION@";
}

std::string build_type()
{
	return "@CMAKE_BUILD_TYPE@";
}
