/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_MAPSELECT_H
#define FULLSCREEN_MENU_MAPSELECT_H

#include "fullscreen_menu_base.h"

#include <string>


/**
 * Data about a map that we're interested in.
 */
struct MapData {
	std::string filename;
	std::string name;
	std::string author;
	std::string description;
	std::string world;
	uint32_t width;
	uint32_t height;
	uint32_t nrplayers;

	MapData()
		: width(0), height(0), nrplayers(0) {}
};

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
struct Fullscreen_Menu_MapSelectImpl;

class Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base
{
public:
	Fullscreen_Menu_MapSelect();
	~Fullscreen_Menu_MapSelect();

	bool is_scenario();
	const MapData* get_map() const;

private:
	void ok();
	void map_selected(uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();

	Fullscreen_Menu_MapSelectImpl* d;
};

#endif
