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

#include "base/i18n.h"
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
	void parse(const std::string& author_list) {
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

	std::string filename;
	std::string name;
	std::string localized_name;
	std::string description;
	std::string hint;
	Tags tags;
	std::vector<Widelands::Map::SuggestedTeamLineup> suggested_teams;

	MapAuthorData authors;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t nrplayers = 0;
	MapData::MapType maptype = MapData::MapType::kNormal;
};

#endif  // end of include guard: WL_WUI_MAPDATA_H
