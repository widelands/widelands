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

#ifndef WL_BASE_LOG_H
#define WL_BASE_LOG_H

#include <cstdint>
#include <limits>

#include "base/macros.h"

#ifdef _WIN32
#include <string>
#endif

enum class LogType {
	kInfo,     // normal info messages
	kDebug,    // additional debug output
	kWarning,  // warnings
	kError     // fatal errors
};

constexpr uint32_t kNoTimestamp = std::numeric_limits<uint32_t>::max();

// Print a formatted log messages to stdout on most systems and 'stdout.txt' on windows.
// If `gametime` is not `kNoTimestamp`, a timestamp will be prepended to the output.
void log_to_stdout(LogType, uint32_t gametime, const char*, ...) PRINTF_FORMAT(3, 4);
#define log_info_time(time, ...) log_to_stdout(LogType::kInfo, time, __VA_ARGS__)
#define log_dbg_time(time, ...) log_to_stdout(LogType::kDebug, time, __VA_ARGS__)
#define log_warn_time(time, ...) log_to_stdout(LogType::kWarning, time, __VA_ARGS__)
#define log_err_time(time, ...) log_to_stdout(LogType::kError, time, __VA_ARGS__)

#define log_info(...) log_to_stdout(LogType::kInfo, kNoTimestamp, __VA_ARGS__)
#define log_dbg(...) log_to_stdout(LogType::kDebug, kNoTimestamp, __VA_ARGS__)
#define log_warn(...) log_to_stdout(LogType::kWarning, kNoTimestamp, __VA_ARGS__)
#define log_err(...) log_to_stdout(LogType::kError, kNoTimestamp, __VA_ARGS__)

extern bool g_verbose;

#ifdef _WIN32
/** Set the directory that stdout.txt shall be written to.
 *  This should be the same dir where widelands writes its config file. Returns true on success.
 */
bool set_logging_dir(const std::string& homedir);
// Set the directory that stdout.txt shall be written to to the directory the program is started
// from. Use this only for test cases.
void set_logging_dir();
#endif

#endif  // end of include guard: WL_BASE_LOG_H
