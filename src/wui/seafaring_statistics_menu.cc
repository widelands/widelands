/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#include "wui/seafaring_statistics_menu.h"

#include <memory>

#include "economy/ship_fleet.h"
#include "graphic/text_layout.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/box.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/shipwindow.h"

inline InteractivePlayer& SeafaringStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

constexpr int kPadding = 5;
constexpr int kButtonSize = 34;

SeafaringStatisticsMenu::SeafaringStatisticsMenu(InteractivePlayer& plr,
                                                 UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&plr,
                      UI::WindowStyle::kWui,
                      "seafaring_statistics",
                      &registry,
                      375,
                      375,
                      _("Seafaring Statistics")),
     main_box_(this,
               UI::PanelStyle::kWui,
               kPadding,
               kPadding,
               UI::Box::Vertical,
               get_inner_w(),
               get_inner_h(),
               kPadding),
     filter_box_(&main_box_,
                 UI::PanelStyle::kWui,
                 0,
                 0,
                 UI::Box::Horizontal,
                 get_inner_w() - 2 * kPadding,
                 kButtonSize,
                 kPadding),
     idle_btn_(&filter_box_,
               "filter_ship_idle",
               0,
               0,
               kButtonSize,
               kButtonSize,
               UI::ButtonStyle::kWuiSecondary,
               status_to_image(ShipFilterStatus::kIdle)),
     waiting_btn_(&filter_box_,
                  "filter_ship_waiting",
                  0,
                  0,
                  kButtonSize,
                  kButtonSize,
                  UI::ButtonStyle::kWuiSecondary,
                  status_to_image(ShipFilterStatus::kExpeditionWaiting)),
     scouting_btn_(&filter_box_,
                   "filter_ship_scouting",
                   0,
                   0,
                   kButtonSize,
                   kButtonSize,
                   UI::ButtonStyle::kWuiSecondary,
                   status_to_image(ShipFilterStatus::kExpeditionScouting)),
     portspace_btn_(&filter_box_,
                    "filter_ship_portspace",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    UI::ButtonStyle::kWuiSecondary,
                    status_to_image(ShipFilterStatus::kExpeditionPortspaceFound)),
     shipping_btn_(&filter_box_,
                   "filter_ship_transporting",
                   0,
                   0,
                   kButtonSize,
                   kButtonSize,
                   UI::ButtonStyle::kWuiSecondary,
                   status_to_image(ShipFilterStatus::kShipping)),
     ship_filter_(ShipFilterStatus::kAll),
     navigation_box_(&main_box_,
                     UI::PanelStyle::kWui,
                     0,
                     0,
                     UI::Box::Horizontal,
                     get_inner_w() - 2 * kPadding,
                     kButtonSize,
                     kPadding),
     watchbtn_(&navigation_box_,
               "seafaring_stats_watch_button",
               0,
               0,
               kButtonSize,
               kButtonSize,
               UI::ButtonStyle::kWuiPrimary,
               g_image_cache->get("images/wui/menus/watch_follow.png"),
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Tooltip in the seafaring statistics window */
                  _("Watch the selected ship"),
                  shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsWatchShip),
                  UI::PanelStyle::kWui)),
     openwindowbtn_(
        &navigation_box_,
        "seafaring_stats_watch_button",
        0,
        0,
        kButtonSize,
        kButtonSize,
        UI::ButtonStyle::kWuiPrimary,
        g_image_cache->get("images/ui_basic/fsel.png"),
        format("%s<br>%s",
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Tooltip in the seafaring statistics window */
                  _("Open the selected ship’s window"),
                  shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsOpenShipWindow),
                  UI::PanelStyle::kWui),
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Tooltip in the seafaring statistics window */
                  _("Go to the selected ship and open its window"),
                  shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsOpenShipWindowAndGoto),
                  UI::PanelStyle::kWui))),
     centerviewbtn_(&navigation_box_,
                    "seafaring_stats_center_main_mapview_button",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    UI::ButtonStyle::kWuiPrimary,
                    g_image_cache->get("images/wui/ship/menu_ship_goto.png"),
                    as_tooltip_text_with_hotkey(
                       /** TRANSLATORS: Tooltip in the seafaring statistics window */
                       _("Center the map on the selected ship"),
                       shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsGotoShip),
                       UI::PanelStyle::kWui)),
     table_(&main_box_, 0, 0, get_inner_w() - 2 * kPadding, 100, UI::PanelStyle::kWui) {

	// Buttons for ship states
	main_box_.add(&filter_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	filter_box_.add(&idle_btn_);
	filter_box_.add(&shipping_btn_);
	filter_box_.add(&waiting_btn_);
	filter_box_.add(&scouting_btn_);
	filter_box_.add(&portspace_btn_);

	main_box_.add(&table_, UI::Box::Resizing::kExpandBoth);

	// Navigation buttons
	main_box_.add(&navigation_box_, UI::Box::Resizing::kFullSize);
	navigation_box_.add(&watchbtn_);
	navigation_box_.add_inf_space();
	navigation_box_.add(&openwindowbtn_);
	navigation_box_.add(&centerviewbtn_);
	main_box_.set_size(get_inner_w() - 2 * kPadding, get_inner_h() - 2 * kPadding);

	// Configure actions
	idle_btn_.sigclicked.connect([this]() { filter_ships(ShipFilterStatus::kIdle); });
	shipping_btn_.sigclicked.connect([this]() { filter_ships(ShipFilterStatus::kShipping); });
	waiting_btn_.sigclicked.connect(
	   [this]() { filter_ships(ShipFilterStatus::kExpeditionWaiting); });
	scouting_btn_.sigclicked.connect(
	   [this]() { filter_ships(ShipFilterStatus::kExpeditionScouting); });
	portspace_btn_.sigclicked.connect(
	   [this]() { filter_ships(ShipFilterStatus::kExpeditionPortspaceFound); });
	ship_filter_ = ShipFilterStatus::kAll;
	set_filter_ships_tooltips();

	watchbtn_.sigclicked.connect([this]() { watch_ship(); });
	openwindowbtn_.sigclicked.connect([this]() { open_ship_window(); });
	centerviewbtn_.sigclicked.connect([this]() { center_view(); });

	// Configure table
	table_.selected.connect([this](unsigned) { selected(); });
	table_.double_clicked.connect([this](unsigned) { double_clicked(); });
	table_.add_column(
	   0, pgettext("ship", "Name"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.add_column(230, pgettext("ship", "Status"));
	table_.set_sort_column(ColName);
	fill_table();

	set_can_focus(true);
	set_thinks(false);
	table_.focus();

	shipnotes_subscriber_ =
	   Notifications::subscribe<Widelands::NoteShip>([this](const Widelands::NoteShip& note) {
		   if (iplayer().get_player() == note.ship->get_owner()) {
			   switch (note.action) {
			   case Widelands::NoteShip::Action::kDestinationChanged:
			   case Widelands::NoteShip::Action::kWaitingForCommand:
			   case Widelands::NoteShip::Action::kNoPortLeft:
			   case Widelands::NoteShip::Action::kGained:
				   assert(note.ship != nullptr);
				   update_ship(*note.ship);
				   return;
			   case Widelands::NoteShip::Action::kLost:
				   remove_ship(note.ship->serial());
				   return;
			   }
			   NEVER_HERE();
		   }
	   });

	initialization_complete();
}

const std::string
SeafaringStatisticsMenu::status_to_string(SeafaringStatisticsMenu::ShipFilterStatus status) const {
	switch (status) {
	case SeafaringStatisticsMenu::ShipFilterStatus::kIdle:
		return pgettext("ship_state", "Empty");
	case SeafaringStatisticsMenu::ShipFilterStatus::kShipping:
		return pgettext("ship_state", "Shipping");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		return pgettext("ship_state", "Waiting");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		return pgettext("ship_state", "Scouting");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		return pgettext("ship_state", "Port Space Found");
	case SeafaringStatisticsMenu::ShipFilterStatus::kAll:
		return "All";  // The user shouldn't see this, so we don't localize
	}
	NEVER_HERE();
}

const Image*
SeafaringStatisticsMenu::status_to_image(SeafaringStatisticsMenu::ShipFilterStatus status) const {
	std::string filename;
	switch (status) {
	case SeafaringStatisticsMenu::ShipFilterStatus::kIdle:
		filename = "images/wui/stats/ship_stats_idle.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kShipping:
		filename = "images/wui/stats/ship_stats_shipping.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		filename = "images/wui/buildings/start_expedition.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		filename = "images/wui/ship/ship_explore_island_cw.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		filename = "images/wui/ship/ship_construct_port_space.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kAll:
		filename = "images/wui/ship/ship_scout_ne.png";
		break;
	}
	return g_image_cache->get(filename);
}

std::unique_ptr<const SeafaringStatisticsMenu::ShipInfo>
SeafaringStatisticsMenu::create_shipinfo(const Widelands::Ship& ship) const {
	const Widelands::Ship::ShipStates state = ship.get_ship_state();
	ShipFilterStatus status = ShipFilterStatus::kAll;
	switch (state) {
	case Widelands::Ship::ShipStates::kTransport:
		if (ship.get_destination() && ship.get_fleet()->get_schedule().is_busy(ship)) {
			status = ShipFilterStatus::kShipping;
		} else {
			status = ShipFilterStatus::kIdle;
		}
		break;
	case Widelands::Ship::ShipStates::kExpeditionWaiting:
		status = ShipFilterStatus::kExpeditionWaiting;
		break;
	case Widelands::Ship::ShipStates::kExpeditionScouting:
		status = ShipFilterStatus::kExpeditionScouting;
		break;
	case Widelands::Ship::ShipStates::kExpeditionPortspaceFound:
	case Widelands::Ship::ShipStates::kExpeditionColonizing:
		// We're grouping the "colonizing" status with the port space.
		status = ShipFilterStatus::kExpeditionPortspaceFound;
		break;
	case Widelands::Ship::ShipStates::kSinkRequest:
	case Widelands::Ship::ShipStates::kSinkAnimation:
		status = ShipFilterStatus::kAll;
	}
	return std::unique_ptr<const ShipInfo>(new ShipInfo(ship.get_shipname(), status, ship.serial()));
}

void SeafaringStatisticsMenu::set_entry_record(UI::Table<uintptr_t>::EntryRecord* er,
                                               const ShipInfo& info) {
	if (info.status != ShipFilterStatus::kAll) {
		er->set_string(ColName, info.name);
		er->set_picture(ColStatus, status_to_image(info.status), status_to_string(info.status));
	}
}

Widelands::Ship* SeafaringStatisticsMenu::serial_to_ship(Widelands::Serial serial) const {
	Widelands::MapObject* obj = iplayer().game().objects().get_object(serial);
	assert(obj->descr().type() == Widelands::MapObjectType::SHIP);
	upcast(Widelands::Ship, ship, obj);
	return ship;
}

void SeafaringStatisticsMenu::update_ship(const Widelands::Ship& ship) {
	assert(iplayer().get_player() == ship.get_owner());
	std::unique_ptr<const ShipInfo> info = create_shipinfo(ship);
	// Remove ships that don't satisfy the filter
	if (ship_filter_ != ShipFilterStatus::kAll && !satisfies_filter(*info, ship_filter_)) {
		remove_ship(info->serial);
		return;
	}
	// Try to find the ship in the table
	if (data_.count(info->serial) == 1) {
		const ShipInfo* old_info = data_[info->serial].get();
		if (info->status != old_info->status) {
			// The status has changed - we need an update
			UI::Table<uintptr_t>::EntryRecord* er = table_.find(info->serial);
			set_entry_record(er, *info);
			data_[info->serial] = std::move(info);
		}
	} else {
		// This is a new ship or it was filtered away before
		UI::Table<uintptr_t>::EntryRecord& er = table_.add(info->serial);
		set_entry_record(&er, *info);
		data_.insert(std::make_pair(info->serial, std::move(info)));
	}
	table_.sort();
	update_button_states();
}

void SeafaringStatisticsMenu::remove_ship(Widelands::Serial serial) {
	if (data_.count(serial) == 1) {
		table_.remove_entry(serial);
		data_.erase(data_.find(serial));
		if (!table_.empty() && !table_.has_selection()) {
			table_.select(0);
		}
		update_button_states();
	}
}

void SeafaringStatisticsMenu::update_entry_record(UI::Table<uintptr_t>::EntryRecord& er,
                                                  const ShipInfo& info) {
	er.set_picture(ColStatus, status_to_image(info.status), status_to_string(info.status));
}

void SeafaringStatisticsMenu::selected() {
	update_button_states();
}

void SeafaringStatisticsMenu::double_clicked() {
	if (table_.has_selection()) {
		center_view();
	}
}

void SeafaringStatisticsMenu::update_button_states() {
	centerviewbtn_.set_enabled(table_.has_selection());
	openwindowbtn_.set_enabled(table_.has_selection());
	watchbtn_.set_enabled(table_.has_selection());
}

bool SeafaringStatisticsMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsGotoShip, code)) {
			center_view();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsOpenShipWindow, code)) {
			open_ship_window();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsOpenShipWindowAndGoto, code)) {
			open_ship_window();
			center_view();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsWatchShip, code)) {
			watch_ship();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterAll, code)) {
			filter_ships(ShipFilterStatus::kAll);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterIdle, code)) {
			filter_ships(ShipFilterStatus::kIdle);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterShipping, code)) {
			filter_ships(ShipFilterStatus::kShipping);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterExpWait, code)) {
			filter_ships(ShipFilterStatus::kExpeditionWaiting);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterExpScout, code)) {
			filter_ships(ShipFilterStatus::kExpeditionScouting);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameSeafaringstatsFilterExpPortspace, code)) {
			filter_ships(ShipFilterStatus::kExpeditionPortspaceFound);
			return true;
		}
	}

	return table_.handle_key(down, code);
}

void SeafaringStatisticsMenu::center_view() {
	if (table_.has_selection()) {
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		iplayer().map_view()->scroll_to_field(ship->get_position(), MapView::Transition::Smooth);
	}
}

void SeafaringStatisticsMenu::watch_ship() {
	if (table_.has_selection()) {
		iplayer().show_watch_window(*serial_to_ship(table_.get_selected()));
	}
}

void SeafaringStatisticsMenu::open_ship_window() {
	if (table_.has_selection()) {
		// Move to ship if CTRL is prssed
		if (SDL_GetModState() & KMOD_CTRL) {
			center_view();
		}
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		iplayer().show_ship_window(ship);
	}
}

void SeafaringStatisticsMenu::filter_ships(ShipFilterStatus status) {
	switch (status) {
	case ShipFilterStatus::kExpeditionWaiting:
		toggle_filter_ships_button(waiting_btn_, status);
		break;
	case ShipFilterStatus::kExpeditionScouting:
		toggle_filter_ships_button(scouting_btn_, status);
		break;
	case ShipFilterStatus::kExpeditionPortspaceFound:
		toggle_filter_ships_button(portspace_btn_, status);
		break;
	case ShipFilterStatus::kShipping:
		toggle_filter_ships_button(shipping_btn_, status);
		break;
	case ShipFilterStatus::kIdle:
		toggle_filter_ships_button(idle_btn_, status);
		break;
	case ShipFilterStatus::kAll:
		set_filter_ships_tooltips();
		ship_filter_ = ShipFilterStatus::kAll;
		waiting_btn_.set_perm_pressed(false);
		scouting_btn_.set_perm_pressed(false);
		portspace_btn_.set_perm_pressed(false);
		shipping_btn_.set_perm_pressed(false);
		idle_btn_.set_perm_pressed(false);
		break;
	}
	fill_table();
}

void SeafaringStatisticsMenu::toggle_filter_ships_button(UI::Button& button,
                                                         ShipFilterStatus status) {
	set_filter_ships_tooltips();
	if (button.style() == UI::Button::VisualState::kPermpressed) {
		button.set_perm_pressed(false);
		ship_filter_ = ShipFilterStatus::kAll;
	} else {
		waiting_btn_.set_perm_pressed(false);
		scouting_btn_.set_perm_pressed(false);
		portspace_btn_.set_perm_pressed(false);
		shipping_btn_.set_perm_pressed(false);
		idle_btn_.set_perm_pressed(false);
		button.set_perm_pressed(true);
		ship_filter_ = status;

		button.set_tooltip(as_tooltip_text_with_hotkey(
		   /** TRANSLATORS: Tooltip in the ship statistics window */
		   _("Show all ships"), shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterAll),
		   UI::PanelStyle::kWui));
	}
}

void SeafaringStatisticsMenu::set_filter_ships_tooltips() {

	idle_btn_.set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the ship statistics window */
	   _("Show empty ships"), shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterIdle),
	   UI::PanelStyle::kWui));
	shipping_btn_.set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the ship statistics window */
	   _("Show ships shipping wares and workers"),
	   shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterShipping),
	   UI::PanelStyle::kWui));
	waiting_btn_.set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the ship statistics window */
	   _("Show waiting expeditions"),
	   shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterExpWait),
	   UI::PanelStyle::kWui));
	scouting_btn_.set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the ship statistics window */
	   _("Show scouting expeditions"),
	   shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterExpScout),
	   UI::PanelStyle::kWui));
	portspace_btn_.set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the ship statistics window */
	   _("Show expeditions that have found a port space or are founding a colony"),
	   shortcut_string_for(KeyboardShortcut::kInGameSeafaringstatsFilterExpPortspace),
	   UI::PanelStyle::kWui));
}

inline bool SeafaringStatisticsMenu::satisfies_filter(const ShipInfo& info,
                                                      ShipFilterStatus filter) {
	return filter == info.status;
}

void SeafaringStatisticsMenu::fill_table() {
	const Widelands::Serial last_selection =
	   table_.has_selection() ? table_.get_selected() : Widelands::INVALID_INDEX;
	table_.clear();
	data_.clear();
	// Disable buttons while we update the data
	update_button_states();
	for (const auto& serial : iplayer().player().ships()) {
		Widelands::Ship* ship = serial_to_ship(serial);
		assert(ship != nullptr);
		assert(iplayer().get_player() == ship->get_owner());
		std::unique_ptr<const ShipInfo> info = create_shipinfo(*ship);
		if (info->status != ShipFilterStatus::kAll) {
			if (ship_filter_ == ShipFilterStatus::kAll || satisfies_filter(*info, ship_filter_)) {
				UI::Table<uintptr_t const>::EntryRecord& er =
				   table_.add(serial, serial == last_selection);
				set_entry_record(&er, *info);
				data_.insert(std::make_pair(serial, std::move(info)));
			}
		}
	}

	if (!table_.empty()) {
		table_.sort();
		if (!table_.has_selection()) {
			// This will take care of the button state too
			table_.select(0);
		}
	} else {
		// Fix column width. Because no entries were added, the table didn't layout itself
		table_.layout();
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& SeafaringStatisticsMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_seafaring;
			r.create();
			assert(r.window);
			SeafaringStatisticsMenu& m = dynamic_cast<SeafaringStatisticsMenu&>(*r.window);
			m.filter_ships(static_cast<ShipFilterStatus>(fr.unsigned_8()));
			m.table_.select(fr.unsigned_32());
			return m;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Seafaring Statistics Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("seafaring statistics menu: %s", e.what());
	}
}
void SeafaringStatisticsMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(static_cast<uint8_t>(ship_filter_));
	fw.unsigned_32(table_.selection_index());
}
