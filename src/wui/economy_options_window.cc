/*
 * Copyright (C) 2008-2016 by the Widelands Development Team
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

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/button.h"

#include <boost/lexical_cast.hpp>

using Widelands::Economy;
using Widelands::EditorGameBase;
using Widelands::Game;
using Widelands::WareDescr;
using Widelands::DescriptionIndex;
using Widelands::WorkerDescr;

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";

EconomyOptionsWindow::EconomyOptionsWindow(InteractiveGameBase& parent, Economy& economy)
   : UI::Window(&parent, "economy_options", 0, 0, 0, 0, _("Economy options")),
     economy_number_(economy.owner().get_economy_number(&economy)),
     owner_(economy.owner()),
     tabpanel_(this, 0, 0, g_gr->images().get("images/ui_basic/but1.png")),
     ware_panel_(new EconomyOptionsWarePanel(&tabpanel_, parent, economy_number_, owner_)),
     worker_panel_(new EconomyOptionsWorkerPanel(&tabpanel_, parent, economy_number_, owner_)) {
	set_center_panel(&tabpanel_);

	tabpanel_.add("wares", g_gr->images().get(pic_tab_wares), ware_panel_, _("Wares"));
	tabpanel_.add("workers", g_gr->images().get(pic_tab_workers), worker_panel_, _("Workers"));
	economy.set_has_window(true);

	economynotes_subscriber_ = Notifications::subscribe<Widelands::NoteEconomyWindow>(
	   [this](const Widelands::NoteEconomyWindow& note) {
		   if (note.old_economy == economy_number_) {
			   switch (note.action) {
			   case Widelands::NoteEconomyWindow::Action::kRefresh:
				   economy_number_ = note.new_economy;
				   ware_panel_->set_economy_number(note.new_economy);
				   worker_panel_->set_economy_number(note.new_economy);
				   owner_.get_economy_by_number(economy_number_)->set_has_window(true);
				   move_to_top();
				   break;
			   case Widelands::NoteEconomyWindow::Action::kClose:
				   // Make sure that the panels stop thinking first.
				   ware_panel_->die();
				   worker_panel_->die();
				   owner_.get_economy_by_number(economy_number_)->set_has_window(false);
				   die();
				   break;
			   }
		   }
		});
}

EconomyOptionsWindow::~EconomyOptionsWindow() {
	owner_.get_economy_by_number(economy_number_)->set_has_window(false);
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
		for (const DescriptionIndex& worker_index : owner_tribe.workers()) {
			const WorkerDescr* worker_descr = owner_tribe.get_worker_descr(worker_index);
			if (!worker_descr->has_demand_check()) {
				hide_ware(worker_index);
			}
		}
	} else {
		for (const DescriptionIndex& ware_index : owner_tribe.wares()) {
			const WareDescr* ware_descr = owner_tribe.get_ware_descr(ware_index);
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
	Economy& economy = *owner_.get_economy_by_number(economy_number_);
	return boost::lexical_cast<std::string>(get_type() == Widelands::wwWORKER ?
	                                           economy.worker_target_quantity(ware).permanent :
	                                           economy.ware_target_quantity(ware).permanent);
}

/**
 * Wraps the wares display together with some buttons
 */
EconomyOptionsWindow::EconomyOptionsWarePanel::EconomyOptionsWarePanel(UI::Panel* parent,
                                                                       InteractiveGameBase& igbase,
                                                                       size_t economy_number,
                                                                       Widelands::Player& owner)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     economy_number_(economy_number),
     owner_(owner),
     can_act_(igbase.can_act(owner.player_number())),
     display_(this, 0, 0, owner.tribe(), Widelands::wwWARE, can_act_, economy_number, owner) {
	add(&display_, UI::Align::kLeft, true);

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(buttons, UI::Align::kLeft);

	UI::Button* b = nullptr;

#define ADD_WARE_BUTTON(callback, text, tooltip)                                                   \
	b = new UI::Button(buttons, #callback, 0, 0, 34, 34,                                            \
	                   g_gr->images().get("images/ui_basic/but4.png"), text, tooltip, can_act_);    \
	b->sigclicked.connect(boost::bind(&EconomyOptionsWarePanel::callback, this));                   \
	buttons->add(b, UI::Align::kHCenter);
	ADD_WARE_BUTTON(decrease_target, "-", _("Decrease target"))
	b->set_repeating(true);
	ADD_WARE_BUTTON(increase_target, "+", _("Increase target"))
	b->set_repeating(true);
	buttons->add_space(8);
	ADD_WARE_BUTTON(reset_target, "R", _("Reset to default"))
}

void EconomyOptionsWindow::EconomyOptionsWarePanel::set_economy_number(size_t economy_number) {
	economy_number_ = economy_number;
	display_.set_economy_number(economy_number);
}

void EconomyOptionsWindow::EconomyOptionsWarePanel::decrease_target() {
	Economy& economy = *owner_.get_economy_by_number(economy_number_);
	for (const DescriptionIndex& ware_index : owner_.tribe().wares()) {
		if (display_.ware_selected(ware_index)) {
			const Economy::TargetQuantity& tq = economy.ware_target_quantity(ware_index);
			if (0 < tq.permanent) {
				Game& game = dynamic_cast<Game&>(owner_.egbase());
				game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
				   game.get_gametime(), owner_.player_number(), economy_number_, ware_index,
				   tq.permanent - 1));
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsWarePanel::increase_target() {
	Economy& economy = *owner_.get_economy_by_number(economy_number_);
	for (const DescriptionIndex& ware_index : owner_.tribe().wares()) {
		if (display_.ware_selected(ware_index)) {
			const Economy::TargetQuantity& tq = economy.ware_target_quantity(ware_index);
			Game& game = dynamic_cast<Game&>(owner_.egbase());
			game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
			   game.get_gametime(), owner_.player_number(), economy_number_, ware_index,
			   tq.permanent + 1));
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsWarePanel::reset_target() {

	for (const DescriptionIndex& ware_index : owner_.tribe().wares()) {
		if (display_.ware_selected(ware_index)) {
			Game& game = dynamic_cast<Game&>(owner_.egbase());
			game.send_player_command(*new Widelands::CmdResetWareTargetQuantity(
			   game.get_gametime(), owner_.player_number(), economy_number_, ware_index));
		}
	}
}

EconomyOptionsWindow::EconomyOptionsWorkerPanel::EconomyOptionsWorkerPanel(
   UI::Panel* parent, InteractiveGameBase& igbase, size_t economy_number, Widelands::Player& owner)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     economy_number_(economy_number),
     owner_(owner),
     can_act_(igbase.can_act(owner.player_number())),
     display_(this, 0, 0, owner.tribe(), Widelands::wwWORKER, can_act_, economy_number, owner) {
	add(&display_, UI::Align::kLeft, true);

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(buttons, UI::Align::kLeft);

	UI::Button* b = nullptr;
#define ADD_WORKER_BUTTON(callback, text, tooltip)                                                 \
	b = new UI::Button(buttons, #callback, 0, 0, 34, 34,                                            \
	                   g_gr->images().get("images/ui_basic/but4.png"), text, tooltip, can_act_);    \
	b->sigclicked.connect(boost::bind(&EconomyOptionsWorkerPanel::callback, this));                 \
	buttons->add(b, UI::Align::kHCenter);

	ADD_WORKER_BUTTON(decrease_target, "-", _("Decrease target"))
	b->set_repeating(true);
	ADD_WORKER_BUTTON(increase_target, "+", _("Increase target"))
	b->set_repeating(true);
	buttons->add_space(8);
	ADD_WORKER_BUTTON(reset_target, "R", _("Reset to default"))
}

void EconomyOptionsWindow::EconomyOptionsWorkerPanel::set_economy_number(size_t economy_number) {
	economy_number_ = economy_number;
	display_.set_economy_number(economy_number);
}

void EconomyOptionsWindow::EconomyOptionsWorkerPanel::decrease_target() {
	Economy& economy = *owner_.get_economy_by_number(economy_number_);
	for (const DescriptionIndex& worker_index : owner_.tribe().workers()) {
		if (display_.ware_selected(worker_index)) {
			const Economy::TargetQuantity& tq = economy.worker_target_quantity(worker_index);
			if (0 < tq.permanent) {
				Game& game = dynamic_cast<Game&>(owner_.egbase());
				game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
				   game.get_gametime(), owner_.player_number(), economy_number_, worker_index,
				   tq.permanent - 1));
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsWorkerPanel::increase_target() {
	Economy& economy = *owner_.get_economy_by_number(economy_number_);
	for (const DescriptionIndex& worker_index : owner_.tribe().workers()) {
		if (display_.ware_selected(worker_index)) {
			const Economy::TargetQuantity& tq = economy.worker_target_quantity(worker_index);
			Game& game = dynamic_cast<Game&>(owner_.egbase());
			game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
			   game.get_gametime(), owner_.player_number(), economy_number_, worker_index,
			   tq.permanent + 1));
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsWorkerPanel::reset_target() {
	for (const DescriptionIndex& worker_index : owner_.tribe().workers()) {
		if (display_.ware_selected(worker_index)) {
			Game& game = dynamic_cast<Game&>(owner_.egbase());
			game.send_player_command(*new Widelands::CmdResetWorkerTargetQuantity(
			   game.get_gametime(), owner_.player_number(), economy_number_, worker_index));
		}
	}
}
