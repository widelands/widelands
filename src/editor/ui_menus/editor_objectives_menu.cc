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

#include "editor_objectives_menu.h"

#include "editorinteractive.h"
#include "i18n.h"
#include "map.h"
#include "trigger/trigger.h"
#include "trigger/trigger_null.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_multilineeditbox.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

#include <map>

using Widelands::Objective;
using Widelands::Trigger;

/**
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Edit_Objective_Window : public UI::Window {
	Edit_Objective_Window
		(Editor_Interactive * const parent,
		 UI::Table<Objective &>::Entry_Record &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive                   * m_parent;
	UI::Table<Objective &>::Entry_Record & m_te;
	UI::Edit_Box                         * m_name;
	UI::Multiline_Editbox                * m_descr;
	UI::Checkbox                         * m_visibleAtBegin;

	void clicked_ok();
};

Edit_Objective_Window::Edit_Objective_Window
(Editor_Interactive * const parent,
 UI::Table<Objective &>::Entry_Record & te)
:
UI::Window(parent, 0, 0, 250, 85, _("Edit Objective")),
m_parent  (parent),
m_te      (te)
{
	int32_t const spacing = 5;
	int32_t       posy    = 5;

	Objective & obj = UI::Table<Objective &>::get(te);

	new UI::Textarea(this, 5, 5, 120, 20, _("Name"), Align_CenterLeft);
	m_name = new UI::Edit_Box(this, 120, 5, 120, 20, 0, 0);
	m_name->set_text(obj.name().c_str());
	posy += 20 + spacing;

	new UI::Textarea
		(this,
		 5, posy, 120, STATEBOX_HEIGHT,
		 _("Visible at Begin: "), Align_CenterLeft);
	m_visibleAtBegin =
		new UI::Checkbox(this, get_inner_w() - STATEBOX_WIDTH - spacing, posy);
	m_visibleAtBegin->set_state(obj.get_is_visible());
	posy += STATEBOX_HEIGHT + spacing;

	new UI::Textarea
		(this,
		 5, posy, 120, STATEBOX_HEIGHT,
		 _("Objective text: "), Align_CenterLeft);
	posy += 20 + spacing;

	int32_t const editbox_height = 140;
	m_descr = new UI::Multiline_Editbox
		(this,
		 5, posy, get_inner_w() - 2 * spacing, editbox_height,
		 obj.descr().c_str());
	posy += editbox_height + spacing + spacing;

	new UI::Button<Edit_Objective_Window>
		(this,
		 get_inner_w() / 2 - 80 - spacing, posy, 80, 20,
		 1,
		 &Edit_Objective_Window::clicked_ok, this,
		 _("Ok"));

	new UI::IDButton<Edit_Objective_Window, int32_t>
		(this,
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 1,
		 &Edit_Objective_Window::end_modal, this, 0,
		 _("Back"));

	posy += 20 + spacing;

	set_inner_size(get_inner_w(), posy);

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
bool Edit_Objective_Window::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Edit_Objective_Window::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/*
 * a button has been clicked
 */
void Edit_Objective_Window::clicked_ok() {
   // Extract value
	Objective & obj = UI::Table<Objective &>::get(m_te);

	obj.set_name(m_name->get_text());
	obj.set_is_visible(m_visibleAtBegin->get_state());
	obj.set_descr(m_descr->get_text().c_str());
	m_te.set_string(0, obj.name());
	m_te.set_string(1, obj.get_is_visible() ? "Yes" : "No");

	obj.get_trigger()->set_name(m_name->get_text());

	end_modal(1);
}


/*
===============
Editor_Objectives_Menu::Editor_Objectives_Menu

Create all the buttons etc...
===============
*/
#define spacing 5
Editor_Objectives_Menu::Editor_Objectives_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 410, 330, _("Objectives Menu")),
m_parent(parent),
m_table(this, 5, 25, get_inner_w() - 2 * spacing, get_inner_h() - 60)
{

	m_table.add_column(_("Name"),    270);
	m_table.add_column(_("Optional"), 70);
	m_table.add_column(_("Visible"),  60);
	m_table.selected.set(this, &Editor_Objectives_Menu::table_selected);
	m_table.double_clicked.set(this, &Editor_Objectives_Menu::table_dblclicked);

   // Buttons
   int32_t posx=spacing;

   new UI::Button<Editor_Objectives_Menu>
		(this,
		 spacing, get_inner_h() - 30, 60, 20,
		 0,
		 &Editor_Objectives_Menu::clicked_new, this,
		 _("New"));

	posx += 60 + spacing;

	m_edit_button = new UI::Button<Editor_Objectives_Menu>
		(this,
		 posx, get_inner_h() - 30, 60, 20,
		 0,
		 &Editor_Objectives_Menu::clicked_edit, this,
		 _("Edit"),
		 std::string(),
		 false);

	posx += 60 + spacing;

	m_delete_button = new UI::Button<Editor_Objectives_Menu>
		(this,
		 posx, get_inner_h() - 30, 60, 20,
		 0,
		 &Editor_Objectives_Menu::clicked_del, this,
		 _("Delete"),
		 std::string(),
		 false);

   posx += 60 + spacing;

   // Trigger name
   new UI::Textarea(this, posx, get_inner_h() - 30, 80, 20, _("Trigger: "), Align_CenterLeft);
   posx += 45 + spacing;
   m_trigger = new UI::Textarea(this, posx, get_inner_h() - 30, 100, 20, "-", Align_CenterLeft);

   // Add all variables
	Manager<Objective> & mom = m_parent->egbase().map().mom();
	Manager<Objective>::Index const nr_objectives = mom.size();
	for (Manager<Objective>::Index i = 0; i < nr_objectives; ++i)
		insert_objective(mom[i]);


	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Unregister from the registry pointer
===============
*/
Editor_Objectives_Menu::~Editor_Objectives_Menu() {}

/*
 * A Button has been clicked
 */
void Editor_Objectives_Menu::clicked_new() {
	char buffer[256];

	Widelands::Map & map = m_parent->egbase().map();
	Manager<Objective> & mom = map.mom();
	Manager<Trigger>   & mtm = map.mtm();
	for (uint32_t n = 1;; ++n) {
		snprintf(buffer, sizeof(buffer), _("Unnamed%u"), n);
		if (not mom[buffer] and not mtm[buffer])
			break;
	}
	//  create a new objective
	Objective & objective = *new Objective;
	objective.set_name(buffer);
	mom.register_new(objective);
	//  create a null trigger for this
	Widelands::Trigger_Null & trigger = *new Widelands::Trigger_Null(buffer);
	objective.set_trigger(&trigger);
	mtm.register_new(trigger);
	insert_objective(objective);
	   clicked_edit();// Fallthrough to edit
}

void Editor_Objectives_Menu::clicked_edit() {
	Edit_Objective_Window evw(m_parent, m_table.get_selected_record());
	if (evw.run()) {
		m_table.sort();
		m_trigger->set_text(m_table.get_selected().get_trigger()->name());
	}
}

void Editor_Objectives_Menu::clicked_del() {
	Objective & obj = m_table.get_selected();

	if (not obj.get_trigger()->referencers().empty()) {
		std::string str =
			_("Can't delete Objective, because it's trigger is in use by ");
		Trigger::Referencers const & referencers =
			obj.get_trigger()->referencers();
		Trigger::Referencers::const_iterator i = referencers.begin();
		while (i != referencers.end()) {
			str += i->first->identifier();
			str += "; ";
		}
		str.erase(str.end() - 2, str.end());
		UI::Modal_Message_Box mmb
			(m_parent, _("Error!"), str.c_str(), UI::Modal_Message_Box::OK);
		mmb.run();
		return;
	}

	Widelands::Map & map = m_parent->egbase().map();
	map.mtm().remove(*obj.get_trigger());
	map.mom().remove(obj);
	m_table.remove_selected();

	m_edit_button  ->set_enabled(false);
	m_delete_button->set_enabled(false);
}


/*
 * The table has been selected
 */
void Editor_Objectives_Menu::table_selected(uint32_t n) {
   m_edit_button->set_enabled(true);
   m_delete_button->set_enabled(true);

   // Baad stuff will happen, if trigger got deleted
	m_trigger->set_text(m_table[n].get_trigger()->name());
}

/*
 * Table has been doubleclicked
 */
void Editor_Objectives_Menu::table_dblclicked(uint32_t) {clicked_edit();}

/*
 * Insert this map variable into the table
 */
void Editor_Objectives_Menu::insert_objective(Objective & var) {
	UI::Table<Objective &>::Entry_Record & t = m_table.add(var, -1, true);

	t.set_string(0, var.name());
	t.set_string(1, var.get_is_visible() ? "Yes" : "No");

	m_table.sort();
}
