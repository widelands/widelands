/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_save_map_make_directory.h"

#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

Main_Menu_Save_Map_Make_Directory::Main_Menu_Save_Map_Make_Directory
	(UI::Panel * const parent, char const * dirname)
:
UI::Window(parent, "make_directory", 0, 0, 230, 120, _("Make Directory"))
{
	int32_t const spacing =  5;
	int32_t const offsy   = 30;
	int32_t       posy    = offsy;

	new UI::Textarea(this, spacing, posy, _("Enter Directory Name: "));
	posy += 20 + spacing;

	m_edit =
		new UI::EditBox
			(this, spacing, posy, get_inner_w() - 2 * spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_edit->setText(dirname);
	m_dirname = dirname;
	m_edit->changed.connect(boost::bind(&Main_Menu_Save_Map_Make_Directory::edit_changed, this));

	posy = get_inner_h() - 30;

	m_ok_button = new
		UI::Button
		(this, "ok",
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 g_gr->images().get("pics/but0.png"),
		 _("OK"),
		 std::string(),
		 m_dirname.size());
	m_ok_button->sigclicked.connect
		(boost::bind(&Main_Menu_Save_Map_Make_Directory::end_modal, boost::ref(*this), 1));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect
		(boost::bind(&Main_Menu_Save_Map_Make_Directory::end_modal, boost::ref(*this), 0));

	center_to_parent();
}


/**
 * Editbox changed
 */
void Main_Menu_Save_Map_Make_Directory::edit_changed() {
	const std::string & text = m_edit->text();
	if (text.size()) {
		m_ok_button->set_enabled(true);
		m_dirname = text;
	}
}
