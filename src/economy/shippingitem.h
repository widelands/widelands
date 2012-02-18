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

struct Economy;
struct Game;
struct Map_Map_Object_Loader;
struct Map_Map_Object_Saver;
class Map_Object;
struct PortDock;
class WareInstance;
class Worker;

/**
 * Wares and workers that are transported through the ship system are
 * encapsulated in this structure during shipping and the waiting time in the @ref PortDock.
 */
struct ShippingItem {
	friend struct PortDock;
	friend struct Ship;

	ShippingItem() {}
	ShippingItem(WareInstance & ware);
	ShippingItem(Worker & worker);

	void get(Editor_Game_Base & game, WareInstance * & ware, Worker * & worker);

	void set_economy(Game &, Economy * e);
	void set_location(Game &, Map_Object * obj);
	void end_shipping(Game &);
	PortDock * get_destination(Game &);
	void fetch_destination(Game &, PortDock &);
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
	Object_Ptr m_object;
	OPtr<PortDock> m_destination_dock;
};

} // namespace Widelands

#endif // ECONOMY_SHIPPINGITEM_H
