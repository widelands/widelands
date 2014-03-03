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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/soldiercapacitycontrol.h"

#include "graphic/graphic.h"
#include "logic/player.h"
#include "logic/soldiercontrol.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "wui/interactive_gamebase.h"

using Widelands::SoldierControl;

/**
 * Widget to control the capacity of \ref MilitaryBuilding and \ref TrainingSite
 * via \ref SoldierControl
 */
struct SoldierCapacityControl : UI::Box {
	SoldierCapacityControl
		(UI::Panel * parent, Interactive_GameBase & igb,
		 Widelands::Building & building);

protected:
	virtual void think() override;

private:
	void change_soldier_capacity(int delta);
	void click_decrease();
	void click_increase();

	Interactive_GameBase & m_igb;
	Widelands::Building & m_building;

	UI::Button m_decrease;
	UI::Button m_increase;
	UI::Textarea m_value;
};

SoldierCapacityControl::SoldierCapacityControl
	(UI::Panel * parent, Interactive_GameBase & igb,
	 Widelands::Building & building)
:
Box(parent, 0, 0, Horizontal),
m_igb(igb),
m_building(building),
m_decrease
	(this, "decrease", 0, 0, 32, 32,
	 g_gr->images().get("pics/but4.png"),
	 g_gr->images().get("pics/menu_down_train.png"), _("Decrease capacity")),
m_increase
	(this, "increase", 0, 0, 32, 32,
	 g_gr->images().get("pics/but4.png"),
	 g_gr->images().get("pics/menu_up_train.png"), _("Increase capacity")),
m_value(this, "199", UI::Align_Center)
{
	m_decrease.sigclicked.connect(boost::bind(&SoldierCapacityControl::click_decrease, boost::ref(*this)));
	m_increase.sigclicked.connect(boost::bind(&SoldierCapacityControl::click_increase, boost::ref(*this)));

	add(new UI::Textarea(this, _("Capacity")), AlignCenter);
	add(&m_decrease, AlignCenter);
	add(&m_value, AlignCenter);
	add(&m_increase, AlignCenter);

	m_decrease.set_repeating(true);
	m_increase.set_repeating(true);

	set_think(true);
}

void SoldierCapacityControl::think()
{
	SoldierControl * soldiers = dynamic_cast<SoldierControl *>(&m_building);
	uint32_t const capacity = soldiers->soldierCapacity();
	char buffer[sizeof("4294967295")];

	sprintf(buffer, "%2u", capacity);
	m_value.set_text(buffer);

	bool const can_act = m_igb.can_act(m_building.owner().player_number());
	m_decrease.set_enabled(can_act && soldiers->minSoldierCapacity() < capacity);
	m_increase.set_enabled(can_act && soldiers->maxSoldierCapacity() > capacity);
}

void SoldierCapacityControl::change_soldier_capacity(int delta)
{
	m_igb.game().send_player_change_soldier_capacity(m_building, delta);
}

void SoldierCapacityControl::click_decrease()
{
	change_soldier_capacity(-1);
}

void SoldierCapacityControl::click_increase()
{
	change_soldier_capacity(1);
}

UI::Panel * create_soldier_capacity_control
	(UI::Panel & parent, Interactive_GameBase & igb,
	 Widelands::Building & building)
{
	return new SoldierCapacityControl(&parent, igb, building);
}
