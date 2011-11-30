/*
 * Copyright (C) 2010-2011 by the Widelands Development Team
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

#ifndef SHIP_H
#define SHIP_H

#include "bob.h"
#include "economy/shippingitem.h"
#include "graphic/diranimations.h"

namespace UI {struct Window;}
struct Interactive_GameBase;

namespace Widelands {

struct Economy;
struct Fleet;
struct PortDock;

struct Ship_Descr : Bob::Descr {
	Ship_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 Tribe_Descr const &);

	virtual uint32_t movecaps() const throw ();
	const DirAnimations & get_sail_anims() const {return m_sail_anims;}

	uint32_t get_capacity() const throw () {return m_capacity;}

	virtual Bob & create_object() const;

private:
	DirAnimations m_sail_anims;
	uint32_t m_capacity;
};

/**
 * Ships belong to a player and to an economy.
 */
struct Ship : Bob {
	MO_DESCR(Ship_Descr);

	Ship(const Ship_Descr & descr);
	virtual ~Ship();

	Fleet * get_fleet() const {return m_fleet;}
	PortDock * get_destination(Editor_Game_Base & egbase);

	virtual Type get_bob_type() const throw ();

	Economy * get_economy() const {return m_economy;}
	void set_economy(Game &, Economy * e);
	void set_destination(Game &, PortDock &);

	void init_auto_task(Game &);

	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

	void start_task_ship(Game &);
	void start_task_movetodock(Game &, PortDock &);

	virtual void log_general_info(Editor_Game_Base const &);

	uint32_t get_capacity() const {return descr().get_capacity();}
	uint32_t get_nritems() const {return m_items.size();}
	const ShippingItem & get_item(uint32_t idx) const {return m_items[idx];}

	void withdraw_items(Game & game, PortDock & pd, std::vector<ShippingItem> & items);
	void add_item(Game &, const ShippingItem & item);

	void show_window(Interactive_GameBase & igb);
	void close_window();

private:
	friend struct Fleet;
	friend struct ShipWindow;

	void wakeup_neighbours(Game &);

	static const Task taskShip;

	void ship_update(Game &, State &);
	void ship_wakeup(Game &);

	void ship_update_idle(Game &, State &);

	void init_fleet(Editor_Game_Base &);
	void set_fleet(Fleet * fleet);

	UI::Window * m_window;

	Fleet * m_fleet;
	Economy * m_economy;
	OPtr<PortDock> m_lastdock;
	OPtr<PortDock> m_destination;
	std::vector<ShippingItem> m_items;

	// saving and loading
protected:
	struct Loader : Bob::Loader {
		Loader();

		virtual const Task * get_task(const std::string & name);

		void load(FileRead & fr, uint8_t version);
		virtual void load_pointers();
		virtual void load_finish();

	private:
		uint32_t m_lastdock;
		uint32_t m_destination;
		std::vector<ShippingItem::Loader> m_items;
	};

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif // SHIP_H
