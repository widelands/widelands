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

#include "editor_variables_menu.h"

#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

#include "upcast.h"

using Widelands::Variable;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct New_Variable_Window : public UI::Window {
	New_Variable_Window(Editor_Interactive &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

	Variable * get_variable() {return m_variable;}

private:
	Editor_Interactive    & m_parent;
	Variable           * m_variable;
	enum Variable_Type {Integer_Type, String_Type};
	UI::IDButton<New_Variable_Window, const Variable_Type> button_integer;
	UI::IDButton<New_Variable_Window, const Variable_Type> button_string;
	UI::IDButton<New_Variable_Window, int32_t>                 button_back;

	void clicked_new(Variable_Type);
};

New_Variable_Window::New_Variable_Window(Editor_Interactive & parent) :
UI::Window(&parent, 0, 0, 135, 55, _("New Variable")),

m_parent(parent),
m_variable(0),

button_integer
(this,
 5, 5, 60, 20,
 0,
 &New_Variable_Window::clicked_new, this, Integer_Type,
 _("Integer")),

button_string
(this,
 70, 5, 60, 20,
 0,
 &New_Variable_Window::clicked_new, this, String_Type,
 _("String")),

button_back
(this,
 (get_inner_w() - 80) / 2, 30, 80, 20,
 1,
 &New_Variable_Window::end_modal, this, 0,
 _("Back"))

{center_to_parent();}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool New_Variable_Window::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool New_Variable_Window::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/*
 * a button has been clicked
 */
void New_Variable_Window::clicked_new(const Variable_Type i) {
	char buffer[256];

	Manager<Variable> & mvm = m_parent.egbase().map().mvm();
	{
		uint32_t n = 1;
		do {
			snprintf(buffer, sizeof(buffer), _("Unnamed%u"), n);
			++n;
		} while (mvm[buffer]);
	}

	std::string name = buffer;
	switch (i) {
	case Integer_Type: m_variable = new Widelands::Variable_Int   (); break;
	case  String_Type: m_variable = new Widelands::Variable_String(); break;
	default:
		assert(false);
	}

	m_variable->set_name(buffer);
	mvm.register_new(*m_variable);
	end_modal(1);
	return;
}

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Edit_Variable_Window : public UI::Window {
	Edit_Variable_Window
		(Editor_Interactive &, UI::Table<Variable &>::Entry_Record &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive                     & m_parent;
	UI::Table<Variable &>::Entry_Record    & m_te;
	UI::Textarea                             m_label_name;
	UI::Edit_Box                             m_name;
	UI::Textarea                             m_label_value;
	UI::Edit_Box                             m_value;
	UI::Button<Edit_Variable_Window>         m_ok;
	UI::IDButton<Edit_Variable_Window, int32_t>  m_back;

	void clicked_ok();
};

#define spacing 5

Edit_Variable_Window::Edit_Variable_Window
(Editor_Interactive & parent, UI::Table<Variable &>::Entry_Record & te)
:
UI::Window(&parent, 0, 0, 250, 85, _("Edit Variable")),

m_parent(parent),
m_te(te),

//  what type
m_label_name(this, 5, 5, 120, 20, _("Name"), Align_CenterLeft),
m_name(this, 120, 5, 120, 20, 0, 0),

m_label_value(this, 5, 30, 120, 20, _("Value"), Align_CenterLeft),
m_value(this, 120, 35, 120, 20, 0, 0),

m_ok
(this,
 get_inner_w() / 2 - 80 - spacing, 60, 80, 20,
 1,
 &Edit_Variable_Window::clicked_ok, this,
 _("Ok")),
m_back
(this,
 get_inner_w() / 2 + spacing, 60, 80, 20,
 1,
 &Edit_Variable_Window::end_modal, this, 0,
 _("Back"))


{
	m_name .set_text(m_te.get_string(1).c_str());
	m_value.set_text(m_te.get_string(2).c_str());
	center_to_parent();
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Edit_Variable_Window::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Edit_Variable_Window::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/*
 * a button has been clicked
 */
void Edit_Variable_Window::clicked_ok() {
	//  Get the a name

	//  Extract value
	Variable & var = UI::Table<Variable &>::get(m_te);
	if (upcast(Widelands::Variable_Int, variable_int, &var)) {
		char * endp;
		int32_t const ivar = strtol(m_value.get_text(), &endp, 0);

		if (endp and *endp) {
			char buffer[1024];
			snprintf
				(buffer, sizeof(buffer),
				 _("\"%s\" is not a valid integer!"), m_value.get_text());
			UI::Modal_Message_Box mb
				(&m_parent, _("Parse error!"), buffer, UI::Modal_Message_Box::OK);
			mb.run();
			return;
		}
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%i", ivar);

		variable_int   ->set_value(ivar);
		m_te.set_string(2, buffer);
	} else if (upcast(Widelands::Variable_String, variable_string, &var)) {
		variable_string->set_value(m_value.get_text());
		m_te.set_string(2, m_value.get_text());
	} else
		assert(false);

	var.set_name(m_name.get_text());
	m_te.set_string(1, var.name());

	end_modal(1);
}


/*
===============
Editor_Variables_Menu::Editor_Variables_Menu

Create all the buttons etc...
===============
*/
#define spacing 5
Editor_Variables_Menu::Editor_Variables_Menu
(Editor_Interactive & parent, UI::UniqueWindow::Registry * registry)
:
UI::UniqueWindow(&parent, registry, 410, 330, _("Variables Menu")),
m_parent(parent),

m_table
(this, 5, 5, get_inner_w() - 2 * spacing, get_inner_h() - 40),

m_button_new
(this,
 get_inner_w() / 2 - 180 - spacing, get_inner_h() - 30, 120, 20,
 0,
 &Editor_Variables_Menu::clicked_new, this,
 _("New")),

m_button_edit
(this,
 get_inner_w() / 2 - 60, get_inner_h() - 30, 120, 20,
 0,
 &Editor_Variables_Menu::clicked_edit, this,
 _("Edit"),
 std::string(),
 false),

m_button_del
(this,
 get_inner_w() / 2 + 60 + spacing, get_inner_h() - 30, 120, 20,
 0,
 &Editor_Variables_Menu::clicked_del, this,
 _("Delete"),
 std::string(),
 false)
{
	m_table.add_column (14);
	m_table.add_column(286, _("Variable"));
	m_table.add_column(100, _("Value"));
	m_table.set_sort_column(1);
	m_table.selected.set(this, &Editor_Variables_Menu::table_selected);
	m_table.double_clicked.set(this, &Editor_Variables_Menu::table_dblclicked);

	Manager<Variable> & mvm = m_parent.egbase().map().mvm();
	Manager<Variable>::Index const nr_variables = mvm.size();
	for (Manager<Variable>::Index i = 0; i < nr_variables; ++i)
		insert_variable(mvm[i]);

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Variables_Menu::~Editor_Variables_Menu

Unregister from the registry pointer
===============
*/
Editor_Variables_Menu::~Editor_Variables_Menu() {}

/*
 * A Button has been clicked
 */
void Editor_Variables_Menu::clicked_new() {
	New_Variable_Window nvw(m_parent);
	if (nvw.run()) {
		insert_variable(*nvw.get_variable());
		clicked_edit();
	}
}
void Editor_Variables_Menu::clicked_edit() {
	Edit_Variable_Window evw(m_parent, m_table.get_selected_record());
	if (evw.run()) m_table.sort();
}
void Editor_Variables_Menu::clicked_del()      {
	//  Otherwise, delete button should be disabled.
	assert(not m_table.get_selected().is_delete_protected());

	m_parent.egbase().map().mvm().remove(m_table.get_selected());
	m_table.remove_selected();

	m_button_edit.set_enabled(false);
	m_button_del .set_enabled(false);
}

/*
 * The table has been selected
 */
void Editor_Variables_Menu::table_selected(uint32_t n) {
	assert(n < UI::Table<Variable &>::no_selection_index());
	m_button_edit.set_enabled(true);

	m_button_del.set_enabled(not m_table[n].is_delete_protected());
}

/*
 * Table has been doubleclicked
 */
void Editor_Variables_Menu::table_dblclicked(uint32_t) {clicked_edit();}

/*
 * Insert this map variable into the table
 */
void Editor_Variables_Menu::insert_variable(Variable & var) {
	char const * pic         = "nothing";
	char const * type_string = " ";
	if        (upcast(Widelands::Variable_String const, svar, &var)) {
		pic = "pics/map_variable_string.png";
		type_string = "S";
	} else if (upcast(Widelands::Variable_Int    const, ivar, &var)) {
		pic = "pics/map_variable_int.png";
		type_string = "I";
	} else
		assert(false);

	UI::Table<Variable &>::Entry_Record & t = m_table.add(var, true);

	t.set_picture(0, g_gr->get_picture(PicMod_UI, pic), type_string);
	t.set_string (1, var.name                     ());
	t.set_string (2, var.get_string_representation());

	if (var.is_delete_protected()) t.set_color(RGBColor(255, 0, 0));

	m_table.sort();
}
