/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#include "wui/seafaring_statistics_menu.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "economy/fleet.h"
#include "graphic/graphic.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/box.h"
#include "wui/interactive_player.h"
#include "wui/shipwindow.h"
#include "wui/watchwindow.h"

// NOCOM documentation
inline InteractivePlayer& SeafaringStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

constexpr int kPadding = 5;
constexpr int kButtonSize = 34;

SeafaringStatisticsMenu::SeafaringStatisticsMenu(InteractivePlayer& plr,
                                                 UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&plr, "seafaring_statistics", &registry, 355, 375, _("Seafaring Statistics")),
     main_box_(this, kPadding, kPadding, UI::Box::Vertical, get_inner_w(), get_inner_h(), kPadding),
     filter_box_(
        &main_box_, 0, 0, UI::Box::Horizontal, get_inner_w() - 2 * kPadding, kButtonSize, kPadding),
     idle_btn_(&filter_box_,
               "filter_ship_idle",
               0,
               0,
               kButtonSize,
               kButtonSize,
               g_gr->images().get("images/ui_basic/but0.png"),
               status_to_image(ShipFilterStatus::kIdle)),
     waiting_btn_(&filter_box_,
                  "filter_ship_waiting",
                  0,
                  0,
                  kButtonSize,
                  kButtonSize,
                  g_gr->images().get("images/ui_basic/but0.png"),
                  status_to_image(ShipFilterStatus::kExpeditionWaiting)),
     scouting_btn_(&filter_box_,
                   "filter_ship_scouting",
                   0,
                   0,
                   kButtonSize,
                   kButtonSize,
                   g_gr->images().get("images/ui_basic/but0.png"),
                   status_to_image(ShipFilterStatus::kExpeditionScouting)),
     portspace_btn_(&filter_box_,
                    "filter_ship_portspace",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    g_gr->images().get("images/ui_basic/but0.png"),
                    status_to_image(ShipFilterStatus::kExpeditionPortspaceFound)),
     colonizing_btn_(&filter_box_,
                     "filter_ship_colonizing",
                     0,
                     0,
                     kButtonSize,
                     kButtonSize,
                     g_gr->images().get("images/ui_basic/but0.png"),
                     status_to_image(ShipFilterStatus::kExpeditionColonizing)),
     shipping_btn_(&filter_box_,
                   "filter_ship_transporting",
                   0,
                   0,
                   kButtonSize,
                   kButtonSize,
                   g_gr->images().get("images/ui_basic/but0.png"),
                   status_to_image(ShipFilterStatus::kShipping)),
     ship_filter_(ShipFilterStatus::kAll),
     navigation_box_(
        &main_box_, 0, 0, UI::Box::Horizontal, get_inner_w() - 2 * kPadding, kButtonSize, kPadding),
     watchbtn_(&navigation_box_,
               "seafaring_stats_watch_button",
               0,
               0,
               kButtonSize,
               kButtonSize,
               g_gr->images().get("images/ui_basic/but2.png"),
               g_gr->images().get("images/wui/menus/menu_watch_follow.png"),
               (boost::format(_("%1% (Hotkey: %2%)"))
                /** TRANSLATORS: Tooltip in the messages window */
                %
                _("Watch the selected ship") % pgettext("hotkey", "W"))
                  .str()),
     openwindowbtn_(&navigation_box_,
                    "seafaring_stats_watch_button",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    g_gr->images().get("images/ui_basic/but2.png"),
                    g_gr->images().get("images/ui_basic/fsel.png"),
                    (boost::format(_("%1% (Hotkey: %2%)"))
                     /** TRANSLATORS: Tooltip in the messages window */
                     %
                     _("Go to the selected ship and open its window") % pgettext("hotkey", "O"))
                       .str()),
     centerviewbtn_(&navigation_box_,
                    "seafaring_stats_center_main_mapview_button",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    g_gr->images().get("images/ui_basic/but2.png"),
                    g_gr->images().get("images/wui/ship/menu_ship_goto.png"),
                    (boost::format(_("%1% (Hotkey: %2%)"))
                     /** TRANSLATORS: Tooltip in the messages window */
                     %
                     _("Center the map on the selected ship") % pgettext("hotkey", "G"))
                       .str()),
     table_(&main_box_,
            0,
            0,
            get_inner_w() - 2 * kPadding,
            100,
            g_gr->images().get("images/ui_basic/but1.png")) {

	// Buttons for ship states
	main_box_.add(&filter_box_, UI::Align::kLeft, true);
	filter_box_.add(&idle_btn_, UI::Align::kLeft);
	filter_box_.add(&shipping_btn_, UI::Align::kLeft);
	filter_box_.add(&waiting_btn_, UI::Align::kLeft);
	filter_box_.add(&scouting_btn_, UI::Align::kLeft);
	filter_box_.add(&portspace_btn_, UI::Align::kLeft);
	filter_box_.add(&colonizing_btn_, UI::Align::kLeft);

	main_box_.add(&table_, UI::Align::kLeft, true, true);

	// Navigation buttons
	main_box_.add(&navigation_box_, UI::Align::kLeft, true);
	navigation_box_.add(&watchbtn_, UI::Align::kLeft);
	navigation_box_.add_inf_space();
	navigation_box_.add(&openwindowbtn_, UI::Align::kLeft);
	navigation_box_.add(&centerviewbtn_, UI::Align::kLeft);
	main_box_.set_size(get_inner_w() - 2 * kPadding, get_inner_h() - 2 * kPadding);

	// Configure actions
	idle_btn_.sigclicked.connect(
	   boost::bind(&SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kIdle));
	shipping_btn_.sigclicked.connect(
	   boost::bind(&SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kShipping));
	waiting_btn_.sigclicked.connect(boost::bind(
	   &SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionWaiting));
	scouting_btn_.sigclicked.connect(boost::bind(
	   &SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionScouting));
	portspace_btn_.sigclicked.connect(boost::bind(
	   &SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionPortspaceFound));
	colonizing_btn_.sigclicked.connect(boost::bind(
	   &SeafaringStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionColonizing));
	ship_filter_ = ShipFilterStatus::kAll;
	set_filter_ships_tooltips();

	watchbtn_.sigclicked.connect(boost::bind(&SeafaringStatisticsMenu::watch_ship, this));
	openwindowbtn_.sigclicked.connect(boost::bind(&SeafaringStatisticsMenu::open_ship_window, this));
	centerviewbtn_.sigclicked.connect(boost::bind(&SeafaringStatisticsMenu::center_view, this));

	// Configure table
	table_.selected.connect(boost::bind(&SeafaringStatisticsMenu::selected, this));
	table_.double_clicked.connect(boost::bind(&SeafaringStatisticsMenu::double_clicked, this));
	table_.add_column(get_inner_w() / 2 - kPadding, pgettext("ship", "Name"));
	table_.add_column(
	   0, pgettext("ship", "Status"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.set_sort_column(ColName);
	fill_table();

	set_can_focus(true);
	set_thinks(false);
	table_.focus();

	// NOCOM unify these when the other branch is in.
	shipwindownotes_subscriber_ = Notifications::subscribe<Widelands::NoteShipWindow>(
	   [this](const Widelands::NoteShipWindow& note) {
		   switch (note.action) {
		   // The ship state has changed, e.g. expedition canceled
		   case Widelands::NoteShipWindow::Action::kRefresh:
		   case Widelands::NoteShipWindow::Action::kDestinationChanged: {
			   update_ship(*serial_to_ship(note.serial));
		   } break;
		   default:
			   break;  // Do nothing
		   }
		});

	shipnotes_subscriber_ = Notifications::subscribe<Widelands::NoteShipMessage>(
	   [this](const Widelands::NoteShipMessage& note) {
		   switch (note.message) {
		   case Widelands::NoteShipMessage::Message::kGained:
		   case Widelands::NoteShipMessage::Message::kWaitingForCommand:
			   update_ship(*note.ship);
			   break;
		   case Widelands::NoteShipMessage::Message::kLost:
			   remove_ship(note.ship->serial());
			   break;
		   default:
			   break;  // Do nothing
		   }
		});
}

const std::string
SeafaringStatisticsMenu::status_to_string(SeafaringStatisticsMenu::ShipFilterStatus status) const {
	switch (status) {
	case SeafaringStatisticsMenu::ShipFilterStatus::kIdle:
		return pgettext("ship_state", "Idle");
	case SeafaringStatisticsMenu::ShipFilterStatus::kShipping:
		return pgettext("ship_state", "Shipping");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		return pgettext("ship_state", "Waiting");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		return pgettext("ship_state", "Scouting");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		return pgettext("ship_state", "Port Space Found");
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionColonizing:
		return pgettext("ship_state", "Founding a Colony");
	case SeafaringStatisticsMenu::ShipFilterStatus::kAll:
		return "All";  // The user shouldn't see this, so we don't localize
	default:
		NEVER_HERE();
	}
}

const Image*
SeafaringStatisticsMenu::status_to_image(SeafaringStatisticsMenu::ShipFilterStatus status) const {
	std::string filename = "";
	switch (status) {
	case SeafaringStatisticsMenu::ShipFilterStatus::kIdle:
		filename = "images/wui/ship/ship_scout_e.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kShipping:
		filename = "images/wui/buildings/menu_tab_wares_dock.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		filename = "images/wui/buildings/start_expedition.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		filename = "images/wui/ship/ship_explore_island_cw.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		filename = "images/wui/editor/fsel_editor_set_port_space.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kExpeditionColonizing:
		filename = "images/wui/fieldaction/menu_tab_buildport.png";
		break;
	case SeafaringStatisticsMenu::ShipFilterStatus::kAll:
		filename = "images/wui/ship/ship_scout_ne.png";
		break;
	default:
		NEVER_HERE();
	}
	return g_gr->images().get(filename);
}

const SeafaringStatisticsMenu::ShipInfo*
SeafaringStatisticsMenu::create_shipinfo(const Widelands::Ship& ship) const {
	if (&ship == nullptr) {
		return new ShipInfo();
	}
	const Widelands::Ship::ShipStates state = ship.get_ship_state();
	ShipFilterStatus status = ShipFilterStatus::kAll;
	switch (state) {
	case Widelands::Ship::ShipStates::kTransport:
		if (ship.get_destination(iplayer().game()) != nullptr) {
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
		status = ShipFilterStatus::kExpeditionPortspaceFound;
		break;
	case Widelands::Ship::ShipStates::kExpeditionColonizing:
		status = ShipFilterStatus::kExpeditionColonizing;
		break;
	case Widelands::Ship::ShipStates::kSinkRequest:
	case Widelands::Ship::ShipStates::kSinkAnimation:
		status = ShipFilterStatus::kAll;
	}
	return new ShipInfo(ship.get_shipname(), status, ship.serial());
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
	const ShipInfo* info = create_shipinfo(ship);
	// Remove ships that don't satisfy the filter
	if (ship_filter_ != ShipFilterStatus::kAll && info->status != ship_filter_) {
		remove_ship(info->serial);
		return;
	}
	// Try to find the ship in the table
	if (data_.count(info->serial) == 1) {
		const ShipInfo* old_info = data_[info->serial];
		if (info->status != old_info->status) {
			// The status has changed - we need an update
			data_[info->serial] = info;
			UI::Table<uintptr_t>::EntryRecord* er = table_.find(info->serial);
			set_entry_record(er, *info);
		}
	} else {
		// This is a new ship or it was filteres away before
		data_.insert(std::make_pair(info->serial, info));
		UI::Table<uintptr_t>::EntryRecord& er = table_.add(info->serial);
		set_entry_record(&er, *info);
	}
	table_.sort();
	set_buttons_enabled();
}

void SeafaringStatisticsMenu::remove_ship(Widelands::Serial serial) {
	if (data_.count(serial) == 1) {
		table_.remove_entry(serial);
		data_.erase(data_.find(serial));
		if (!table_.empty() && !table_.has_selection()) {
			table_.select(0);
		}
		set_buttons_enabled();
	}
}

void SeafaringStatisticsMenu::update_entry_record(UI::Table<uintptr_t>::EntryRecord& er,
                                                  const ShipInfo& info) {
	er.set_picture(ColStatus, status_to_image(info.status), status_to_string(info.status));
}

/*
 * Something has been selected
 */
void SeafaringStatisticsMenu::selected() {
	set_buttons_enabled();
}

/**
 * a message was double clicked
 */
void SeafaringStatisticsMenu::double_clicked() {
	if (table_.has_selection()) {
		center_view();
	}
}

void SeafaringStatisticsMenu::set_buttons_enabled() {
	centerviewbtn_.set_enabled(table_.has_selection());
	openwindowbtn_.set_enabled(table_.has_selection());
	watchbtn_.set_enabled(table_.has_selection());
}

/**
 * Handle message menu hotkeys.
 */
bool SeafaringStatisticsMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		// Don't forget to change the tooltips if any of these get reassigned
		case SDLK_g:
			center_view();
			return true;
		case SDLK_o:
			open_ship_window();
			return true;
		case SDLK_w:
			watch_ship();
			return true;
		case SDLK_0:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kAll);
				return true;
			}
			return false;
		case SDLK_1:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kIdle);
				return true;
			}
			return false;
		case SDLK_2:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kShipping);
				return true;
			}
			return false;
		case SDLK_3:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kExpeditionWaiting);
				return true;
			}
			return false;
		case SDLK_4:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kExpeditionScouting);
				return true;
			}
			return false;
		case SDLK_5:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kExpeditionPortspaceFound);
				return true;
			}
			return false;
		case SDLK_6:
			if (code.mod & KMOD_ALT) {
				filter_ships(ShipFilterStatus::kExpeditionColonizing);
				return true;
			}
			return false;
		case SDL_SCANCODE_KP_PERIOD:
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		default:
			break;  // not handled
		}
	}

	return table_.handle_key(down, code);
}

void SeafaringStatisticsMenu::center_view() {
	if (table_.has_selection()) {
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		iplayer().scroll_to_field(ship->get_position(), MapView::Transition::Smooth);
	}
}

void SeafaringStatisticsMenu::watch_ship() {
	if (table_.has_selection()) {
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		WatchWindow* window = show_watch_window(iplayer(), ship->get_position());
		window->follow();
	}
}

void SeafaringStatisticsMenu::open_ship_window() {
	if (table_.has_selection()) {
		center_view();
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		// NOCOM refactor after merging ship window branch
		new ShipWindow(iplayer(), *ship);
	}
}

/**
 * Show only the ships that have the given status
 */
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
	case ShipFilterStatus::kExpeditionColonizing:
		toggle_filter_ships_button(colonizing_btn_, status);
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
		colonizing_btn_.set_perm_pressed(false);
		shipping_btn_.set_perm_pressed(false);
		idle_btn_.set_perm_pressed(false);
		break;
	}
	fill_table();
}

/**
 * Helper for filter_ships
 */
void SeafaringStatisticsMenu::toggle_filter_ships_button(UI::Button& button,
                                                         ShipFilterStatus status) {
	set_filter_ships_tooltips();
	if (button.style() == UI::Button::Style::kPermpressed) {
		button.set_perm_pressed(false);
		ship_filter_ = ShipFilterStatus::kAll;
	} else {
		waiting_btn_.set_perm_pressed(false);
		scouting_btn_.set_perm_pressed(false);
		portspace_btn_.set_perm_pressed(false);
		colonizing_btn_.set_perm_pressed(false);
		shipping_btn_.set_perm_pressed(false);
		idle_btn_.set_perm_pressed(false);
		button.set_perm_pressed(true);
		ship_filter_ = status;

		/** TRANSLATORS: %1% is a tooltip, %2% is the corresponding hotkey */
		button.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
		                    /** TRANSLATORS: Tooltip in the messages window */
		                    % _("Show all ships") % pgettext("hotkey", "Alt + 0"))
		                      .str());
	}
}

/**
 * Helper for filter_ships
 */
void SeafaringStatisticsMenu::set_filter_ships_tooltips() {

	idle_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                       /** TRANSLATORS: Tooltip in the messages window */
	                       % _("Show idle ships") % pgettext("hotkey", "Alt + 1"))
	                         .str());
	shipping_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                           /** TRANSLATORS: Tooltip in the messages window */
	                           % _("Show ships shipping wares and workers") %
	                           pgettext("hotkey", "Alt + 2"))
	                             .str());
	waiting_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                          /** TRANSLATORS: Tooltip in the messages window */
	                          % _("Show waiting expeditions") % pgettext("hotkey", "Alt + 3"))
	                            .str());
	scouting_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                           /** TRANSLATORS: Tooltip in the messages window */
	                           % _("Show scouting expeditions") % pgettext("hotkey", "Alt + 4"))
	                             .str());
	portspace_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                            /** TRANSLATORS: Tooltip in the messages window */
	                            % _("Show expeditions with port space found") %
	                            pgettext("hotkey", "Alt + 5"))
	                              .str());
	colonizing_btn_.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                             /** TRANSLATORS: Tooltip in the messages window */
	                             % _("Show colonizing expeditions") % pgettext("hotkey", "Alt + 6"))
	                               .str());
}

void SeafaringStatisticsMenu::fill_table() {
	data_.clear();
	table_.clear();
	set_buttons_enabled();
	for (const auto& serial : iplayer().player().ships()) {
		const ShipInfo* info = create_shipinfo(*serial_to_ship(serial));
		if (info->status != ShipFilterStatus::kAll) {
			if (ship_filter_ == ShipFilterStatus::kAll || info->status == ship_filter_) {
				data_.insert(std::make_pair(serial, info));
				UI::Table<uintptr_t const>::EntryRecord& er = table_.add(serial);
				set_entry_record(&er, *info);
			}
		}
	}

	if (!table_.empty()) {
		table_.sort();
		table_.select(0);
	}
}
