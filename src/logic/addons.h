/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_LOGIC_ADDONS_H
#define WL_LOGIC_ADDONS_H

#include <ctime>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace AddOns {

enum class AddOnCategory {
	kNone,
	kWorld,
	kTribes,
	kScript,
	kMaps,
	kCampaign,
	kWinCondition,
	kStartingCondition,
	kTheme
};

// Note: Below you will see some lines like `std::function<std::string()> descname`.
// The reason why we implement translatable texts as functions here is that the encapsulating
// objects will stick around a long time and we don't want to have to re-create them when we
// change languages. The values implemented as one-liner functions are used only rarely so
// that this does not matter performance-wise.

struct AddOnCategoryInfo {
	std::string internal_name;
	std::function<std::string()> descname;
	std::string icon;
	bool can_disable_addons;
};

// TODO(Nordfriese): Ugly hack required for the dummy server. Can go when we have a real server.
struct AddOnFileList {
	std::vector<std::string> directories, files, locales, checksums;
};

using AddOnVersion = std::vector<uint32_t>;
std::string version_to_string(const AddOnVersion&, bool localize = true);
AddOnVersion string_to_version(std::string);
// Returns true if and only if version `compare` is newer than version `base`
bool is_newer_version(const AddOnVersion& base, const AddOnVersion& compare);

// Required add-ons for an add-on, map, or savegame with the recommended version
using AddOnRequirements = std::vector<std::pair<std::string, AddOnVersion>>;

struct AddOnComment {
	std::string username, message;
	AddOnVersion version;  // The version on which the user commented
	std::time_t timestamp;
};

constexpr uint8_t kMaxRating = 10;

struct AddOnInfo {
	std::string internal_name;  // "cool_feature.wad"

	std::string unlocalized_descname;
	std::string unlocalized_description;
	std::string unlocalized_author;
	std::function<std::string()> descname;
	std::function<std::string()> description;
	std::function<std::string()> author;

	AddOnVersion version;   // Add-on version (e.g. 1.2.3)
	uint32_t i18n_version;  // (see doc/sphinx/source/add-ons.rst)

	AddOnCategory category;

	std::vector<std::string> requirements;  // This add-on will only work correctly if these
	                                        // add-ons are present in this order and active

	bool verified;  // Only valid for Remote add-ons

	AddOnFileList file_list;  // Get rid of this ASAP
	std::map<std::string /* name */, std::string /* description */> screenshots;

	uint32_t total_file_size;     // total size of all files, in bytes
	std::string upload_username;  // who uploaded (may be different from author)

	// TODO(Nordfriese): These are not yet implemented on the server-side
	std::time_t upload_timestamp;  // date and time when this version was uploaded
	uint32_t download_count;       // total times downloaded
	uint32_t votes[kMaxRating];    // total number of votes for each of the ratings 1-10
	std::vector<AddOnComment> user_comments;

	uint32_t number_of_votes() const;
	double average_rating() const;
};

// Sorted list of all add-ons mapped to whether they are currently enabled
using AddOnState = std::pair<AddOnInfo, bool>;
extern std::vector<AddOnState> g_addons;

extern const std::map<AddOnCategory, AddOnCategoryInfo> kAddOnCategories;
AddOnCategory get_category(const std::string&);

// Creates a string informing about missing or wrong-version add-ons
// for use in map- and savegame selection screens
std::string check_requirements(const AddOnRequirements&);

unsigned count_all_dependencies(const std::string&, const std::map<std::string, AddOnState>&);

AddOnInfo preload_addon(const std::string&);

}  // namespace AddOns

#endif  // end of include guard: WL_LOGIC_ADDONS_H
