/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __WARE_DESCR_H
#define __WARE_DESCR_H

class Economy;
class PlayerImmovable;
class Request;
class Route;
class Tribe_Descr;
class Worker_Descr;


/*
Wares
-----

Wares can be stored in warehouses. They can be transferred across an Economy.
They can be traded.
Both items (lumber, stone, ...) and workers are considered wares.
Every ware has a unique name. Note that an item must not have the same name as
a worker.

Item wares are defined on a per-world basis, workers are defined on a per-tribe
basis.
Since the life-times of world and tribe descriptions are a bit dodgy, the
master list of wares is kept by the Game class. The list is created just before
the game starts.

Note that multiple tribes can define a worker with the same name. The different
"version" of a worker must perform the same job, but they can look differently.
*/
#define WARE_MENU_PIC_W		24
#define WARE_MENU_PIC_H		24

class Ware_Descr {
public:
	Ware_Descr(const char *name);
	virtual ~Ware_Descr();

	virtual void load_graphics();

	virtual bool is_worker() = 0;

	inline const char *get_name() const { return m_name; }

private:
	char		m_name[30];
};

class Item_Ware_Descr : public Ware_Descr {
public:
	Item_Ware_Descr(const char *name);
	virtual ~Item_Ware_Descr();

	virtual void load_graphics();

	virtual bool is_worker();

	inline uint get_menu_pic() { return m_menu_pic; }
	inline uint get_idle_anim() { return m_idle_anim; }

private:
	void parse(const char *directory, Profile *prof);

private:
	std::string		m_descname;
	std::string		m_helptext;
	std::string		m_menu_pic_fname;
	uint				m_menu_pic;
	uint				m_idle_anim;

public:
	static Item_Ware_Descr* create_from_dir(const char* name, const char* directory);
};


class Worker_Ware_Descr : public Ware_Descr {
public:
	Worker_Ware_Descr(const char *name);
	virtual ~Worker_Ware_Descr();

	virtual bool is_worker();

	Worker_Descr *get_worker(Tribe_Descr *tribe);
	void add_worker(Tribe_Descr *tribe, Worker_Descr *worker);

private:
	typedef std::map<Tribe_Descr*,Worker_Descr*> Worker_map;

	Worker_map	m_workers;
};


/*
WareList is a simple wrapper around an array of ware types.
It is useful for warehouses and for economy-wide inventory.
*/
class WareList {
public:
	WareList();
	~WareList();

	WareList &operator=(const WareList &wl);

	void clear();

	inline int get_nrwareids() const { return m_wares.size(); } // highest possible ware id

	void add(int id, int count = 1);
	void add(const WareList &wl);
	void remove(int id, int count = 1);
	void remove(const WareList &wl);
	int stock(int id) const;

private:
	std::vector<int>	m_wares;

	friend bool operator==(const WareList &wl1, const WareList &wl2);
};

bool operator==(const WareList &wl1, const WareList &wl2);
bool operator!=(const WareList &wl1, const WareList &wl2);


/*
WareInstance
------------
WareInstance represents one item while it is being carried around.

The WareInstance never draws itself; the carrying worker or the current flag
location are responsible for that.
*/
class WareInstance {
public:
	enum {
		State_None = 0,

		State_Request,		// moving to fulfill a request
		State_Idle,			// waiting on the nearest flag
		State_Return,		// return to warehouse
	};

public:
	WareInstance(int ware);
	~WareInstance();

	PlayerImmovable* get_location(Editor_Game_Base* g) { return (PlayerImmovable*)m_location.get(g); }
	int get_ware() const { return m_ware; }
	Item_Ware_Descr* get_ware_descr() const { return m_ware_descr; }
	Route* get_route() { return m_route; }

	void init(Game* g);
	void cleanup(Game* g);

	void remove_from_economy(Economy* e);
	void add_to_economy(Economy* e);

	void set_location(PlayerImmovable* loc);

	void set_state_request(Game* g, Request* rq, const Route* route);
	void end_state_request(Game* g);
	void set_state_idle(Game* g);

	bool is_moving(Game* g);
	PlayerImmovable* get_next_move_step(Game* g);
	PlayerImmovable* get_final_move_step(Game* g);

	void arrived(Game* g);
	void cancel(Game* g);

private:
	Object_Ptr			m_location;
	int					m_ware;
	Item_Ware_Descr*	m_ware_descr;

	int		m_state;
	Route*	m_route;

	Request*	m_request;	// State_Request: the request we need to fulfill
};


#endif
