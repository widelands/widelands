/*
 * Copyright (C) 2024-2026 by the Widelands Development Team
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

#ifdef PRINT_BACKTRACE_CPPTRACE
#include <cpptrace/cpptrace.hpp>
#elif defined(PRINT_BACKTRACE_EXECINFO)
#include <execinfo.h>
#include <unistd.h>
#endif

#include "base/multithreading.h"
#include "base/time_string.h"
#include "build_info.h"
#include "logic/filesystem_constants.h"

std::string BacktraceProvider::crash_dir = "./widelands_crash_report_";

#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
// Execinfo implementation taken from https://stackoverflow.com/a/77336
// Cpptrace implementation described in https://github.com/jeremy-rifkin/cpptrace
static void segfault_handler(const int sig) {
	const std::string signal_description = BacktraceProvider::get_signal_description(sig);

	std::cout << std::endl
	          << "##############################" << std::endl
	          << "FATAL ERROR: Received signal " << signal_description << std::endl
	          << "Backtrace:" << std::endl;

#ifdef PRINT_BACKTRACE_CPPTRACE
	std::ostringstream stacktrace;
	cpptrace::generate_trace().print(stacktrace, false);
	std::cout << stacktrace.str();
#elif defined(PRINT_BACKTRACE_EXECINFO)
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

	const std::string timestr = timestring();
	std::string filename = BacktraceProvider::get_crash_dir() + timestr;

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
#ifdef PRINT_BACKTRACE_CPPTRACE
		fputs(stacktrace.str().c_str(), file);
#elif defined(PRINT_BACKTRACE_EXECINFO)
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
#endif  // PRINT_BACKTRACE_CPPTRACE || PRINT_BACKTRACE_EXECINFO

BacktraceProvider::DefaultAbortGuard::~DefaultAbortGuard() {
#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
	signal(SIGABRT, segfault_handler);
#else
	signal(SIGABRT, SIG_DFL);
#endif
}

void BacktraceProvider::register_signal_handler() {
#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
	for (int s : {
#ifdef SIGBUS
	        SIGBUS,  // Not available on all systems
#endif
	        SIGABRT, SIGFPE, SIGILL, SIGSEGV}) {
		signal(s, segfault_handler);
	}
#endif  // PRINT_BACKTRACE_CPPTRACE || PRINT_BACKTRACE_EXECINFO
}

std::string BacktraceProvider::get_signal_description(int sig) {
	std::ostringstream s;

#ifdef _WIN32
	s << sig;
#else
	s << sig << " (" << strsignal(sig) << ")";
#endif

	return s.str();
}

void BacktraceProvider::set_crash_dir(const std::string& homedir) {
	crash_dir = homedir + "/" + kCrashDir + "/";
}

const std::string& BacktraceProvider::get_crash_dir() {
	return crash_dir;
}
