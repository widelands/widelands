/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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
	inline uint get_pic_queue_full() { return m_pic_queue_full; }
	inline uint get_pic_queue_empty() { return m_pic_queue_empty; }

private:
	void parse(const char *directory, Profile *prof);

private:
	std::string		m_descname;
	std::string		m_helptext;
	std::string		m_menu_pic_fname;
	std::string		m_pic_queue_full_fname;
	std::string		m_pic_queue_empty_fname;
	
	uint				m_menu_pic;
	uint				m_idle_anim;

	uint				m_pic_queue_full;
	uint				m_pic_queue_empty;

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


#endif
