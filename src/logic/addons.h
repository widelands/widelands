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

#ifndef WL_LOGIC_ADDONS_H
#define WL_LOGIC_ADDONS_H

#include <functional>
#include <map>
#include <string>
#include <vector>

struct AddOnCategory {
	std::string name;
	std::function<std::string()> descname;
	std::string icon;
	bool can_disable_addons;
};

constexpr uint32_t kNotInstalled = 0;

struct AddOnInfo {
	std::string internal_name;          // "cool_feature.wad"
	std::string descname;               // "Cool Feature"
	std::string description;            // "This add-on is a really cool feature."
	std::string author;                 // "The Widelands Bunnybot"
	uint32_t version;
	const AddOnCategory* category;
	std::vector<std::string> requires;  // TODO(Nordfriese): unused, not yet implemented
	bool verified;
	// TODO(Nordfriese): in the future, we might also want to include:
	// uploader username, upload date&time, average rating, number of votes, (what else?)
};

// Sorted list of all add-ons mapped to whether they are currently enabled
extern std::vector<std::pair<AddOnInfo, bool>> g_addons;

extern const std::map<std::string, AddOnCategory> kAddOnCategories;

AddOnInfo preload_addon(const std::string&);

#endif  // end of include guard: WL_LOGIC_ADDONS_H
