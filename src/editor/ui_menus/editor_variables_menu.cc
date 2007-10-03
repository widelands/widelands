/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "map_variable_manager.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"


/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct New_Variable_Window : public UI::Window {
      New_Variable_Window(Editor_Interactive*);

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

      MapVariable* get_variable() {return m_variable;}

private:
      Editor_Interactive* m_parent;
      MapVariable* m_variable;
	enum Variable_Type {Integer_Type, String_Type};
	UI::IDButton<New_Variable_Window, const Variable_Type> button_integer;
	UI::IDButton<New_Variable_Window, const Variable_Type> button_string;
	UI::IDButton<New_Variable_Window, int32_t>                 button_back;

	void clicked_new(const Variable_Type);
};

New_Variable_Window::New_Variable_Window(Editor_Interactive* parent) :
UI::Window(parent, 0, 0, 135, 55, _("New Variable").c_str()),

m_parent(parent),
m_variable(0),

   // What type

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
   // Get the a name

   char buffer[256];

	MapVariableManager & mvm = m_parent->egbase().map().get_mvm();
	for (uint32_t n = 1; mvm.get_variable(buffer); ++n)
		snprintf(buffer, sizeof(buffer), "%s%i", _("Unnamed").c_str(), n);

   std::string name = buffer;
	switch (i) {
	case Integer_Type:
         m_variable = new Int_MapVariable(0);
         break;

	case String_Type:
         m_variable = new String_MapVariable(0);
         break;
	}

	m_variable->set_name(buffer);
	mvm.register_new_variable(m_variable);
	end_modal(1);
   return;
}

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Edit_Variable_Window : public UI::Window {
	Edit_Variable_Window
		(Editor_Interactive &, UI::Table<MapVariable &>::Entry_Record &);

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive                     & m_parent;
	UI::Table<MapVariable &>::Entry_Record & m_te;
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
(Editor_Interactive & parent, UI::Table<MapVariable &>::Entry_Record & te)
:
UI::Window(&parent, 0, 0, 250, 85, _("Edit Variable").c_str()),

m_parent(parent),
m_te(te),

   // What type
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
	m_name .set_text(m_te.get_string(0).c_str());
	m_value.set_text(m_te.get_string(1).c_str());
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
   // Get the a name

   // Extract value
	MapVariable & var = UI::Table<MapVariable &>::get(m_te);

	switch (var.get_type()) {
	case MapVariable::MVT_INT: {
         char* endp;
			const int32_t ivar = strtol(m_value.get_text(), &endp, 0);

		if (endp and *endp) {
            char buffer[1024];
				snprintf
					(buffer, sizeof(buffer),
					 "%s %s",
					 m_value.get_text(),
					 _("is not a valid integer!").c_str());
				UI::Modal_Message_Box mb
					(&m_parent, _("Parse error!"), buffer, UI::Modal_Message_Box::OK);
            mb.run();
            return;
		}
         char buffer[256];
         snprintf(buffer, sizeof(buffer), "%i", ivar);

			static_cast<Int_MapVariable &>(var).set_value(ivar);
			m_te.set_string(1, buffer);
	}
		break;

	case MapVariable::MVT_STRING: {
			static_cast<String_MapVariable &>(var).set_value(m_value.get_text());
			m_te.set_string(1, m_value.get_text());
	}
      break;
	}

	var.set_name(m_name.get_text());
	m_te.set_string(0, var.get_name());

   end_modal(1);
}


/*
===============
Editor_Variables_Menu::Editor_Variables_Menu

Create all the buttons etc...
===============
*/
#define spacing 5
Editor_Variables_Menu::Editor_Variables_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 410, 330, _("Variables Menu")),
m_parent(parent),

m_table
(this, 5, 25, get_inner_w() - 2 * spacing, get_inner_h() - 60),

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

	m_table.add_column(_("Variable"), 300);
	m_table.add_column(_("Value"),    100);
   m_table.selected.set(this, &Editor_Variables_Menu::table_selected);
   m_table.double_clicked.set(this, &Editor_Variables_Menu::table_dblclicked);

   // Add all variables
	const MapVariableManager & mvm = m_parent->egbase().map().get_mvm();
	const MapVariableManager::Index nr_variables = mvm.get_nr_variables();
	for (MapVariableManager::Index i = 0; i < nr_variables; ++i)
		insert_variable(mvm.get_variable_by_nr(i));

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
Editor_Variables_Menu::~Editor_Variables_Menu()
{
}

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
	Edit_Variable_Window evw(*m_parent, m_table.get_selected_record());
	if (evw.run()) m_table.sort();
}
void Editor_Variables_Menu::clicked_del()      {
	MapVariable & mv = m_table.get_selected();

         // Otherwise, delete button should be disabled
	assert(not mv.is_delete_protected());

	m_parent->egbase().map().get_mvm().delete_variable(mv.get_name());
	m_table.remove_selected();

	m_button_edit.set_enabled(false);
	m_button_del .set_enabled(false);
}

/*
 * The table has been selected
 */
void Editor_Variables_Menu::table_selected(uint32_t n) {
	assert(n < UI::Table<MapVariable &>::no_selection_index());
	m_button_edit.set_enabled(true);

	m_button_del.set_enabled(m_table[n].is_delete_protected());
}

/*
 * Table has been doubleclicked
 */
void Editor_Variables_Menu::table_dblclicked(uint32_t) {clicked_edit();}

/*
 * Insert this map variable into the table
 */
void Editor_Variables_Menu::insert_variable(MapVariable & var) {
   const char* pic = 0;

	const MapVariable::Type type = var.get_type();
	switch (type) {
	case MapVariable::MVT_INT:    pic = "pics/map_variable_int.png";    break;
	case MapVariable::MVT_STRING: pic = "pics/map_variable_string.png"; break;
	default:                      pic = "nothing";
	};

	UI::Table<MapVariable &>::Entry_Record & t = m_table.add
		(var, g_gr->get_picture(PicMod_UI, pic), true);
	t.set_string(0, var.get_name());

   std::string val;

	switch (type) {
	case MapVariable::MVT_INT: {
         char buffer[256];
         snprintf(buffer, sizeof(buffer), "%i", static_cast<Int_MapVariable &>(var).get_value());
         val = buffer;
	}
		break;

	case MapVariable::MVT_STRING:
      val = static_cast<String_MapVariable &>(var).get_value();
      break;

	default: val = "";
	}
	t.set_string(0, var.get_name());
	t.set_string(1, val.c_str());

	if (var.is_delete_protected()) t.set_color(RGBColor(255, 0, 0));

	m_table.sort();
}
