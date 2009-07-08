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

#include "editor/editorinteractive.h"
#include "i18n.h"
#include "map.h"
#include "trigger/trigger.h"
#include "trigger/trigger_time.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

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
	UI::EditBox                          * m_name;
	UI::Multiline_Editbox                * m_descr;
	UI::Checkbox                         * m_visibleAtBegin;

	void clicked_ok();
};

Edit_Objective_Window::Edit_Objective_Window
	(Editor_Interactive                   * const parent,
	 UI::Table<Objective &>::Entry_Record &       te)
:
UI::Window(parent, 0, 0, 250, 85, _("Edit Objective")),
m_parent  (parent),
m_te      (te)
{
	int32_t const spacing = 5;
	Point         pos       (get_inner_w() - STATEBOX_WIDTH - spacing, 5);

	Objective & obj = UI::Table<Objective &>::get(te);

	new UI::Textarea(this, 5, 5, 120, 20, _("Name"), Align_CenterLeft);
	m_name = new UI::EditBox(this, 120, 5, 120, 20);
	m_name->setText(obj.name());
	pos.y += 20 + spacing;

	new UI::Textarea
		(this,
		 5, pos.y, 120, STATEBOX_HEIGHT,
		 _("Visible at Begin: "), Align_CenterLeft);
	m_visibleAtBegin = new UI::Checkbox(this, pos);
	m_visibleAtBegin->set_state(obj.get_is_visible());
	pos.x  = 5;
	pos.y += STATEBOX_HEIGHT + spacing;

	new UI::Textarea
		(this,
		 pos.x, pos.y, 120, STATEBOX_HEIGHT,
		 _("Objective text: "), Align_CenterLeft);
	pos.y += 20 + spacing;

	int32_t const editbox_height = 140;
	m_descr = new UI::Multiline_Editbox
		(this,
		 pos.x, pos.y, get_inner_w() - 2 * spacing, editbox_height,
		 obj.descr().c_str());
	pos.x  = get_inner_w() / 2 - 80 - spacing;
	pos.y += editbox_height + spacing + spacing;

	new UI::Callback_Button<Edit_Objective_Window>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Edit_Objective_Window::clicked_ok, *this,
		 _("Ok"));

	pos.x += 80 + 2 * spacing;
	new UI::Callback_IDButton<Edit_Objective_Window, int32_t>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Edit_Objective_Window::end_modal, *this, 0,
		 _("Back"));

	set_inner_size(get_inner_w(), pos.y + 20 + spacing);
	center_to_parent();
}

/**
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Edit_Objective_Window::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		end_modal(0);
		return true;
	}
	return false;
}
bool Edit_Objective_Window::handle_mouserelease(const Uint8, int32_t, int32_t)
{
	return false;
}

/**
 * A button has been clicked
 */
void Edit_Objective_Window::clicked_ok() {
	Objective & obj = UI::Table<Objective &>::get(m_te); //  extract value

	obj.set_name(m_name->text());
	obj.set_is_visible(m_visibleAtBegin->get_state());
	obj.set_descr(m_descr->get_text().c_str());
	m_te.set_string(0, obj.name());
	m_te.set_string(1, obj.get_is_visible() ? "Yes" : "No");

	obj.get_trigger()->set_name(m_name->text());

	end_modal(1);
}


/**
 * Create all the buttons etc...
*/
#define spacing 5
Editor_Objectives_Menu::Editor_Objectives_Menu
		(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 410, 330, _("Objectives Menu")),
m_parent(parent),
m_table(this, 5, 25, get_inner_w() - 2 * spacing, get_inner_h() - 60)
{
	m_table.add_column(270, _("Name"));
	m_table.add_column (60, _("Visible"));
	m_table.selected.set(this, &Editor_Objectives_Menu::table_selected);
	m_table.double_clicked.set(this, &Editor_Objectives_Menu::table_dblclicked);

	int32_t posx = spacing;

	new UI::Callback_Button<Editor_Objectives_Menu>
		(this,
		 spacing, get_inner_h() - 30, 60, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Objectives_Menu::clicked_new, *this,
		 _("New"));

	posx += 60 + spacing;

	m_edit_button = new UI::Callback_Button<Editor_Objectives_Menu>
		(this,
		 posx, get_inner_h() - 30, 60, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Objectives_Menu::clicked_edit, *this,
		 _("Edit"),
		 std::string(),
		 false);

	posx += 60 + spacing;

	m_delete_button = new UI::Callback_Button<Editor_Objectives_Menu>
		(this,
		 posx, get_inner_h() - 30, 60, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Objectives_Menu::clicked_del, *this,
		 _("Delete"),
		 std::string(),
		 false);

	posx += 60 + spacing;

	m_trigger =
		new UI::Textarea
			(this,
			 posx, get_inner_h() - 20, 0, 0,
			 std::string(), Align_CenterLeft);

	Manager<Objective> & mom = m_parent->egbase().map().mom();
	Manager<Objective>::Index const nr_objectives = mom.size();
	for (Manager<Objective>::Index i = 0; i < nr_objectives; ++i)
		insert_objective(mom[i]);


	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}


/**
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
	//  create a timer for this
	Widelands::Trigger_Time & trigger = *new Widelands::Trigger_Time(buffer);
	objective.set_trigger(&trigger);
	mtm.register_new(trigger);
	insert_objective(objective);
	clicked_edit(); //  fallthrough to edit
}

void Editor_Objectives_Menu::clicked_edit() {
	Edit_Objective_Window evw(m_parent, m_table.get_selected_record());
	if (evw.run()) {
		m_table.sort();
		m_trigger->set_text
			(_("Trigger: ") + m_table.get_selected().get_trigger()->name());
	}
}

void Editor_Objectives_Menu::clicked_del() {
	Widelands::Map & map = m_parent->egbase().map();
	Trigger * trigger;
	{
		Objective & obj = m_table.get_selected();
		m_table.remove_selected();
		trigger = obj.get_trigger();
		map.mom().remove(obj);
	}
	m_edit_button  ->set_enabled(false);
	m_delete_button->set_enabled(false);
	m_trigger      ->set_text   (std::string());

	//  Now the objective itself is removed. If its former trigger is no longer
	//  referenced by anything, remove it as well. Otherwise inform the user.
	Trigger::Referencers const & referencers = trigger->referencers();
	if (referencers.size()) {
		std::string str =
			_
				("The objective was deleted, but not its trigger, because it is "
				 "still referenced by:\n");
		for
			(struct {
			 	Trigger::Referencers::const_iterator       current;
			 	Trigger::Referencers::const_iterator const end;
			 } i = {referencers.begin(), referencers.end()};;)
		{
			str += i.current->first->identifier();
			if (++i.current == i.end)
				break;
			str += '\n';
		}
		UI::WLMessageBox mmb
			(m_parent, _("Note"), str.c_str(), UI::WLMessageBox::OK);
		mmb.run();
	} else
		map.mtm().remove(*trigger);
}


/**
 * The table has been selected
 */
void Editor_Objectives_Menu::table_selected(uint32_t const n) {
	m_edit_button->set_enabled(true);
	m_delete_button->set_enabled(true);

	//  Baad stuff will happen, if trigger got deleted.
	m_trigger->set_text(_("Trigger: ") + m_table[n].get_trigger()->name());
}

/**
 * Table has been doubleclicked
 */
void Editor_Objectives_Menu::table_dblclicked(uint32_t) {clicked_edit();}

/**
 * Insert this map variable into the table
 */
void Editor_Objectives_Menu::insert_objective(Objective & var) {
	UI::Table<Objective &>::Entry_Record & t = m_table.add(var, true);

	t.set_string(0, var.name());
	t.set_string(1, var.get_is_visible() ? "Yes" : "No");

	m_table.sort();
}
