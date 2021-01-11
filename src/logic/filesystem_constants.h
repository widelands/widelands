/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_FILESYSTEM_CONSTANTS_H
#define WL_LOGIC_FILESYSTEM_CONSTANTS_H

#include <string>

/// Stop writing screenshots or syncstreams if we have less disk space left than this
constexpr unsigned long long kMinimumDiskSpace = 256 * 1024 * 1024;  // NOLINT

/// Filesystem names and timeouts for ai
const std::string kAiDir = "ai";
const std::string kAiExtension = ".wai";
// We delete AI files older than one week
constexpr double kAIFilesKeepAroundTime = 7 * 24 * 60 * 60;

/// Filesystem names for maps
const std::string kMapsDir = "maps";
const std::string kCampaignsDir = "campaigns";
const std::string kWidelandsMapExtension = ".wmf";
const std::string kS2MapExtension1 = ".swd";
const std::string kS2MapExtension2 = ".wld";

/// Filesystem names for temp files holding static data that needs to be accessible via filesystem
/// Kept in a separate dir to avoid filesystem conflicts
const std::string kTempFileDir = "temp";
const std::string kTempFileExtension = ".tmp";
// We delete (accidentally remaining) temp files older than a week
constexpr double kTempFilesKeepAroundTime = 7 * 24 * 60 * 60;

/// Filesystem names for for temporary backup when overwriting files during saving
const std::string kTempBackupExtension = ".tmp";
// We delete (accidentally remaining) temp backup files older than a day
constexpr double kTempBackupsKeepAroundTime = 24 * 60 * 60;

/// Filesystem names and timeouts for replays
const std::string kReplayDir = "replays";
const std::string kReplayExtension = ".wrpl";
const std::string kSyncstreamExtension = ".wss";
const std::string kSyncstreamExcerptExtension = ".wse";
// The time in seconds for how long old replays/syncstreams should be kept
// around, in seconds. Right now this is 4 weeks.
constexpr double kReplayKeepAroundTime = 4 * 7 * 24 * 60 * 60;

/// Filesystem names and intervals for savegames
const std::string kSaveDir = "save";
const std::string kCampVisFile = "save/campaigns.conf";
constexpr const char* const kTrainingWheelsFile = "save/training_wheels.conf";
const std::string kSavegameExtension = ".wgf";
const std::string kAutosavePrefix = "wl_autosave";
// Default autosave interval in minutes
constexpr int kDefaultAutosaveInterval = 15;

// Filesystem names for campaign data
const std::string kCampaignDataDir = "campaigns";
const std::string kCampaignDataExtension = ".wcd";

/// Filesystem names for screenshots
const std::string kScreenshotsDir = "screenshots";

/// Filesystem names for config
const std::string kConfigFile = "config";

const std::string kEconomyProfilesDir = "tribes/economy_profiles";

#endif  // end of include guard: WL_LOGIC_FILESYSTEM_CONSTANTS_H
