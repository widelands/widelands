/*
 * Copyright (C) 2008-2021 by the Widelands Development Team
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

#ifndef WL_WUI_ECONOMY_OPTIONS_WINDOW_H
#define WL_WUI_ECONOMY_OPTIONS_WINDOW_H

#include <memory>

#include "economy/economy.h"
#include "notifications/notifications.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/waresdisplay.h"

const std::string kDefaultEconomyProfile = "Default";

class InteractiveBase;
namespace Widelands {
class MapObjectLoader;
}

// Used to indicate that a profile has been saved or deleted, so all open windows can update it
struct NoteEconomyProfile {
	NoteEconomyProfile(Widelands::Serial ware, Widelands::Serial worker)
	   : ware_serial(ware), worker_serial(worker) {
	}
	Widelands::Serial ware_serial;
	Widelands::Serial worker_serial;
	CAN_BE_SENT_AS_NOTE(NoteId::EconomyProfile)
};

class EconomyOptionsWindow : public UI::Window {
public:
	EconomyOptionsWindow(UI::Panel* parent,
	                     Widelands::Descriptions* descriptions,
	                     Widelands::Economy* ware_economy,
	                     Widelands::Economy* worker_economy,
	                     Widelands::WareWorker type,
	                     bool can_act);
	~EconomyOptionsWindow() override;

	struct PredefinedTargets {
		using Targets = std::map<Widelands::DescriptionIndex, Widelands::Quantity>;
		Targets wares;
		Targets workers;
		bool undeletable = false;
	};

	// Create an economy options window for the given flag
	static EconomyOptionsWindow& create(UI::Panel* parent,
	                                    Widelands::Descriptions* descriptions,
	                                    const Widelands::Flag& flag,
	                                    Widelands::WareWorker type,
	                                    bool can_act);
	void activate_tab(Widelands::WareWorker type);

	void create_target();
	void do_create_target(const std::string&);
	void save_targets();
	void read_targets();
	void update_profiles();
	std::map<std::string, PredefinedTargets>& get_predefined_targets() {
		return predefined_targets_;
	}
	const PredefinedTargets& get_selected_target() const {
		return predefined_targets_.at(dropdown_.get_selected());
	}

	void change_target(int amount);
	void toggle_infinite();
	void reset_target();

	void layout() override;

	void close_save_profile_window();

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kConfigureEconomy;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window* load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader&);

private:
	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay(UI::Panel* const parent,
		                   int32_t const x,
		                   int32_t const y,
		                   Widelands::Serial serial,
		                   Widelands::Player* player,
		                   Widelands::WareWorker type,
		                   bool selectable);

		void set_economy(Widelands::Serial serial);

	protected:
		std::string info_for_ware(Widelands::DescriptionIndex const ware) override;

	private:
		Widelands::Serial serial_;
		Widelands::Player* player_;
	};

	/**
	 * Wraps the wares/workers display together with some buttons
	 */
	struct EconomyOptionsPanel : UI::Box {
		EconomyOptionsPanel(UI::Panel* parent,
		                    EconomyOptionsWindow* eco_window,
		                    Widelands::Serial serial,
		                    Widelands::Player* player,
		                    bool can_act,
		                    Widelands::WareWorker type,
		                    int32_t min_w);

		void set_economy(Widelands::Serial serial);
		void change_target(int amount);
		void toggle_infinite();
		void reset_target();
		void update_desired_size() override;

	private:
		Widelands::Serial serial_;
		Widelands::Player* player_;
		Widelands::WareWorker type_;
		TargetWaresDisplay display_;
		EconomyOptionsWindow* economy_options_window_;

		std::map<Widelands::DescriptionIndex, Widelands::Quantity> infinity_substitutes_;
	};

	/// Translation for the default profile is sourced from the widelands textdomain, and for the
	/// other profiles from the tribes.
	static std::string localize_profile_name(const std::string& name);
	/// Actions performed when a NoteEconomyWindow is received.
	void on_economy_note(const Widelands::NoteEconomy& note);

	UI::Box main_box_;
	Widelands::Serial ware_serial_;
	Widelands::Serial worker_serial_;
	Widelands::Player* player_;
	UI::TabPanel tabpanel_;
	EconomyOptionsPanel* ware_panel_;
	EconomyOptionsPanel* worker_panel_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteEconomy>> economynotes_subscriber_;
	std::unique_ptr<Notifications::Subscriber<NoteEconomyProfile>> profilenotes_subscriber_;

	std::map<std::string, PredefinedTargets> predefined_targets_;
	UI::Box dropdown_box_;
	UI::Dropdown<std::string> dropdown_;

	std::string applicable_target();
	std::set<std::string> last_added_to_dropdown_;
	void think() override;
	Time time_last_thought_;

	struct SaveProfileWindow : public UI::Window {
		SaveProfileWindow(UI::Panel* parent, EconomyOptionsWindow* eco);
		~SaveProfileWindow() override;

		void update_save_enabled();
		void table_selection_changed();
		void update_table();
		void save();
		void delete_selected();

		void unset_parent();

		void think() override;

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

	// Helper functions for update_profiles()
	void update_profiles_needed(const std::string&);
	void update_profiles_select(const std::string&);

	SaveProfileWindow* save_profile_dialog_;
	// Mutable to allow dynamic loading of the correct ware/worker indices in case an old savegame
	// has been loaded
	Widelands::Descriptions* descriptions_;
};

#endif  // end of include guard: WL_WUI_ECONOMY_OPTIONS_WINDOW_H
