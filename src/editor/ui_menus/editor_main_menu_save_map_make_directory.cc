/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "editor_main_menu_save_map_make_directory.h"

#include "constants.h"
#include "i18n.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"


Main_Menu_Save_Map_Make_Directory::Main_Menu_Save_Map_Make_Directory(UI::Panel* parent, const char* dirname) :
UI::Window(parent, 0, 0, 230, 120, _("Make Directory").c_str())
{
	int32_t const spacing =  5;
	int32_t const offsx   = spacing;
	int32_t const offsy   = 30;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	new UI::Textarea
		(this, spacing, posy, _("Enter Directory Name: "), Align_Left);
	posy += 20 + spacing;

	m_edit =
		new UI::Edit_Box
		(this, spacing, posy, get_inner_w() - 2 * spacing, 20, 1, 0);
	m_edit->set_text(dirname);
	m_dirname = dirname;
	m_edit->changed.set(this, &Main_Menu_Save_Map_Make_Directory::edit_changed);

	posx = 5;
	posy = get_inner_h() - 30;

	m_ok_button = new UI::IDButton<Main_Menu_Save_Map_Make_Directory, int32_t>
		(this,
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 0,
		 &Main_Menu_Save_Map_Make_Directory::end_modal, this, 1,
		 _("OK"),
		 std::string(),
		 m_dirname.size());

	new UI::IDButton<Main_Menu_Save_Map_Make_Directory, int32_t>
		(this,
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 1,
		 &Main_Menu_Save_Map_Make_Directory::end_modal, this, 0,
	  _("Cancel"));

	center_to_parent();
}

/*
 * handle mouseclick for a modal
 * We are not draggable.
 */
bool Main_Menu_Save_Map_Make_Directory::handle_mousepress
(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Main_Menu_Save_Map_Make_Directory::handle_mouserelease
(const Uint8, int32_t, int32_t)
{return false;}

/*
 * Editbox changed
 */
void Main_Menu_Save_Map_Make_Directory::edit_changed() {
	std::string const & text = m_edit->get_text();
	if (text.size()) {
		m_ok_button->set_enabled(true);
		m_dirname=text;
	}
}
