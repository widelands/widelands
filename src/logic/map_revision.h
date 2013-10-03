/*
 * Copyright (C) 2013 by the Widelands Development Team
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

#ifndef MAP_REVISION_H
#define MAP_REVISION_H

#include <cstring>
#include <ctime>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Widelands {




struct MapVersion {

	std::string m_map_source_url;
	std::string m_map_source_release;
	std::string m_map_creator_version;
	int32_t     m_map_version_major;
	int32_t     m_map_version_minor;
	uint32_t    m_map_version_timestamp;

	MapVersion();

};

}



#endif
