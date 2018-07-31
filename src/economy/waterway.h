/*
 * Copyright (C) 2004-2018 by the Widelands Development Team
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

#ifndef WL_ECONOMY_WATERWAY_H
#define WL_ECONOMY_WATERWAY_H

#include <vector>

#include "base/macros.h"
#include "economy/roadbase.h"
#include "logic/path.h"
#include "logic/roadtype.h"

namespace Widelands {
struct Ferry;
struct Fleet;
class Request;

class WaterwayDescr : public RoadBaseDescr {
public:
	WaterwayDescr(char const* const init_name, char const* const init_descname)
	   : RoadBaseDescr(init_name, init_descname, MapObjectType::WATERWAY) {
	}
	~WaterwayDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(WaterwayDescr);
};

struct Waterway : public RoadBase {
	friend class MapRoaddataPacket;  // For saving
	friend class MapRoadPacket;      // For init()

	static bool is_waterway_descr(MapObjectDescr const*);

	explicit Waterway();

	static Waterway& create(EditorGameBase&, Flag& start, Flag& end, const Path&);

	bool notify_ware(Game& game, FlagId flagid);

	void remove_worker(Worker&);
	void assign_carrier(Carrier&, uint8_t);

	void request_ferry_callback(Game&, Ferry*);

	Fleet* get_fleet() const;

private:
	friend struct Fleet;
	void set_fleet(Fleet* fleet);

	void link_into_flags(EditorGameBase&);

	void request_ferry();

	Ferry* ferry_;
	Fleet* fleet_;
};
}

#endif  // end of include guard: WL_ECONOMY_WATERWAY_H
