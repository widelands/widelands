#include "game_data_error.h"

#include <cstdarg>
#include <cstdio>

namespace Widelands {

game_data_error::game_data_error(char const * const fmt, ...)
{
	char buffer[512];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	m_what = buffer;
}

}
