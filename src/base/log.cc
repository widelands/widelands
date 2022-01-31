/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "base/log.h"

#include <cassert>
#include <cstdarg>
#ifdef _WIN32
#include <cstdio>
#include <fstream>
#endif
#include <iostream>
#include <memory>
#include <vector>

#include <SDL_log.h>
#include <SDL_timer.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "base/multithreading.h"
#include "base/string.h"
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

// This Logger emulates the SDL1.2 behavior of writing a stdout.txt and stderr.txt.
class WindowsLogger {
public:
	WindowsLogger(const std::string& dir)
	   : stdout_filename_(dir + "\\stdout.txt"), stderr_filename_(dir + "\\stderr.txt") {
		stdout_.open(stdout_filename_);
		stderr_.open(stderr_filename_);
		if (!stdout_.good() || !stderr_.good()) {
			throw wexception(
			   "Unable to initialize stdout logging destination: %s", stdout_filename_.c_str());
		}
		SDL_LogSetOutputFunction(sdl_logging_func, this);
		std::cout << "Log output will be written to: " << stdout_filename_ << std::endl;

		// Configure redirection
		std::cout.rdbuf(stdout_.rdbuf());
		std::cerr.rdbuf(stderr_.rdbuf());
		// Repeat version info so that we'll have it available in the log file too
		std::cout << "This is Widelands Version " << build_id() << " (" << build_type() << ")"
		          << std::endl;
		stdout_.flush();
	}

	void log_cstring(const char* buffer) {
		stdout_ << buffer;
		stdout_.flush();
	}

private:
	const std::string stdout_filename_, stderr_filename_;
	std::ofstream stdout_, stderr_;

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
void set_testcase_logging_dir() {
	logger.reset(new WindowsLogger(get_output_directory()));
}

#else
std::unique_ptr<Logger> logger(new Logger());
#endif

static const char* to_string(const LogType& type) {
	switch (type) {
	case LogType::kInfo:
		return "INFO";
	case LogType::kDebug:
		return "DEBUG";
	case LogType::kWarning:
		return "WARNING";
	case LogType::kError:
		return "ERROR";
	}
	NEVER_HERE();
}

void do_log(const LogType type, const Time& gametime, const char* const fmt, ...) {
	MutexLock m(MutexLock::ID::kLog);
	assert(logger != nullptr);

	// message type and timestamp
	char buffer_prefix[32];
	{
		uint32_t t = gametime.is_valid() ? gametime.get() : SDL_GetTicks();
		const uint32_t hours = t / (1000 * 60 * 60);
		t -= hours * 1000 * 60 * 60;
		const uint32_t minutes = t / (1000 * 60);
		t -= minutes * 1000 * 60;
		const uint32_t seconds = t / 1000;
		t -= seconds * 1000;
		snprintf(buffer_prefix, sizeof(buffer_prefix), "[%02u:%02u:%02u.%03u %s] %s: ", hours,
		         minutes, seconds, t, gametime.is_invalid() ? "real" : "game", to_string(type));
	}

	// actual log output
	char buffer[2048];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	std::vector<std::string> vec;
	split(vec, buffer, {'\n'});
	for (std::string& str : vec) {
		if (str.find_first_not_of(' ') == std::string::npos) {
			continue;
		}
		logger->log_cstring(buffer_prefix);
		str.push_back('\n');
		logger->log_cstring(str.c_str());
	}
}
