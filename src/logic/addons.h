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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/i18n.h"
#include "logic/filesystem_constants.h"

class Image;

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
};

using AddOnVersion = std::vector<uint32_t>;
std::string version_to_string(const AddOnVersion&, bool localize = true);
AddOnVersion string_to_version(std::string);
// Returns true if and only if version `compare` is newer than version `base`
bool is_newer_version(const AddOnVersion& base, const AddOnVersion& compare);

// Required add-ons for an add-on, map, or savegame with the recommended version
using AddOnRequirements = std::vector<std::pair<std::string, AddOnVersion>>;
using AddOnConflict = std::pair<std::string /* localized_message */, bool /* has_conflicts */>;

struct AddOnComment {
	std::string username, editor, message;
	AddOnVersion version;  // The version on which the user commented
	std::time_t timestamp, edit_timestamp;
};

constexpr uint8_t kMaxRating = 10;

struct AddOnInfo {
	AddOnInfo() = default;
	AddOnInfo& operator=(const AddOnInfo&) = default;
	AddOnInfo(const AddOnInfo&) = default;
	~AddOnInfo() = default;

	/*
	 * When adding any new add-on properties that are stored in the `addon` file,
	 * be sure to add them to MutableAddon as well so they are preserved/updated
	 * correctly when saving an add-on in the packager.
	 */

	std::string internal_name;  // "cool_feature.wad"

	std::string unlocalized_descname;
	std::string unlocalized_description;
	std::string unlocalized_author;
	std::function<std::string()> descname;
	std::function<std::string()> description;
	std::function<std::string()> author;

	AddOnVersion version;       ///< Add-on version (e.g. 1.2.3)
	uint32_t i18n_version = 0;  ///< (see doc/sphinx/source/add-ons.rst)

	AddOnCategory category = AddOnCategory::kNone;
	const Image* icon = nullptr;  ///< A little icon to display in the add-ons manager.

	std::vector<std::string> requirements;  // This add-on will only work correctly if these
	                                        // add-ons are present in this order and active

	bool sync_safe = false;      // Whether this add-on will not desync in MP and replays.
	std::string min_wl_version;  // Minimum required Widelands version, or "" if invalid.
	std::string max_wl_version;  // Maximum supported Widelands version, or "" if invalid.

	std::map<std::string /* name */, std::string /* description */> screenshots;

	bool verified = false;             ///< Only valid for Remote add-ons.
	uint32_t total_file_size = 0;      ///< Total size of all files, in bytes.
	std::string upload_username;       ///< Who uploaded (may be different from author).
	std::time_t upload_timestamp = 0;  ///< Date and time when this version was uploaded.

	uint32_t download_count = 0;       ///< Total times downloaded.
	uint32_t votes[kMaxRating] = {0};  ///< Total number of votes for each of the ratings 1-10.
	std::vector<AddOnComment> user_comments;

	bool matches_widelands_version() const;
	uint32_t number_of_votes() const;
	double average_rating() const;
	bool requires_texture_atlas_rebuild() const;
};

using AddOnsList = std::vector<std::shared_ptr<AddOns::AddOnInfo>>;

inline static std::string theme_addon_template_dir(const std::string& name) {
	std::string s = kAddOnDir;
	s += '/';
	s += name;
	s += '/';
	return s;
}

// Sorted list of all add-ons mapped to whether they are currently enabled
using AddOnState = std::pair<std::shared_ptr<AddOnInfo>, bool>;
extern std::vector<AddOnState> g_addons;

extern const std::unordered_map<std::string, std::string> kDifficultyIcons;
extern const std::map<AddOnCategory, AddOnCategoryInfo> kAddOnCategories;
AddOnCategory get_category(const std::string&);

// Creates a string informing about missing or wrong-version add-ons
// for use in map- and savegame selection screens
AddOnConflict check_requirements(const AddOnRequirements&);

unsigned count_all_dependencies(const std::string&, const std::map<std::string, AddOnState>&);

std::shared_ptr<AddOnInfo> preload_addon(const std::string&);

i18n::GenericTextdomain* create_textdomain_for_addon(std::string addon);
i18n::GenericTextdomain* create_textdomain_for_map(std::string mapfilename);

enum class UpdateThemeAction { kEnableArgument, kLoadFromConfig, kAutodetect };
void update_ui_theme(UpdateThemeAction, std::string = "");

// This guard allows you to modify `g_addons` in any way you like
// and ensures that it is reset to the initial state later.
struct AddOnsGuard {
	AddOnsGuard() : former_addons_(g_addons) {
	}
	~AddOnsGuard() {
		reset();
	}
	void reset() {
		g_addons = former_addons_;
	}

private:
	const std::vector<AddOnState> former_addons_;
};

}  // namespace AddOns

#endif  // end of include guard: WL_LOGIC_ADDONS_H
