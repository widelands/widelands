/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
	enum {
		Subtype_Normal,
		Subtype_HQ,
		Subtype_Port
	};

	Warehouse_Descr(const Tribe_Descr & tribe, const std::string & name);

	virtual void parse(char const * directory, Profile *, const EncodeData *);
	virtual Building * create_object() const;

	int32_t get_subtype() const {return m_subtype;}
	virtual uint32_t get_conquers() const {return m_conquers;}

private:
	int32_t m_subtype;
	int32_t m_conquers; //  HQs conquer
};


class Warehouse : public Building {
	friend struct Map_Buildingdata_Data_Packet;

	MO_DESCR(Warehouse_Descr);

public:
	Warehouse(const Warehouse_Descr &);
	virtual ~Warehouse();

	virtual int32_t get_building_type() const throw () {return Building::WAREHOUSE;}
	char const * type_name() const throw () {return "warehouse";}
	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

	virtual void act(Game *g, uint32_t data);

	virtual void set_economy(Economy *e);

	const WareList &get_wares() const;
	const WareList &get_workers() const;
	void insert_wares(int32_t id, int32_t count);
	void remove_wares(int32_t id, int32_t count);
	void insert_workers(int32_t id, int32_t count);
	void remove_workers(int32_t id, int32_t count);

	virtual bool fetch_from_flag(Game* g);

	uint32_t count_workers(Game* g, Ware_Index, const Requirements&);
	Worker* launch_worker(Game* g, Ware_Index, const Requirements&);
	void incorporate_worker(Game *g, Worker *w);

	WareInstance & launch_item(Game *, Ware_Index);
	void do_launch_item(Game *, WareInstance &);
	void incorporate_item(Game* g, WareInstance* item);

	bool can_create_worker(Game *, Ware_Index);
	void     create_worker(Game *, Ware_Index);

	/// Military stuff
	virtual bool has_soldiers();
	virtual void conquered_by (Player *);

protected:
	virtual UI::Window *create_options_window(Interactive_Player *plr, UI::Window **registry);

private:
	static void idle_request_cb
		(Game *, Request *, Ware_Index, Worker *, void * data);
	void sort_worker_in(Editor_Game_Base*, std::string, Worker*);

	WarehouseSupply       * m_supply;
	std::vector<Request *>  m_requests; // one idle request per ware type
	std::vector<Object_Ptr> m_incorporated_workers; // Workers who live here at the moment
	int32_t                 m_next_carrier_spawn; //  time of next carrier growth
	int32_t                 m_next_military_act; // time of next military action
};

};

#endif
