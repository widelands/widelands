/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_base.h"

using Widelands::SoldierControl;

/**
 * Widget to control the capacity of \ref MilitaryBuilding and \ref TrainingSite
 * via \ref SoldierControl
 */
struct SoldierCapacityControl : UI::Box {
	SoldierCapacityControl(UI::Panel* parent, InteractiveBase& ib, Widelands::Building& building);

protected:
	void think() override;

private:
	void change_soldier_capacity(int delta);
	void click_decrease();
	void click_increase();

	InteractiveBase& ibase_;
	Widelands::Building& building_;

	UI::Button decrease_;
	UI::Button increase_;
	UI::Textarea value_;
};

SoldierCapacityControl::SoldierCapacityControl(UI::Panel* parent,
                                               InteractiveBase& ib,
                                               Widelands::Building& building)
   : Box(parent, 0, 0, Horizontal),
     ibase_(ib),
     building_(building),
     decrease_(this,
               "decrease",
               0,
               0,
               32,
               32,
               UI::ButtonStyle::kWuiMenu,
               g_image_cache->get("images/wui/buildings/menu_down_train.png"),
               _("Decrease capacity. Hold down Ctrl to set the capacity to the lowest value")),
     increase_(this,
               "increase",
               0,
               0,
               32,
               32,
               UI::ButtonStyle::kWuiMenu,
               g_image_cache->get("images/wui/buildings/menu_up_train.png"),
               _("Increase capacity. Hold down Ctrl to set the capacity to the highest value")),
     value_(this, "199", UI::Align::kCenter) {
	decrease_.sigclicked.connect([this]() { click_decrease(); });
	increase_.sigclicked.connect([this]() { click_increase(); });

	add(new UI::Textarea(this, _("Capacity")), UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&decrease_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&value_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&increase_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	decrease_.set_repeating(true);
	increase_.set_repeating(true);

	set_thinks(true);
}

void SoldierCapacityControl::think() {
	const SoldierControl* soldiers = building_.soldier_control();
	assert(soldiers != nullptr);
	uint32_t const capacity = soldiers->soldier_capacity();
	value_.set_text(boost::lexical_cast<std::string>(capacity));

	bool const can_act = ibase_.omnipotent() || ibase_.can_act(building_.owner().player_number());
	decrease_.set_enabled(can_act && soldiers->min_soldier_capacity() < capacity);
	increase_.set_enabled(can_act && soldiers->max_soldier_capacity() > capacity);
}

void SoldierCapacityControl::change_soldier_capacity(int delta) {
	if (ibase_.get_game()) {
		ibase_.game().send_player_change_soldier_capacity(building_, delta);
	} else {
		SoldierControl* soldier_control = building_.mutable_soldier_control();
		Widelands::Quantity const old_capacity = soldier_control->soldier_capacity();
		Widelands::Quantity const new_capacity =
		   std::min(static_cast<Widelands::Quantity>(
		               std::max(static_cast<int32_t>(old_capacity) + delta,
		                        static_cast<int32_t>(soldier_control->min_soldier_capacity()))),
		            soldier_control->max_soldier_capacity());
		if (old_capacity != new_capacity) {
			soldier_control->set_soldier_capacity(new_capacity);
		}
	}
}

void SoldierCapacityControl::click_decrease() {
	const SoldierControl* soldiers = building_.soldier_control();
	assert(soldiers);
	change_soldier_capacity((SDL_GetModState() & KMOD_CTRL) ?
	                           soldiers->min_soldier_capacity() - soldiers->soldier_capacity() :
	                           -1);
}

void SoldierCapacityControl::click_increase() {
	const SoldierControl* soldiers = building_.soldier_control();
	assert(soldiers);
	change_soldier_capacity((SDL_GetModState() & KMOD_CTRL) ?
	                           soldiers->max_soldier_capacity() - soldiers->soldier_capacity() :
	                           1);
}

UI::Panel* create_soldier_capacity_control(UI::Panel& parent,
                                           InteractiveBase& ib,
                                           Widelands::Building& building) {
	return new SoldierCapacityControl(&parent, ib, building);
}
