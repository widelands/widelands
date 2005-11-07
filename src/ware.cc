/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <stdio.h>
#include "animation.h"
#include "error.h"
#include "graphic.h"
#include "profile.h"
#include "tribe.h"
#include "ware.h"
#include "wexception.h"
#include "worker.h"
#include "system.h"

/*
==============================================================================

Item_Ware_Descr IMPLEMENTATION

==============================================================================
*/

/*
===============
Item_Ware_Descr::Item_Ware_Descr
Item_Ware_Descr::~Item_Ware_Descr
===============
*/
Item_Ware_Descr::Item_Ware_Descr(const char *name)
{
   m_name=name;
   m_menu_pic = 0;
	m_pic_queue_full = 0;
	m_pic_queue_empty = 0;
}

Item_Ware_Descr::~Item_Ware_Descr()
{
}


/*
===============
Item_Ware_Descr::parse

Parse the item data from the given profile.
===============
*/
void Item_Ware_Descr::parse(const char *directory, Profile *prof)
{
	char buf[256];
	const char* string;
	Section* global = prof->get_safe_section("global");

	m_descname = global->get_string("descname", get_name());
	m_helptext = global->get_string("help", _("Doh... someone forgot the help text!"));

	snprintf(buf, sizeof(buf),	"%s_menu.png", get_name());
	string = global->get_string("menu_pic", buf);
	snprintf(buf, sizeof(buf), "%s/%s", directory, string);
	m_menu_pic_fname = buf;

	snprintf(buf, sizeof(buf), "%s_queue_full.png", get_name());
	string = global->get_string("queue_full_pic", buf);
	snprintf(buf, sizeof(buf), "%s/%s", directory, string);
	m_pic_queue_full_fname = buf;

	snprintf(buf, sizeof(buf), "%s_queue_empty.png", get_name());
	string = global->get_string("queue_empty_pic", buf);
	snprintf(buf, sizeof(buf), "%s/%s", directory, string);
	m_pic_queue_empty_fname = buf;

	add_animation("idle", g_anim.get(directory, prof->get_safe_section("idle")));
}


/*
===============
Item_Ware_Descr::load_graphics
===============
*/
void Item_Ware_Descr::load_graphics()
{
	m_menu_pic = g_gr->get_picture( PicMod_Game,  m_menu_pic_fname.c_str() );
	m_pic_queue_full = g_gr->get_picture( PicMod_Game,  m_pic_queue_full_fname.c_str() );
	m_pic_queue_empty = g_gr->get_picture( PicMod_Game,  m_pic_queue_empty_fname.c_str() );
}

/*
===============
Item_Ware_Descr::create_from_dir [static]

Create a new Item_Ware_Descr from the given data.
Throws an exception if something goes wrong.
===============
*/
Item_Ware_Descr* Item_Ware_Descr::create_from_dir(const char* name, const char* directory)
{
	Item_Ware_Descr* descr = new Item_Ware_Descr(name);

	try
	{
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", directory);

		Profile prof(fname);

		descr->parse(directory, &prof);
	}
	catch(...)
	{
		delete descr;
		throw;
	}

	return descr;
}


/*
==============================================================================

WareList IMPLEMENTATION

==============================================================================
*/

/*
===============
WareList::WareList

Zero-initialize the storage.
===============
*/
WareList::WareList()
{
}

/*
===============
WareList::~WareList

Delete the list. Print a warning message if the storage is not empty.
This is because most of the time, a WareList should be zeroed by cleanup
operations before the destructor is called. If you are sure of what you're
doing, call clear().
===============
*/
WareList::~WareList()
{
	for(uint id = 0; id < m_wares.size(); id++) {
		if (m_wares[id])
			log("WareList: %i items of %i left.\n", m_wares[id], id);
	}
}

/*
===============
WareList::operator=

Assignment operator
===============
*/
WareList &WareList::operator=(const WareList &wl)
{
	m_wares = wl.m_wares;
	return *this;
}

/*
===============
WareList::clear

Clear the storage.
===============
*/
void WareList::clear()
{
	m_wares.clear();
}

/*
===============
WareList::add

Add the given number of items (default = 1) to the storage.
===============
*/
void WareList::add(int id, int count)
{
	if (!count)
		return;

	assert(id >= 0);

	if (id >= (int)m_wares.size())
		m_wares.resize(id+1, 0);
	m_wares[id] += count;
	assert(m_wares[id] >= count);
}

void WareList::add(const WareList &wl)
{
	if (wl.m_wares.size() > m_wares.size())
		m_wares.reserve(wl.m_wares.size());

	for(uint id = 0; id < wl.m_wares.size(); id++)
		if (wl.m_wares[id])
			add(id, wl.m_wares[id]);
}

/*
===============
WareList::remove

Remove the given number of items (default = 1) from the storage.
===============
*/
void WareList::remove(int id, int count)
{
	if (!count)
		return;

	assert(id >= 0 && id < (int)m_wares.size());
	log("WareList: Wanting to remove %i/%i wares of type (%i)\n", count, m_wares[id], id);
   assert(m_wares[id] >= count);
	m_wares[id] -= count;
}

void WareList::remove(const WareList &wl)
{
	for(uint id = 0; id < wl.m_wares.size(); id++)
		if (wl.m_wares[id])
			remove(id, wl.m_wares[id]);
}

/*
===============
WareList::stock

Return the number of wares of a given type stored in this storage.
===============
*/
int WareList::stock(int id) const
{
	assert(id >= 0);

	if (id >= (int)m_wares.size())
		return 0;
	return m_wares[id];
}

/*
===============
operator==
operator!=

Two WareLists are only equal when they contain the exact same stock of
of all wares types.
===============
*/
bool operator==(const WareList &wl1, const WareList &wl2)
{
	uint i = 0;

	while(i < wl1.m_wares.size()) {
		int count = wl1.m_wares[i];
		if (i < wl2.m_wares.size()) {
			if (count != wl2.m_wares[i])
				return false;
		} else {
			if (count) // wl2 has 0 stock per definition
				return false;
		}
		i++;
	}

	while(i < wl2.m_wares.size()) {
		if (wl2.m_wares[i]) // wl1 has 0 stock per definition
			return false;
		i++;
	}

	return true;
}

bool operator!=(const WareList &wl1, const WareList &wl2)
{
	return !(wl1 == wl2);
}

