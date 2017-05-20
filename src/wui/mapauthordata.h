/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_WUI_MAPAUTHORDATA_H
#define WL_WUI_MAPAUTHORDATA_H

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
	const std::string& get_names() const {
		return names_;
	}
	size_t get_number() const {
		return number_;
	}

	// Parses author list string into localized contatenated list
	// string. Use , as list separator and no whitespaces between
	// author names.
	MapAuthorData(const std::string& author_list) {
		std::vector<std::string> authors;
		boost::split(authors, author_list, boost::is_any_of(","));
		names_ = i18n::localize_list(authors, i18n::ConcatenateWith::AMPERSAND);
		number_ = authors.size();
	}

private:
	std::string names_;
	size_t number_;
};

#endif  // end of include guard: WL_WUI_MAPAUTHORDATA_H
