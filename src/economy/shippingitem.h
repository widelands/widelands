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

#ifndef ECONOMY_SHIPPINGITEM_H
#define ECONOMY_SHIPPINGITEM_H

#include "logic/game.h"
#include "logic/instances.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

class Economy;
class Game;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;
class Map_Object;
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
	void get(Editor_Game_Base& game, WareInstance** ware, Worker** worker) const;

	void set_economy(Game &, Economy * e);
	PortDock * get_destination(Game &);
	void schedule_update(Game &, int32_t delay);

	void remove(Editor_Game_Base &);

	struct Loader {
		void load(FileRead & fr);
		ShippingItem get(Map_Map_Object_Loader & mol);

	private:
		uint32_t m_serial;
	};

	void save(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw);

private:
	friend class PortDock;
	friend struct Ship;

	// Called when a port is reached. The item will act again on its own.
	void end_shipping(Game &);

	// Sets the location of this shippingitem, this could be a ship, a portdock or a warehouse.
	void set_location(Game&, Map_Object* obj);

	// Updates m_destination_dock.
	void update_destination(Game &, PortDock &);

	Object_Ptr m_object;
	OPtr<PortDock> m_destination_dock;
};

} // namespace Widelands

#endif // ECONOMY_SHIPPINGITEM_H
