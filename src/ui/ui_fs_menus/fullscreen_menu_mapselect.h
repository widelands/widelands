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

#ifndef __S__MAPSELECTMENUE_H
#define __S__MAPSELECTMENUE_H

#include "fullscreen_menu_base.h"
class Map_Loader;
class Map;

class Editor_Game_Base;

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base {
	Editor_Game_Base		*egbase;

	Listselect *list;
	Textarea *taname;
	Textarea *taauthor;
	Textarea *tasize;
	Textarea *taworld;
	Textarea *tanplayers;
	Multiline_Textarea *tadescr;
	Button *m_ok;

	filenameset_t	m_mapfiles;
	
	Map_Loader*			m_maploader;
	Map*              m_map;
   
public:
	Fullscreen_Menu_MapSelect(Editor_Game_Base *g);
	~Fullscreen_Menu_MapSelect();

	const char *get_mapname() { return (const char*)list->get_selection(); }

	void ok();
	void map_selected(int id);
};


#endif
