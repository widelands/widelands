/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SHIPPINGITEM_H
#define WL_ECONOMY_SHIPPINGITEM_H

#include "logic/game.h"
#include "logic/map_objects/map_object.h"

class FileRead;
class FileWrite;

namespace Widelands {

class Economy;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
class MapObject;
class PortDock;
class WareInstance;
class Worker;

/**
 * Wares and workers that are transported through the ship system are
 * encapsulated in this structure during shipping and the waiting time in the @ref PortDock.
 */
struct ShippingItem {
	ShippingItem() {}
	ShippingItem(WareInstance & ware);
	ShippingItem(Worker & worker);

	// Unboxes the item that is shipped which might be either a ware or a
	// worker. It is safe to pass nullptr for 'ware' or 'worker' in case you are
	// only interested in the ware if it is the one or the other.
	void get(EditorGameBase& game, WareInstance** ware, Worker** worker) const;

	void set_economy(Game &, Economy * e);
	PortDock * get_destination(Game &);
	void schedule_update(Game &, int32_t delay);

	void remove(EditorGameBase &);

	struct Loader {
		void load(FileRead & fr);
		ShippingItem get(MapObjectLoader & mol);

	private:
		uint32_t m_serial;
	};

	void save(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw);

private:
	friend class PortDock;
	friend struct Ship;

	// Called when a port is reached. The item will act again on its own.
	void end_shipping(Game &);

	// Sets the location of this shippingitem, this could be a ship, a portdock or a warehouse.
	void set_location(Game&, MapObject* obj);

	// Updates m_destination_dock.
	void update_destination(Game &, PortDock &);

	ObjectPointer m_object;
	OPtr<PortDock> m_destination_dock;
};

} // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SHIPPINGITEM_H
