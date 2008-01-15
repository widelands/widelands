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
#include "transport.h"

struct EncodeData;
struct Interactive_Player;
struct Profile;

namespace Widelands {

class Editor_Game_Base;
class Soldier;
class Tribe_Descr;
class WareInstance;
class WareList;


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

	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
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

	void mark_as_used (Game* g, int32_t ware, Requeriments* r);
	Soldier* launch_soldier(Game* g, int32_t ware, Requeriments* req);
	Worker* launch_worker(Game* g, int32_t ware);
	void incorporate_worker(Game *g, Worker *w);

	WareInstance & launch_item(Game *, int32_t ware);
	void do_launch_item(Game *, WareInstance &);
	void incorporate_item(Game* g, WareInstance* item);

	int32_t get_soldiers_passing (Game*, int32_t, Requeriments*);
	bool can_create_worker(Game *, int32_t worker);
	void create_worker(Game *, int32_t worker);

   /// Military stuff
   virtual bool has_soldiers();
   virtual void conquered_by (Player*);

protected:
	virtual UI::Window *create_options_window(Interactive_Player *plr, UI::Window **registry);

private:
	static void idle_request_cb(Game* g, Request* rq, int32_t ware, Worker* w, void* data);
   void sort_worker_in(Editor_Game_Base*, std::string, Worker*);

	WarehouseSupply       * m_supply;
	std::vector<Request *>  m_requests; // one idle request per ware type
   std::vector<Object_Ptr> m_incorporated_workers; // Workers who live here at the moment
	int32_t                     m_next_carrier_spawn; //  time of next carrier growth
   int32_t                     m_next_military_act; // time of next military action
};

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
struct WarehouseSupply : public Supply {
	WarehouseSupply(Warehouse * const wh) : m_economy(0), m_warehouse(wh) {}
	virtual ~WarehouseSupply();

	void set_economy(Economy* e);

   void set_nrworkers(int32_t i);
   void set_nrwares(int32_t i);


	const WareList &get_wares() const {return m_wares;}
	const WareList &get_workers() const {return m_workers;}
	int32_t stock_wares(int32_t id) const {return m_wares.stock(id);}
	int32_t stock_workers(int32_t id) const {return m_workers.stock(id);}
	void add_wares(int32_t id, int32_t count);
	void remove_wares(int32_t id, int32_t count);
   void add_workers(int32_t id, int32_t count);
   void remove_workers(int32_t id, int32_t count);

	// Supply implementation
	virtual PlayerImmovable* get_position(Game* g);
	virtual int32_t get_amount(const int32_t ware) const;
	virtual bool is_active() const throw ();

	virtual WareInstance & launch_item(Game *, int32_t ware);
	virtual Worker* launch_worker(Game* g, int32_t ware);

	virtual Soldier* launch_soldier(Game* g, int32_t ware, Requeriments* req);
	virtual int32_t get_passing_requeriments(Game* g, int32_t ware, Requeriments* r);
	virtual void mark_as_used (Game* g, int32_t ware, Requeriments* r);
private:
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
	Warehouse * m_warehouse;
};

};

#endif
