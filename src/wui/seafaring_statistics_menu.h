/*
 * Copyright (C) 2017-2021 by the Widelands Development Team
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

#ifndef WL_WUI_SEAFARING_STATISTICS_MENU_H
#define WL_WUI_SEAFARING_STATISTICS_MENU_H

#include <memory>

#include "base/i18n.h"
#include "logic/map_objects/tribes/ship.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

class InteractivePlayer;

/// Shows a list of the ships owned by the interactive player with filtering and navigation options.
struct SeafaringStatisticsMenu : public UI::UniqueWindow {
	SeafaringStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kSeafaringStats;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	/// For identifying the columns in the table.
	enum Cols { ColName, ColStatus };
	/**
	 * A list of ship status that we can filter for. This differs a bit from that the Widelands::Ship
	 * class has, so we define our own.
	 * */
	enum class ShipFilterStatus {
		kIdle,
		kShipping,
		kExpeditionWaiting,
		kExpeditionScouting,
		kExpeditionPortspaceFound,
		kAll
	};

	/// Returns the localized strings that we use to display the 'status' in the table.
	const std::string status_to_string(ShipFilterStatus status) const;
	/// Returns the icon that we use to represent the 'status' in the table and on the filter
	/// buttons.
	const Image* status_to_image(ShipFilterStatus status) const;

	/// The dataset that we need to display ships in the table.
	struct ShipInfo {
		ShipInfo(const std::string& init_name,
		         const ShipFilterStatus init_status,
		         const Widelands::Serial init_serial)
		   : name(init_name), status(init_status), serial(init_serial) {
		}
		ShipInfo() : ShipInfo("", ShipFilterStatus::kAll, 0) {
		}
		ShipInfo(const ShipInfo& other) : ShipInfo(other.name, other.status, other.serial) {
		}
		bool operator==(const ShipInfo& other) const {
			return serial == other.serial;
		}
		bool operator<(const ShipInfo& other) const {
			return serial < other.serial;
		}

		const std::string name;
		ShipFilterStatus status;
		const Widelands::Serial serial;
	};

	/// Creates our dataset from a 'ship'.
	std::unique_ptr<const ShipInfo> create_shipinfo(const Widelands::Ship& ship) const;
	/// Uses the 'serial' to identify and get a ship from the game.
	Widelands::Ship* serial_to_ship(Widelands::Serial serial) const;

	/// Convenience function to upcast the panel's parent.
	InteractivePlayer& iplayer() const;

	/// A ship was selected, so enable the navigation buttons.
	void selected();
	/// A ship was double clicked. Centers main view on ship.
	void double_clicked();
	/// Handle filter and navigation hotkeys
	bool handle_key(bool down, SDL_Keysym code) override;

	/// Enables the navigation buttons if a ship is selected, disables them otherwise.
	void update_button_states();
	/// Center the mapview on the currently selected ship.
	void center_view();
	/// Follow the selected ship in a watch window.
	void watch_ship();
	/// Open the currently selected ship's window. If CTRL is pressed, center the mapview on it.
	void open_ship_window();

	/**
	 * Updates the status for the ship. If the ship is new and satisfies the 'ship_filter_',
	 * adds it to the table and data. If it doesn't satisfy the 'ship_filter_', removes the ship
	 * instead.
	 * */
	void update_ship(const Widelands::Ship&);
	/// If we listed the ship, remove it from table and data.
	void remove_ship(Widelands::Serial serial);
	/// Sets the contents for the entry record in the table.
	void set_entry_record(UI::Table<uintptr_t>::EntryRecord*, const ShipInfo& info);
	/// Updates the ship status display in the table.
	void update_entry_record(UI::Table<uintptr_t>::EntryRecord& er, const ShipInfo&);
	/// Rebuilds data and table with all ships that satisfy the current 'ship_filter_'.
	void fill_table();

	/// Show only the ships that have the given status. Toggle the appropriate buttons.
	void filter_ships(ShipFilterStatus status);
	/// Helper for filter_ships
	void toggle_filter_ships_button(UI::Button&, ShipFilterStatus);
	/// Helper for filter_ships
	void set_filter_ships_tooltips();

	/// We group colonizing status with port space found. Anything else needs to have an identical
	/// status.
	bool satisfies_filter(const ShipInfo& info, ShipFilterStatus filter);

	UI::Box main_box_;
	// Buttons for ship states
	UI::Box filter_box_;
	UI::Button idle_btn_;
	UI::Button waiting_btn_;
	UI::Button scouting_btn_;
	UI::Button portspace_btn_;
	UI::Button shipping_btn_;
	ShipFilterStatus ship_filter_;
	// Navigation buttons
	UI::Box navigation_box_;
	UI::Button watchbtn_;
	UI::Button openwindowbtn_;
	UI::Button centerviewbtn_;

	// Data
	UI::Table<uintptr_t> table_;
	std::unordered_map<Widelands::Serial, std::unique_ptr<const ShipInfo>> data_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShip>> shipnotes_subscriber_;
};

#endif  // end of include guard: WL_WUI_SEAFARING_STATISTICS_MENU_H
