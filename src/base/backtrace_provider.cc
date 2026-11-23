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

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef PRINT_BACKTRACE_CPPTRACE
#include <cpptrace/cpptrace.hpp>
#elif defined(PRINT_BACKTRACE_EXECINFO)
#include <execinfo.h>
#include <unistd.h>
#endif

#include "base/macros.h"
#include "base/multithreading.h"
#include "base/time_string.h"
#include "build_info.h"
#include "logic/filesystem_constants.h"

bool BacktraceProvider::error_handling = true;
std::string BacktraceProvider::crash_dir = "./widelands_crash_report_";

#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
// Execinfo implementation taken from https://stackoverflow.com/a/77336
// Cpptrace implementation described in https://github.com/jeremy-rifkin/cpptrace
static void segfault_handler(const int sig) {
#ifdef _WIN32
	// Windows has no SA_RESETHAND, restore default handling manually
	signal(sig, SIG_DFL);
#endif
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

	// Do not run atexit handlers or static destructors on a possibly corrupted state.
	// Re-raise with default action to keep core dumps, OS crash reporters and exit status.
	signal(sig, SIG_DFL);
	raise(sig);
	std::_Exit(128 + sig);
}

// Install the crash handler as one-shot handler: a crash inside the handler itself
// must not re-enter it (endless recursion or deadlock).
static void install_crash_handler(const int sig) {
#ifdef _WIN32
	signal(sig, segfault_handler);
#else
	struct sigaction action = {};
	// glibc defines sa_handler as a macro that expands to an expression containing its own name
	CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
	action.sa_handler = segfault_handler;
	CLANG_DIAG_ON("-Wdisabled-macro-expansion")
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESETHAND | SA_NODEFER;
	sigaction(sig, &action, nullptr);
#endif
}

#ifdef _WIN32
// The CRT keeps SIGSEGV, SIGILL and SIGFPE handlers per thread, so signal() in main()
// does not catch crashes in other threads. The unhandled exception filter is process-wide.
static LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS* info) {
	int sig = SIGSEGV;
	switch (info->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_PRIV_INSTRUCTION:
		sig = SIGILL;
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		sig = SIGFPE;
		break;
	default:
		break;
	}
	segfault_handler(sig);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif  // _WIN32
#endif  // PRINT_BACKTRACE_CPPTRACE || PRINT_BACKTRACE_EXECINFO

BacktraceProvider::AbortHandlerGuard::AbortHandlerGuard(void (*handler)(int)) {
	signal(SIGABRT, handler);
}

BacktraceProvider::AbortHandlerGuard::~AbortHandlerGuard() {
#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
	if (error_handling) {
		install_crash_handler(SIGABRT);
	} else {
		signal(SIGABRT, SIG_DFL);
	}
#else
	signal(SIGABRT, SIG_DFL);
#endif
}

bool BacktraceProvider::parse_error_handling(int argc, char* argv[]) {
	// This is simplified argument parsing, which we need before
	// WLApplication::parse_commandline().
	const std::string arg("--no-error-handling");
	for (int i = 1; i < argc; ++i) {
		if (arg == argv[i]) {
			error_handling = false;
			break;
		}
	}
	return error_handling;
}

void BacktraceProvider::register_signal_handler() {
#if defined(PRINT_BACKTRACE_CPPTRACE) || defined(PRINT_BACKTRACE_EXECINFO)
	if (!error_handling) {
		return;
	}
	for (int s : {
#ifdef SIGBUS
	        SIGBUS,  // Not available on all systems
#endif
	        SIGABRT, SIGFPE, SIGILL, SIGSEGV}) {
		install_crash_handler(s);
	}
#ifdef _WIN32
	SetUnhandledExceptionFilter(unhandled_exception_filter);
#endif
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
