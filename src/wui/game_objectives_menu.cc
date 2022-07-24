/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "wui/game_objectives_menu.h"

#include "logic/game_data_error.h"
#include "logic/objective.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "wui/interactive_player.h"

GameObjectivesMenu::GameObjectivesMenu(InteractivePlayer& parent,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "objectives", &registry, 300, 200, _("Objectives")),
     iplayer_(parent),
     objective_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     objective_list_(&objective_box_, 0, 0, 550, 180, UI::PanelStyle::kWui),
     objective_text_(&objective_box_,
                     0,
                     0,
                     100,
                     150,
                     UI::PanelStyle::kWui,
                     "",
                     UI::Align::kLeft,
                     UI::MultilineTextarea::ScrollMode::kScrollNormalForced) {

	objective_box_.add(&objective_list_, UI::Box::Resizing::kExpandBoth);
	objective_box_.add(&objective_text_, UI::Box::Resizing::kExpandBoth);

	objective_list_.selected.connect([this](uint32_t a) { selected(a); });

	set_center_panel(&objective_box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	initialization_complete();
}

void GameObjectivesMenu::think() {
	//  Adjust the list according to the game state.
	for (const auto& pair : iplayer_.game().map().objectives()) {
		const Widelands::Objective& obj = *(pair.second);
		bool should_show = obj.visible() && !obj.done();
		uint32_t const list_size = objective_list_.size();
		for (uint32_t j = 0;; ++j) {
			if (j == list_size) {  //  the objective is not in our list
				if (should_show) {
					objective_list_.add(obj.descname(), obj);
				}
				break;
			}
			if (&objective_list_[j] == &obj) {  //  the objective is in our list
				if (!should_show) {
					objective_list_.remove(j);
				} else if (objective_list_[j].descname() != obj.descname() ||
				           objective_list_[j].descr() != obj.descr()) {
					// Update
					objective_list_.remove(j);
					objective_list_.add(obj.descname(), obj);
				}
				break;
			}
		}
	}
	objective_list_.sort();
	if (!objective_list_.empty() && !objective_list_.has_selection()) {
		objective_list_.select(0);
	}
}

/**
 * An entry in the objectives menu has been selected
 */
void GameObjectivesMenu::selected(uint32_t const t) {
	objective_text_.set_text(t == ListType::no_selection_index() ? "" : objective_list_[t].descr());
}

void GameObjectivesMenu::draw(RenderTarget& rt) {
	UI::UniqueWindow::draw(rt);
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& GameObjectivesMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractivePlayer&>(ib).objectives_;
			r.create();
			assert(r.window);
			GameObjectivesMenu& m = dynamic_cast<GameObjectivesMenu&>(*r.window);
			m.think();  // Fills the list
			m.objective_list_.select(fr.unsigned_32());
			return m;
		}
		throw Widelands::UnhandledVersionError(
		   "Objectives Menu", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("objectives menu: %s", e.what());
	}
}
void GameObjectivesMenu::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_32(objective_list_.selection_index());
}
