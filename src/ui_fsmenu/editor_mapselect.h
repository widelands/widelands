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

#ifndef WL_UI_FSMENU_EDITOR_MAPSELECT_H
#define WL_UI_FSMENU_EDITOR_MAPSELECT_H

#include "ui_fsmenu/base.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/load_map_or_game.h"

/**
 * Select a Map in Fullscreen Mode. It's a modal fullscreen menu
 */
struct FullscreenMenuEditorMapSelect : public FullscreenMenuLoadMapOrGame {
	FullscreenMenuEditorMapSelect();

	std::string get_map();

private:
	void ok();
	void map_selected(uint32_t);
	void changed(bool);
	void double_clicked(uint32_t);
	void fill_list();

	UI::Textarea                  m_title;
	UI::Textarea                  m_label_mapname;
	UI::MultilineTextarea         m_ta_mapname;
	UI::Textarea                  m_label_author;
	UI::MultilineTextarea         m_ta_author;
	UI::Textarea                  m_label_size;
	UI::MultilineTextarea         m_ta_size;
	UI::Textarea                  m_label_players;
	UI::MultilineTextarea         m_ta_players;
	UI::Textarea                  m_label_description;
	UI::MultilineTextarea         m_ta_description;

	UI::Listselect<std::string>   m_list;
	std::string                   m_parentdir;
	std::string                   m_curdir;
	std::string                   m_basedir;

	FilenameSet                   m_mapfiles;
};

#endif  // end of include guard: WL_UI_FSMENU_EDITOR_MAPSELECT_H
