/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/path.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/map_object.h"
#include "logic/widelands_geometry_io.h"

namespace Widelands {

constexpr uint8_t kCurrentPacketVersion = 1;

Path::Path(CoordPath& o) : start_(o.get_start()), end_(o.get_end()), path_(o.steps()) {
	std::reverse(path_.begin(), path_.end());  //  path stored in reverse order
}

/*
===============
Change the path so that it goes in the opposite direction
===============
*/
void Path::reverse() {
	std::swap(start_, end_);
	std::reverse(path_.begin(), path_.end());

	for (Direction& dir : path_) {
		dir = get_reverse_dir(dir);
	}
}

/*
===============
Add the given step at the end of the path.
===============
*/
void Path::append(const Map& map, const Direction dir) {
	path_.insert(path_.begin(), dir);  // stores in reversed order!
	map.get_neighbour(end_, dir, &end_);
}

/**
 * Save the given path in the given file
 */
void Path::save(FileWrite& fw) const {
	fw.unsigned_8(kCurrentPacketVersion);
	write_coords_32(&fw, start_);

	// Careful: steps are stored in the reverse order in path_
	// However, we save them in the forward order, to make loading easier
	fw.unsigned_32(path_.size());
	for (uint32_t i = path_.size(); i > 0; --i) {
		write_direction_8(&fw, path_[i - 1]);
	}
}

/**
 * Load the path from the given file.
 *
 * The path previously contained in \p this object is entirely
 * replaced by the path from the file.
 */
void Path::load(FileRead& fr, const Map& map) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			start_ = end_ = read_coords_32(&fr, map.extent());
			path_.clear();
			uint32_t steps = fr.unsigned_32();
			while ((steps--) != 0u) {
				append(map, read_direction_8(&fr));
			}
		} else {
			throw UnhandledVersionError("Path", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("player names and tribes: %s", e.what());
	}
}

/*
===============
Initialize from a path, calculating the coordinates as needed
===============
*/
CoordPath::CoordPath(const Map& map, const Path& path) {
	coords_.clear();
	path_.clear();

	coords_.push_back(path.get_start());

	Coords c = path.get_start();

	Path::StepVector::size_type nr_steps = path.get_nsteps();
	for (Path::StepVector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = path[i];

		path_.push_back(dir);
		map.get_neighbour(c, dir, &c);
		coords_.push_back(c);
	}
}

/// After which step does the node appear in this path?
/// \return -1 if node is not part of this path.
int32_t CoordPath::get_index(const Coords& field) const {
	for (uint32_t i = 0; i < coords_.size(); ++i) {
		if (coords_[i] == field) {
			return i;
		}
	}

	return -1;
}

/*
===============
Reverse the direction of the path.
===============
*/
void CoordPath::reverse() {
	std::reverse(path_.begin(), path_.end());
	std::reverse(coords_.begin(), coords_.end());

	for (Direction& dir : path_) {
		dir = get_reverse_dir(dir);
	}
}

/*
===============
Truncate the path after the given number of steps
===============
*/
void CoordPath::truncate(const std::vector<char>::size_type after) {
	assert(after <= path_.size());

	path_.erase(path_.begin() + after, path_.end());
	coords_.erase(coords_.begin() + after + 1, coords_.end());
}

/*
===============
Opposite of truncate: remove the first n steps of the path.
===============
*/
void CoordPath::trim_start(const std::vector<char>::size_type before) {
	assert(before <= path_.size());

	path_.erase(path_.begin(), path_.begin() + before);
	coords_.erase(coords_.begin(), coords_.begin() + before);
}

/*
===============
Append the given path. Automatically created the necessary coordinates.
===============
*/
void CoordPath::append(const Map& map, const Path& tail) {
	assert(tail.get_start() == get_end());

	Coords c = get_end();

	const Path::StepVector::size_type nr_steps = tail.get_nsteps();
	for (CoordPath::StepVector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = tail[i];

		map.get_neighbour(c, dir, &c);
		path_.push_back(dir);
		coords_.push_back(c);
	}
}

/*
===============
Append the given path.
===============
*/
void CoordPath::append(const CoordPath& tail) {
	assert(tail.get_start() == get_end());

	path_.insert(path_.end(), tail.path_.begin(), tail.path_.end());
	coords_.insert(coords_.end(), tail.coords_.begin() + 1, tail.coords_.end());
}
}  // namespace Widelands
