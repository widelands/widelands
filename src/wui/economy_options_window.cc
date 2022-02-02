/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include "wui/economy_options_window.h"

#include <memory>

#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/style_manager.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_base.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";

constexpr int kDesiredWidth = 216;

EconomyOptionsWindow::EconomyOptionsWindow(Panel* parent,
                                           Widelands::Descriptions* descriptions,
                                           Widelands::Economy* ware_economy,
                                           Widelands::Economy* worker_economy,
                                           Widelands::WareWorker type,
                                           bool can_act)
   : UI::Window(parent, UI::WindowStyle::kWui, "economy_options", 0, 0, 0, 0, _("Economy options")),
     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     ware_serial_(ware_economy->serial()),
     worker_serial_(worker_economy->serial()),
     player_(&ware_economy->owner()),
     tabpanel_(&main_box_, UI::TabPanelStyle::kWuiDark),
     ware_panel_(new EconomyOptionsPanel(
        &tabpanel_, this, ware_serial_, player_, can_act, Widelands::wwWARE, kDesiredWidth)),
     worker_panel_(new EconomyOptionsPanel(
        &tabpanel_, this, worker_serial_, player_, can_act, Widelands::wwWORKER, kDesiredWidth)),
     dropdown_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     dropdown_(&dropdown_box_,
               "economy_profiles",
               0,
               0,
               174,
               10,
               34,
               "",
               UI::DropdownType::kTextual,
               UI::PanelStyle::kWui,
               UI::ButtonStyle::kWuiSecondary),
     time_last_thought_(0),
     save_profile_dialog_(nullptr),
     descriptions_(descriptions) {
	set_center_panel(&main_box_);

	tabpanel_.add("wares", g_image_cache->get(pic_tab_wares), ware_panel_, _("Wares"));
	tabpanel_.add("workers", g_image_cache->get(pic_tab_workers), worker_panel_, _("Workers"));

	UI::Box* buttons = new UI::Box(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Button* b = new UI::Button(
	   buttons, "decrease_target_fast", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
	   g_image_cache->get("images/ui_basic/scrollbar_down_fast.png"), _("Decrease target by 10"));
	b->sigclicked.connect([this] { change_target(-10); });
	buttons->add(b);
	b->set_repeating(true);
	b->set_enabled(can_act);
	buttons->add_space(6);
	b = new UI::Button(buttons, "decrease_target", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
	                   g_image_cache->get("images/ui_basic/scrollbar_down.png"),
	                   _("Decrease target"));
	b->sigclicked.connect([this] { change_target(-1); });
	buttons->add(b);
	b->set_repeating(true);
	b->set_enabled(can_act);
	buttons->add_space(6);

	b = new UI::Button(buttons, "toggle_infinite", 0, 0, 32, 28, UI::ButtonStyle::kWuiSecondary,
	                   g_image_cache->get("images/wui/menus/infinity.png"),
	                   _("Toggle infinite target"));
	b->sigclicked.connect([this] { toggle_infinite(); });
	buttons->add(b);
	b->set_repeating(false);
	b->set_enabled(can_act);
	buttons->add_space(6);

	b = new UI::Button(buttons, "increase_target", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
	                   g_image_cache->get("images/ui_basic/scrollbar_up.png"), _("Increase target"));
	b->sigclicked.connect([this] { change_target(1); });
	buttons->add(b);
	b->set_repeating(true);
	b->set_enabled(can_act);
	buttons->add_space(6);
	b = new UI::Button(buttons, "increase_target_fast", 0, 0, 40, 28, UI::ButtonStyle::kWuiSecondary,
	                   g_image_cache->get("images/ui_basic/scrollbar_up_fast.png"),
	                   _("Increase target by 10"));
	b->sigclicked.connect([this] { change_target(10); });
	buttons->add(b);
	b->set_repeating(true);
	b->set_enabled(can_act);

	dropdown_.set_tooltip(_("Profile to apply to the selected items"));
	dropdown_box_.set_size(40, 20);  // Prevent assert failures
	dropdown_box_.add(&dropdown_, UI::Box::Resizing::kFullSize);
	if (can_act) {
		dropdown_.selected.connect([this] { reset_target(); });
	} else {
		dropdown_.set_enabled(false);
	}

	b = new UI::Button(&dropdown_box_, "save_targets", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
	                   g_image_cache->get("images/wui/menus/save_game.png"),
	                   _("Save target settings"));
	b->sigclicked.connect([this] { create_target(); });
	dropdown_box_.add_space(8);
	dropdown_box_.add(b);

	main_box_.add(&tabpanel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_space(8);
	main_box_.add(buttons, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_space(8);
	main_box_.add(&dropdown_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	ware_economy->set_options_window(static_cast<void*>(this));
	worker_economy->set_options_window(static_cast<void*>(this));
	economynotes_subscriber_ = Notifications::subscribe<Widelands::NoteEconomy>(
	   [this](const Widelands::NoteEconomy& note) { on_economy_note(note); });
	profilenotes_subscriber_ =
	   Notifications::subscribe<NoteEconomyProfile>([this](const NoteEconomyProfile& n) {
		   if (n.ware_serial == ware_serial_ && n.worker_serial == worker_serial_) {
			   // We already updated ourself before we changed something
			   return;
		   }
		   read_targets();
		   if (save_profile_dialog_) {
			   save_profile_dialog_->update_table();
		   }
	   });

	read_targets();

	activate_tab(type);

	initialization_complete();
}

EconomyOptionsWindow::~EconomyOptionsWindow() {
	if (Widelands::Economy* e_wa = player_->get_economy(ware_serial_)) {
		e_wa->set_options_window(nullptr);
	}
	if (Widelands::Economy* e_wo = player_->get_economy(worker_serial_)) {
		e_wo->set_options_window(nullptr);
	}
	if (save_profile_dialog_) {
		save_profile_dialog_->unset_parent();
	}
}

EconomyOptionsWindow& EconomyOptionsWindow::create(Panel* parent,
                                                   Widelands::Descriptions* descriptions,
                                                   const Widelands::Flag& flag,
                                                   Widelands::WareWorker type,
                                                   bool can_act) {
	Widelands::Economy* ware_economy = flag.get_economy(Widelands::wwWARE);
	Widelands::Economy* worker_economy = flag.get_economy(Widelands::wwWORKER);
	EconomyOptionsWindow* window_open = nullptr;
	if (ware_economy->get_options_window()) {
		EconomyOptionsWindow& window =
		   *static_cast<EconomyOptionsWindow*>(ware_economy->get_options_window());
		window_open = &window;
		window.activate_tab(type);
		if (window.is_minimal()) {
			window.restore();
		}
		window.move_to_top();
	}
	if (worker_economy->get_options_window()) {
		EconomyOptionsWindow& window =
		   *static_cast<EconomyOptionsWindow*>(worker_economy->get_options_window());
		window_open = &window;
		window.activate_tab(type);
		if (window.is_minimal()) {
			window.restore();
		}
		window.move_to_top();
	}
	if (window_open) {
		return *window_open;
	}
	return *new EconomyOptionsWindow(
	   parent, descriptions, ware_economy, worker_economy, type, can_act);
}
void EconomyOptionsWindow::activate_tab(Widelands::WareWorker type) {
	tabpanel_.activate(type == Widelands::WareWorker::wwWARE ? "wares" : "workers");
}

std::string EconomyOptionsWindow::localize_profile_name(const std::string& name) {
	// Translation for the default profile is sourced from the widelands textdomain, and for the
	// other profiles from the tribes.
	std::string localized_name = _(name);
	{
		i18n::Textdomain td("tribes");
		localized_name = _(localized_name);
	}
	return localized_name;
}

void EconomyOptionsWindow::on_economy_note(const Widelands::NoteEconomy& note) {
	Widelands::Serial* serial = note.old_economy == ware_serial_   ? &ware_serial_ :
	                            note.old_economy == worker_serial_ ? &worker_serial_ :
                                                                    nullptr;
	if (serial) {
		switch (note.action) {
		case Widelands::NoteEconomy::Action::kMerged: {
			*serial = note.new_economy;
			Widelands::Economy* economy = player_->get_economy(*serial);
			if (economy == nullptr) {
				die();
				return;
			}
			economy->set_options_window(static_cast<void*>(this));
			(*serial == ware_serial_ ? ware_panel_ : worker_panel_)->set_economy(note.new_economy);
			move_to_top();
		} break;
		case Widelands::NoteEconomy::Action::kDeleted:
			// Make sure that the panels stop thinking first.
			die();
			break;
		}
	}
}

void EconomyOptionsWindow::layout() {
	int w, h;
	tabpanel_.get_desired_size(&w, &h);
	main_box_.set_desired_size(w, h + 78);
	update_desired_size();
	UI::Window::layout();
}

void EconomyOptionsWindow::EconomyOptionsPanel::update_desired_size() {
	display_.set_hgap(AbstractWaresDisplay::calc_hgap(display_.get_extent().w, kDesiredWidth));
	Box::update_desired_size();
	get_parent()->layout();
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
	const Widelands::Quantity amount = economy->target_quantity(ware).permanent;
	if (amount == Widelands::kEconomyTargetInfinity) {
		/** TRANSLATORS: Infinite number of wares or workers */
		return g_style_manager->font_style(UI::FontStyle::kWuiLabel).as_font_tag(_("∞"));
	}
	return as_string(amount);
}

/**
 * Wraps the wares/workers display together with some buttons
 */
EconomyOptionsWindow::EconomyOptionsPanel::EconomyOptionsPanel(UI::Panel* parent,
                                                               EconomyOptionsWindow* eco_window,
                                                               Widelands::Serial serial,
                                                               Widelands::Player* player,
                                                               bool can_act,
                                                               Widelands::WareWorker type,
                                                               int32_t min_w)
   : UI::Box(parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     serial_(serial),
     player_(player),
     type_(type),
     display_(this, 0, 0, serial_, player_, type, can_act),
     economy_options_window_(eco_window) {
	add(&display_, UI::Box::Resizing::kFullSize);

	display_.set_hgap(AbstractWaresDisplay::calc_hgap(display_.get_extent().w, min_w));
}

void EconomyOptionsWindow::EconomyOptionsPanel::set_economy(Widelands::Serial serial) {
	serial_ = serial;
	display_.set_economy(serial);
}

void EconomyOptionsWindow::toggle_infinite() {
	if (tabpanel_.active() == 0) {
		ware_panel_->toggle_infinite();
	} else {
		worker_panel_->toggle_infinite();
	}
}

void EconomyOptionsWindow::change_target(int amount) {
	if (tabpanel_.active() == 0) {
		ware_panel_->change_target(amount);
	} else {
		worker_panel_->change_target(amount);
	}
}

void EconomyOptionsWindow::reset_target() {
	if (dropdown_.get_selected().empty()) {
		return;
	}
	if (tabpanel_.active() == 0) {
		ware_panel_->reset_target();
	} else {
		worker_panel_->reset_target();
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::toggle_infinite() {
	Widelands::Economy* economy = player_->get_economy(serial_);
	if (!economy) {
		return die();
	}
	assert(economy->type() == type_);
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(player_->egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? player_->tribe().wares() : player_->tribe().workers();
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			const Widelands::Economy::TargetQuantity& tq = economy->target_quantity(index);
			Widelands::Quantity new_quantity;
			if (tq.permanent == Widelands::kEconomyTargetInfinity) {
				auto it = infinity_substitutes_.find(index);
				if (it == infinity_substitutes_.end()) {
					// The window was opended with the target set to infinite,
					// so we just use the default value
					new_quantity = is_wares ? economy_options_window_->get_predefined_targets()
					                             .at(kDefaultEconomyProfile)
					                             .wares.at(index) :
                                         economy_options_window_->get_predefined_targets()
					                             .at(kDefaultEconomyProfile)
					                             .workers.at(index);
				} else {
					// Restore saved old value
					new_quantity = it->second;
					infinity_substitutes_.erase(it);
				}
			} else {
				new_quantity = Widelands::kEconomyTargetInfinity;
				// Save old target for when the infinity option is disabled again
				infinity_substitutes_[index] = tq.permanent;
			}
			if (is_wares) {
				game.send_player_command(new Widelands::CmdSetWareTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, new_quantity));
			} else {
				game.send_player_command(new Widelands::CmdSetWorkerTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, new_quantity));
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::change_target(int delta) {
	if (delta == 0) {
		return;
	}
	Widelands::Economy* economy = player_->get_economy(serial_);
	if (!economy) {
		return die();
	}
	assert(economy->type() == type_);
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(player_->egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? player_->tribe().wares() : player_->tribe().workers();
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			const Widelands::Economy::TargetQuantity& tq = economy->target_quantity(index);
			if (tq.permanent == Widelands::kEconomyTargetInfinity) {
				// Infinity ± finite value = infinity
				continue;
			}
			// Don't allow negative new amount
			const int old_amount = static_cast<int>(tq.permanent);
			const int new_amount = std::max(0, old_amount + delta);
			assert(old_amount >= 0);
			assert(new_amount >= 0);
			if (new_amount == old_amount) {
				continue;
			}
			if (is_wares) {
				game.send_player_command(new Widelands::CmdSetWareTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, new_amount));
			} else {
				game.send_player_command(new Widelands::CmdSetWorkerTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, new_amount));
			}
		}
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::reset_target() {
	Widelands::Game& game = dynamic_cast<Widelands::Game&>(player_->egbase());
	const bool is_wares = type_ == Widelands::wwWARE;
	const auto& items = is_wares ? player_->tribe().wares() : player_->tribe().workers();
	const PredefinedTargets settings = economy_options_window_->get_selected_target();

	bool anything_selected = false;
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index)) {
			anything_selected = true;
			break;
		}
	}
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index) ||
		    (!anything_selected && !display_.is_ware_hidden(index))) {
			if (is_wares) {
				auto setting = settings.wares.find(index);
				if (setting == settings.wares.end()) {
					continue;
				}
				game.send_player_command(new Widelands::CmdSetWareTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, setting->second));
			} else {
				auto setting = settings.workers.find(index);
				if (setting == settings.workers.end()) {
					continue;
				}
				game.send_player_command(new Widelands::CmdSetWorkerTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, setting->second));
			}
		}
	}
}

constexpr Duration kThinkInterval(200);

void EconomyOptionsWindow::think() {
	const Time& time = player_->egbase().get_gametime();
	if (time - time_last_thought_ < kThinkInterval || !player_->get_economy(ware_serial_) ||
	    !player_->get_economy(worker_serial_)) {
		// If our economy has been deleted, die() was already called, no need to do anything
		return;
	}
	time_last_thought_ = time;
	update_profiles();
}

std::string EconomyOptionsWindow::applicable_target() {
	const Widelands::Economy* eco_ware = player_->get_economy(ware_serial_);
	const Widelands::Economy* eco_worker = player_->get_economy(worker_serial_);
	for (const auto& pair : predefined_targets_) {
		bool matches = true;
		if (tabpanel_.active() == 0) {
			for (const Widelands::DescriptionIndex& index : player_->tribe().wares()) {
				const auto it = pair.second.wares.find(index);
				if (it != pair.second.wares.end() &&
				    eco_ware->target_quantity(index).permanent != it->second) {
					matches = false;
					break;
				}
			}
		} else {
			for (const Widelands::DescriptionIndex& index : player_->tribe().workers()) {
				const auto it = pair.second.workers.find(index);
				if (it != pair.second.workers.end() &&
				    eco_worker->target_quantity(index).permanent != it->second) {
					matches = false;
					break;
				}
			}
		}
		if (matches) {
			return pair.first;
		}
	}
	return "";
}

void EconomyOptionsWindow::update_profiles() {
	const std::string current_profile = applicable_target();

	for (const auto& pair : predefined_targets_) {
		if (last_added_to_dropdown_.count(pair.first) == 0) {
			return update_profiles_needed(current_profile);
		}
	}
	for (const auto& string : last_added_to_dropdown_) {
		if (!string.empty() && predefined_targets_.find(string) == predefined_targets_.end()) {
			return update_profiles_needed(current_profile);
		}
	}
	if (last_added_to_dropdown_.count("") == (current_profile.empty() ? 0 : 1)) {
		return update_profiles_needed(current_profile);
	}

	update_profiles_select(current_profile);
}

void EconomyOptionsWindow::update_profiles_needed(const std::string& current_profile) {
	dropdown_.clear();
	last_added_to_dropdown_.clear();
	for (const auto& pair : predefined_targets_) {
		dropdown_.add(EconomyOptionsWindow::localize_profile_name(pair.first), pair.first);
		last_added_to_dropdown_.insert(pair.first);
	}
	if (current_profile.empty()) {
		// Nothing selected
		dropdown_.add("–", "");
		last_added_to_dropdown_.insert("");
	}
	update_profiles_select(current_profile);
}

void EconomyOptionsWindow::update_profiles_select(const std::string& current_profile) {
	if (dropdown_.is_expanded()) {
		return;
	}
	if (!dropdown_.has_selection() || dropdown_.get_selected() != current_profile) {
		dropdown_.select(current_profile);
	}
	assert(dropdown_.has_selection());
}

void EconomyOptionsWindow::SaveProfileWindow::update_save_enabled() {
	const std::string& text = profile_name_.text();
	if (text.empty() || text == kDefaultEconomyProfile) {
		save_.set_enabled(false);
		save_.set_tooltip(text.empty() ? _("The profile name cannot be empty") :
                                       _("The default profile cannot be overwritten"));
	} else {
		save_.set_enabled(true);
		save_.set_tooltip(_("Save the profile under this name"));
	}
}

void EconomyOptionsWindow::SaveProfileWindow::table_selection_changed() {
	if (!table_.has_selection()) {
		delete_.set_enabled(false);
		delete_.set_tooltip("");
		return;
	}
	const std::string& sel = table_[table_.selection_index()];
	if (economy_options_->get_predefined_targets().at(sel).undeletable) {
		delete_.set_tooltip(_("The predefined profiles cannot be deleted"));
		delete_.set_enabled(false);
	} else {
		delete_.set_tooltip(_("Delete the selected profiles"));
		delete_.set_enabled(true);
	}
	profile_name_.set_text(sel);
	update_save_enabled();
}

void EconomyOptionsWindow::SaveProfileWindow::update_table() {
	table_.clear();
	for (const auto& pair : economy_options_->get_predefined_targets()) {
		table_.add(pair.first).set_string(0, EconomyOptionsWindow::localize_profile_name(pair.first));
	}
	layout();
}

void EconomyOptionsWindow::SaveProfileWindow::save_profile() {
	const std::string name = profile_name_.text();
	assert(!name.empty());
	assert(name != kDefaultEconomyProfile);
	for (const auto& pair : economy_options_->get_predefined_targets()) {
		if (pair.first == name) {
			UI::WLMessageBox m(
			   this, UI::WindowStyle::kWui, _("Overwrite?"),
			   _("A profile with this name already exists. Do you wish to replace it?"),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
			break;
		}
	}
	economy_options_->do_create_target(name);
	die();
}

void EconomyOptionsWindow::SaveProfileWindow::delete_selected() {
	assert(table_.has_selection());

	auto& map = economy_options_->get_predefined_targets();
	const std::string& name = table_[table_.selection_index()];

	assert(name != kDefaultEconomyProfile);
	assert(!map.at(name).undeletable);
	auto it = map.find(name);
	assert(it != map.end());
	map.erase(it);

	economy_options_->save_targets();
	economy_options_->update_profiles();
	update_table();
}

void EconomyOptionsWindow::SaveProfileWindow::unset_parent() {
	economy_options_ = nullptr;
	die();
}

void EconomyOptionsWindow::SaveProfileWindow::think() {
	if (!economy_options_) {
		die();
	}
	UI::Window::think();
}

EconomyOptionsWindow::SaveProfileWindow::SaveProfileWindow(UI::Panel* parent,
                                                           EconomyOptionsWindow* eco)
   : UI::Window(parent,
                UI::WindowStyle::kWui,
                "save_economy_options_profile",
                0,
                0,
                0,
                0,
                _("Save Profile")),
     economy_options_(eco),
     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     table_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     table_(&table_box_, 0, 0, 460, 120, UI::PanelStyle::kWui),
     buttons_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     profile_name_(&main_box_, 0, 0, 240, UI::PanelStyle::kWui),
     save_(&buttons_box_, "save", 0, 0, 80, 34, UI::ButtonStyle::kWuiPrimary, _("Save")),
     cancel_(&buttons_box_, "cancel", 0, 0, 80, 34, UI::ButtonStyle::kWuiSecondary, _("Cancel")),
     delete_(&buttons_box_, "delete", 0, 0, 80, 34, UI::ButtonStyle::kWuiSecondary, _("Delete")) {
	table_.add_column(200, _("Existing Profiles"));
	update_table();

	table_.selected.connect([this](uint32_t) { table_selection_changed(); });
	profile_name_.changed.connect([this] { update_save_enabled(); });
	profile_name_.ok.connect([this] { save_profile(); });
	profile_name_.cancel.connect([this] { die(); });
	save_.sigclicked.connect([this] { save_profile(); });
	cancel_.sigclicked.connect([this] { die(); });
	delete_.sigclicked.connect([this] { delete_selected(); });

	table_box_.add(&table_, UI::Box::Resizing::kFullSize);

	buttons_box_.add(UI::g_fh->fontset()->is_rtl() ? &save_ : &delete_);
	buttons_box_.add(&cancel_);
	buttons_box_.add(UI::g_fh->fontset()->is_rtl() ? &delete_ : &save_);

	main_box_.add(&table_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&profile_name_, UI::Box::Resizing::kFullSize);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
	set_center_panel(&main_box_);

	table_selection_changed();
	update_save_enabled();

	initialization_complete();
}

EconomyOptionsWindow::SaveProfileWindow::~SaveProfileWindow() {
	if (economy_options_) {
		economy_options_->close_save_profile_window();
	}
}

void EconomyOptionsWindow::create_target() {
	if (save_profile_dialog_) {
		// Already open
		return;
	}
	save_profile_dialog_ = new SaveProfileWindow(get_parent(), this);
}

void EconomyOptionsWindow::close_save_profile_window() {
	assert(save_profile_dialog_);
	save_profile_dialog_ = nullptr;
}

void EconomyOptionsWindow::do_create_target(const std::string& name) {
	assert(!name.empty());
	assert(name != kDefaultEconomyProfile);
	const Widelands::Descriptions& descriptions = player_->egbase().descriptions();
	const Widelands::TribeDescr& tribe = player_->tribe();
	Widelands::Economy* ware_economy = player_->get_economy(ware_serial_);
	Widelands::Economy* worker_economy = player_->get_economy(worker_serial_);
	PredefinedTargets t;
	for (Widelands::DescriptionIndex di : tribe.wares()) {
		if (descriptions.get_ware_descr(di)->has_demand_check(tribe.name())) {
			t.wares[di] = ware_economy->target_quantity(di).permanent;
		}
	}
	for (Widelands::DescriptionIndex di : tribe.workers()) {
		if (descriptions.get_worker_descr(di)->has_demand_check()) {
			t.workers[di] = worker_economy->target_quantity(di).permanent;
		}
	}
	predefined_targets_[name] = t;

	save_targets();
	update_profiles();
}

void EconomyOptionsWindow::save_targets() {
	const Widelands::Descriptions& descriptions = player_->egbase().descriptions();
	Profile profile;

	std::map<std::string, uint32_t> serials;
	for (const auto& pair : predefined_targets_) {
		if (pair.first != kDefaultEconomyProfile) {
			serials.insert(std::make_pair(pair.first, serials.size()));
		}
	}
	Section& global_section = profile.create_section(kDefaultEconomyProfile.c_str());
	for (const auto& pair : serials) {
		global_section.set_string(std::to_string(pair.second).c_str(), pair.first);
	}

	for (const auto& pair : predefined_targets_) {
		if (pair.first == kDefaultEconomyProfile) {
			continue;
		}
		Section& section = profile.create_section(std::to_string(serials.at(pair.first)).c_str());
		for (const auto& setting : pair.second.wares) {
			section.set_natural(
			   descriptions.get_ware_descr(setting.first)->name().c_str(), setting.second);
		}
		for (const auto& setting : pair.second.workers) {
			section.set_natural(
			   descriptions.get_worker_descr(setting.first)->name().c_str(), setting.second);
		}
	}

	Section& section = profile.create_section("undeletable");
	for (const auto& pair : predefined_targets_) {
		if (pair.first != kDefaultEconomyProfile) {
			section.set_bool(std::to_string(serials.at(pair.first)).c_str(), pair.second.undeletable);
		}
	}

	g_fs->ensure_directory_exists(kEconomyProfilesDir);
	std::string complete_filename =
	   kEconomyProfilesDir + FileSystem::file_separator() + player_->tribe().name();
	profile.write(complete_filename.c_str(), false);

	// Inform the windows of other economies of new and deleted profiles
	Notifications::publish(NoteEconomyProfile(ware_serial_, worker_serial_));
}

void EconomyOptionsWindow::read_targets() {
	predefined_targets_.clear();
	const Widelands::TribeDescr& tribe = player_->tribe();

	{
		PredefinedTargets t;
		t.undeletable = true;
		for (Widelands::DescriptionIndex di : tribe.wares()) {
			const Widelands::WareDescr* descr = descriptions_->get_ware_descr(di);
			if (descr->has_demand_check(tribe.name())) {
				t.wares.insert(std::make_pair(di, descr->default_target_quantity(tribe.name())));
			}
		}
		for (Widelands::DescriptionIndex di : tribe.workers()) {
			const Widelands::WorkerDescr* descr = descriptions_->get_worker_descr(di);
			if (descr->has_demand_check()) {
				t.workers.insert(std::make_pair(di, descr->default_target_quantity()));
			}
		}
		predefined_targets_.insert(std::make_pair(kDefaultEconomyProfile, t));
	}

	std::string complete_filename =
	   kEconomyProfilesDir + FileSystem::file_separator() + player_->tribe().name();
	Profile profile;
	profile.read(complete_filename.c_str());

	Section* global_section = profile.get_section(kDefaultEconomyProfile);
	if (global_section) {
		std::map<std::string, std::string> serials;
		while (Section::Value* v = global_section->get_next_val()) {
			serials.insert(std::make_pair(v->get_name(), v->get_string()));
		}

		for (const auto& pair : serials) {
			Section* section = profile.get_section(pair.first);
			PredefinedTargets t;
			while (Section::Value* v = section->get_next_val()) {
				const std::string name(v->get_name());
				try {
					const std::pair<Widelands::WareWorker, Widelands::DescriptionIndex> wareworker =
					   descriptions_->load_ware_or_worker(name);
					assert(wareworker.second != Widelands::INVALID_INDEX);
					switch (wareworker.first) {
					case Widelands::WareWorker::wwWARE:
						t.wares.insert(std::make_pair(wareworker.second, v->get_natural()));
						break;
					case Widelands::WareWorker::wwWORKER:
						t.workers.insert(std::make_pair(wareworker.second, v->get_natural()));
						break;
					}
				} catch (const Widelands::GameDataError&) {
					log_warn("Unknown ware or worker '%s' in economy profile '%s'", name.c_str(),
					         pair.second.c_str());
				}
			}
			predefined_targets_.insert(std::make_pair(pair.second, t));
		}

		if (Section* section = profile.get_section("undeletable")) {
			while (Section::Value* v = section->get_next_val()) {
				predefined_targets_.at(serials.at(std::string(v->get_name()))).undeletable =
				   v->get_bool();
			}
		}
	}

	update_profiles();
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window*
EconomyOptionsWindow::load(FileRead& fr, InteractiveBase& ib, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const uint32_t flag_serial = fr.unsigned_32();
			if (flag_serial == 0) {
				return nullptr;
			}
			Widelands::Flag& flag = mol.get<Widelands::Flag>(flag_serial);
			return &create(&ib, ib.egbase().mutable_descriptions(), flag,
			               fr.unsigned_8() > 0 ? Widelands::wwWORKER : Widelands::wwWARE,
			               ib.can_act(flag.owner().player_number()));
		} else {
			throw Widelands::UnhandledVersionError(
			   "Economy Options Window", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("economy options window: %s", e.what());
	}
}
void EconomyOptionsWindow::save(FileWrite& fw, Widelands::MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);
	Widelands::Economy* e_wa = player_->get_economy(ware_serial_);
	Widelands::Economy* e_wo = player_->get_economy(worker_serial_);
	if (!e_wa || !e_wo) {
		fw.unsigned_32(0);
		return;
	}
	Widelands::Flag* f = e_wa->get_arbitrary_flag(e_wo);
	if (!f) {
		log_warn("EconomyOptionsWindow::save: No flag exists in both economies (%u & %u)",
		         ware_serial_, worker_serial_);
		f = e_wa->get_arbitrary_flag();
	}
	if (!f) {
		fw.unsigned_32(0);
		return;
	}
	fw.unsigned_32(mos.get_object_file_index(*f));
	fw.unsigned_8(tabpanel_.active());
}
