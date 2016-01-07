/*
 * Copyright (C) 2002, 2006-2009, 2011, 2014-2015 by the Widelands Development Team
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

#ifndef WL_WUI_MAPDATA_H
#define WL_WUI_MAPDATA_H

#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "io/filesystem/filesystem.h"
#include "logic/map.h"


/**
 * Author data for a map or scenario.
 */
struct MapAuthorData {
	const std::string& get_names() const {return m_names;}
	size_t get_number()            const {return m_number;}

	// Parses author list string into localized contatenated list
	// string. Use , as list separator and no whitespaces between
	// author names.
	MapAuthorData(const std::string& author_list) {
		std::vector<std::string> authors;
		boost::split(authors, author_list, boost::is_any_of(","));
		m_names = i18n::localize_list(authors, i18n::ConcatenateWith::AMPERSAND);
		m_number = authors.size();
	}

private:
	std::string m_names;
	size_t      m_number;
};

/**
 * Data about a map that we're interested in.
 */
struct MapData {
	using Tags = std::set<std::string>;

	enum class MapType {
		kNormal,
		kDirectory,
		kScenario,
		kSettlers2
	};

	enum class DisplayType {
		kFilenames,
		kMapnames,
		kMapnamesLocalized
	};


	/// For incomplete data
	MapData() : authors(""), nrplayers(0), width(0), height(0),
		maptype(MapData::MapType::kNormal), displaytype(MapData::DisplayType::kMapnamesLocalized) {}

	/// For normal maps and scenarios
	MapData(const Widelands::Map& map, const std::string& init_filename,
			  const MapData::MapType& init_maptype,
			  const MapData::DisplayType& init_displaytype) :
		MapData() {
		i18n::Textdomain td("maps");
		filename = init_filename;
		name = map.get_name();
		localized_name = name.empty() ? "" : _(name);
		authors = MapAuthorData(map.get_author());
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

	/// For directories
	MapData(const std::string& init_filename, const std::string& init_localized_name) :
		MapData() {
		filename = init_filename;
		name = init_localized_name;
		localized_name = init_localized_name;
		maptype = MapData::MapType::kDirectory;
	}

	/// Get the ".." directory
	static MapData create_parent_dir(const std::string& current_dir) {
#ifndef _WIN32
		const std::string filename = current_dir.substr(0, current_dir.rfind('/'));
#else
		const std::string filename = current_dir.substr(0, current_dir.rfind('\\'));
#endif
		return MapData(filename, (boost::format("\\<%s\\>") % _("parent")).str());
	}

	/// Create a subdirectory
	static MapData create_directory(const std::string& directory) {
		std::string localized_name;
		if (boost::equals(directory, "maps/MP_Scenarios")) {
			/** TRANSLATORS: Directory name for MP Scenarios in map selection */
			localized_name = _("Multiplayer Scenarios");
		} else {
			localized_name = FileSystem::fs_filename(directory.c_str());
		}
		return MapData(directory, localized_name);
	}

	std::string filename;
	std::string name;
	std::string localized_name;
	MapAuthorData authors;
	std::string description;
	std::string hint;
	uint32_t nrplayers;
	uint32_t width;
	uint32_t height;
	std::vector<Widelands::Map::SuggestedTeamLineup> suggested_teams;
	Tags tags;
	MapData::MapType maptype;
	MapData::DisplayType displaytype;

	bool compare_names(const MapData& other) {
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
			default:
				this_name = localized_name;
				other_name = other.localized_name;
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

	bool compare_players(const MapData& other) {
		if (nrplayers == other.nrplayers) {
			return compare_names(other);
		}
		return nrplayers < other.nrplayers;
	}

	bool compare_size(const MapData& other) {
		if (width == other.width && height == other.height) {
			return compare_names(other);
		}
		if (width != other.width) {
			return width < other.width;
		}
		return height < other.height;
	}
};

#endif  // end of include guard: WL_WUI_MAPDATA_H
