#include "build_info.h"
#include <string>

constexpr const char* const wl_bid = "@WL_VERSION@";
constexpr const char* const wl_bt  = "@CMAKE_BUILD_TYPE@";

const std::string & build_id()
{
	return wl_bid;
}

const std::string & build_type()
{
	return wl_bt;
}
