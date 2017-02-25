/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <boost/lexical_cast.hpp>

#include "graphic/graphic.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "wui/interactive_gamebase.h"

using Widelands::SoldierControl;

/**
 * Widget to control the capacity of \ref MilitaryBuilding and \ref TrainingSite
 * via \ref SoldierControl
 */
struct SoldierCapacityControl : UI::Box {
	SoldierCapacityControl(UI::Panel* parent,
	                       InteractiveGameBase& igb,
	                       Widelands::Building& building);

protected:
	void think() override;

private:
	void change_soldier_capacity(int delta);
	void click_decrease();
	void click_increase();

	InteractiveGameBase& igbase_;
	Widelands::Building& building_;

	UI::Button decrease_;
	UI::Button increase_;
	UI::Textarea value_;
};

SoldierCapacityControl::SoldierCapacityControl(UI::Panel* parent,
                                               InteractiveGameBase& igb,
                                               Widelands::Building& building)
   : Box(parent, 0, 0, Horizontal),
     igbase_(igb),
     building_(building),
     decrease_(this,
               "decrease",
               0,
               0,
               32,
               32,
               g_gr->images().get("images/ui_basic/but4.png"),
               g_gr->images().get("images/wui/buildings/menu_down_train.png"),
               _("Decrease capacity")),
     increase_(this,
               "increase",
               0,
               0,
               32,
               32,
               g_gr->images().get("images/ui_basic/but4.png"),
               g_gr->images().get("images/wui/buildings/menu_up_train.png"),
               _("Increase capacity")),
     value_(this, "199", UI::Align::kCenter) {
	decrease_.sigclicked.connect(
	   boost::bind(&SoldierCapacityControl::click_decrease, boost::ref(*this)));
	increase_.sigclicked.connect(
	   boost::bind(&SoldierCapacityControl::click_increase, boost::ref(*this)));

	add(new UI::Textarea(this, _("Capacity")));
	add(&decrease_, UI::Align::kCenter);
	add(&value_, UI::Align::kCenter);
	add(&increase_, UI::Align::kCenter);

	decrease_.set_repeating(true);
	increase_.set_repeating(true);

	set_thinks(true);
}

void SoldierCapacityControl::think() {

	SoldierControl* soldiers = dynamic_cast<SoldierControl*>(&building_);

	uint32_t const capacity = soldiers->soldier_capacity();
	value_.set_text(boost::lexical_cast<std::string>(capacity));

	bool const can_act = igbase_.can_act(building_.owner().player_number());
	decrease_.set_enabled(can_act && soldiers->min_soldier_capacity() < capacity);
	increase_.set_enabled(can_act && soldiers->max_soldier_capacity() > capacity);
}

void SoldierCapacityControl::change_soldier_capacity(int delta) {
	igbase_.game().send_player_change_soldier_capacity(building_, delta);
}

void SoldierCapacityControl::click_decrease() {
	change_soldier_capacity(-1);
}

void SoldierCapacityControl::click_increase() {
	change_soldier_capacity(1);
}

UI::Panel* create_soldier_capacity_control(UI::Panel& parent,
                                           InteractiveGameBase& igb,
                                           Widelands::Building& building) {
	return new SoldierCapacityControl(&parent, igb, building);
}
