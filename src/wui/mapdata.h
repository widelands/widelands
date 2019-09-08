/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
#include "wui/mapauthordata.h"

/**
 * Data about a map that we're interested in.
 */
struct MapData {
	enum class MapType { kNormal, kDirectory, kScenario, kSettlers2 };

	enum class DisplayType { kFilenames, kMapnames, kMapnamesLocalized };

private:
	/// For common properties
	MapData(const std::string& init_filename,
	        const std::string& init_localized_name,
	        const std::string& init_author,
	        const MapData::MapType& init_maptype,
	        const MapData::DisplayType& init_displaytype);

public:
	/// For normal maps and scenarios
	MapData(const Widelands::Map& map,
	        const std::string& init_filename,
	        const MapData::MapType& init_maptype,
	        const MapData::DisplayType& init_displaytype);

	/// For directories
	MapData(const std::string& init_filename, const std::string& init_localized_name);

	/// The localized name of the parent directory
	static std::string parent_name();

	/// Get the ".." directory
	static MapData create_parent_dir(const std::string& current_dir);

	/// To display if the directory is empty and has no parent
	static MapData create_empty_dir(const std::string& current_dir);

	/// Create a subdirectory
	static MapData create_directory(const std::string& directory);

	// Sorting functions to order by different categories.
	bool compare_names(const MapData& other);
	bool compare_players(const MapData& other);
	bool compare_size(const MapData& other);

	std::string filename;
	std::string name;
	std::string localized_name;
	MapAuthorData authors;
	std::string description;
	std::string hint;
	uint32_t nrplayers;
	uint32_t width;
	uint32_t height;
	std::vector<Widelands::SuggestedTeamLineup> suggested_teams;
	std::set<std::string> tags;
	MapData::MapType maptype;
	MapData::DisplayType displaytype;
};

#endif  // end of include guard: WL_WUI_MAPDATA_H
