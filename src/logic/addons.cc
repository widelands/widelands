/*
 * Copyright (C) 2020-2023 by the Widelands Development Team
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

#include "logic/addons.h"

#include <list>
#include <memory>
#include <set>

#include "base/log.h"
#include "base/math.h"
#include "base/wexception.h"
#include "build_info.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "wlapplication_options.h"

namespace AddOns {

// Taken from data/campaigns/campaigns.lua
const std::unordered_map<std::string, std::string> kDifficultyIcons = {
   {"Challenging.", "images/ui_fsmenu/challenging.png"},
   {"Hard.", "images/ui_fsmenu/hard.png"},
   {"Medium.", "images/ui_fsmenu/medium.png"},
   {"Easy.", "images/ui_fsmenu/easy.png"}};

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
                                            "images/wui/menus/toggle_minimap.png", true}},
   {AddOnCategory::kMapGenerator,
    AddOnCategoryInfo{"map_generator", []() { return _("Map Generator"); },
                      "images/wui/editor/menus/new_random_map.png", false}},
   {AddOnCategory::kCampaign, AddOnCategoryInfo{"campaign", []() { return _("Campaign"); },
                                                "images/wui/messages/messages_warfare.png", false}},
   {AddOnCategory::kWinCondition,
    AddOnCategoryInfo{"win_condition", []() { return _("Win Condition"); },
                      "images/wui/menus/objectives.png", true}},
   {AddOnCategory::kStartingCondition,
    AddOnCategoryInfo{"starting_condition", []() { return _("Starting Condition"); },
                      "tribes/buildings/warehouses/atlanteans/headquarters/menu.png", true}},
   {AddOnCategory::kTheme, AddOnCategoryInfo{"theme", []() { return _("Theme"); },
                                             "images/wui/menus/main_menu.png", false}}};

std::vector<AddOnState> g_addons;

const AddOnInfo* find_addon(const std::string& name) {
	for (const auto& pair : g_addons) {
		if (pair.first->internal_name == name) {
			return pair.first.get();
		}
	}
	return nullptr;
}

i18n::GenericTextdomain* create_textdomain_for_addon(std::string addon, const std::string& dflt) {
	if (const AddOnInfo* a = find_addon(addon)) {
		return new i18n::AddOnTextdomain(addon, a->i18n_version);
	}
	return dflt.empty() ? nullptr : new i18n::Textdomain(dflt);
}

i18n::GenericTextdomain* create_textdomain_for_map(std::string mapfilename) {
	if (mapfilename.compare(0, kAddOnDir.size(), kAddOnDir) != 0) {
		return new i18n::Textdomain("maps");
	}

	mapfilename = mapfilename.substr(kAddOnDir.size() + 1);
	const size_t pos = std::min(mapfilename.find('/'), mapfilename.find('\\'));
	if (pos != std::string::npos) {
		mapfilename = mapfilename.substr(0, pos);
	}

	return create_textdomain_for_addon(mapfilename);
}

std::string version_to_string(const AddOnVersion& version, const bool localize) {
	std::string s;
	for (uint32_t v : version) {
		if (s.empty()) {
			s = std::to_string(v);
		} else if (localize) {
			s = format(_("%1$s.%2$u"), s, v);
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
			result.push_back(math::to_long(input));
			return result;
		}
		result.push_back(math::to_long(input.substr(0, pos)));
		input = input.substr(pos + 1);
	}
	NEVER_HERE();
}

bool is_newer_version(const AddOnVersion& base, const AddOnVersion& compare) {
	const size_t s_a = base.size();
	const size_t s_b = compare.size();
	for (size_t i = 0; i < s_a && i < s_b; ++i) {
		if (base[i] != compare[i]) {
			return base[i] < compare[i];
		}
	}
	return s_a < s_b;
}

bool order_matters(AddOnCategory base, AddOnCategory dependency) {
	switch (base) {
	case AddOnCategory::kScript:
		return dependency == AddOnCategory::kScript;
	case AddOnCategory::kWorld:
	case AddOnCategory::kTribes:
		return dependency == AddOnCategory::kWorld || dependency == AddOnCategory::kTribes;
	default:
		return false;
	}
}

static AddOnConflict check_requirements_conflicts(const AddOnRequirements& required_addons) {
	std::set<std::string> addons_missing;
	std::map<std::string, std::pair<AddOnVersion, AddOnVersion>> addons_wrong_version;
	for (const auto& requirement : required_addons) {
		bool found = false;
		for (const auto& pair : g_addons) {
			if (pair.first->internal_name == requirement.first) {
				found = true;
				if (pair.first->version != requirement.second) {
					addons_wrong_version[requirement.first] =
					   std::make_pair(pair.first->version, requirement.second);
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
			return std::make_pair(_("No conflicts"), false);
		}
		std::string list;
		for (const auto& a : addons_wrong_version) {
			if (list.empty()) {
				list = format(_("%1$s (expected version %2$s, found %3$s)"), a.first,
				              version_to_string(a.second.second), version_to_string(a.second.first));
			} else {
				list = format(_("%1$s, %2$s (expected version %3$s, found %4$s)"), list, a.first,
				              version_to_string(a.second.second), version_to_string(a.second.first));
			}
		}

		// Wrong versions might work, so do not forbid loading
		return std::make_pair(
		   format(ngettext("%1$u add-on with wrong version: %2$s",
		                   "%1$u add-ons with wrong version: %2$s", addons_wrong_version.size()),
		          addons_wrong_version.size(), list),
		   false);
	}

	if (addons_wrong_version.empty()) {
		std::string list;
		for (const std::string& a : addons_missing) {
			if (list.empty()) {
				list = a;
			} else {
				list = format(_("%1$s, %2$s"), list, a);
			}
		}
		return std::make_pair(format(ngettext("%1$u missing add-on: %2$s",
		                                      "%1$u missing add-ons: %2$s", addons_missing.size()),
		                             addons_missing.size(), list),
		                      true);
	}
	std::string list;
	for (const std::string& a : addons_missing) {
		if (list.empty()) {
			list = format(_("%s (missing)"), a);
		} else {
			list = format(_("%1$s, %2$s (missing)"), list, a);
		}
	}
	for (const auto& a : addons_wrong_version) {
		list = format(_("%1$s, %2$s (expected version %3$s, found %4$s)"), list, a.first,
		              version_to_string(a.second.second), version_to_string(a.second.first));
	}
	return std::make_pair(
	   format(_("%1$s and %2$s: %3$s"),
	          format(ngettext("%u missing add-on", "%u missing add-ons", addons_missing.size()),
	                 addons_missing.size()),
	          format(ngettext("%u add-on with wrong version", "%u add-ons with wrong version",
	                          addons_missing.size()),
	                 addons_missing.size()),
	          list),
	   true);
}

AddOnConflict check_requirements(const AddOnRequirements& required_addons) {
	const size_t nr_req = required_addons.size();
	if (nr_req == 0) {
		/** TRANSLATORS: This map or savegame uses no add-ons */
		return std::make_pair(_("None"), false);
	}
	AddOnConflict result = check_requirements_conflicts(required_addons);
	for (const auto& pair : required_addons) {
		result.first = format(_("%1$s<br>· %2$s (version %3$s)"), result.first, pair.first,
		                      version_to_string(pair.second));
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
	for (const std::string& req : it->second.first->requirements) {
		deps += count_all_dependencies(req, all);
	}
	return deps;
}

std::string list_game_relevant_addons() {
	std::vector<std::string> addons;
	for (const auto& pair : AddOns::g_addons) {
		if (pair.second && AddOns::kAddOnCategories.at(pair.first->category).network_relevant) {
			addons.push_back(pair.first->descname());
		}
	}

	if (addons.empty()) {
		return as_richtext(g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		                      .as_font_tag(_("No game-relevant add-ons in use.")));
	}

	std::string addons_text =
	   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	      .as_font_tag(format(ngettext("%u game-relevant add-on in use:",
	                                   "%u game-relevant add-ons in use:", addons.size()),
	                          addons.size()));
	for (const std::string& a : addons) {
		addons_text += as_listitem(a, UI::FontStyle::kFsMenuInfoPanelParagraph);
	}
	return as_richtext(addons_text);
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
	double total = 0;
	double sum = 0;
	for (uint8_t i = 1; i <= kMaxRating; ++i) {
		total += votes[i - 1];
		sum += votes[i - 1] * i;
	}
	return (total > 0) ? (sum / total) : 0;
}

static bool contains_png(const std::string& dir) {
	const auto dirs = g_fs->list_directory(dir);
	return std::any_of(dirs.begin(), dirs.end(), [](const std::string& f) {
		if (g_fs->is_directory(f)) {
			return contains_png(f);
		}
		return FileSystem::filename_ext(f) == ".png";
	});
}
// Rebuilding the texture atlas is required if an add-on defines new terrain, flag, or road
// textures.
bool AddOnInfo::requires_texture_atlas_rebuild() const {
	std::string dir_to_check = kAddOnDir;
	dir_to_check += FileSystem::file_separator();
	dir_to_check += internal_name;
	switch (category) {
	case AddOnCategory::kTribes: {
		// We do not support replacing road or flag images of existing tribes,
		// so we only need to check in case there's a new tribe.
		return g_fs->is_directory(dir_to_check + FileSystem::file_separator() + "tribes") &&
		       contains_png(dir_to_check);
	}
	case AddOnCategory::kWorld:
		return contains_png(dir_to_check);
	default:
		return false;
	}
}

void update_ui_theme(const UpdateThemeAction action, std::string arg) {
	AddOnState* previously_enabled = nullptr;
	std::list<AddOnState*> installed;
	for (AddOnState& s : g_addons) {
		if (s.first->category == AddOnCategory::kTheme) {
			if (s.second) {
				previously_enabled = &s;
			}
			s.second = false;
			installed.push_back(&s);
		}
	}

	switch (action) {
	case UpdateThemeAction::kEnableArgument:
		for (AddOnState* s : installed) {
			if (s->first->internal_name == arg) {
				s->second = true;
				set_template_dir(theme_addon_template_dir(arg));
				set_config_string("theme", arg);
				return;
			}
		}
		NEVER_HERE();

	case UpdateThemeAction::kLoadFromConfig:
		arg = get_config_string("theme", "");
		if (arg.empty()) {
			return set_template_dir("");
		}
		for (AddOnState* s : installed) {
			if (s->first->internal_name == arg) {
				s->second = true;
				set_template_dir(theme_addon_template_dir(arg));
				return;
			}
		}
		log_warn("Theme '%s' not found", arg.c_str());
		FALLS_THROUGH;
	case UpdateThemeAction::kAutodetect:
		if (previously_enabled == nullptr) {
			set_config_string("theme", "");
			set_template_dir("");
			return;
		}
		previously_enabled->second = true;
		set_config_string("theme", previously_enabled->first->internal_name);
		set_template_dir(theme_addon_template_dir(previously_enabled->first->internal_name));
		return;
	}
	NEVER_HERE();
}

bool AddOnInfo::matches_widelands_version() const {
	if (min_wl_version.empty() && max_wl_version.empty()) {
		return true;
	}

	const std::string& wl_version = build_id();
	// Two cases. Either we have a release version such as "1.0".
	// Or we have a development version such as "1.0~git25169[9d77594@master]" –
	// which is then considered older than the version string before the '~'
	// but newer than any version less than that.
	const size_t tilde = wl_version.find('~');
	if (tilde == std::string::npos) {
		AddOnVersion wl = string_to_version(wl_version);
		if (!min_wl_version.empty() && wl < string_to_version(min_wl_version)) {
			return false;
		}
		if (!max_wl_version.empty() && wl > string_to_version(max_wl_version)) {
			return false;
		}
	} else {
		AddOnVersion next_wl = string_to_version(wl_version.substr(0, tilde));
		if (!min_wl_version.empty() && next_wl <= string_to_version(min_wl_version)) {
			return false;
		}
		if (!max_wl_version.empty() && next_wl > string_to_version(max_wl_version)) {
			return false;
		}
	}
	return true;
}

std::shared_ptr<AddOnInfo> preload_addon(const std::string& name) {
	std::unique_ptr<FileSystem> fs(
	   g_fs->make_sub_file_system(kAddOnDir + FileSystem::file_separator() + name));
	Profile profile;
	profile.read("addon", nullptr, *fs);
	Section& s = profile.get_safe_section("global");

	Profile i18n_profile(kAddOnLocaleVersions.c_str());
	Section* i18n_section = i18n_profile.get_section("global");

	AddOnInfo* i = new AddOnInfo();
	std::shared_ptr<AddOnInfo> pointer(i);
	i->internal_name = name;
	i->unlocalized_descname = s.get_safe_untranslated_string("name");
	i->unlocalized_description = s.get_safe_untranslated_string("description");
	i->unlocalized_author = s.get_safe_untranslated_string("author");
	i->version = string_to_version(s.get_safe_string("version"));
	i->i18n_version = i18n_section != nullptr ? i18n_section->get_natural(name.c_str(), 0) : 0;
	i->category = get_category(s.get_safe_string("category"));
	i->sync_safe = s.get_bool("sync_safe", false);
	i->min_wl_version = s.get_string("min_wl_version", "");
	i->max_wl_version = s.get_string("max_wl_version", "");
	i->descname = [i]() {
		i18n::AddOnTextdomain td(i->internal_name, i->i18n_version);
		return i18n::translate(i->unlocalized_descname);
	};
	i->description = [i]() {
		i18n::AddOnTextdomain td(i->internal_name, i->i18n_version);
		return i18n::translate(i->unlocalized_description);
	};
	i->author = [i]() {
		i18n::AddOnTextdomain td(i->internal_name, i->i18n_version);
		return i18n::translate(i->unlocalized_author);
	};
	i->icon = g_image_cache->get(fs->file_exists(kAddOnIconFile) ?
                                   kAddOnDir + FileSystem::file_separator() + name +
	                                   FileSystem::file_separator() + kAddOnIconFile :
                                   kAddOnCategories.at(i->category).icon);

	if (i->category == AddOnCategory::kNone) {
		throw wexception("preload_addon (%s): category is None", name.c_str());
	}
	if (i->version.empty()) {
		throw wexception("preload_addon (%s): version string is empty", name.c_str());
	}

	for (std::string req(s.get_safe_string("requires")); !req.empty();) {
		const size_t commapos = req.find(',');
		if (commapos == std::string::npos) {
			i->requirements.push_back(req);
			break;
		}
		i->requirements.push_back(req.substr(0, commapos));
		req = req.substr(commapos + 1);
	}

	return pointer;
}

}  // namespace AddOns
