/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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

#include <string>
#include <vector>

#include "instances.h"

class Economy;
class PlayerImmovable;
class Profile;
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

class Item_Ware_Descr : public Map_Object_Descr {
public:
	Item_Ware_Descr(const char *name);
	virtual ~Item_Ware_Descr();

	virtual void load_graphics();

	uint get_menu_pic       () const {return m_menu_pic;}
	uint get_pic_queue_full () const {return m_pic_queue_full;}
	uint get_pic_queue_empty() const {return m_pic_queue_empty;}

   inline const char *get_name() const { return m_name.c_str(); }
   inline const char *get_descname() const { return m_descname.c_str(); }

private:
	void parse(const char *directory, Profile *prof);

private:
   std::string    m_name;
   std::string		m_descname;
	std::string		m_helptext;
	std::string		m_menu_pic_fname;
	std::string		m_pic_queue_full_fname;
	std::string		m_pic_queue_empty_fname;

	uint				m_menu_pic;

	uint				m_pic_queue_full;
	uint				m_pic_queue_empty;

public:
	static Item_Ware_Descr* create_from_dir(const char* name, const char* directory);
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

   void set_nrwares( int i ) { assert(m_wares.size()==0); m_wares.resize(i, 0); }

private:
	std::vector<int>	m_wares;

	friend bool operator==(const WareList &wl1, const WareList &wl2);
};

bool operator==(const WareList &wl1, const WareList &wl2);
bool operator!=(const WareList &wl1, const WareList &wl2);


#endif
