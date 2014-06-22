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

#include <boost/noncopyable.hpp>

class LuaTable;

namespace Widelands {

// Describes the parameters and the pickiness of Immovables towards terrain
// parameters. Alls immovables that use 'grow' in any of their programs must
// define this.
class TerrainAffinity : boost::noncopyable {
public:
	explicit TerrainAffinity(const LuaTable& table);

	// Preferred temperature in Kelvin.
	double preferred_temperature() const;

	// Preferred fertility in percent [0, 1].
	double preferred_fertility() const;

	// Preferred humidity in percent [0, 1].
	double preferred_humidity() const;

	// NOCOM(#sirver): figure this out
	double pickiness() const;

private:
	double preferred_temperature_;
	double preferred_fertility_;
	double preferred_humidity_;
	double pickiness_;
};

}  // namespace Widelands
