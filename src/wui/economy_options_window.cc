/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#include "wui/economy_options_window.h"

#include <boost/lexical_cast.hpp>

#include "graphic/graphic.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/button.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";

EconomyOptionsWindow::EconomyOptionsWindow(UI::Panel* parent,
                                           Widelands::Economy* economy,
                                           bool can_act)
   : UI::Window(parent, "economy_options", 0, 0, 0, 0, _("Economy options")),
     economy_number_(economy->owner().get_economy_number(economy)),
     owner_(economy->owner()),
     tabpanel_(this, UI::TabPanelStyle::kWuiDark),
     ware_panel_(
        new EconomyOptionsPanel(&tabpanel_, can_act, Widelands::wwWARE, economy_number_, owner_)),
     worker_panel_(new EconomyOptionsPanel(
        &tabpanel_, can_act, Widelands::wwWORKER, economy_number_, owner_)) {
	set_center_panel(&tabpanel_);

	tabpanel_.add("wares", g_gr->images().get(pic_tab_wares), ware_panel_, _("Wares"));
	tabpanel_.add("workers", g_gr->images().get(pic_tab_workers), worker_panel_, _("Workers"));
	economy->set_has_window(true);
	economynotes_subscriber_ = Notifications::subscribe<Widelands::NoteEconomy>(
	   [this](const Widelands::NoteEconomy& note) { on_economy_note(note); });
}

EconomyOptionsWindow::~EconomyOptionsWindow() {
	owner_.get_economy_by_number(economy_number_)->set_has_window(false);
}

void EconomyOptionsWindow::on_economy_note(const Widelands::NoteEconomy& note) {
	if (note.old_economy == economy_number_) {
		switch (note.action) {
		case Widelands::NoteEconomy::Action::kMerged:
			economy_number_ = note.new_economy;
			ware_panel_->set_economy_number(note.new_economy);
			worker_panel_->set_economy_number(note.new_economy);
			owner_.get_economy_by_number(economy_number_)->set_has_window(true);
			move_to_top();
			break;
		case Widelands::NoteEconomy::Action::kDeleted:
			// Make sure that the panels stop thinking first.
			ware_panel_->die();
			worker_panel_->die();
			owner_.get_economy_by_number(economy_number_)->set_has_window(false);
			die();
			break;
		}
	}
}

EconomyOptionsWindow::TargetWaresDisplay::TargetWaresDisplay(UI::Panel* const parent,
                                                             int32_t const x,
                                                             int32_t const y,
                                                             const Widelands::TribeDescr& tribe,
                                                             Widelands::WareWorker type,
                                                             bool selectable,
                                                             size_t economy_number,
                                                             Widelands::Player& owner)
   : AbstractWaresDisplay(parent, x, y, tribe, type, selectable),
     economy_number_(economy_number),
     owner_(owner) {
	const Widelands::TribeDescr& owner_tribe = owner_.tribe();
	if (type == Widelands::wwWORKER) {
		for (const Widelands::DescriptionIndex& worker_index : owner_tribe.workers()) {
			const Widelands::WorkerDescr* worker_descr = owner_tribe.get_worker_descr(worker_index);
			if (!worker_descr->has_demand_check()) {
				hide_ware(worker_index);
			}
		}
	} else {
		for (const Widelands::DescriptionIndex& ware_index : owner_tribe.wares()) {
			const Widelands::WareDescr* ware_descr = owner_tribe.get_ware_descr(ware_index);
			if (!ware_descr->has_demand_check(owner_tribe.name())) {
				hide_ware(ware_index);
			}
		}
	}
}

void EconomyOptionsWindow::TargetWaresDisplay::set_economy_number(size_t economy_number) {
	economy_number_ = economy_number;
}

std::string
EconomyOptionsWindow::TargetWaresDisplay::info_for_ware(Widelands::DescriptionIndex const ware) {
	Widelands::Economy& economy = *owner_.get_economy_by_number(economy_number_);
	return boost::lexical_cast<std::string>(get_type() == Widelands::wwWORKER ?
	                                           economy.worker_target_quantity(ware).permanent :
	                                           economy.ware_target_quantity(ware).permanent);
}

/**
 * Wraps the wares/workers display together with some buttons
 */
EconomyOptionsWindow::EconomyOptionsPanel::EconomyOptionsPanel(UI::Panel* parent,
                                                               bool can_act,
                                                               Widelands::WareWorker type,
                                                               size_t economy_number,
                                                               Widelands::Player& owner)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     type_(type),
     economy_number_(economy_number),
     owner_(owner),
     can_act_(can_act),
     display_(this, 0, 0, owner.tribe(), type_, can_act_, economy_number, owner) {
	add(&display_, UI::Box::Resizing::kFullSize);

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(buttons);

	UI::Button* b = new UI::Button(buttons, "decrease_target", 0, 0, 34, 34,
	                               UI::ButtonStyle::kWuiMenu, "-", _("Decrease target"));
	b->set_enabled(can_act_);
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::change_target, this, -1));
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);

	b = new UI::Button(buttons, "increase_target", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, "+",
	                   _("Increase target"));
	b->set_enabled(can_act_);
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::change_target, this, 1));
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);

	b = new UI::Button(
	   buttons, "reset_target", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, "R", _("Reset to default"));
	b->set_enabled(can_act_);
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::reset_target, this));
	buttons->add(b);
}

void EconomyOptionsWindow::EconomyOptionsPanel::set_economy_number(size_t economy_number) {
	economy_number_ = economy_number;
	display_.set_economy_number(economy_number);
}

void EconomyOptionsWindow::EconomyOptionsPanel::change_target(int amount) {
	Widelands::Economy& economy = *owner_.get_economy_by_number(economy_number_);
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(owner_.egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? owner_.tribe().wares() : owner_.tribe().workers();
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			const Widelands::Economy::TargetQuantity& tq =
			   is_wares ? economy.ware_target_quantity(index) : economy.worker_target_quantity(index);
			// Don't allow negative new amount.
			if (amount >= 0 || -amount <= static_cast<int>(tq.permanent)) {
				if (is_wares) {
					game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
					   game.get_gametime(), owner_.player_number(), economy_number_, index,
					   tq.permanent + amount));
				} else {
					game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
					   game.get_gametime(), owner_.player_number(), economy_number_, index,
					   tq.permanent + amount));
				}
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::reset_target() {
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(owner_.egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? owner_.tribe().wares() : owner_.tribe().workers();
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			if (is_wares) {
				game.send_player_command(*new Widelands::CmdResetWareTargetQuantity(
				   game.get_gametime(), owner_.player_number(), economy_number_, index));
			} else {
				game.send_player_command(*new Widelands::CmdResetWorkerTargetQuantity(
				   game.get_gametime(), owner_.player_number(), economy_number_, index));
			}
		}
	}
}
