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
#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/table.h"
#include "ui_basic/window.h"

struct EditorInteractive;
namespace UI {
struct Button;
struct EditBox;
template <typename T> struct Listselect;
}

/**
 * Choose a filename and save your brand new created map
*/
struct MainMenuSaveMap : public UI::Window {
	MainMenuSaveMap(EditorInteractive &);

private:
	struct SaveMapData {
		std::string filename;
		std::string mapname;
		std::string localized_name;
		std::string description;
		std::string authors;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t nrplayers = 0;
		bool isdir = false;
	};

	bool compare_filenames(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);

	EditorInteractive & eia();
	void clicked_ok();
	void clicked_make_directory();
	void clicked_item();
	void double_clicked_item();
	void edit_box_changed();

	void fill_list();
	bool save_map(std::string, bool);

	const int padding_;
	const int butw_, buth_;
	const int details_box_x_, details_box_y_;
	const int details_box_w_, details_box_h_;
	const int details_label_w_;

	UI::Box details_box_;
	UI::Box name_box_;
	UI::Textarea name_label_;
	UI::MultilineTextarea name_;

	UI::Box author_box_;
	UI::Textarea author_label_, author_;

	UI::Box size_box_;
	UI::Textarea size_label_, size_;

	UI::Box nrplayers_box_;
	UI::Textarea nrplayers_label_, nrplayers_;

	UI::Box descr_box_;
	UI::Textarea descr_label_;
	UI::MultilineTextarea descr_;

	UI::Box list_box_;

	UI::Textarea editbox_label_;
	UI::EditBox* editbox_;

	UI::Table<uintptr_t const>    m_table;

	UI::Button * m_ok_btn;

	std::string   m_basedir;
	std::string   m_curdir;

	std::vector<SaveMapData> m_maps_data;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_SAVE_MAP_H
