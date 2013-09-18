/*
 * Copyright (C) 2008, 2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef FULLSCREEN_MENU_EDITOR_MAPSELECT_H
#define FULLSCREEN_MENU_EDITOR_MAPSELECT_H

#include "ui_fsmenu/base.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */

struct Fullscreen_Menu_Editor_MapSelect : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Editor_MapSelect();

	std::string get_map();

private:
	void ok();
	void map_selected(uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();

	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	UI::Textarea    m_title,            m_label_name,       m_name;
	UI::Textarea    m_label_author,     m_author,           m_label_size;
	UI::Textarea    m_size,             m_label_world,      m_world;
	UI::Textarea    m_label_nr_players, m_nr_players,       m_label_descr;
	UI::Multiline_Textarea              m_descr;
	UI::Button             m_back,             m_ok;
	UI::Listselect<std::string>                             m_list;
	std::string     m_parentdir,        m_curdir,           m_basedir;
	filenameset_t   m_mapfiles;
};

#endif
