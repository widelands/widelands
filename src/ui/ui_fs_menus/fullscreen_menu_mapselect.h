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

#include "filesystem.h"
#include "fullscreen_menu_base.h"

#include "ui_checkbox.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"

namespace Widelands {
class Editor_Game_Base;
class Map;
class Map_Loader;
};

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
class Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base
{
	Widelands::Editor_Game_Base                * egbase;

	UI::Textarea                                 title;
	UI::Textarea                                 label_load_map_as_scenario;
	UI::Checkbox                                 load_map_as_scenario;
	UI::Listselect<const char *>                 list;
	UI::Textarea                                 label_name;
	UI::Textarea                                 taname;
	UI::Textarea                                 label_author;
	UI::Textarea                                 taauthor;
	UI::Textarea                                 label_size;
	UI::Textarea                                 tasize;
	UI::Textarea                                 label_world;
	UI::Textarea                                 taworld;
	UI::Textarea                                 label_nplayers;
	UI::Textarea                                 tanplayers;
	UI::Textarea                                 label_descr;
	UI::Multiline_Textarea                       tadescr;
	UI::IDButton<Fullscreen_Menu_MapSelect, int32_t> back;
	UI::Button<Fullscreen_Menu_MapSelect>        m_ok;

	filenameset_t                                m_mapfiles;

	Widelands::Map_Loader *                    * m_ml;
	Widelands::Map                             * m_map;
	bool              m_is_scenario;

public:
	Fullscreen_Menu_MapSelect
		(Widelands::Editor_Game_Base *, Widelands::Map_Loader * *);
	~Fullscreen_Menu_MapSelect();

	const char * get_mapname() const throw () {return list.get_selected();}

	void ok();
	void map_selected  (uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();

	std::string m_curdir;
	std::string m_basedir;
	std::string m_parentdir;
};


#endif
