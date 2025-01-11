/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include "base/backtrace_provider.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef PRINT_SEGFAULT_BACKTRACE
#include <execinfo.h>
#include <unistd.h>
#endif

#include "base/multithreading.h"
#include "base/time_string.h"
#include "build_info.h"
#include "logic/filesystem_constants.h"
#include "wlapplication.h"

// Taken from https://stackoverflow.com/a/77336
// TODO(Nordfriese): Implement this on Windows as well (see https://stackoverflow.com/a/26398082)
[[maybe_unused]] static void segfault_handler(const int sig) {
	const std::string signal_description = BacktraceProvider::get_signal_description(sig);

	std::cout << std::endl
	          << "##############################" << std::endl
	          << "FATAL ERROR: Received signal " << signal_description << std::endl
	          << "Backtrace:" << std::endl;

#ifdef PRINT_SEGFAULT_BACKTRACE
	constexpr int kMaxBacktraceSize = 256;
	void* array[kMaxBacktraceSize];
	size_t size = backtrace(array, kMaxBacktraceSize);
	backtrace_symbols_fd(array, size, STDOUT_FILENO);
#endif

	std::cout
	   << std::endl
	   << "Please report this problem to help us improve Widelands, and provide the complete output."
	   << std::endl
	   << "##############################" << std::endl;

	std::string filename;
	if (WLApplication::segfault_backtrace_dir.empty()) {
		filename = "./widelands_crash_report_";
	} else {
		filename = WLApplication::segfault_backtrace_dir;
		filename += "/";
	}

	const std::string timestr = timestring();
	filename += timestr;

	std::string thread_name;
	if (is_initializer_thread()) {
		filename += "_ui";
		thread_name = "UI thread";
	} else if (is_logic_thread()) {
		filename += "_logic";
		thread_name = "logic thread";
	} else {
		std::ostringstream thread_id;
		thread_id << std::this_thread::get_id();
		filename += "_";
		filename += thread_id.str();
		thread_name = "thread " + thread_id.str();
	}

	filename += kCrashExtension;

	// Mode 'b' is suitable on Windows platforms, to prevent pollution with '\r' characters
	// in CrashReportWindow::CrashReportWindow()
	FILE* file = fopen(filename.c_str(), "w+b");
	if (file == nullptr) {
		std::cout << "The crash report could not be saved to file " << filename << std::endl
		          << std::endl;
	} else {
		fprintf /* NOLINT codecheck */ (
		   file, "Crash report for Widelands %s %s at %s, signal %s\n\n**** BEGIN BACKTRACE ****\n",
		   build_ver_details().c_str(), thread_name.c_str(), timestr.c_str(),
		   signal_description.c_str());
#ifdef PRINT_SEGFAULT_BACKTRACE
		fflush(file);
		backtrace_symbols_fd(array, size, fileno(file));
		fflush(file);
#endif
		fputs("**** END BACKTRACE ****\n", file);

		fclose(file);
		std::cout << "The crash report was also saved to " << filename << std::endl << std::endl;
	}

	::exit(sig);
}

BacktraceProvider::DefaultAbortGuard::~DefaultAbortGuard() {
#if defined PRINT_SEGFAULT_BACKTRACE || defined _WIN32
	signal(SIGABRT, segfault_handler);
#else
	signal(SIGABRT, SIG_DFL);
#endif  // PRINT_SEGFAULT_BACKTRACE || _WIN32
}

void BacktraceProvider::register_signal_handler() {
#if defined PRINT_SEGFAULT_BACKTRACE || defined _WIN32
	for (int s : {
#ifdef SIGBUS
	        SIGBUS,  // Not available on all systems
#endif
	        SIGABRT, SIGFPE, SIGILL, SIGSEGV}) {
		signal(s, segfault_handler);
	}
#endif  // PRINT_SEGFAULT_BACKTRACE || _WIN32
}

std::string BacktraceProvider::get_signal_description(int sig) {
	std::ostringstream s;

#ifdef PRINT_SEGFAULT_BACKTRACE
	s << sig << " (" << strsignal(sig) << ")";
#else
	s << sig;
#endif

	return s.str();
}
