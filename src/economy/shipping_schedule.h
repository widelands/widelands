/*
 * Copyright (C) 2019-2020 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SHIPPING_SCHEDULE_H
#define WL_ECONOMY_SHIPPING_SCHEDULE_H

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "logic/widelands.h"

class FileRead;
class FileWrite;

namespace Widelands {

class EditorGameBase;
class Game;
struct MapObjectLoader;
struct MapObjectSaver;
class PortDock;
class Ship;
struct ShipFleet;

using CargoList = std::vector<std::pair<PortDock*, uint32_t>>;
using CargoListLoader = std::vector<std::pair<Serial, uint32_t>>;

template <typename DockT, typename CargosT>
struct SchedulingStateT {
	DockT dock;
	bool expedition;
	CargosT load_there;
	Duration duration_from_previous_location;

	SchedulingStateT(DockT pd, bool exp = false, Duration d = 0) : dock(pd), expedition(exp), duration_from_previous_location(d) {
	}
	SchedulingStateT(const SchedulingStateT&) = default;
	SchedulingStateT& operator=(const SchedulingStateT&) = default;
	~SchedulingStateT() {
	}
};

using SchedulingState = SchedulingStateT<PortDock*, CargoList>;
using ShipPlan = std::list<SchedulingState>;

struct ShippingSchedule {
public:
	ShippingSchedule(ShipFleet&);
	~ShippingSchedule() {
	}

	// called by ShipFleet::act()
	void update(Game&);

	void ship_added(Game&, Ship&) {
		// Nothing to do currently
	}
	void port_added(Game&, PortDock&);
	/**
	  * Forget the plans for this ship. Disappointed items will be
	  * taken care of again by the next update().
	  */
	void ship_removed(const Game&, Ship* dangling_pointer_to_deleted_ship);
	/**
	  * This one is more critical. Most of the consequences will be handled by update(),
	  * but we also have to take some action (rerouting ships, updating
	  * shippingitems, …) immediately or we risk segfaults.
	  */
	void port_removed(Game&, PortDock* dangling_pointer_to_deleted_portdock);

	// Load wares&workers onto the ship and set the destination
	void ship_arrived(Game&, Ship&, PortDock&);

	bool empty() const {
		return plans_.empty();
	}

	void save(const EditorGameBase&, MapObjectSaver&, FileWrite&) const;
	void load(FileRead&);
	void load_pointers(MapObjectLoader&);

private:
	ShipFleet& fleet_;
	std::map<Ship*, ShipPlan> plans_;

	uint32_t last_updated_;
	uint32_t last_actual_durations_recalculation_;

	using ScheduleLoader = std::map<Serial, std::list<SchedulingStateT<Serial, CargoListLoader>>>;
	std::unique_ptr<ScheduleLoader> loader_;

	DISALLOW_COPY_AND_ASSIGN(ShippingSchedule);
};

} // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SHIPPING_SCHEDULE_H
