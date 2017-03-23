/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#ifndef WL_ECONOMY_ROUTE_H
#define WL_ECONOMY_ROUTE_H

#include <vector>

#include "economy/iroute.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {

struct Flag;
class EditorGameBase;
struct MapObjectSaver;
class MapObjectLoader;
struct RoutingNode;

/**
 * Route stores a route from flag to flag.
 * The number of steps is the number of flags stored minus one.
 */
struct Route : public IRoute {
	friend struct Router;
	friend class Request;

	Route();

	void init(int32_t) override;

	int32_t get_totalcost() const {
		return totalcost_;
	}
	int32_t get_nrsteps() const {
		return route_.size() - 1;
	}
	Flag& get_flag(EditorGameBase&, std::vector<Flag*>::size_type);

	void trim_start(int32_t count);
	void truncate(int32_t count);

	struct LoadData {
		std::vector<uint32_t> flags;
	};

	void load(LoadData&, FileRead&);
	void load_pointers(const LoadData&, MapObjectLoader&);
	void save(FileWrite&, EditorGameBase&, MapObjectSaver&);

	void insert_as_first(RoutingNode* node) override;

private:
	int32_t totalcost_;
	std::vector<OPtr<Flag>> route_;  ///< includes start and end flags
};
}

#endif  // end of include guard: WL_ECONOMY_ROUTE_H
