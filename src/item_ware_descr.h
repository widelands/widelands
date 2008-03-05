/*
 * Copyright (C) 2002-2003, 2006-2008 by the Widelands Development Team
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

#ifndef ITEM_WARE_DESCR_H
#define ITEM_WARE_DESCR_H

#include <string>
#include <cstring>

#include <stdint.h>

#define WARE_MENU_PIC_WIDTH   24  //< Default width for ware's menu icons
#define WARE_MENU_PIC_HEIGHT  24  //< Default height for ware's menu icons

namespace Widelands {

/**
 * Wares can be stored in warehouses. They can be transferred across an
 * Economy. They can be traded.
 * Both items (lumber, stone, ...) and workers are considered wares.
 * Every ware has a unique name. Note that an item must not have the same
 * name as a worker.
 *
 * Item wares are defined on a per-world basis, workers are defined on a
 * per-tribe basis.
 * Since the life-times of world and tribe descriptions are a bit dodgy, the
 * master list of wares is kept by the Game class. The list is created just
 * before the game starts.
 *
 * Note that multiple tribes can define a worker with the same name. The
 * different "version" of a worker must perform the same job, but they can
 * look differently.
*/
struct Item_Ware_Descr : public Map_Object_Descr {
	typedef Ware_Index::value_t Index;
	Item_Ware_Descr(const std::string & ware_name)
		: m_name(ware_name), m_icon(0)
	{}

	virtual ~Item_Ware_Descr() {};

	/// \return index to ware's icon inside picture stack
	const uint32_t get_icon() const throw () {return m_icon;}

	/// \return ware's unique name
	const std::string & name() const throw () {return m_name;}

	/// \return ware's localized short name
	const std::string & descname() const throw () {return m_descname;}

	/// \return ware's localized descriptive text
	const char * get_helptext() const throw () {return m_helptext.c_str();}

	virtual void load_graphics();
	static Item_Ware_Descr* create_from_dir(const char*, const char*);

private:
	void parse(const char *directory, Profile *prof);

	std::string m_name;       ///< Ware's unique name into tribe
	std::string m_descname;   ///< Short localized name
	std::string m_helptext;   ///< Long descriptive text
	std::string m_icon_fname; ///< Filename of ware's main picture
	uint32_t    m_icon;       ///< Index of ware's picture in picture stack
};

};

#endif
