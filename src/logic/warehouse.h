/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "attackable.h"
#include "building.h"

struct EncodeData;
struct Interactive_Player;
struct Profile;

namespace Widelands {

struct Editor_Game_Base;
struct Request;
struct Requirements;
class Soldier;
struct Tribe_Descr;
class WareInstance;
struct WareList;


/*
Warehouse
*/
class WarehouseSupply;

struct Warehouse_Descr : public Building_Descr {
	Warehouse_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 Tribe_Descr const &, EncodeData const *);

	virtual Building & create_object() const;

	virtual uint32_t get_conquers() const {return m_conquers;}

private:
	int32_t m_conquers;
};


class Warehouse : public Building, public Attackable {
	friend struct Map_Buildingdata_Data_Packet;

	MO_DESCR(Warehouse_Descr);

public:
	Warehouse(const Warehouse_Descr &);
	virtual ~Warehouse();

	void prefill
		(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *);
	void postfill
		(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *);

	char const * type_name() const throw () {return "warehouse";}
	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

	virtual void act(Game & game, uint32_t data);

	virtual void set_economy(Economy *);
	virtual int32_t get_priority
		(int32_t type, Ware_Index ware_index, bool adjust = true) const;
	void set_needed(Ware_Index, uint32_t value = 1);

	const WareList &get_wares() const;
	const WareList &get_workers() const;
	void insert_wares  (Ware_Index, uint32_t count);
	void remove_wares  (Ware_Index, uint32_t count);
	void insert_workers(Ware_Index, uint32_t count);
	void remove_workers(Ware_Index, uint32_t count);

	virtual bool fetch_from_flag(Game &);

	uint32_t count_workers(Game const &, Ware_Index, Requirements const &);
	Worker & launch_worker(Game &, Ware_Index, Requirements const &);
	void incorporate_worker(Game &, Worker &);

	WareInstance & launch_item(Game &, Ware_Index);
	void do_launch_item(Game &, WareInstance &);
	void incorporate_item(Game &, WareInstance &);

	bool can_create_worker(Game &, Ware_Index) const;
	void     create_worker(Game &, Ware_Index);

	// Begin Attackable implementation
	virtual bool canAttack();
	virtual void aggressor(Soldier &);
	virtual bool attack   (Soldier &);
	// End Attackable implementation

protected:

	/// Create the warehouse information window.
	virtual void create_options_window
		(Interactive_Player &, UI::Window * & registry);

private:
	static void idle_request_cb
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);
	void sort_worker_in(Editor_Game_Base &, std::string const &, Worker &);

	WarehouseSupply       * m_supply;
	std::vector<Request *>  m_requests; // one idle request per ware type

	// Workers who live here at the moment
	std::vector<Object_Ptr> m_incorporated_workers;
	int32_t                 m_next_carrier_spawn; //  time of next carrier growth
	int32_t                 m_next_military_act; // time of next military action

	/// how many do we want to store in this building
	std::vector<size_t> m_target_supply;
};

};

#endif
