/*
 * Copyright (C) 2002, 2006-2007, 2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "base/log.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "base/macros.h"
#include "base/wexception.h"

namespace {

// Forward declaration to work around cyclic dependency.
void sdl_logging_func(void* userdata, int, SDL_LogPriority, const char* message);

#ifdef _WIN32

std::string get_output_directory() {
	// This took inspiration from SDL 1.2 logger code.
#ifdef _WIN32_WCE
	wchar_t path[MAX_PATH];
#else
	char path[MAX_PATH];
#endif
	auto pathlen = GetModuleFileName(NULL, path, MAX_PATH);
	while (pathlen > 0 && path[pathlen] != '\\') {
		--pathlen;
	}
	path[pathlen] = '\0';
	return path;
}

// This Logger emulates the SDL1.2 behavior of writing a stdout.txt.
class WindowsLogger {
public:
	WindowsLogger() : stdout_filename_(get_output_directory() + "\\stdout.txt") {
		stdout_.open(stdout_filename_);
		if (!stdout_.good()) {
			throw wexception("Unable to initialize logging to stdout.txt");
		}
		SDL_LogSetOutputFunction(sdl_logging_func, this);
	}

	void log_cstring(const char* buffer) {
		stdout_ << buffer;
		stdout_.flush();
	}

private:
	const std::string stdout_filename_;
	std::ofstream stdout_;

	DISALLOW_COPY_AND_ASSIGN(WindowsLogger);
};

void sdl_logging_func(void* userdata,
                      int /* category */,
                      SDL_LogPriority /* priority */,
                      const char* message) {
	static_cast<WindowsLogger*>(userdata)->log_cstring(message);
}
#else // _WIN32

class Logger {
public:
	Logger() {
		SDL_LogSetOutputFunction(sdl_logging_func, this);
	}

	void log_cstring(const char* buffer) {
		std::cout << buffer;
		std::cout.flush();
	}

private:
	DISALLOW_COPY_AND_ASSIGN(Logger);
};

void sdl_logging_func(void* userdata,
                      int /* category */,
                      SDL_LogPriority /* priority */,
                      const char* message) {
	static_cast<Logger*>(userdata)->log_cstring(message);
}
#endif

}  // namespace

// Default to stdout for logging.
bool g_verbose = false;

void log(const char* const fmt, ...) {
#ifdef _WIN32
	static WindowsLogger logger;
#else
	static Logger logger;
#endif
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);
	logger.log_cstring(buffer);
}
