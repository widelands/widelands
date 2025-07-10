/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "wui/soldier_preference_control.h"

#include <functional>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/radiobutton.h"
#include "wui/interactive_base.h"

/**
 * Soldier preference radiobutton
 */
struct SoldierPreferenceControl : UI::Box {
	SoldierPreferenceControl(UI::Panel& parent, InteractiveBase& ib, Widelands::MapObject& building_or_ship);

private:
	void set_soldier_preference(int32_t changed_to);
	void think() override;

	InteractiveBase& ibase_;
	Widelands::OPtr<Widelands::MapObject> building_or_ship_;
	UI::Radiogroup soldier_preference_;
};

SoldierPreferenceControl::SoldierPreferenceControl(UI::Panel& parent,
                                                   InteractiveBase& ib,
                                                   Widelands::MapObject& building_or_ship)
   : UI::Box(&parent, UI::PanelStyle::kWui, "soldier_preference", 0, 0, UI::Box::Horizontal),
     ibase_(ib),
     building_or_ship_(&building_or_ship) {

	bool can_act = ibase_.can_act(building_or_ship.owner().player_number());

	// Make sure the creation order is consistent with enum SoldierPreference!
	soldier_preference_.add_button(
	   this, UI::PanelStyle::kWui, "prefer_rookies", Vector2i::zero(),
	   g_image_cache->get("images/wui/buildings/prefer_rookies.png"), _("Prefer rookies"));
	soldier_preference_.add_button(
	   this, UI::PanelStyle::kWui, "prefer_heroes", Vector2i::zero(),
	   g_image_cache->get("images/wui/buildings/prefer_heroes.png"), _("Prefer heroes"));
	soldier_preference_.add_button(this, UI::PanelStyle::kWui, "prefer_any", Vector2i::zero(),
	                               g_image_cache->get("images/wui/buildings/prefer_any.png"),
	                               _("No preference"));
	UI::Radiobutton* button = soldier_preference_.get_first_button();
	while (button != nullptr) {
		add(button);
		button = button->next_button();
	}

	think();  // update soldier preference from building or ship

	if (can_act) {
		soldier_preference_.changedto.connect([this](int32_t a) { set_soldier_preference(a); });
	} else {
		soldier_preference_.set_enabled(false);
	}
}

void SoldierPreferenceControl::think() {
	MutexLock m(MutexLock::ID::kObjects);
	const Widelands::MapObject* object = building_or_ship_.get(ibase_.egbase());
	if (object == nullptr) {
		return;
	}

	Widelands::SoldierPreference
	current_pref = static_cast<Widelands::SoldierPreference>(soldier_preference_.get_state());

	switch (object->descr().type()) {
	case Widelands::MapObjectType::MILITARYSITE: {
		upcast(const Widelands::MilitarySite, ms, object);
		current_pref = ms->get_soldier_preference();
		break;
	}
	case Widelands::MapObjectType::WAREHOUSE: {
		upcast(const Widelands::Warehouse, wh, object);
		current_pref = wh->get_soldier_preference();
		break;
	}
	case Widelands::MapObjectType::SHIP: {
		upcast(const Widelands::Ship, ship, object);
		current_pref = ship->get_soldier_preference();
		break;
	}
	case Widelands::MapObjectType::TRAININGSITE: {
		upcast(const Widelands::TrainingSite, ts, object);
		current_pref = ts->get_build_heroes();
		break;
	}
	case Widelands::MapObjectType::CONSTRUCTIONSITE: {
		upcast(const Widelands::ConstructionSite, cs, object);
		if (upcast(const Widelands::MilitarysiteSettings, ms, cs->get_settings())) {
			current_pref = ms->soldier_preference;
		} else if (upcast(const Widelands::TrainingsiteSettings, ts, cs->get_settings())) {
			current_pref = ts->build_heroes;
		} else if (upcast(const Widelands::WarehouseSettings, wh, cs->get_settings())) {
			current_pref = wh->soldier_preference;
		}
		break;
	}
	default:
		NEVER_HERE();
	}

	soldier_preference_.set_state(static_cast<uint8_t>(current_pref), false);
}

void SoldierPreferenceControl::set_soldier_preference(int32_t changed_to) {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::MapObject* object = building_or_ship_.get(ibase_.egbase());
	if (object == nullptr) {
		return;
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_soldier_preference(
		   *object, static_cast<Widelands::SoldierPreference>(changed_to));
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

UI::Panel* create_soldier_preference_control(UI::Panel& parent,
                                             InteractiveBase& ib,
                                             Widelands::MapObject& building_or_ship) {
	return new SoldierPreferenceControl(parent, ib, building_or_ship);
}
