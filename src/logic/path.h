/*
 * Copyright (C) 2004-2024 by the Widelands Development Team
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

#ifndef WL_LOGIC_PATH_H
#define WL_LOGIC_PATH_H

#include <cassert>

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

class FileRead;
class FileWrite;

namespace Widelands {

/** struct Path
 *
 * Represents a cross-country path found by Path::findpath, for example
 */
struct CoordPath;
class Map;

struct Path {
	friend class Map;
	friend struct MapAStarBase;

	Path() = default;
	explicit Path(const Coords& c) : start_(c), end_(c) {
	}
	Path(CoordPath&);  // NOLINT allow implicit conversion

	void reverse();

	[[nodiscard]] Coords get_start() const {
		return start_;
	}
	[[nodiscard]] Coords get_end() const {
		return end_;
	}

	using StepVector = std::vector<Direction>;
	[[nodiscard]] StepVector::size_type get_nsteps() const {
		return path_.size();
	}
	Direction operator[](StepVector::size_type const i) const {
		assert(i < path_.size());
		return path_[path_.size() - i - 1];
	}

	void append(const Map& map, Direction);

	void reorigin(const Coords& new_origin, const Extent& extent) {
		start_.reorigin(new_origin, extent);
		end_.reorigin(new_origin, extent);
	}

	void save(FileWrite& fw) const;
	void load(FileRead& fr, const Map& map);

private:
	Coords start_;
	Coords end_;
	StepVector path_;
};

// CoordPath is an extended path that also caches related Coords
struct CoordPath {
	CoordPath() = default;
	explicit CoordPath(Coords c) {
		coords_.push_back(c);
	}
	CoordPath(const Map& map, const Path& path);

	[[nodiscard]] Coords get_start() const {
		return coords_.front();
	}
	[[nodiscard]] Coords get_end() const {
		return coords_.back();
	}
	[[nodiscard]] const std::vector<Coords>& get_coords() const {
		return coords_;
	}

	using StepVector = std::vector<Direction>;
	[[nodiscard]] StepVector::size_type get_nsteps() const {
		return path_.size();
	}
	Direction operator[](StepVector::size_type const i) const {
		assert(i < path_.size());
		return path_[i];
	}
	[[nodiscard]] const StepVector& steps() const {
		return path_;
	}

	[[nodiscard]] int32_t get_index(const Coords& field) const;

	void reverse();
	void truncate(std::vector<char>::size_type after);
	void trim_start(std::vector<char>::size_type before);
	void append(const Map& map, const Path& tail);
	void append(const CoordPath& tail);

private:
	StepVector path_;             //  directions
	std::vector<Coords> coords_;  //  coords_.size() == path_.size() + 1
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PATH_H
