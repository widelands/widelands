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

#include <memory>

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
#include "ui_basic/editbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/table.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";

constexpr int kDesiredWidth = 216;

static inline int32_t calc_hgap(int32_t columns, int32_t total_w) {
	return (total_w - columns * kWareMenuPicWidth) / (columns - 1);
}

EconomyOptionsWindow::EconomyOptionsWindow(UI::Panel* parent,
                                           Widelands::Economy* economy,
                                           bool can_act)
   : UI::Window(parent, "economy_options", 0, 0, 0, 0, _("Economy options")),
     main_box_(this, 0, 0, UI::Box::Vertical),
     serial_(economy->serial()),
     player_(&economy->owner()),
     tabpanel_(this, UI::TabPanelStyle::kWuiDark),
     ware_panel_(new EconomyOptionsPanel(&tabpanel_, this, serial_, player_, can_act, Widelands::wwWARE, kDesiredWidth)),
     worker_panel_(
        new EconomyOptionsPanel(&tabpanel_, this, serial_, player_, can_act, Widelands::wwWORKER, kDesiredWidth)),
     dropdown_box_(this, 0, 0, UI::Box::Horizontal),
     dropdown_(&dropdown_box_, 0, 0, 174, 200, 34, "", UI::DropdownType::kTextual, UI::PanelStyle::kWui),
     time_last_thought_(0) {
	set_center_panel(&main_box_);

	tabpanel_.add("wares", g_gr->images().get(pic_tab_wares), ware_panel_, _("Wares"));
	tabpanel_.add("workers", g_gr->images().get(pic_tab_workers), worker_panel_, _("Workers"));

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	UI::Button* b = new UI::Button(buttons, "decrease_target_fast", 0, 0, 44, 28, UI::ButtonStyle::kWuiSecondary,
			g_gr->images().get("images/ui_basic/scrollbar_down_fast.png"), _("Decrease target by 10"));
	b->sigclicked.connect([this] { change_target(-10); });
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);
	b = new UI::Button(buttons, "decrease_target", 0, 0, 44, 28, UI::ButtonStyle::kWuiSecondary,
			g_gr->images().get("images/ui_basic/scrollbar_down.png"), _("Decrease target"));
	b->sigclicked.connect([this] { change_target(-1); });
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(24);

	b = new UI::Button(buttons, "increase_target", 0, 0, 44, 28, UI::ButtonStyle::kWuiSecondary,
			g_gr->images().get("images/ui_basic/scrollbar_up.png"), _("Increase target"));
	b->sigclicked.connect([this] { change_target(1); });
	buttons->add(b);
	b->set_repeating(true);
	buttons->add_space(8);
	b = new UI::Button(buttons, "increase_target_fast", 0, 0, 44, 28, UI::ButtonStyle::kWuiSecondary,
	                   g_gr->images().get("images/ui_basic/scrollbar_up_fast.png"), _("Increase target by 10"));
	b->sigclicked.connect([this] { change_target(10); });
	buttons->add(b);
	b->set_repeating(true);

	dropdown_.set_tooltip(_("Profile to apply to the selected items"));
	dropdown_box_.set_size(40, 20);  // Prevent assert failures
	dropdown_box_.add(&dropdown_, UI::Box::Resizing::kFullSize);
	dropdown_.selected.connect([this] { reset_target(); });

	b = new UI::Button(&dropdown_box_, "save_targets", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			g_gr->images().get("images/wui/menus/menu_save_game.png"), _("Save target settings"));
	b->sigclicked.connect([this] { create_target(); });
	dropdown_box_.add_space(8);
	dropdown_box_.add(b);

	main_box_.add(&tabpanel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_space(8);
	main_box_.add(buttons, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_space(8);
	main_box_.add(&dropdown_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

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

void EconomyOptionsWindow::layout() {
	int w, h;
	tabpanel_.get_desired_size(&w, &h);
	main_box_.set_desired_size(w, h + 78);
	update_desired_size();
	UI::Window::layout();
}

void EconomyOptionsWindow::EconomyOptionsPanel::update_desired_size() {
	display_.set_hgap(std::max(3, calc_hgap(display_.get_extent().w, kDesiredWidth)));
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
                                                               Widelands::WareWorker type,
                                                               int32_t min_w)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     serial_(serial),
     player_(player),
     type_(type),
     can_act_(can_act),
     display_(this, 0, 0, serial_, player_, type_, can_act_),
     economy_options_window_(eco_window) {
	add(&display_, UI::Box::Resizing::kFullSize);

	display_.set_hgap(std::max(3, calc_hgap(display_.get_extent().w, min_w)));

	if (!can_act_) {
		return;
	}
}

void EconomyOptionsWindow::EconomyOptionsPanel::set_economy(Widelands::Serial serial) {
	serial_ = serial;
	display_.set_economy(serial);
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

void EconomyOptionsWindow::EconomyOptionsPanel::change_target(int delta) {
	if (delta == 0) {
		return;
	}
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
			// Don't allow negative new amount
			const int old_amount = static_cast<int>(tq.permanent);
			const int new_amount = std::max(0, old_amount + delta);
			if (new_amount == old_amount) {
				continue;
			}
			if (is_wares) {
				game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
				   game.get_gametime(), player_->player_number(), serial_, index, new_amount));
			} else {
				game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
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
	bool second_phase = false;

run_second_phase:
	for (const Widelands::DescriptionIndex& index : items) {
		if (display_.ware_selected(index) || (second_phase && !display_.is_ware_hidden(index))) {
			anything_selected = true;
			if (is_wares) {
				game.send_player_command(*new Widelands::CmdSetWareTargetQuantity(
						game.get_gametime(), player_->player_number(), serial_, index, settings.wares.at(index)));
			} else {
				game.send_player_command(*new Widelands::CmdSetWorkerTargetQuantity(
						game.get_gametime(), player_->player_number(), serial_, index, settings.workers.at(index)));
			}
		}
	}

	if (!second_phase && !anything_selected) {
		// Nothing was selected, now go through the loop again and change everything
		second_phase = true;
		goto run_second_phase;
	}
}

constexpr unsigned kThinkInterval = 200;

void EconomyOptionsWindow::think() {
	const uint32_t time = player_->egbase().get_gametime();
	if (time - time_last_thought_ < kThinkInterval) {
		return;
	}
	time_last_thought_ = time;
	update_profiles();
}

std::string EconomyOptionsWindow::applicable_target() {
	const Widelands::Economy* eco = player_->get_economy(serial_);
	for (const auto& pair : predefined_targets_) {
		bool matches = true;
		if (tabpanel_.active() == 0) {
			for (const Widelands::DescriptionIndex& index : player_->tribe().wares()) {
				const auto it = pair.second.wares.find(index);
				if (it != pair.second.wares.end() && eco->ware_target_quantity(index).permanent != it->second) {
					matches = false;
					break;
				}
			}
		} else {
			for (const Widelands::DescriptionIndex& index : player_->tribe().workers()) {
				const auto it = pair.second.workers.find(index);
				if (it != pair.second.workers.end() && eco->worker_target_quantity(index).permanent != it->second) {
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
			goto update_needed;
		}
	}
	for (const auto& string : last_added_to_dropdown_) {
		if (!string.empty() && predefined_targets_.find(string) == predefined_targets_.end()) {
			goto update_needed;
		}
	}
	if (last_added_to_dropdown_.count("") == (current_profile.empty() ? 0 : 1)) {
		goto update_needed;
	}
	goto do_select;

update_needed:
	dropdown_.clear();
	last_added_to_dropdown_.clear();
	for (const auto& pair : predefined_targets_) {
		dropdown_.add(_(pair.first), pair.first);
		last_added_to_dropdown_.insert(pair.first);
	}

	if (current_profile.empty()) {
		// Nothing selected
		dropdown_.add("", "");
		last_added_to_dropdown_.insert("");
	}

do_select:
	if (dropdown_.is_expanded()) {
		return;
	}
	const std::string select = _(current_profile);
	if (!dropdown_.has_selection() || dropdown_.get_selected() != select) {
		dropdown_.select(select);
	}
}

struct SaveProfileWindow : public UI::Window {
	void update_save_enabled() {
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

	void table_selection_changed() {
		if (!table_.has_selection()) {
			delete_.set_enabled(false);
			delete_.set_tooltip("");
			return;
		}
		const std::string& sel = table_[table_.selection_index()];
		if (sel == kDefaultEconomyProfile) {
			delete_.set_tooltip(_("The default profile cannot be deleted"));
			delete_.set_enabled(false);
		} else {
			delete_.set_tooltip(_("Delete the selected profiles"));
			delete_.set_enabled(true);
		}
		profile_name_.set_text(sel);
		update_save_enabled();
	}

	void close(bool ok) {
		end_modal(ok ? UI::Panel::Returncodes::kOk : UI::Panel::Returncodes::kBack);
		die();
	}

	void update_table() {
		table_.clear();
		for (const auto& pair : economy_options_->get_predefined_targets()) {
			table_.add(pair.first).set_string(0, _(pair.first));
		}
		layout();
	}

	void save() {
		const std::string name = profile_name_.text();
		assert(!name.empty());
		assert(name != kDefaultEconomyProfile);
		for (const auto& pair : economy_options_->get_predefined_targets()) {
			if (pair.first == name) {
				UI::WLMessageBox m(this, _("Overwrite?"),
						_("A profile with this name already exists.\nDo you wish to replace it?"),
						UI::WLMessageBox::MBoxType::kOkCancel);
				if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
					return;
				}
				break;
			}
		}
		economy_options_->do_create_target(name);
		close(true);
	}

	void delete_selected() {
		assert(table_.has_selection());
		auto& map = economy_options_->get_predefined_targets();
		const std::string& name = table_[table_.selection_index()];
		assert(name != kDefaultEconomyProfile);
		for (auto it = map.begin(); it != map.end(); ++it) {
			if (it->first == name) {
				map.erase(it);
				break;
			}
		}
		economy_options_->save_targets();
		economy_options_->update_profiles();
		update_table();
	}

	explicit SaveProfileWindow(UI::Panel* parent, EconomyOptionsWindow* eco)
	   : UI::Window(parent, "save_economy_options_profile", 0, 0, 0, 0, _("Save Profile")),
	     economy_options_(eco),
	     main_box_(this, 0, 0, UI::Box::Vertical),
	     table_box_(&main_box_, 0, 0, UI::Box::Vertical),
	     table_(&table_box_, 0, 0, 460, 120, UI::PanelStyle::kWui),
	     buttons_box_(&main_box_, 0, 0, UI::Box::Horizontal),
	     profile_name_(&buttons_box_, 0, 0, 240, 0, 0, UI::PanelStyle::kWui),
	     save_(&buttons_box_, "save", 0, 0, 80, 34, UI::ButtonStyle::kWuiPrimary, _("Save")),
	     cancel_(&buttons_box_, "cancel", 0, 0, 80, 34, UI::ButtonStyle::kWuiSecondary, _("Cancel")),
	     delete_(&buttons_box_, "delete", 0, 0, 80, 34, UI::ButtonStyle::kWuiSecondary, _("Delete")) {
		table_.add_column(200, _("Existing Profiles"));
		update_table();

		table_.selected.connect([this](uint32_t) { table_selection_changed(); });
		profile_name_.changed.connect([this] { update_save_enabled(); });
		profile_name_.ok.connect([this] { save(); });
		profile_name_.cancel.connect([this] { close(false); });
		save_.sigclicked.connect([this] { save(); });
		cancel_.sigclicked.connect([this] { close(false); });
		delete_.sigclicked.connect([this] { delete_selected(); });

		table_box_.add(&table_, UI::Box::Resizing::kFullSize);
		buttons_box_.add(&profile_name_, UI::Box::Resizing::kFullSize);
		buttons_box_.add(&save_);
		buttons_box_.add(&cancel_);
		buttons_box_.add(&delete_);
		main_box_.add(&table_box_, UI::Box::Resizing::kFullSize);
		main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
		set_center_panel(&main_box_);

		table_selection_changed();
		update_save_enabled();
	}
	~SaveProfileWindow() {
	}

private:
	EconomyOptionsWindow* economy_options_;
	UI::Box main_box_;
	UI::Box table_box_;
	UI::Table<const std::string&> table_;
	UI::Box buttons_box_;
	UI::EditBox profile_name_;
	UI::Button save_;
	UI::Button cancel_;
	UI::Button delete_;
};

void EconomyOptionsWindow::create_target() {
	std::unique_ptr<SaveProfileWindow> s (new SaveProfileWindow(get_parent(), this));
	s->run<UI::Panel::Returncodes>();
}

void EconomyOptionsWindow::do_create_target(const std::string& name) {
	assert(!name.empty());
	assert(name != kDefaultEconomyProfile);
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
	update_profiles();
}

void EconomyOptionsWindow::save_targets() {
	const Widelands::Tribes& tribes = player_->egbase().tribes();
	Profile profile;

	std::map<std::string, uint32_t> serials;
	for (const auto& pair : predefined_targets_) {
		if (pair.first != kDefaultEconomyProfile) {
			serials.emplace(pair.first, serials.size());
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

void EconomyOptionsWindow::read_targets() {
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

	Section* global_section = profile.get_section(kDefaultEconomyProfile);
	if (global_section) {
		std::map<std::string, std::string> serials;
		while (Section::Value* v = global_section->get_next_val()) {
			serials.emplace(std::string(v->get_name()), v->get_string());
		}

		for (const auto& pair : serials) {
			Section* section = profile.get_section(pair.first);
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
			predefined_targets_.emplace(pair.second, t);
		}
	}

	update_profiles();
}
