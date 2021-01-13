/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#include "logic/addons.h"

#include <memory>
#include <set>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"

namespace AddOns {

const std::map<AddOnCategory, AddOnCategoryInfo> kAddOnCategories = {
   {AddOnCategory::kNone,
    AddOnCategoryInfo{"", []() { return _("Error"); }, "images/ui_basic/stop.png", false}},
   {AddOnCategory::kTribes,
    AddOnCategoryInfo{"tribes", []() { return _("Tribes"); },
                      "images/wui/stats/menu_tab_wares_warehouse.png", true}},
   {AddOnCategory::kWorld, AddOnCategoryInfo{"world", []() { return _("World"); },
                                             "images/wui/menus/toggle_immovables.png", true}},
   {AddOnCategory::kScript, AddOnCategoryInfo{"script", []() { return _("Script"); },
                                              "images/logos/WL-Editor-32.png", true}},
   {AddOnCategory::kMaps, AddOnCategoryInfo{"maps", []() { return _("Map Set"); },
                                            "images/wui/menus/toggle_minimap.png", false}},
   {AddOnCategory::kCampaign, AddOnCategoryInfo{"campaign", []() { return _("Campaign"); },
                                                "images/wui/messages/messages_warfare.png", false}},
   {AddOnCategory::kWinCondition,
    AddOnCategoryInfo{"win_condition", []() { return _("Win Condition"); },
                      "images/wui/menus/objectives.png", false}},
   {AddOnCategory::kStartingCondition,
    AddOnCategoryInfo{"starting_condition", []() { return _("Starting Condition"); },
                      "tribes/buildings/warehouses/atlanteans/headquarters/menu.png", false}},
   {AddOnCategory::kTheme, AddOnCategoryInfo{"theme", []() { return _("Theme"); },
                                             "images/wui/menus/main_menu.png", false}}};

std::vector<std::pair<AddOnInfo, bool>> g_addons;

std::string version_to_string(const AddOnVersion& version, const bool localize) {
	std::string s;
	for (uint32_t v : version) {
		if (s.empty()) {
			s = std::to_string(v);
		} else if (localize) {
			s = (boost::format(_("%1$s.%2$u")) % s % v).str();
		} else {
			s += '.';
			s += std::to_string(v);
		}
	}
	return s;
}
AddOnVersion string_to_version(std::string input) {
	AddOnVersion result;
	for (;;) {
		const size_t pos = input.find('.');
		if (pos == std::string::npos) {
			result.push_back(std::stol(input));
			return result;
		}
		result.push_back(std::stol(input.substr(0, pos)));
		input = input.substr(pos + 1);
	}
	NEVER_HERE();
}

bool is_newer_version(const AddOnVersion& a, const AddOnVersion& b) {
	const size_t s_a = a.size();
	const size_t s_b = b.size();
	for (size_t i = 0; i < s_a && i < s_b; ++i) {
		if (a[i] != b[i]) {
			return a[i] < b[i];
		}
	}
	return s_a < s_b;
}

static std::string check_requirements_conflicts(const AddOnRequirements& required_addons) {
	std::set<std::string> addons_missing;
	std::map<std::string, std::pair<AddOnVersion, AddOnVersion>> addons_wrong_version;
	for (const auto& requirement : required_addons) {
		bool found = false;
		for (const auto& pair : g_addons) {
			if (pair.first.internal_name == requirement.first) {
				found = true;
				if (pair.first.version != requirement.second) {
					addons_wrong_version[requirement.first] =
					   std::make_pair(pair.first.version, requirement.second);
				}
				break;
			}
		}
		if (!found) {
			addons_missing.insert(requirement.first);
		}
	}

	if (addons_missing.empty()) {
		if (addons_wrong_version.empty()) {
			return _("No conflicts");
		} else {
			std::string list;
			for (const auto& a : addons_wrong_version) {
				if (list.empty()) {
					list = (boost::format(_("%1$s (expected version %2$s, found %3$s)")) % a.first %
					        version_to_string(a.second.second) % version_to_string(a.second.first))
					          .str();
				} else {
					list =
					   (boost::format(_("%1$s, %2$s (expected version %3$s, found %4$s)")) % list %
					    a.first % version_to_string(a.second.second) % version_to_string(a.second.first))
					      .str();
				}
			}
			return (boost::format(ngettext("%1$u add-on with wrong version: %2$s",
			                               "%1$u add-ons with wrong version: %2$s",
			                               addons_wrong_version.size())) %
			        addons_wrong_version.size() % list)
			   .str();
		}
	} else {
		if (addons_wrong_version.empty()) {
			std::string list;
			for (const std::string& a : addons_missing) {
				if (list.empty()) {
					list = a;
				} else {
					list = (boost::format(_("%1$s, %2$s")) % list % a).str();
				}
			}
			return (boost::format(ngettext("%1$u missing add-on: %2$s", "%1$u missing add-ons: %2$s",
			                               addons_missing.size())) %
			        addons_missing.size() % list)
			   .str();
		} else {
			std::string list;
			for (const std::string& a : addons_missing) {
				if (list.empty()) {
					list = (boost::format(_("%s (missing)")) % a).str();
				} else {
					list = (boost::format(_("%1$s, %2$s (missing)")) % list % a).str();
				}
			}
			for (const auto& a : addons_wrong_version) {
				list =
				   (boost::format(_("%1$s, %2$s (expected version %3$s, found %4$s)")) % list %
				    a.first % version_to_string(a.second.second) % version_to_string(a.second.first))
				      .str();
			}
			return (boost::format(_("%1$s and %2$s: %3$s")) %
			        (boost::format(ngettext(
			            _("%u missing add-on"), _("%u missing add-ons"), addons_missing.size())) %
			         addons_missing.size())
			           .str() %
			        (boost::format(ngettext(_("%u add-on with wrong version"),
			                                _("%u add-ons with wrong version"),
			                                addons_missing.size())) %
			         addons_missing.size())
			           .str() %
			        list)
			   .str();
		}
	}
}

std::string check_requirements(const AddOnRequirements& required_addons) {
	const size_t nr_req = required_addons.size();
	if (nr_req == 0) {
		/** TRANSLATORS: This map or savegame uses no add-ons */
		return _("None");
	}
	std::string result = check_requirements_conflicts(required_addons);
	for (const auto& pair : required_addons) {
		result = (boost::format(_("%1$s<br>· %2$s (version %3$s)")) % result % pair.first %
		          version_to_string(pair.second))
		            .str();
	}
	return result;
}

unsigned count_all_dependencies(const std::string& info,
                                const std::map<std::string, AddOnState>& all) {
	const auto it = all.find(info);
	if (it == all.end()) {
		log_warn("count_all_dependencies: '%s' not installed", info.c_str());
		return 0;
	}
	unsigned deps = 0;
	for (const std::string& req : it->second.first.requirements) {
		deps += count_all_dependencies(req, all);
	}
	return deps;
}

AddOnCategory get_category(const std::string& name) {
	for (const auto& pair : kAddOnCategories) {
		if (pair.second.internal_name == name) {
			return pair.first;
		}
	}
	throw wexception("Invalid add-on category '%s'", name.c_str());
}

uint32_t AddOnInfo::number_of_votes() const {
	uint32_t total = 0;
	for (uint32_t v : votes) {
		total += v;
	}
	return total;
}
double AddOnInfo::average_rating() const {
	double total = 0, sum = 0;
	for (uint8_t i = 1; i <= kMaxRating; ++i) {
		total += votes[i - 1];
		sum += votes[i - 1] * i;
	}
	return (total > 0) ? (sum / total) : 0;
}

AddOnInfo preload_addon(const std::string& name) {
	std::unique_ptr<FileSystem> fs(
	   g_fs->make_sub_file_system(kAddOnDir + FileSystem::file_separator() + name));
	Profile profile;
	profile.read("addon", nullptr, *fs);
	Section& s = profile.get_safe_section("global");

	// Fetch strings from the correct textdomain
	i18n::AddOnTextdomain addon_textdomain(name);
	Profile i18n_profile(kAddOnLocaleVersions.c_str());
	Section* i18n_section = i18n_profile.get_section("global");

	const std::string unlocalized_descname = s.get_safe_untranslated_string("name");
	const std::string unlocalized_description = s.get_safe_untranslated_string("description");
	const std::string unlocalized_author = s.get_safe_untranslated_string("author");

	AddOnInfo i = {name,
	               unlocalized_descname,
	               unlocalized_description,
	               unlocalized_author,
	               [name, unlocalized_descname]() {
		               i18n::AddOnTextdomain td(name);
		               return i18n::translate(unlocalized_descname);
	               },
	               [name, unlocalized_description]() {
		               i18n::AddOnTextdomain td(name);
		               return i18n::translate(unlocalized_description);
	               },
	               [name, unlocalized_author]() {
		               i18n::AddOnTextdomain td(name);
		               return i18n::translate(unlocalized_author);
	               },
	               string_to_version(s.get_safe_string("version")),
	               i18n_section ? i18n_section->get_natural(name.c_str(), 0) : 0,
	               get_category(s.get_safe_string("category")),
	               {},
	               false,
	               {{}, {}, {}, {}},
	               {},
	               0,
	               "",
	               0,
	               0,
	               {},
	               {}};

	if (i.category == AddOnCategory::kNone) {
		throw wexception("preload_addon (%s): category is None", name.c_str());
	}
	if (i.version.empty()) {
		throw wexception("preload_addon (%s): version string is empty", name.c_str());
	}

	for (std::string req(s.get_safe_string("requires")); !req.empty();) {
		const size_t commapos = req.find(',');
		if (commapos == std::string::npos) {
			i.requirements.push_back(req);
			break;
		} else {
			i.requirements.push_back(req.substr(0, commapos));
			req = req.substr(commapos + 1);
		}
	}

	return i;
}

}  // namespace AddOns
