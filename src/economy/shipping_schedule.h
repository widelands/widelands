/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SHIPPING_SCHEDULE_H
#define WL_ECONOMY_SHIPPING_SCHEDULE_H

#include <list>
#include <map>
#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/ship.h"

class FileRead;
class FileWrite;

namespace Widelands {

class EditorGameBase;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
class PortDock;
struct PrioritisedPortPair;

using CargoList = std::map<OPtr<PortDock>, uint32_t>;
using CargoListLoader = std::map<Serial, uint32_t>;

template <typename DockT, typename CargosT> struct SchedulingStateT {
	DockT dock;
	bool expedition;
	CargosT load_there;
	Duration duration_from_previous_location;

	explicit SchedulingStateT(DockT pd, bool exp = false, const Duration& d = Duration(0))
	   : dock(pd), expedition(exp), duration_from_previous_location(d) {
	}
	SchedulingStateT(const SchedulingStateT&) = default;
	SchedulingStateT& operator=(const SchedulingStateT&) = default;
	~SchedulingStateT() {
	}
};

using SchedulingState = SchedulingStateT<OPtr<PortDock>, CargoList>;
using ShipPlan = std::list<SchedulingState>;

struct ShippingSchedule {
public:
	explicit ShippingSchedule(ShipFleet&);
	~ShippingSchedule() {
	}

	// called by ShipFleet::act()
	// returns the time until the next update
	Duration update(Game&);

	void ship_added(Game&, Ship&);
	void port_added(Game&, PortDock&);
	/**
	 * Forget the plans for this ship. Items which this ship was supposed to
	 * pick up later will be taken care of again by the next update().
	 */
	void ship_removed(const Game&, Ship*);
	/**
	 * This one is more critical. Most of the consequences will be handled by update(),
	 * but we also have to take some action (rerouting ships, updating
	 * shippingitems, …) immediately or we risk segfaults.
	 */
	void port_removed(Game&, PortDock*);

	// Load wares and workers onto the ship and set the destination.
	// Unloading was previously performed by the ship in ship_update_transport().
	void ship_arrived(Game&, Ship&, PortDock&);

	/**
	 * Check if there is any ship with a plan.
	 * We may have ships and/or ports but no plans to carry anything.
	 * @return true when no plans for any ship exist
	 */
	bool empty() const;

	bool is_busy(const Ship&) const;

	void log_general_info(const EditorGameBase&) const;

	void save(const EditorGameBase&, MapObjectSaver&, FileWrite&) const;
	void load(FileRead&);
	void load_pointers(MapObjectLoader&);

private:
	ShipFleet& fleet_;
	std::map<OPtr<Ship>, ShipPlan> plans_;

	// Absolute gametime of last update
	Time last_updated_;
	// Absolute gametimes of last recalculation update
	// (we only perform such updates once in a while because they are very costly)
	std::map<OPtr<Ship>, Time> last_actual_duration_recalculation_;

	void start_expedition(Game&, Ship&, PortDock&);
	bool do_remove_port_from_plan(Game&, PortDock*, Ship&, ShipPlan&);

	// Some helper functions for update()
	uint32_t get_free_capacity_at(Game&, Ship&, PortDock&);
	void load_on_ship(Game&, PrioritisedPortPair&, std::list<PrioritisedPortPair>&);
	void get_free_capacity_between(Game&,
	                               const Ship&,
	                               ShipPlan&,
	                               PortDock& start,
	                               PortDock& end,
	                               bool& found_start,
	                               bool& found_end,
	                               bool& expedition,
	                               bool& start_is_last,
	                               Duration& arrival_time,
	                               Duration& detour_start_end,
	                               uint32_t& free_capacity);

	struct ScheduleLoader {
		std::map<Serial, std::list<SchedulingStateT<Serial, CargoListLoader>>> plan;
		std::map<Serial, Time> recalc;
	};
	std::unique_ptr<ScheduleLoader> loader_;

	DISALLOW_COPY_AND_ASSIGN(ShippingSchedule);
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SHIPPING_SCHEDULE_H
