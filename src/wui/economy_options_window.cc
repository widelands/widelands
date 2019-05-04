/*
 * Copyright (C) 2008-2019 by the Widelands Development Team
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
#include "logic/filesystem_constants.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "profile/profile.h"
#include "ui_basic/button.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";

EconomyOptionsWindow::EconomyOptionsWindow(UI::Panel* parent,
                                           Widelands::Economy* economy,
                                           bool can_act)
   : UI::Window(parent, "economy_options", 0, 0, 0, 0, _("Economy options")),
     serial_(economy->serial()),
     player_(&economy->owner()),
     tabpanel_(this, UI::TabPanelStyle::kWuiDark),
     ware_panel_(new EconomyOptionsPanel(&tabpanel_, this, serial_, player_, can_act, Widelands::wwWARE)),
     worker_panel_(
        new EconomyOptionsPanel(&tabpanel_, this, serial_, player_, can_act, Widelands::wwWORKER)) {
	set_center_panel(&tabpanel_);

	tabpanel_.add("wares", g_gr->images().get(pic_tab_wares), ware_panel_, _("Wares"));
	tabpanel_.add("workers", g_gr->images().get(pic_tab_workers), worker_panel_, _("Workers"));
	economy->set_has_window(true);
	economynotes_subscriber_ = Notifications::subscribe<Widelands::NoteEconomy>(
	   [this](const Widelands::NoteEconomy& note) { on_economy_note(note); });

	read_targets();
}

EconomyOptionsWindow::~EconomyOptionsWindow() {
	Widelands::Economy* economy = player_->get_economy(serial_);
	if (economy != nullptr) {
		economy->set_has_window(false);
	}
}

void EconomyOptionsWindow::on_economy_note(const Widelands::NoteEconomy& note) {
	if (note.old_economy == serial_) {
		switch (note.action) {
		case Widelands::NoteEconomy::Action::kMerged: {
			serial_ = note.new_economy;
			Widelands::Economy* economy = player_->get_economy(serial_);
			if (economy == nullptr) {
				die();
				return;
			}
			economy->set_has_window(true);
			ware_panel_->set_economy(note.new_economy);
			worker_panel_->set_economy(note.new_economy);
			move_to_top();
		} break;
		case Widelands::NoteEconomy::Action::kDeleted:
			// Make sure that the panels stop thinking first.
			die();
			break;
		}
	}
}

EconomyOptionsWindow::TargetWaresDisplay::TargetWaresDisplay(UI::Panel* const parent,
                                                             int32_t const x,
                                                             int32_t const y,
                                                             Widelands::Serial serial,
                                                             Widelands::Player* player,
                                                             Widelands::WareWorker type,
                                                             bool selectable)
   : AbstractWaresDisplay(parent, x, y, player->tribe(), type, selectable),
     serial_(serial),
     player_(player) {
	const Widelands::TribeDescr& owner_tribe = player->tribe();
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

void EconomyOptionsWindow::TargetWaresDisplay::set_economy(Widelands::Serial serial) {
	serial_ = serial;
}

std::string
EconomyOptionsWindow::TargetWaresDisplay::info_for_ware(Widelands::DescriptionIndex const ware) {
	Widelands::Economy* economy = player_->get_economy(serial_);
	if (economy == nullptr) {
		die();
		return *(new std::string());
	}
	return boost::lexical_cast<std::string>(get_type() == Widelands::wwWORKER ?
	                                           economy->worker_target_quantity(ware).permanent :
	                                           economy->ware_target_quantity(ware).permanent);
}

/**
 * Wraps the wares/workers display together with some buttons
 */
EconomyOptionsWindow::EconomyOptionsPanel::EconomyOptionsPanel(UI::Panel* parent,
		                    								   EconomyOptionsWindow* eco_window,
                                                               Widelands::Serial serial,
                                                               Widelands::Player* player,
                                                               bool can_act,
                                                               Widelands::WareWorker type)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     serial_(serial),
     player_(player),
     type_(type),
     can_act_(can_act),
     display_(this, 0, 0, serial_, player_, type_, can_act_),
     economy_options_window_(eco_window),
     dropdown_box_(this, 0, 0, UI::Box::Horizontal),
     dropdown_(&dropdown_box_, 0, 0, 160, 200, 34, "", UI::DropdownType::kTextual, UI::PanelStyle::kWui) {
	add(&display_, UI::Box::Resizing::kFullSize);

	if (!can_act_) {
		return;
	}
	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(buttons);

	UI::Button* b = new UI::Button(buttons, "decrease_target", 0, 0, 34, 34,
	                               UI::ButtonStyle::kWuiMenu, "-", _("Decrease target"));
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::change_target, this, -1));
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);

	b = new UI::Button(buttons, "increase_target", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, "+",
	                   _("Increase target"));
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::change_target, this, 1));
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);

	b = new UI::Button(
	   buttons, "reset_target", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, "R", _("Reset to chosen default"));
	b->sigclicked.connect(boost::bind(&EconomyOptionsPanel::reset_target, this));
	buttons->add(b);
	buttons->add_space(8);

	b = new UI::Button(
	   buttons, "save_targets", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, "S", _("Save target settings"));
	b->sigclicked.connect(boost::bind(&EconomyOptionsWindow::create_target, eco_window));
	buttons->add(b);

	dropdown_.set_tooltip(_("Profile to apply when clicking Reset"));
	add(&dropdown_box_, UI::Box::Resizing::kFullSize);
	dropdown_box_.set_size(40, 20);  // Prevent assert failures
	dropdown_box_.add(&dropdown_, UI::Box::Resizing::kFullSize);
}

void EconomyOptionsWindow::EconomyOptionsPanel::set_economy(Widelands::Serial serial) {
	serial_ = serial;
	display_.set_economy(serial);
}

void EconomyOptionsWindow::EconomyOptionsPanel::change_target(int amount) {
	Widelands::Economy* economy = player_->get_economy(serial_);
	if (economy == nullptr) {
		die();
		return;
	}
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(player_->egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? player_->tribe().wares() : player_->tribe().workers();
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			const Widelands::Economy::TargetQuantity& tq = is_wares ?
			                                                  economy->ware_target_quantity(index) :
			                                                  economy->worker_target_quantity(index);
			// Don't allow negative new amount.
			if (amount >= 0 || -amount <= static_cast<int>(tq.permanent)) {
				if (is_wares) {
					game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
					   game.get_gametime(), player_->player_number(), serial_, index,
					   tq.permanent + amount));
				} else {
					game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
					   game.get_gametime(), player_->player_number(), serial_, index,
					   tq.permanent + amount));
				}
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::reset_target() {
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(player_->egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? player_->tribe().wares() : player_->tribe().workers();
	const PredefinedTargets settings = economy_options_window_->get_predefined_targets().at(dropdown_.get_selected());
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			if (is_wares) {
				game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
						game.get_gametime(), player_->player_number(), serial_, index, settings.wares.at(index)));
			} else {
				game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
						game.get_gametime(), player_->player_number(), serial_, index, settings.workers.at(index)));
			}
		}
	}
}

void EconomyOptionsWindow::read_targets(const std::string& select) {
	predefined_targets_.clear();
	const Widelands::Tribes& tribes = player_->egbase().tribes();
	const Widelands::TribeDescr& tribe = player_->tribe();

	{
		PredefinedTargets t;
		for (Widelands::DescriptionIndex di : tribe.wares()) {
			const Widelands::WareDescr* descr = tribes.get_ware_descr(di);
			if (descr->has_demand_check(tribe.name())) {
				t.wares.emplace(di, descr->default_target_quantity(tribe.name()));
			}
		}
		for (Widelands::DescriptionIndex di : tribe.workers()) {
			const Widelands::WorkerDescr* descr = tribes.get_worker_descr(di);
			if (descr->has_demand_check()) {
				t.workers.emplace(di, descr->default_target_quantity());
			}
		}
		predefined_targets_.emplace(kDefaultEconomyProfile, t);
	}

	std::string complete_filename = kEconomyProfilesDir + g_fs->file_separator() + player_->tribe().name();
	Profile profile;
	profile.read(complete_filename.c_str());
	while (Section* section = profile.get_next_section()) {
		PredefinedTargets t;
		while (Section::Value* v = section->get_next_val()) {
			const std::string name = std::string(v->get_name());
			Widelands::DescriptionIndex di = tribes.ware_index(name);
			if (di == Widelands::INVALID_INDEX) {
				di = tribes.worker_index(name);
				assert(di != Widelands::INVALID_INDEX);
				t.workers.emplace(di, v->get_natural());
			} else {
				t.wares.emplace(di, v->get_natural());
			}
		}
		predefined_targets_.emplace(std::string(section->get_name()), t);
	}

	ware_panel_->update_profiles(select);
	worker_panel_->update_profiles(select);
}

void EconomyOptionsWindow::EconomyOptionsPanel::update_profiles(const std::string& select) {
	dropdown_.clear();
	for (const auto& pair : economy_options_window_->get_predefined_targets()) {
		dropdown_.add(_(pair.first), pair.first, nullptr, pair.first == select);
	}
}

void EconomyOptionsWindow::create_target() {
	/* Show a dialog similar to the savegame dialog:
	 * The main part of the window is a Table listing all current profiles.
	 * Below is a Textbox for the name.
	 * The "Delete" button removes the selected profile(s) from the list.
	 * The "Save" button checks whether a profile will be overwritten; if so, a confirm box pops up.
	 * The profile kDefaultEconomyProfile cannot be deleted or overwritten.
	 */



	std::string name = "CustomSettings" + std::to_string(predefined_targets_.size());
	log("NOCOM: EconomyOptionsWindow::create_target(): not yet implemented, saving current settings as \"%s\"\n", name.c_str());
	const Widelands::Tribes& tribes = player_->egbase().tribes();
	const Widelands::TribeDescr& tribe = player_->tribe();
	Widelands::Economy* economy = player_->get_economy(serial_);
	PredefinedTargets t;
	for (Widelands::DescriptionIndex di : tribe.wares()) {
		if (tribes.get_ware_descr(di)->has_demand_check(tribe.name())) {
			t.wares[di] = economy->ware_target_quantity(di).permanent;
		}
	}
	for (Widelands::DescriptionIndex di : tribe.workers()) {
		if (tribes.get_worker_descr(di)->has_demand_check()) {
			t.workers[di] = economy->worker_target_quantity(di).permanent;
		}
	}
	predefined_targets_[name] = t;



	save_targets();
	ware_panel_->update_profiles(name);
	worker_panel_->update_profiles(name);
}

// Write all predefined settings (except the defaults) to file
void EconomyOptionsWindow::save_targets() {
	const Widelands::Tribes& tribes = player_->egbase().tribes();
	Profile profile;
	for (const auto& pair : predefined_targets_) {
		if (pair.first == kDefaultEconomyProfile) {
			continue;
		}
		Section& section = profile.create_section(pair.first.c_str());
		for (const auto& setting : pair.second.wares) {
			section.set_natural(tribes.get_ware_descr(setting.first)->name().c_str(), setting.second);
		}
		for (const auto& setting : pair.second.workers) {
			section.set_natural(tribes.get_worker_descr(setting.first)->name().c_str(), setting.second);
		}
	}

	g_fs->ensure_directory_exists(kEconomyProfilesDir);
	std::string complete_filename = kEconomyProfilesDir + g_fs->file_separator() + player_->tribe().name();
	profile.write(complete_filename.c_str(), false);
}
