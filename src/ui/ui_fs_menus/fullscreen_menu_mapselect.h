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

#include "filesystem.h"
#include "fullscreen_menu_base.h"
#include "ui_listselect.h"

class Map;
class Map_Loader;
class UIButton;
class UIMultiline_Textarea;
class UITextarea;

class Editor_Game_Base;

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base {
	Editor_Game_Base		*egbase;

	UIListselect *list;
	UITextarea *taname;
	UITextarea *taauthor;
	UITextarea *tasize;
	UITextarea *taworld;
	UITextarea *tanplayers;
	UIMultiline_Textarea *tadescr;
	UIButton *m_ok;

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
