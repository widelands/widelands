/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#include "io/filesystem/filesystem.h"

MapData::MapData() : authors(""), nrplayers(0), width(0), height(0),
		maptype(MapData::MapType::kNormal), displaytype(MapData::DisplayType::kMapnamesLocalized) {}

		MapData::MapData(const Widelands::Map& map, const std::string& init_filename,
			  const MapData::MapType& init_maptype,
			  const MapData::DisplayType& init_displaytype) :
		MapData() {
		i18n::Textdomain td("maps");
		filename = init_filename;
		name = map.get_name();
		localized_name = name.empty() ? "" : _(name);
		// Localizing this, because some author fields now have "edited by" text.
		const std::string& author = map.get_author();
		authors = MapAuthorData(author.empty() ? _("No Author") : _(author));
		description = map.get_description().empty() ? "" : _(map.get_description());
		hint = map.get_hint().empty() ? "" : _(map.get_hint());
		nrplayers = map.get_nrplayers();
		width = map.get_width();
		height = map.get_height();
		suggested_teams = map.get_suggested_teams();
		tags = map.get_tags();
		maptype = init_maptype;
		displaytype = init_displaytype;

		if (maptype == MapData::MapType::kScenario) {
			tags.insert("scenario");
		}
	}

MapData::MapData(const std::string& init_filename, const std::string& init_localized_name) :
		MapData() {
		filename = init_filename;
		name = init_localized_name;
		localized_name = init_localized_name;
		maptype = MapData::MapType::kDirectory;
	}

bool MapData::compare_names(const MapData& other) {
	// The parent directory gets special treatment.
	if (localized_name == parent_name() && maptype == MapData::MapType::kDirectory) {
		return true;
	} else if (other.localized_name == parent_name() &&
	           other.maptype == MapData::MapType::kDirectory) {
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
	if (!width && !other.width) {
		return this_name < other_name;
	} else if (!width && other.width) {
		return true;
	} else if (width && !other.width) {
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
	}
	return MapData(filename, parent_name());
}

// static
std::string MapData::parent_name() {
	/** TRANSLATORS: Parent directory/folder */
	return (boost::format("<%s>") % _("parent")).str();
}

// static
MapData MapData::create_directory(const std::string& directory) {
	std::string localized_name;
	if (boost::equals(directory, "maps/MP_Scenarios")) {
		/** TRANSLATORS: Directory name for MP Scenarios in map selection */
		localized_name = _("Multiplayer Scenarios");
	} else {
		localized_name = FileSystem::fs_filename(directory.c_str());
	}
	return MapData(directory, localized_name);
}
