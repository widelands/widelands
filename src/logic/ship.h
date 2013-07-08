/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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
#include <list>

namespace UI {class Window;}
class Interactive_GameBase;

namespace Widelands {

class Economy;
struct Fleet;
struct PortDock;

struct Ship_Descr : Bob::Descr {
	Ship_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const Tribe_Descr &);

	virtual uint32_t movecaps() const throw ();
	const DirAnimations & get_sail_anims() const {return m_sail_anims;}

	uint32_t get_capacity() const throw () {return m_capacity;}
	uint32_t vision_range() const throw () {return m_vision_range;}

	virtual Bob & create_object() const;

private:
	DirAnimations m_sail_anims;
	uint32_t m_capacity;
	uint32_t m_vision_range;
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
	void start_task_expedition(Game &);

	virtual void log_general_info(const Editor_Game_Base &);

	uint32_t get_capacity() const {return descr().get_capacity();}
	virtual uint32_t vision_range() const throw () {return descr().vision_range();}
	uint32_t get_nritems() const {return m_items.size();}
	const ShippingItem & get_item(uint32_t idx) const {return m_items[idx];}

	void withdraw_items(Game & game, PortDock & pd, std::vector<ShippingItem> & items);
	void add_item(Game &, const ShippingItem & item);

	void show_window(Interactive_GameBase &, bool avoid_fastclick = false);
	void close_window();
	void refresh_window(Interactive_GameBase &);

	// A ship with task expedition can be in four states: EXP_WAITING, EXP_SCOUTING,
	// EXP_FOUNDPORTSPACE or EXP_COLONIZING in the first states, the owning player of this ship can
	// give direction change commands to change the direction of the moving ship / send the ship in a
	// direction. Once the ship is on its way, it is in EXP_SCOUTING state. In the backend, a click
	// on a direction begins to the movement into that direction until a coast is reached or the user
	// cancels the direction through a direction change.
	//
	// The EXP_WAITING state means, that an event happend and thus the ship stopped and waits for a
	// new command by the owner. An event leading to a EXP_WAITING state can be:
	// * expedition is ready to start
	// * new island appeared in vision range (only outer ring of vision range has to be checked due to the
	//   always ongoing movement).
	// * island was completely surrounded
	//
	// The EXP_FOUNDPORTSPACE state means, that a port build space was found.
	//
	enum {
		TRANSPORT          = 0,
		EXP_WAITING        = 1,
		EXP_SCOUTING       = 2,
		EXP_FOUNDPORTSPACE = 3,
		EXP_COLONIZING     = 4
	};

	/// \returns the current state the ship is in
	uint8_t get_ship_state() {return m_ship_state;}

	/// \returns (in expedition mode only!) whether the next field in direction \arg dir is swimable
	bool exp_dir_swimable(Direction dir) {
		if (m_ship_state == TRANSPORT)
			return false;
		assert(m_expedition);
		return m_expedition->swimable[dir - 1];
	}

	/// \returns whether the expedition ship is close to the coast
	bool exp_close_to_coast() {
		if (m_ship_state == TRANSPORT)
			return false;
		assert(m_expedition);
		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir)
			if (!m_expedition->swimable[dir - 1])
				return true;
		return false;
	}

	/// \returns (in expedition mode only!) the list of currently seen port build spaces
	const std::list<Coords>* exp_port_spaces() {
		if (m_ship_state == TRANSPORT)
			return 0;
		assert(m_expedition);
		return m_expedition->seen_port_buildspaces.get();
	}

	void exp_scout_direction(Game &, uint8_t);
	void exp_construct_port (Game &, const Coords&);
	void exp_explore_island (Game &, bool);

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

	void send_message
		(Game &, const std::string &, const std::string &, const std::string &, const std::string &);

	UI::Window * m_window;

	Fleet   * m_fleet;
	Economy * m_economy;
	OPtr<PortDock> m_lastdock;
	OPtr<PortDock> m_destination;
	std::vector<ShippingItem> m_items;
	uint8_t m_ship_state;

	struct Expedition {
		boost::scoped_ptr<std::list<Coords> > seen_port_buildspaces;
		bool swimable[LAST_DIRECTION];
		bool island_exploration;
		uint8_t direction;
		Coords exploration_start;
		bool clockwise;
	};
	boost::scoped_ptr<Expedition> m_expedition;

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
		uint8_t  m_ship_state;
		boost::scoped_ptr<Expedition> m_expedition;
		std::vector<ShippingItem::Loader> m_items;
	};

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif // SHIP_H
