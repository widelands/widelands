/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SHIPPINGITEM_H
#define WL_ECONOMY_SHIPPINGITEM_H

#include "logic/game.h"
#include "logic/map_objects/map_object.h"

class FileRead;
class FileWrite;

namespace Widelands {

class Economy;
class MapObjectLoader;
struct MapObjectSaver;
class WareInstance;
class Worker;

/**
 * Wares and workers that are transported through the ship system are
 * encapsulated in this structure during shipping and the waiting time in the @ref PortDock.
 */
struct ShippingItem {
	ShippingItem() {
	}
	explicit ShippingItem(WareInstance& ware);
	explicit ShippingItem(Worker& worker);

	// Unboxes the item that is shipped which might be either a ware or a
	// worker. It is safe to pass nullptr for 'ware' or 'worker' in case you are
	// only interested in the ware if it is the one or the other.
	void get(const EditorGameBase& game, WareInstance** ware, Worker** worker) const;

	void set_economy(const Game&, Economy* e, WareWorker);
	const PortDock* get_destination(Game&) const;

	void remove(EditorGameBase&);

	struct Loader {
		void load(FileRead& fr);
		ShippingItem get(MapObjectLoader& mol);

	private:
		uint32_t serial_ = 0U;
		uint32_t destination_serial_ = 0U;
	};

	void save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw);

private:
	friend class PortDock;
	friend struct Ship;
	friend struct ShipFleet;
	friend struct ShippingSchedule;

	// Called when a port is reached. The item will act again on its own.
	void end_shipping(Game&);

	// Sets the location of this shippingitem, this could be a ship, a portdock or a warehouse.
	void set_location(Game&, MapObject* obj);

	// Updates destination_dock_.
	void update_destination(Game&, PortDock&);

	ObjectPointer object_;
	OPtr<PortDock> destination_dock_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SHIPPINGITEM_H
