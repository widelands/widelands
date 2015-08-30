/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_SAVE_MAP_H
#define WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_SAVE_MAP_H

#include "io/filesystem/filesystem.h"
#include "ui_basic/window.h"

struct EditorInteractive;
namespace UI {
struct Button;
struct EditBox;
template <typename T> struct Listselect;
struct MultilineTextarea;
struct Textarea;
}

/**
 * Choose a filename and save your brand new created map
*/
struct MainMenuSaveMap : public UI::Window {
	MainMenuSaveMap(EditorInteractive &);

private:
	EditorInteractive & eia();
	void clicked_ok();
	void clicked_make_directory();
	void        clicked_item(uint32_t);
	void double_clicked_item(uint32_t);
	void edit_box_changed();

	void fill_list();
	bool save_map(std::string, bool);

	UI::EditBox * m_editbox;
	UI::MultilineTextarea * m_name;
	UI::Textarea * m_author, * m_size, * m_nrplayers;
	UI::MultilineTextarea * m_descr;
	UI::Listselect<const char *> * m_ls;
	UI::Button * m_ok_btn;

	std::string   m_basedir;
	std::string   m_curdir;
	std::string   m_parentdir;
	FilenameSet m_mapfiles;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_SAVE_MAP_H
