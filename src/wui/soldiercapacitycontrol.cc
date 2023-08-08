/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/soldiercapacitycontrol.h"

#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wlapplication_mousewheel_options.h"
#include "wui/interactive_base.h"

/**
 * Widget to control the capacity of \ref MilitaryBuilding and \ref TrainingSite
 * via \ref SoldierControl
 */
struct SoldierCapacityControl : UI::Box {
	SoldierCapacityControl(UI::Panel* parent,
	                       InteractiveBase& ib,
	                       Widelands::MapObject& building_or_ship);

public:
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

protected:
	void think() override;

	[[nodiscard]] uint32_t get_cur_capacity() const;
	[[nodiscard]] uint32_t get_min_capacity() const;
	[[nodiscard]] uint32_t get_max_capacity() const;

	[[nodiscard]] inline bool get_can_act() const {
		return ibase_.can_act(
		   (building_ != nullptr ? building_->owner() : ship_->owner()).player_number());
	}

private:
	void change_soldier_capacity(int delta);
	void click_decrease();
	void click_increase();

	InteractiveBase& ibase_;
	Widelands::Ship* ship_;
	Widelands::Building* building_;

	UI::Button decrease_;
	UI::Button increase_;
	UI::Textarea value_;
};

SoldierCapacityControl::SoldierCapacityControl(UI::Panel* parent,
                                               InteractiveBase& ib,
                                               Widelands::MapObject& building_or_ship)
   : Box(parent, UI::PanelStyle::kWui, "soldier_capacity_control", 0, 0, Horizontal),
     ibase_(ib),
     ship_(building_or_ship.descr().type() == Widelands::MapObjectType::SHIP ?
              &dynamic_cast<Widelands::Ship&>(building_or_ship) :
              nullptr),
     building_(ship_ == nullptr ? &dynamic_cast<Widelands::Building&>(building_or_ship) : nullptr),
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
     value_(
        this, UI::PanelStyle::kWui, "value", UI::FontStyle::kWuiLabel, "199", UI::Align::kCenter) {
	decrease_.sigclicked.connect([this]() { click_decrease(); });
	increase_.sigclicked.connect([this]() { click_increase(); });

	add(new UI::Textarea(
	       this, UI::PanelStyle::kWui, "label_capacity", UI::FontStyle::kWuiLabel, _("Capacity")),
	    UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&decrease_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&value_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&increase_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	decrease_.set_repeating(true);
	increase_.set_repeating(true);

	set_thinks(true);
}

uint32_t SoldierCapacityControl::get_cur_capacity() const {
	return building_ != nullptr ? building_->soldier_control()->soldier_capacity() :
                                 ship_->get_warship_soldier_capacity();
}
uint32_t SoldierCapacityControl::get_min_capacity() const {
	return building_ != nullptr ? building_->soldier_control()->min_soldier_capacity() :
                                 ship_->min_warship_soldier_capacity();
}
uint32_t SoldierCapacityControl::get_max_capacity() const {
	return building_ != nullptr ? building_->soldier_control()->max_soldier_capacity() :
                                 ship_->get_capacity();
}

void SoldierCapacityControl::think() {
	const uint32_t capacity = get_cur_capacity();
	value_.set_text(as_string(capacity));

	bool const can_act = get_can_act();
	decrease_.set_enabled(can_act && get_min_capacity() < capacity);
	increase_.set_enabled(can_act && get_max_capacity() > capacity);
}

void SoldierCapacityControl::change_soldier_capacity(const int delta) {
	if (building_ != nullptr) {
		if (Widelands::Game* game = ibase_.get_game(); game != nullptr) {
			game->send_player_change_soldier_capacity(*building_, delta);
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	} else {
		const int32_t new_capacity = std::max<int32_t>(
		   get_min_capacity(),
		   std::min<int32_t>(get_max_capacity(),
		                     static_cast<int32_t>(ship_->get_warship_soldier_capacity()) + delta));
		if (Widelands::Game* game = ibase_.get_game(); game != nullptr) {
			game->send_player_warship_command(
			   *ship_, Widelands::WarshipCommand::kSetCapacity, {static_cast<uint32_t>(new_capacity)});
		} else {
			ship_->set_warship_soldier_capacity(new_capacity);
		}
	}
}

void SoldierCapacityControl::click_decrease() {
	change_soldier_capacity(
	   (SDL_GetModState() & KMOD_CTRL) != 0 ? get_min_capacity() - get_cur_capacity() : -1);
}

void SoldierCapacityControl::click_increase() {
	change_soldier_capacity(
	   (SDL_GetModState() & KMOD_CTRL) != 0 ? get_max_capacity() - get_cur_capacity() : 1);
}

bool SoldierCapacityControl::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	if (!get_can_act()) {
		return false;
	}

	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValue, x, y, modstate);
	if (change == 0) {
		// Try big step
		change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValueBig, x, y, modstate);
		if (change == 0) {
			return false;
		}
		change *= ChangeBigStep::kSmallRange;
	}
	change_soldier_capacity(change);
	return true;
}

UI::Panel* create_soldier_capacity_control(UI::Panel& parent,
                                           InteractiveBase& ib,
                                           Widelands::MapObject& building_or_ship) {
	return new SoldierCapacityControl(&parent, ib, building_or_ship);
}
