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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_BASE_LOG_H
#define WL_BASE_LOG_H

#include <cstdint>
#include <limits>
#include <string>

#include "base/macros.h"
#include "base/times.h"

enum class LogType {
	kInfo,     // normal info messages
	kDebug,    // additional debug output
	kWarning,  // warnings
	kError     // fatal errors
};

extern bool g_verbose;

// Print a formatted log messages to stdout on most systems and 'stdout.txt' on windows.
// If `gametime` is not invalid, a timestamp for the gametime will be prepended to the
// output; otherwise, the real time will be used for the timestamp.
void do_log(LogType, const Time& gametime, const char*, ...) PRINTF_FORMAT(3, 4);

#define log_info_time(time, ...) do_log(LogType::kInfo, time, __VA_ARGS__)
#define log_dbg_time(time, ...) do_log(LogType::kDebug, time, __VA_ARGS__)
#define log_warn_time(time, ...) do_log(LogType::kWarning, time, __VA_ARGS__)
#define log_err_time(time, ...) do_log(LogType::kError, time, __VA_ARGS__)

#define log_info(...) do_log(LogType::kInfo, Time(), __VA_ARGS__)
#define log_dbg(...) do_log(LogType::kDebug, Time(), __VA_ARGS__)
#define log_warn(...) do_log(LogType::kWarning, Time(), __VA_ARGS__)
#define log_err(...) do_log(LogType::kError, Time(), __VA_ARGS__)

#define verb_log_info_time(time, ...)                                                              \
	if (g_verbose)                                                                                  \
	do_log(LogType::kInfo, time, __VA_ARGS__)
#define verb_log_dbg_time(time, ...)                                                               \
	if (g_verbose)                                                                                  \
	do_log(LogType::kDebug, time, __VA_ARGS__)
#define verb_log_warn_time(time, ...)                                                              \
	if (g_verbose)                                                                                  \
	do_log(LogType::kWarning, time, __VA_ARGS__)
#define verb_log_err_time(time, ...)                                                               \
	if (g_verbose)                                                                                  \
	do_log(LogType::kError, time, __VA_ARGS__)

#define verb_log_info(...)                                                                         \
	if (g_verbose)                                                                                  \
	do_log(LogType::kInfo, Time(), __VA_ARGS__)
#define verb_log_dbg(...)                                                                          \
	if (g_verbose)                                                                                  \
	do_log(LogType::kDebug, Time(), __VA_ARGS__)
#define verb_log_warn(...)                                                                         \
	if (g_verbose)                                                                                  \
	do_log(LogType::kWarning, Time(), __VA_ARGS__)
#define verb_log_err(...)                                                                          \
	if (g_verbose)                                                                                  \
	do_log(LogType::kError, Time(), __VA_ARGS__)

#ifdef _WIN32
/** Set the directory that stdout.txt shall be written to.
 *  This should be the same dir where widelands writes its config file. Returns true on success.
 */
bool set_logging_dir(const std::string& homedir);
// Set the directory that stdout.txt shall be written to to the directory the program is started
// from. Use this only for test cases.
void set_testcase_logging_dir();

#else
inline void set_testcase_logging_dir() {
}
#endif
#endif  // end of include guard: WL_BASE_LOG_H
