/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_LOGIC_CONSTANTS_H
#define WL_LOGIC_CONSTANTS_H

#include <cstdint>

/// How often are statistics to be sampled.
constexpr uint32_t kStatisticsSampleTime = 30000;

/// Stop writing screenshots or syncstreams if we have less disk space left than this
constexpr unsigned long long kMinimumDiskSpace = 256 * 1024 * 1024;

/// Filesystem names and timeouts for ai
constexpr const char* const kAiDir = "ai";
constexpr const char* const kAiExtension = ".wai";
// We delete AI files older than one week
constexpr double kAIFilesKeepAroundTime = 7 * 24 * 60 * 60;

/// Filesystem names for maps
constexpr const char* const kMapsDir = "maps";
constexpr const char* const kWidelandsMapExtension = ".wmf";
constexpr const char* const kS2MapExtension1 = ".swd";
constexpr const char* const kS2MapExtension2 = ".wld";

/// Filesystem names and timeouts for replays
constexpr const char* const kReplayDir = "replays";
constexpr const char* const kReplayExtension = ".wrpl";
constexpr const char* const kSyncstreamExtension = ".wss";
// The time in seconds for how long old replays/syncstreams should be kept
// around, in seconds. Right now this is 4 weeks.
constexpr double kReplayKeepAroundTime = 4 * 7 * 24 * 60 * 60;

/// Filesystem names and intervals for savegames
constexpr const char* const kSaveDir = "save";
constexpr const char* const kCampVisFile = "save/campvis";
constexpr const char* const kSavegameExtension = ".wgf";
constexpr const char* const kAutosavePrefix = "wl_autosave";
// Default autosave interval in minutes
constexpr int kDefaultAutosaveInterval = 15;

/// Filesystem names for screenshots
constexpr const char* const kScreenshotsDir = "screenshots";

/// Filesystem names for config
constexpr const char* const kConfigFile = "config";

#endif  // end of include guard: WL_LOGIC_CONSTANTS_H
