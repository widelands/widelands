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

#include "base/i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"

const std::map<std::string, AddOnCategory> kAddOnCategories = {
	{"tribes", AddOnCategory {"tribes", []() { return _("Tribes"); }, "images/wui/stats/menu_tab_wares_warehouse.png", true}},
	{"world", AddOnCategory {"world", []() { return _("World"); }, "images/wui/menus/toggle_immovables.png", true}},
	{"script", AddOnCategory {"script", []() { return _("Script"); }, "images/logos/WL-Editor-32.png", true}},
	{"maps", AddOnCategory {"maps", []() { return _("Map Set"); }, "images/wui/menus/toggle_minimap.png", false}},
	{"campaign", AddOnCategory {"campaign", []() { return _("Campaign"); }, "images/wui/menus/chat.png", false}},
	{"win_condition", AddOnCategory {"win_condition", []() { return _("Win Condition"); }, "images/wui/menus/objectives.png", false}},
	{"starting_condition", AddOnCategory {"starting_condition", []() { return _("Starting Condition"); }, "images/players/player_position_menu.png", false}}
};

std::vector<std::pair<AddOnInfo, bool>> g_addons;

AddOnInfo preload_addon(const std::string& name) {
	std::unique_ptr<FileSystem> fs(g_fs->make_sub_file_system(kAddOnDir + g_fs->file_separator() + name));
	Profile profile;
	profile.read("addon", nullptr, *fs);
	Section& s = profile.get_safe_section("global");
	AddOnInfo i = {
		name,
		s.get_safe_string("name"),
		s.get_safe_string("description"),
		s.get_safe_string("author"),
		s.get_safe_positive("version"),
		&kAddOnCategories.at(s.get_safe_string("category")),
		{}, false
	};
	for (std::string req(s.get_safe_string("requires")); !req.empty();) {
		const size_t commapos = req.find(',');
		if (commapos == std::string::npos) {
			i.requires.push_back(req);
			break;
		} else {
			i.requires.push_back(req.substr(0, commapos));
			req = req.substr(commapos);
		}
	}
	return i;
}
