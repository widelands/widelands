/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "wui/mapdata.h"

#include <memory>

#include <boost/format.hpp>

#include "io/filesystem/filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"

MapData::MapData(const std::string& init_filename,
                 const std::string& init_localized_name,
                 const std::string& init_author,
                 const MapData::MapType& init_maptype,
                 const MapData::DisplayType& init_displaytype)
   : filename(init_filename),
     name(init_localized_name),
     localized_name(init_localized_name),
     authors(init_author),
     nrplayers(0),
     width(0),
     height(0),
     maptype(init_maptype),
     displaytype(init_displaytype) {
}

MapData::MapData(const Widelands::Map& map,
                 const std::string& init_filename,
                 const MapData::MapType& init_maptype,
                 const MapData::DisplayType& init_displaytype)
   : MapData(init_filename,
             _("No Name"),
             map.get_author().empty() ? _("No Author") : map.get_author(),
             init_maptype,
             init_displaytype) {

	std::unique_ptr<i18n::GenericTextdomain> td(AddOns::create_textdomain_for_map(init_filename));
	if (!map.get_name().empty()) {
		name = map.get_name();
		localized_name = _(name);
	}
	description = map.get_description().empty() ? "" : _(map.get_description());
	hint = map.get_hint().empty() ? "" : _(map.get_hint());
	required_addons = map.required_addons();
	theme = map.get_background_theme();
	background = map.get_background();
	nrplayers = map.get_nrplayers();
	width = map.get_width();
	height = map.get_height();
	suggested_teams = map.get_suggested_teams();
	tags = map.get_tags();

	if (maptype == MapData::MapType::kScenario) {
		tags.insert("scenario");
	}
}

MapData::MapData(const std::string& init_filename, const std::string& init_localized_name)
   : MapData(init_filename,
             init_localized_name,
             _("No Author"),
             MapData::MapType::kDirectory,
             MapData::DisplayType::kMapnamesLocalized) {
}

bool MapData::compare_names(const MapData& other) {
	// The parent directory gets special treatment.
	if (localized_name == parent_name() && maptype == MapData::MapType::kDirectory) {
		return true;
	}
	if (other.localized_name == parent_name() && other.maptype == MapData::MapType::kDirectory) {
		return false;
	}

	std::string this_name;
	std::string other_name;
	switch (displaytype) {
	case MapData::DisplayType::kFilenames:
		this_name = filename;
		other_name = other.filename;
		break;

	case MapData::DisplayType::kMapnames:
		this_name = name;
		other_name = other.name;
		break;

	case MapData::DisplayType::kMapnamesLocalized:
		this_name = localized_name;
		other_name = other.localized_name;
		break;
	}

	// If there is no width, we have a directory - we want them first.
	if (!width) {
		if (other.width) {
			return true;
		}
		return this_name < other_name;
	}

	if (!other.width) {
		return false;
	}
	return this_name < other_name;
}

bool MapData::compare_players(const MapData& other) {
	if (nrplayers == other.nrplayers) {
		return compare_names(other);
	}
	return nrplayers < other.nrplayers;
}

bool MapData::compare_size(const MapData& other) {
	if (width == other.width && height == other.height) {
		return compare_names(other);
	}
	if (width != other.width) {
		return width < other.width;
	}
	return height < other.height;
}

// static
MapData MapData::create_parent_dir(const std::string& current_dir) {
	std::string filename = FileSystem::fs_dirname(current_dir);
	if (!filename.empty()) {
		// fs_dirname always returns a directory with a separator at the end.
		filename.pop_back();

		// If the parent is an add-on, use the maps directory as parent instead.
		// Whether this is the case is determined by checking whether the parent
		// directory's parent is the add-ons directory.
		std::string parent = FileSystem::fs_dirname(filename);
		if (!parent.empty()) {
			parent.pop_back();
			if (parent == kAddOnDir) {
				filename = kMapsDir;
			}
		}
	}
	return MapData(filename, parent_name());
}

// static
std::string MapData::parent_name() {
	/** TRANSLATORS: Parent directory/folder */
	return (boost::format("<%s>") % _("parent")).str();
}

// static
MapData MapData::create_empty_dir(const std::string& current_dir) {
	/** TRANSLATORS: This label is shown when a folder is empty */
	return MapData(current_dir, (boost::format("<%s>") % _("empty")).str());
}

// static
MapData MapData::create_directory(const std::string& directory) {
	std::string localized_name = FileSystem::fs_filename(directory.c_str());
	if (boost::equals(directory, "maps/MP_Scenarios")) {
		/** TRANSLATORS: Directory name for MP Scenarios in map selection */
		localized_name = _("Multiplayer Scenarios");
	} else if (boost::equals(directory, "maps/My_Maps")) {
		/** TRANSLATORS: Directory name for user maps in map selection */
		localized_name = _("My Maps");
	} else if (boost::equals(directory, "maps/Downloaded")) {
		/** TRANSLATORS: Directory name for downloaded maps in map selection */
		localized_name = _("Downloaded Maps");
	} else if (directory.compare(0, kAddOnDir.size(), kAddOnDir) == 0) {
		std::string addon = directory.substr(kAddOnDir.size() + 1);
		addon = addon.substr(0, addon.find('/'));
		std::unique_ptr<i18n::GenericTextdomain> td(AddOns::create_textdomain_for_addon(addon));
		std::string profilepath = kAddOnDir;
		profilepath += FileSystem::file_separator();
		profilepath += addon;
		profilepath += FileSystem::file_separator();
		profilepath += "dirnames";
		Profile p(profilepath.c_str());
		if (Section* s = p.get_section("global")) {
			const std::string fname = FileSystem::fs_filename(directory.c_str());
			if (s->has_val(fname.c_str())) {
				localized_name = s->get_safe_string(fname);
			}
		}
	}
	return MapData(directory, localized_name);
}
