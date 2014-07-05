/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef ONE_WORLD_LEGACY_CONVERSION_H
#define ONE_WORLD_LEGACY_CONVERSION_H

#include <map>
#include <memory>
#include <string>

#include <boost/noncopyable.hpp>

class OneWorldLegacyLookupTable : boost::noncopyable {
public:
	virtual ~OneWorldLegacyLookupTable();

	/// Looks up the new name for the 'resource'.
	virtual std::string lookup_resource(const std::string& resource) const = 0;

	/// Looks up the new name for the 'terrain'.
	virtual std::string lookup_terrain(const std::string& terrain) const = 0;

	/// Looks up the new name for the 'critter'.
	virtual std::string lookup_critter(const std::string& critter) const = 0;

	/// Looks up the new name for the 'immovable'.
	virtual std::string lookup_immovable(const std::string& immovable) const = 0;
};

std::unique_ptr<OneWorldLegacyLookupTable>
create_one_world_legacy_lookup_table(const std::string& old_world);

#endif
