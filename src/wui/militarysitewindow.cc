/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "buildingwindow.h"
#include "logic/constructionsite.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "ui_basic/table.h"

using Widelands::MilitarySite;
using Widelands::Soldier;
using Widelands::atrAttack;
using Widelands::atrDefense;
using Widelands::atrEvade;
using Widelands::atrHP;

static char const * pic_up_capacity     = "pics/menu_up_train.png";
static char const * pic_down_capacity   = "pics/menu_down_train.png";
static char const * pic_drop_soldier = "pics/menu_drop_soldier.png";

/**
 * Status window for \ref MilitarySite
 */
struct MilitarySite_Window : public Building_Window {
	MilitarySite_Window
		(Interactive_GameBase & parent,
		 MilitarySite       &,
		 UI::Window *       & registry);

	MilitarySite & militarysite() {
		return ref_cast<MilitarySite, Widelands::Building>(building());
	}

	virtual void think();
private:
	void update();
	void drop_button_clicked ();
	void soldier_capacity_up () {act_change_soldier_capacity (1);}
	void soldier_capacity_down() {act_change_soldier_capacity(-1);}

	Widelands::Coords               m_ms_location;
	UI::Box                         m_vbox;
	UI::Table<Soldier &>            m_table;
	UI::Callback_Button<MilitarySite_Window> m_drop_button;
	UI::Box                         m_bottom_box;
	UI::Panel                       m_capsbuttons;
	UI::Textarea                    m_capacity;
	UI::Callback_Button<MilitarySite_Window> m_capacity_down;
	UI::Callback_Button<MilitarySite_Window> m_capacity_up;
};


MilitarySite_Window::MilitarySite_Window
	(Interactive_GameBase & parent,
	 MilitarySite       & ms,
	 UI::Window *       & registry)
	:
	Building_Window(parent, ms, registry),
	m_ms_location  (ms.get_position()),
	m_vbox         (this, 5, 5, UI::Box::Vertical),
	m_table        (&m_vbox, 0, 0, 360, 200),
	m_drop_button
		(&m_vbox,
		 0, 0, 360, 32,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_drop_soldier),
		 &MilitarySite_Window::drop_button_clicked, *this),
	m_bottom_box   (&m_vbox, 0, 0, UI::Box::Horizontal),
	m_capsbuttons  (&m_bottom_box, 0, 34, 34 * 7, 34),
	m_capacity     (&m_bottom_box, 0, 0, _("Capacity"), UI::Align_Right),
	m_capacity_down
		(&m_bottom_box,
		 0, 0, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_down_capacity),
		 &MilitarySite_Window::soldier_capacity_down, *this),
	m_capacity_up
		(&m_bottom_box,
		 0, 0, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_up_capacity),
		 &MilitarySite_Window::soldier_capacity_up,   *this)
{
	//  soldiers view
	m_table.add_column(100, _("Name"));
	m_table.add_column (40, _("HP"));
	m_table.add_column (40, _("AT"));
	m_table.add_column (40, _("DE"));
	m_table.add_column (40, _("EV"));
	m_table.add_column(100, _("Level")); // enough space for scrollbar
	m_vbox.add(&m_table, UI::Box::AlignCenter);

	m_vbox.add_space(8);

	// Add drop soldier button
	m_vbox.add(&m_drop_button, UI::Box::AlignCenter);


	//  Add the bottom row of buttons.

	m_bottom_box.add_space(5);

	// Add the caps button
	create_capsbuttons(&m_capsbuttons);
	m_bottom_box.add(&m_capsbuttons, UI::Box::AlignLeft);

	// Capacity buttons
	m_bottom_box.add(&m_capacity,      UI::Box::AlignRight);
	m_bottom_box.add_space(8);
	m_bottom_box.add(&m_capacity_down, UI::Box::AlignRight);
	m_bottom_box.add(&m_capacity_up,   UI::Box::AlignRight);
	m_capacity_down.set_repeating(true);
	m_capacity_up  .set_repeating(true);

	m_bottom_box.add_space(5);

	fit_inner(m_bottom_box);

	m_vbox.add_space(8);

	m_vbox.add(&m_bottom_box, UI::Box::AlignCenter);

	fit_inner(m_vbox);

	move_inside_parent();
}


void MilitarySite_Window::think()
{
	Building_Window::think();

	Widelands::BaseImmovable const * const base_immovable =
		igbase().egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Widelands::Building *>(base_immovable)
		 or
		 dynamic_cast<const Widelands::ConstructionSite *>(base_immovable))
	{
		// The Site has been removed. Die quickly.
		die();
		return;
	}
	update();
}

/*
==============
Update the listselect, maybe there are new soldiers
=============
*/
void MilitarySite_Window::update() {
	MilitarySite const & ms = militarysite();
	std::vector<Soldier *> soldiers = ms.presentSoldiers();

	if (soldiers.size() < m_table.size())
		m_table.clear();

	for (uint32_t i = 0; i < soldiers.size(); ++i) {
		Soldier & s = *soldiers[i];
		UI::Table<Soldier &>::Entry_Record * er =
			m_table.find(s);
		if (not er)
			er = &m_table.add(s);
		uint32_t const  hl = s.get_hp_level         ();
		uint32_t const mhl = s.get_max_hp_level     ();
		uint32_t const  al = s.get_attack_level     ();
		uint32_t const mal = s.get_max_attack_level ();
		uint32_t const  dl = s.get_defense_level    ();
		uint32_t const mdl = s.get_max_defense_level();
		uint32_t const  el = s.get_evade_level      ();
		uint32_t const mel = s.get_max_evade_level  ();
		er->set_string(0, s.descname().c_str());
		char buffer[sizeof("4294967295 / 4294967295")];
		sprintf(buffer,  "%u / %u", hl,                mhl);
		er->set_string(1, buffer);
		sprintf(buffer,  "%u / %u",      al,                 mal);
		er->set_string(2, buffer);
		sprintf(buffer,  "%u / %u",           dl,                  mdl);
		er->set_string(3, buffer);
		sprintf(buffer,  "%u / %u",                el,                   mel);
		er->set_string(4, buffer);
		sprintf(buffer, "%2u / %u", hl + al + dl + el, mhl + mal + mdl + mel);
		er->set_string(5, buffer);
	}
	m_table.sort();

	uint32_t const capacity     = ms.   soldierCapacity();
	char buffer[200];
	snprintf(buffer, sizeof(buffer), _("Capacity: %2u"), capacity);
	m_capacity.set_text (buffer);
	uint32_t const capacity_min = ms.minSoldierCapacity();
	bool const can_act = igbase().can_act(ms.owner().player_number());
	m_drop_button.set_enabled
		(can_act and m_table.has_selection() and capacity_min < m_table.size());
	m_capacity_down.set_enabled
		(can_act and capacity_min < capacity);
	m_capacity_up  .set_enabled
		(can_act and                capacity < ms.maxSoldierCapacity());
}

void MilitarySite_Window::drop_button_clicked()
{
	assert(m_table.has_selection());
	act_drop_soldier(m_table.get_selected().serial());
}

/*
===============
Create the production site information window.
===============
*/
void MilitarySite::create_options_window
	(Interactive_GameBase & plr, UI::Window * & registry)
{
	new MilitarySite_Window(plr, *this, registry);
}
