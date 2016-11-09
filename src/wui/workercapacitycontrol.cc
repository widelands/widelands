/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/workercapacitycontrol.h"

#include "economy/workers_queue.h"
#include "graphic/graphic.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "wui/interactive_gamebase.h"

/**
 * Widget to control the capacity of \ref ProductionBuilding
 * Adapted copy of \ref SoldierCapacityControl
 */
// NOCOM(#codereview): If we go for this UI version, we should try to create a common superclass to avoid code duplication.
struct WorkerCapacityControl : UI::Box {
	WorkerCapacityControl(UI::Panel* parent,
	                      InteractiveGameBase& igb,
	                      Widelands::Building& building,
	                      Widelands::DescriptionIndex index,
	                      Widelands::WorkersQueue& workers);

protected:
	void think() override;

private:
	void change_worker_capacity(int16_t delta);
	void click_decrease();
	void click_increase();

	InteractiveGameBase& igbase_;
	Widelands::Building& building_;
	Widelands::DescriptionIndex index_;
	Widelands::WorkersQueue& workers_;

	UI::Button decrease_;
	UI::Button increase_;
	UI::Textarea value_;
};

WorkerCapacityControl::WorkerCapacityControl(UI::Panel* parent,
                                             InteractiveGameBase& igb,
                                             Widelands::Building& building,
                                             Widelands::DescriptionIndex index,
                                             Widelands::WorkersQueue& workers)
   : Box(parent, 0, 0, Horizontal),
     igbase_(igb),
     building_(building),
     index_(index),
     workers_(workers),
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
	   boost::bind(&WorkerCapacityControl::click_decrease, boost::ref(*this)));
	increase_.sigclicked.connect(
	   boost::bind(&WorkerCapacityControl::click_increase, boost::ref(*this)));

	add(new UI::Textarea(this, _("Capacity")), UI::Align::kHCenter);
	add(&decrease_, UI::Align::kHCenter);
	add(&value_, UI::Align::kHCenter);
	add(&increase_, UI::Align::kHCenter);

	decrease_.set_repeating(true);
	increase_.set_repeating(true);

	set_thinks(true);
}

void WorkerCapacityControl::think() {
	uint32_t const capacity = workers_.capacity();
	// NOCOM(#codereview): We should use boost::format or boost::lexical_cast here.
	// While we're at it, fix the code in SoldierCapacityControl as well.
	char buffer[sizeof("4294967295")];

	sprintf(buffer, "%2u", capacity);
	value_.set_text(buffer);

	bool const can_act = igbase_.can_act(building_.owner().player_number());
	decrease_.set_enabled(can_act && 0 < capacity);
	increase_.set_enabled(can_act && workers_.max_capacity() > capacity);
}

void WorkerCapacityControl::change_worker_capacity(int16_t delta) {
	igbase_.game().send_player_change_worker_capacity(building_, workers_.get_worker(), delta);
}

void WorkerCapacityControl::click_decrease() {
	change_worker_capacity(-1);
}

void WorkerCapacityControl::click_increase() {
	change_worker_capacity(1);
}

UI::Panel* create_worker_capacity_control(UI::Panel& parent,
                                          InteractiveGameBase& igb,
                                          Widelands::Building& building,
                                          Widelands::DescriptionIndex index,
                                          Widelands::WorkersQueue& workers) {
	return new WorkerCapacityControl(&parent, igb, building, index, workers);
}
