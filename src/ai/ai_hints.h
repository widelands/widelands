/*
 * Copyright (C) 2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_AI_AI_HINTS_H
#define WL_AI_AI_HINTS_H

#include <stdint.h>

#include <boost/noncopyable.hpp>

class Section;

/// This struct is used to read out the data given in [aihints] section of a
/// buildings conf file. It is used to tell the computer player about the
/// special properties of a building.
struct BuildingHints : boost::noncopyable {
	BuildingHints(Section*);
	~BuildingHints();

	char const* get_renews_map_resource() const {
		return renews_map_resource;
	}

	char const* get_mines() const {
		return mines_;
	}

	bool is_basic() const {
		return basic_;
	}

	bool prod_build_material() const {
		return build_material_;
	}

	bool is_logproducer() const {
		return log_producer_;
	}

	bool is_stoneproducer() const {
		return stone_producer_;
	}

	bool is_marbleproducer() const {
		return marble_producer_;
	}

	bool mines_water() const {
		return mines_water_;
	}

	bool get_needs_water() const {
		return needs_water_;
	}

	bool for_recruitment() const {
		return recruitment_;
	}

	bool is_space_consumer() const {
		return space_consumer_;
	}

	uint8_t get_mines_percent() const {
		return mines_percent_;
	}

private:
	char* renews_map_resource;
	char* mines_;
	bool basic_;
	bool build_material_;  // whether the building produces build material
	bool log_producer_;
	bool stone_producer_;
	bool marble_producer_;
	bool needs_water_;
	bool mines_water_;
	bool recruitment_;  // whether building recruits special workers
	bool space_consumer_;
	uint8_t mines_percent_;
};

#endif  // end of include guard: WL_AI_AI_HINTS_H
