/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include <cassert>
#include <cstdarg>
#ifdef _WIN32
#include <fstream>
#endif
#include <iostream>
#include <memory>

#include <SDL_log.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "base/macros.h"
#include "base/wexception.h"
#ifdef _WIN32
#include "build_info.h"
#endif

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
	WindowsLogger(const std::string& dir) : stdout_filename_(dir + "\\stdout.txt") {
		stdout_.open(stdout_filename_);
		if (!stdout_.good()) {
			throw wexception(
			   "Unable to initialize stdout logging destination: %s", stdout_filename_.c_str());
		}
		SDL_LogSetOutputFunction(sdl_logging_func, this);
		std::cout << "Log output will be written to: " << stdout_filename_ << std::endl;

		// Repeat version info so that we'll have it available in the log file too
		stdout_ << "This is Widelands Version " << build_id() << " (" << build_type() << ")"
		        << std::endl;
		stdout_.flush();
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
#else  // _WIN32

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

#ifdef _WIN32
// Start with nullptr so that we won't initialize an empty file in the program's directory
std::unique_ptr<WindowsLogger> logger(nullptr);

// Set the logging dir to the given homedir
bool set_logging_dir(const std::string& homedir) {
	try {
		logger.reset(new WindowsLogger(homedir));
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		return false;
	}
	return true;
}

// Set the logging dir to the program's dir. For running test cases where we don't have a homedir.
void set_logging_dir() {
	logger.reset(new WindowsLogger(get_output_directory()));
}

#else
std::unique_ptr<Logger> logger(new Logger());
#endif

static const char* to_string(const LogType& type) {
	switch (type) {
	case LogType::kInfo:
		return "Info   ";
	case LogType::kDebug:
		return "Debug  ";
	case LogType::kWarning:
		return "WARNING";
	case LogType::kError:
		return "ERROR  ";
	default:
		NEVER_HERE();
	}
}

void log_to_stdout(const LogType type, int64_t gametime, const char* const fmt, ...) {
	assert(logger != nullptr);

	{  // message type
		char buffer[16];
		sprintf(buffer, "[%s", to_string(type));
		logger->log_cstring(buffer);
	}
	if (gametime >= 0) {  // timestamp
		const unsigned hours = gametime / (1000 * 60 * 60);
		gametime -= hours * 1000 * 60 * 60;
		const unsigned minutes = gametime / (1000 * 60);
		gametime -= minutes * 1000 * 60;
		const unsigned seconds = gametime / 1000;
		gametime -= seconds * 1000;

		char buffer[32];
		sprintf(buffer, " @ %u:%u:%u.%li", hours, minutes, seconds, gametime);
		logger->log_cstring(buffer);
	}
	logger->log_cstring("] ");

	// actual log output
	char buffer[2048];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);
	logger->log_cstring(buffer);
}
