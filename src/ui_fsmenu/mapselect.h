/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

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
	bool scenario; // whether this map can be played as scenario

	MapData()
		: width(0), height(0), nrplayers(0) {}
};

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */

struct Fullscreen_Menu_MapSelect : public Fullscreen_Menu_Base {
	Fullscreen_Menu_MapSelect();

	bool is_scenario();
	MapData const * get_map() const;
	void setScenarioSelectionVisible(bool);

private:
	void ok();
	void map_selected(uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();

	uint32_t     m_butw;
	uint32_t     m_buth;
	uint32_t     m_fs;
	std::string  m_fn;
	UI::Textarea m_title;
	UI::Textarea m_label_load_map_as_scenario;
	UI::Textarea m_label_name,       m_name;
	UI::Textarea m_label_author,     m_author;
	UI::Textarea m_label_size,       m_size;
	UI::Textarea m_label_world,      m_world;
	UI::Textarea m_label_nr_players, m_nr_players;
	UI::Textarea m_label_descr;
	UI::Multiline_Textarea m_descr;
	UI::Callback_IDButton<Fullscreen_Menu_MapSelect, int32_t> m_back;
	UI::Callback_Button<Fullscreen_Menu_MapSelect>            m_ok;
	UI::Checkbox                                     m_load_map_as_scenario;
	UI::Listselect<MapData>                          m_list;
	std::string                                      m_curdir, m_basedir;
};

#endif
