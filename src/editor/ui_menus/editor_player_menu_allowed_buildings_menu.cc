/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_player_menu_allowed_buildings_menu.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"

using Widelands::Building_Index;

/**
 * Create all the buttons etc...
*/
#define margin                0
#define hmargin margin
#define vmargin margin
#define spacing               3
#define hspacing spacing
#define vspacing spacing
#define list_width          240
#define list_height         520
#define middle_button_width  40
#define middle_button_height 20
#define label_height         20
Editor_Player_Menu_Allowed_Buildings_Menu::
Editor_Player_Menu_Allowed_Buildings_Menu
		(UI::Panel                  * parent,
		 Widelands::Player          & player,
		 UI::UniqueWindow::Registry * registry)
	:
	UI::UniqueWindow
		(parent, "allowed_buildings_menu",
		 registry,
		 hmargin +
		 list_width + hspacing + middle_button_width + hspacing + list_width
		 + hmargin,
		 vmargin + label_height + vspacing + list_height + vmargin,
		 _("Allowed Buildings")),
	m_player(player),
	m_allowed_label
		(this,
		 hmargin, vmargin, list_width, label_height,
		 _("Allowed Buildings:"), UI::Align_CenterLeft),
	m_forbidden_label
		(this,
		 hmargin + list_width + hspacing + middle_button_width + hspacing,
		 m_allowed_label.get_y(),
		 list_width, label_height,
		 _("Forbidden Buildings:"), UI::Align_CenterLeft),
	m_allowed
		(this,
		 m_allowed_label.get_x(),
		 vmargin + label_height + vspacing,
		 list_width, list_height),
	m_forbidden
		(this,
		 m_forbidden_label.get_x(), m_allowed.get_y(), list_width, list_height),
	m_forbid_button
		(this, "forbid",
		 hmargin + list_width + hspacing,
		 m_allowed.get_y()
		 + (list_height - middle_button_height * 2 - vspacing) / 2,
		 middle_button_width, middle_button_height,
		 g_gr->images().get("pics/but1.png"),
		 ("->"),
		 _("Forbid"),
		 false),
	m_allow_button
		(this, "allow",
		 m_forbid_button.get_x(),
		 m_forbid_button.get_y() + middle_button_height + vspacing,
		 middle_button_width, middle_button_height,
		 g_gr->images().get("pics/but1.png"),
		 ("<-"),
		 _("Allow"),
		 false)
{
	m_forbid_button.sigclicked.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::clicked, boost::ref(*this), false));
	m_allow_button.sigclicked.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::clicked, boost::ref(*this), true));

	m_allowed.selected.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::allowed_selected, this, _1));
	m_allowed.double_clicked.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::allowed_double_clicked, this, _1));
	m_forbidden.selected.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_selected, this, _1));
	m_forbidden.double_clicked.connect
		(boost::bind(&Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_double_clicked, this, _1));

	const Widelands::Tribe_Descr & tribe = player.tribe();
	Building_Index const nr_buildings = tribe.get_nrbuildings();
	for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i) {
		const Widelands::Building_Descr & building =
			*tribe.get_building_descr(i);
		if (not building.is_enhanced() and not building.is_buildable())
			continue;
		(m_player.is_building_type_allowed(i) ? m_allowed : m_forbidden).add
			(building.descname().c_str(), i, building.get_buildicon());
	}
	m_forbidden.sort();
	m_allowed  .sort();
	update();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void Editor_Player_Menu_Allowed_Buildings_Menu::update() {}

/**
 * Unregister from the registry pointer
*/
Editor_Player_Menu_Allowed_Buildings_Menu::
~Editor_Player_Menu_Allowed_Buildings_Menu
	()
{}


/**
 * UI Action callback functions
 */

void Editor_Player_Menu_Allowed_Buildings_Menu::clicked(const bool allow) {
	UI::Listselect<Building_Index> & source = allow ? m_forbidden : m_allowed;
	UI::Listselect<Building_Index> & target = allow ? m_allowed : m_forbidden;

	assert //  The button should have been disabled if nothing is selected.
		(source.selection_index()
		 !=
		 UI::Listselect<intptr_t>::no_selection_index());

	Building_Index const building_index = source.get_selected();
	source.remove_selected();
	const Widelands::Building_Descr & building =
		*m_player.tribe().get_building_descr(building_index);
	target.add
		(building.descname().c_str(),
		 building_index,
		 building.get_buildicon());
	target.sort();
	m_player.allow_building_type(building_index, allow);
}

void Editor_Player_Menu_Allowed_Buildings_Menu::
	allowed_selected(uint32_t index)
{
	m_forbid_button.set_enabled
		(index != UI::Listselect<intptr_t>::no_selection_index());
}

void Editor_Player_Menu_Allowed_Buildings_Menu::
	forbidden_selected(uint32_t index)
{
	m_allow_button.set_enabled
		(index != UI::Listselect<intptr_t>::no_selection_index());
}

void Editor_Player_Menu_Allowed_Buildings_Menu::allowed_double_clicked(uint32_t)
{
	clicked(false);
}

void Editor_Player_Menu_Allowed_Buildings_Menu::
	forbidden_double_clicked(uint32_t)
{
	clicked(true);
}
