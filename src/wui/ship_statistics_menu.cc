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

#include "wui/ship_statistics_menu.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "economy/fleet.h"
#include "graphic/graphic.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/box.h"
#include "wui/interactive_player.h"

// NOCOM documentation
// NOCOM watch and open window buttons would be nice
inline InteractivePlayer& ShipStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

constexpr int kWindowWidth = 355;
constexpr int kWindowHeight = 375;
constexpr int kTableHeight = 300;
constexpr int kPadding = 5;
constexpr int kButtonSize = 34;

ShipStatisticsMenu::ShipStatisticsMenu(InteractivePlayer& plr, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &plr, "ship_statistics", &registry, kWindowWidth, kWindowHeight, _("Ship Statistics")),
     table_(this,
            kPadding,
            kButtonSize + 2 * kPadding,
            kWindowWidth - 2 * kPadding,
            kTableHeight,
            g_gr->images().get("images/ui_basic/but1.png")) {

	table_.selected.connect(boost::bind(&ShipStatisticsMenu::selected, this));
	table_.double_clicked.connect(boost::bind(&ShipStatisticsMenu::double_clicked, this));
	table_.add_column(kWindowWidth / 2 - kPadding, pgettext("ship", "Name"));
	table_.add_column(
	   0, pgettext("ship", "Status"), "", UI::Align::kHCenter, UI::TableColumnType::kFlexible);
	table_.focus();

	// Buttons for ship states
	UI::Box* button_box = new UI::Box(
	   this, kPadding, kPadding, UI::Box::Horizontal, table_.get_w(), kButtonSize, kPadding);

	idle_btn_ = new UI::Button(button_box, "filter_ship_idle", 0, 0, kButtonSize, kButtonSize,
	                           g_gr->images().get("images/ui_basic/but0.png"),
	                           status_to_image(ShipFilterStatus::kIdle));
	button_box->add(idle_btn_, UI::Align::kLeft);
	idle_btn_->sigclicked.connect(
	   boost::bind(&ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kIdle));

	shipping_btn_ = new UI::Button(button_box, "filter_ship_transporting", 0, 0, kButtonSize,
	                               kButtonSize, g_gr->images().get("images/ui_basic/but0.png"),
	                               status_to_image(ShipFilterStatus::kShipping));
	button_box->add(shipping_btn_, UI::Align::kLeft);
	shipping_btn_->sigclicked.connect(
	   boost::bind(&ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kShipping));

	waiting_btn_ = new UI::Button(button_box, "filter_ship_waiting", 0, 0, kButtonSize, kButtonSize,
	                              g_gr->images().get("images/ui_basic/but0.png"),
	                              status_to_image(ShipFilterStatus::kExpeditionWaiting));
	button_box->add(waiting_btn_, UI::Align::kLeft);
	waiting_btn_->sigclicked.connect(
	   boost::bind(&ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionWaiting));

	scouting_btn_ = new UI::Button(button_box, "filter_ship_scouting", 0, 0, kButtonSize,
	                               kButtonSize, g_gr->images().get("images/ui_basic/but0.png"),
	                               status_to_image(ShipFilterStatus::kExpeditionScouting));
	button_box->add(scouting_btn_, UI::Align::kLeft);
	scouting_btn_->sigclicked.connect(
	   boost::bind(&ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionScouting));

	portspace_btn_ = new UI::Button(button_box, "filter_ship_portspace", 0, 0, kButtonSize,
	                                kButtonSize, g_gr->images().get("images/ui_basic/but0.png"),
	                                status_to_image(ShipFilterStatus::kExpeditionPortspaceFound));
	button_box->add(portspace_btn_, UI::Align::kLeft);
	portspace_btn_->sigclicked.connect(boost::bind(
	   &ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionPortspaceFound));

	colonizing_btn_ = new UI::Button(button_box, "filter_ship_colonizing", 0, 0, kButtonSize,
	                                 kButtonSize, g_gr->images().get("images/ui_basic/but0.png"),
	                                 status_to_image(ShipFilterStatus::kExpeditionColonizing));
	button_box->add(colonizing_btn_, UI::Align::kLeft);
	colonizing_btn_->sigclicked.connect(boost::bind(
	   &ShipStatisticsMenu::filter_ships, this, ShipFilterStatus::kExpeditionColonizing));

	button_box->set_size(table_.get_w(), kButtonSize);

	ship_filter_ = ShipFilterStatus::kAll;
	set_filter_ships_tooltips();
	// End: Buttons for message types
	centerviewbtn_ =
	   new UI::Button(this, "center_main_mapview_on_location", kWindowWidth - kPadding - kButtonSize,
	                  kWindowHeight - kPadding - kButtonSize, kButtonSize, kButtonSize,
	                  g_gr->images().get("images/ui_basic/but2.png"),
	                  g_gr->images().get("images/wui/ship/menu_ship_goto.png"),
	                  /** TRANSLATORS: %s is a tooltip, G is the corresponding hotkey */
	                  (boost::format(_("G: %s"))
	                   /** TRANSLATORS: Tooltip in the messages window */
	                   % _("Center main mapview on location"))
	                     .str());
	centerviewbtn_->sigclicked.connect(boost::bind(&ShipStatisticsMenu::center_view, this));
	centerviewbtn_->set_enabled(false);

	table_.set_sort_column(ColName);
	fill_table();
	set_can_focus(true);
	focus();
	set_thinks(false);

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
			   remove_ship(*note.ship);
			   break;
		   default:
			   break;  // Do nothing
		   }
		});
}

const std::string
ShipStatisticsMenu::status_to_string(ShipStatisticsMenu::ShipFilterStatus status) const {
	switch (status) {
	case ShipStatisticsMenu::ShipFilterStatus::kIdle:
		return pgettext("ship_state", "Idle");
	case ShipStatisticsMenu::ShipFilterStatus::kShipping:
		return pgettext("ship_state", "Shipping");
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		return pgettext("ship_state", "Waiting");
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		return pgettext("ship_state", "Scouting");
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		return pgettext("ship_state", "Port Space Found");
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionColonizing:
		return pgettext("ship_state", "Founding a Colony");
	case ShipStatisticsMenu::ShipFilterStatus::kAll:
		return "All";  // The user shouldn't see this, so we don't localize
	default:
		NEVER_HERE();
	}
}

const Image*
ShipStatisticsMenu::status_to_image(ShipStatisticsMenu::ShipFilterStatus status) const {
	std::string filename = "";
	switch (status) {
	case ShipStatisticsMenu::ShipFilterStatus::kIdle:
		filename = "images/wui/ship/ship_scout_e.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kShipping:
		filename = "images/wui/buildings/menu_tab_wares_dock.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionWaiting:
		filename = "images/wui/buildings/start_expedition.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionScouting:
		filename = "images/wui/ship/ship_explore_island_cw.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionPortspaceFound:
		filename = "images/wui/editor/fsel_editor_set_port_space.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kExpeditionColonizing:
		filename = "images/wui/fieldaction/menu_tab_buildport.png";
		break;
	case ShipStatisticsMenu::ShipFilterStatus::kAll:
		filename = "images/wui/ship/ship_scout_ne.png";
		break;
	default:
		NEVER_HERE();
	}
	return g_gr->images().get(filename);
}

const ShipStatisticsMenu::ShipInfo*
ShipStatisticsMenu::create_shipinfo(const Widelands::Ship& ship) const {
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

void ShipStatisticsMenu::set_entry_record(UI::Table<uintptr_t>::EntryRecord* er,
                                          const ShipInfo& info) {
	if (info.status != ShipFilterStatus::kAll) {
		er->set_string(ColName, info.name);
		er->set_picture(ColStatus, status_to_image(info.status), status_to_string(info.status));
	}
}

Widelands::Ship* ShipStatisticsMenu::serial_to_ship(Widelands::Serial serial) const {
	Widelands::MapObject* obj = iplayer().game().objects().get_object(serial);
	assert(obj->descr().type() == Widelands::MapObjectType::SHIP);
	upcast(Widelands::Ship, ship, obj);
	return ship;
}

void ShipStatisticsMenu::update_ship(const Widelands::Ship& ship) {
	const ShipInfo* info = create_shipinfo(ship);
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
		// This is a new ship
		data_.insert(std::make_pair(info->serial, info));
		UI::Table<uintptr_t>::EntryRecord& er = table_.add(info->serial);
		set_entry_record(&er, *info);
	}
	table_.sort();
}

void ShipStatisticsMenu::remove_ship(const Widelands::Ship& ship) {
	const ShipInfo* const info = create_shipinfo(ship);
	if (data_.count(info->serial) == 1) {
		table_.remove_entry(info->serial);
		data_.erase(data_.find(info->serial));
	}
}

void ShipStatisticsMenu::update_entry_record(UI::Table<uintptr_t>::EntryRecord& er,
                                             const ShipInfo& info) {
	er.set_picture(ColStatus, status_to_image(info.status), status_to_string(info.status));
}

/*
 * Something has been selected
 */
void ShipStatisticsMenu::selected() {
	centerviewbtn_->set_enabled(table_.has_selection());
}

/**
 * a message was double clicked
 */
void ShipStatisticsMenu::double_clicked() {
	if (centerviewbtn_->enabled()) {
		center_view();
	}
}

/**
 * Handle message menu hotkeys.
 */
bool ShipStatisticsMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		// Don't forget to change the tooltips if any of these get reassigned
		case SDLK_g:
			if (centerviewbtn_->enabled())
				center_view();
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

void ShipStatisticsMenu::center_view() {
	if (table_.has_selection()) {
		Widelands::Ship* ship = serial_to_ship(table_.get_selected());
		iplayer().scroll_to_field(ship->get_position(), MapView::Transition::Smooth);
	}
}

/**
 * Show only messages of a certain type
 * @param msgtype the types of messages to show
 */
void ShipStatisticsMenu::filter_ships(ShipFilterStatus msgtype) {
	switch (msgtype) {
	case ShipFilterStatus::kExpeditionWaiting:
		toggle_filter_ships_button(*waiting_btn_, msgtype);
		break;
	case ShipFilterStatus::kExpeditionScouting:
		toggle_filter_ships_button(*scouting_btn_, msgtype);
		break;
	case ShipFilterStatus::kExpeditionPortspaceFound:
		toggle_filter_ships_button(*portspace_btn_, msgtype);
		break;
	case ShipFilterStatus::kExpeditionColonizing:
		toggle_filter_ships_button(*colonizing_btn_, msgtype);
		break;
	case ShipFilterStatus::kShipping:
		toggle_filter_ships_button(*shipping_btn_, msgtype);
		break;
	case ShipFilterStatus::kIdle:
		toggle_filter_ships_button(*idle_btn_, msgtype);
		break;
	case ShipFilterStatus::kAll:
		set_filter_ships_tooltips();
		ship_filter_ = ShipFilterStatus::kAll;
		waiting_btn_->set_perm_pressed(false);
		scouting_btn_->set_perm_pressed(false);
		portspace_btn_->set_perm_pressed(false);
		colonizing_btn_->set_perm_pressed(false);
		shipping_btn_->set_perm_pressed(false);
		idle_btn_->set_perm_pressed(false);
		break;
	}
	// NOCOM filter the ships
}

/**
 * Helper for filter_ships
 */
void ShipStatisticsMenu::toggle_filter_ships_button(UI::Button& button, ShipFilterStatus msgtype) {
	set_filter_ships_tooltips();
	if (button.style() == UI::Button::Style::kPermpressed) {
		button.set_perm_pressed(false);
		ship_filter_ = ShipFilterStatus::kAll;
	} else {
		waiting_btn_->set_perm_pressed(false);
		scouting_btn_->set_perm_pressed(false);
		portspace_btn_->set_perm_pressed(false);
		colonizing_btn_->set_perm_pressed(false);
		shipping_btn_->set_perm_pressed(false);
		idle_btn_->set_perm_pressed(false);
		button.set_perm_pressed(true);
		ship_filter_ = msgtype;

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
void ShipStatisticsMenu::set_filter_ships_tooltips() {

	idle_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                        /** TRANSLATORS: Tooltip in the messages window */
	                        % _("Show idle ships only") % pgettext("hotkey", "Alt + 1"))
	                          .str());
	shipping_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                            /** TRANSLATORS: Tooltip in the messages window */
	                            % _("Show ships transporting wares only") %
	                            pgettext("hotkey", "Alt + 2"))
	                              .str());
	waiting_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                           /** TRANSLATORS: Tooltip in the messages window */
	                           % _("Show waiting expeditions only") % pgettext("hotkey", "Alt + 3"))
	                             .str());
	scouting_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                            /** TRANSLATORS: Tooltip in the messages window */
	                            % _("Show scouting expeditions only") %
	                            pgettext("hotkey", "Alt + 4"))
	                              .str());
	portspace_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                             /** TRANSLATORS: Tooltip in the messages window */
	                             % _("Show ships that found a portspace only") %
	                             pgettext("hotkey", "Alt + 5"))
	                               .str());
	colonizing_btn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
	                              /** TRANSLATORS: Tooltip in the messages window */
	                              % _("Show solonizing ships only") % pgettext("hotkey", "Alt + 6"))
	                                .str());
}

void ShipStatisticsMenu::fill_table() {
	data_.clear();
	table_.clear();
	for (const auto& serial : iplayer().player().ships()) {
		const ShipInfo* info = create_shipinfo(*serial_to_ship(serial));
		if (info->status != ShipFilterStatus::kAll) {
			data_.insert(std::make_pair(serial, info));
			UI::Table<uintptr_t const>::EntryRecord& er = table_.add(serial);
			set_entry_record(&er, *info);
		}
	}

	if (!table_.empty()) {
		table_.sort();
		table_.select(0);
	}
}
