/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

class Editor_Game_Base;
class Map;
class Map_Loader;
namespace UI {
struct Button;
struct Multiline_Textarea;
struct Textarea;
};

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base {
	Editor_Game_Base		*egbase;

	UI::Listselect<const char * const> *list;
	UI::Textarea *taname;
	UI::Textarea *taauthor;
	UI::Textarea *tasize;
	UI::Textarea *taworld;
	UI::Textarea *tanplayers;
	UI::Multiline_Textarea *tadescr;
	UI::Button *m_ok;

	filenameset_t	m_mapfiles;

	Map_Loader**		m_ml;
	Map*              m_map;
   bool              m_is_scenario;

public:
	Fullscreen_Menu_MapSelect(Editor_Game_Base *g, Map_Loader**);
	~Fullscreen_Menu_MapSelect();

	const char * get_mapname() const throw () {return list->get_selection();}

	void ok();
	void map_selected  (uint);
   void changed(bool);
	void double_clicked(uint);
   void fill_list(void);

   std::string m_curdir;
   std::string m_basedir;
   std::string m_parentdir;
};


#endif
