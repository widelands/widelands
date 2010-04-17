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

#include "soldierlist.h"

#include "interactive_gamebase.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/soldiercontrol.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"

using Widelands::Soldier;
using Widelands::SoldierControl;

static char const * pic_drop_soldier = "pics/menu_drop_soldier.png";

/**
 * List of soldiers and a "drop soldiers" button suitable for
 * \ref MilitarySiteWindow and \ref TrainingSiteWindow
 */
struct SoldierList : UI::Box {
	SoldierList
		(UI::Panel& parent,
		 Interactive_GameBase& igb,
		 Widelands::Building& building);

	SoldierControl& soldiers() const;

protected:
	void think();

private:
	void drop_button_clicked();

	Interactive_GameBase& m_igb;
	Widelands::Building& m_building;

	UI::Table<Soldier&> m_table;
	UI::Callback_Button<SoldierList> m_drop_button;
};

SoldierList::SoldierList
	(UI::Panel& parent,
	 Interactive_GameBase& igb,
	 Widelands::Building& building)
:
UI::Box(&parent, 0, 0, UI::Box::Vertical),

m_igb(igb),
m_building(building),

m_table(this, 0, 0, 360, 200),

m_drop_button
	(this,
	 0, 0, 360, 32,
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_picture(PicMod_Game, pic_drop_soldier),
	 &SoldierList::drop_button_clicked, *this)
{
	set_think(true);

	m_table.add_column(100, _("Name"));
	m_table.add_column (40, _("HP"));
	m_table.add_column (40, _("AT"));
	m_table.add_column (40, _("DE"));
	m_table.add_column (40, _("EV"));
	m_table.add_column(100, _("Level")); // enough space for scrollbar
	add(&m_table, UI::Box::AlignCenter);

	add_space(4);

	add(&m_drop_button, UI::Box::AlignCenter);
}

SoldierControl& SoldierList::soldiers() const
{
	return *dynamic_cast<SoldierControl*>(&m_building);
}

void SoldierList::think()
{
	std::vector<Soldier *> soldierlist = soldiers().presentSoldiers();

	uint32_t index = m_table.size();
	while(index > 0) {
		index--;

		if (std::find(soldierlist.begin(), soldierlist.end(), &m_table[index]) == soldierlist.end())
			m_table.remove(index);
	}

	for(std::vector<Soldier*>::const_iterator it = soldierlist.begin(); it != soldierlist.end(); ++it) {
		Soldier & s = **it;
		if (!m_table.find(**it))
			m_table.add(s);
	}

	// Update soldier stats (they may change in training sites)
	for(index = 0; index < m_table.size(); ++index) {
		UI::Table<Soldier &>::Entry_Record& er = m_table.get_record(index);
		Soldier& s = m_table[index];
		uint32_t const  hl = s.get_hp_level         ();
		uint32_t const mhl = s.get_max_hp_level     ();
		uint32_t const  al = s.get_attack_level     ();
		uint32_t const mal = s.get_max_attack_level ();
		uint32_t const  dl = s.get_defense_level    ();
		uint32_t const mdl = s.get_max_defense_level();
		uint32_t const  el = s.get_evade_level      ();
		uint32_t const mel = s.get_max_evade_level  ();
		er.set_string(0, s.descname().c_str());
		char buffer[sizeof("4294967295 / 4294967295")];
		sprintf(buffer,  "%u / %u", hl,                mhl);
		er.set_string(1, buffer);
		sprintf(buffer,  "%u / %u",      al,                 mal);
		er.set_string(2, buffer);
		sprintf(buffer,  "%u / %u",           dl,                  mdl);
		er.set_string(3, buffer);
		sprintf(buffer,  "%u / %u",                el,                   mel);
		er.set_string(4, buffer);
		sprintf(buffer, "%2u / %u", hl + al + dl + el, mhl + mal + mdl + mel);
		er.set_string(5, buffer);
	}

	m_table.sort();

	uint32_t const capacity_min = soldiers().minSoldierCapacity();
	bool can_act = m_igb.can_act(m_building.owner().player_number());
	bool over_min = capacity_min < m_table.size();

	m_drop_button.set_enabled(can_act && over_min && m_table.has_selection());
}

void SoldierList::drop_button_clicked()
{
	if (m_table.has_selection())
		m_igb.game().send_player_drop_soldier(m_building, m_table.get_selected().serial());
}


UI::Panel* create_soldier_list
	(UI::Panel& parent,
	 Interactive_GameBase& igb,
	 Widelands::Building& building)
{
	return new SoldierList(parent, igb, building);
}
