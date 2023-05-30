/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#include "wui/fleet_options_window.h"

#include "economy/ferry_fleet.h"
#include "economy/ship_fleet.h"
#include "graphic/font_handler.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_base.h"

static std::set<FleetOptionsWindow*> living_fleet_option_windows;

constexpr Widelands::Quantity kMaxTarget = 1000 * 1000;  // Arbitrary limit

constexpr int kPadding = 4;

constexpr const char kIconEndInfinity[] = "images/wui/menus/end_infinity.png";
constexpr const char kIconInfinity[] = "images/wui/menus/infinity.png";

FleetOptionsWindow&
FleetOptionsWindow::create(UI::Panel* parent, InteractiveBase& ibase, Widelands::Bob* interface) {
	MutexLock m(MutexLock::ID::kObjects);

	// Check for an existing window pointing to the same fleet.
	// We can't cache the fleet serial or pointer, they can change around a lot.
	for (FleetOptionsWindow* window : living_fleet_option_windows) {
		Widelands::Bob* window_interface = window->interface_.get(ibase.egbase());
		if (window_interface == nullptr ||
		    window_interface->descr().type() != interface->descr().type()) {
			continue;  // Different type of window
		}
		if (interface->descr().type() == Widelands::MapObjectType::SHIP_FLEET_YARD_INTERFACE) {
			if (dynamic_cast<const Widelands::ShipFleetYardInterface*>(interface)->get_fleet() !=
			    dynamic_cast<const Widelands::ShipFleetYardInterface*>(window_interface)
			       ->get_fleet()) {
				continue;  // Different fleets
			}
		} else {
			if (dynamic_cast<const Widelands::FerryFleetYardInterface*>(interface)->get_fleet() !=
			    dynamic_cast<const Widelands::FerryFleetYardInterface*>(window_interface)
			       ->get_fleet()) {
				continue;  // Different fleets
			}
		}

		// Same fleet
		if (window->is_minimal()) {
			window->restore();
		}
		window->move_to_top();
		return *window;
	}

	return *new FleetOptionsWindow(
	   parent, ibase,
	   interface->descr().type() == Widelands::MapObjectType::SHIP_FLEET_YARD_INTERFACE ?
         Type::kShip :
         Type::kFerry,
	   interface);
}

FleetOptionsWindow::~FleetOptionsWindow() {
	living_fleet_option_windows.erase(this);
}

FleetOptionsWindow::FleetOptionsWindow(UI::Panel* parent,
                                       InteractiveBase& ibase,
                                       Type t,
                                       Widelands::Bob* interface)
   : UI::Window(parent,
                UI::WindowStyle::kWui,
                format("fleet_options_%u", interface->serial()),
                0,
                0,
                100,
                100,
                t == Type::kShip ? _("Ship Fleet Options") : _("Ferry Fleet Options")),

     ibase_(ibase),
     can_act_(ibase_.can_act(interface->owner().player_number())),
     type_(t),
     interface_(interface),

     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     buttons_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     spinbox_(&buttons_box_,
              0,
              0,
              300,
              300,
              0,
              0,
              0,
              UI::PanelStyle::kWui,
              "",
              UI::SpinBox::Units::kNone,
              UI::SpinBox::Type::kBig),
     infinite_target_(&buttons_box_,
                      "toggle_infinite",
                      0,
                      0,
                      34,
                      34,
                      UI::ButtonStyle::kWuiSecondary,
                      g_image_cache->get(kIconInfinity),
                      _("Toggle infinite production")) {
	living_fleet_option_windows.insert(this);

	const bool rtl = UI::g_fh->fontset()->is_rtl();

	buttons_box_.add(&infinite_target_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add(&spinbox_, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel,
	                               _("Fleet Statistics"), UI::Align::kCenter),
	              UI::Box::Resizing::kFullSize);

	UI::Box* columns_box = new UI::Box(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Box* column1 = new UI::Box(columns_box, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	UI::Box* column2 = new UI::Box(columns_box, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);

	auto create_textarea = [column1, column2, rtl](UI::Textarea** txt, std::string label) {
		column1->add_space(kPadding);
		column2->add_space(kPadding);

		column1->add(
		   new UI::Textarea(column1, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, label,
		                    UI::mirror_alignment(UI::Align::kLeft, rtl)),
		   UI::Box::Resizing::kFullSize);
		*txt = new UI::Textarea(column2, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelParagraph,
		                        "", UI::mirror_alignment(UI::Align::kRight, rtl));
		column2->add(*txt, UI::Box::Resizing::kFullSize);
	};
	if (type_ == Type::kShip) {
		create_textarea(&txt_ports_, _("Ports:"));
		create_textarea(&txt_ships_, _("Ships:"));
	} else {
		create_textarea(&txt_waterways_total_, _("Total waterways:"));
		create_textarea(&txt_waterways_lacking_, _("Unserviced waterways:"));
		create_textarea(&txt_ferries_total_, _("Total ferries:"));
		create_textarea(&txt_ferries_unemployed_, _("Unemployed ferries:"));
	}

	columns_box->add(column1, UI::Box::Resizing::kExpandBoth);
	columns_box->add(column2, UI::Box::Resizing::kExpandBoth);
	main_box_.add(columns_box, UI::Box::Resizing::kFullSize);

	main_box_.add_space(kPadding * 2);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel,
	                               type_ == Type::kShip ? _("Desired number of transport ships:") :
                                                         _("Desired number of unemployed ferries:"),
	                               UI::Align::kCenter),
	              UI::Box::Resizing::kFullSize);

	main_box_.add_space(kPadding);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	if (can_act_) {
		spinbox_.changed.connect([this]() { set_target(spinbox_.get_value()); });

		infinite_target_.sigclicked.connect([this]() {
			bool was_pressed = infinite_target_.style() == UI::Button::VisualState::kPermpressed;
			infinite_target_.set_perm_pressed(!was_pressed);
			set_target(was_pressed ? previous_target_ : Widelands::kEconomyTargetInfinity);
		});

		const Widelands::Quantity current_target = get_current_target();
		const bool infinite = current_target == Widelands::kEconomyTargetInfinity;
		previous_target_ = infinite ? 0 : current_target;
		infinite_target_.set_perm_pressed(infinite);
	} else {
		infinite_target_.set_enabled(false);
	}

	set_center_panel(&main_box_);
	think();
	initialization_complete();
}

void FleetOptionsWindow::set_target(Widelands::Quantity target) {
	if (is_updating_ || !can_act_) {
		return;
	}

	MutexLock m(MutexLock::ID::kObjects);
	const Widelands::Bob* bob = interface_.get(ibase_.egbase());
	if (bob == nullptr) {
		return;
	}

	if (target != Widelands::kEconomyTargetInfinity) {
		previous_target_ = target;
	}

	if (ibase_.egbase().is_game()) {
		ibase_.game().send_player_fleet_targets(
		   bob->owner().player_number(), interface_.serial(), target);
	} else {
		if (type_ == Type::kShip) {
			dynamic_cast<const Widelands::ShipFleetYardInterface*>(bob)->get_fleet()->set_ships_target(
			   ibase_.egbase(), target);
		} else {
			dynamic_cast<const Widelands::FerryFleetYardInterface*>(bob)
			   ->get_fleet()
			   ->set_idle_ferries_target(ibase_.egbase(), target);
		}
	}
}

Widelands::Quantity FleetOptionsWindow::get_current_target() const {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Bob* bob = interface_.get(ibase_.egbase());
	if (bob == nullptr) {
		return 0;
	}

	if (type_ == Type::kShip) {
		return dynamic_cast<const Widelands::ShipFleetYardInterface*>(bob)
		   ->get_fleet()
		   ->get_ships_target();
	}
	return dynamic_cast<const Widelands::FerryFleetYardInterface*>(bob)
	   ->get_fleet()
	   ->get_idle_ferries_target();
}

void FleetOptionsWindow::think() {
	UI::Window::think();

	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Bob* bob = interface_.get(ibase_.egbase());
	if (bob == nullptr) {
		return die();
	}

	is_updating_ = true;

	const Widelands::Quantity current_target = get_current_target();
	const bool infinite = current_target == Widelands::kEconomyTargetInfinity;

	if (type_ == Type::kShip) {
		const Widelands::ShipFleet* fleet =
		   dynamic_cast<const Widelands::ShipFleetYardInterface*>(bob)->get_fleet();

		txt_ships_->set_text(as_string(fleet->count_ships()));
		txt_ports_->set_text(as_string(fleet->count_ports()));

	} else {
		const Widelands::FerryFleet* fleet =
		   dynamic_cast<const Widelands::FerryFleetYardInterface*>(bob)->get_fleet();

		txt_ferries_total_->set_text(as_string(fleet->count_ferries()));
		txt_ferries_unemployed_->set_text(as_string(fleet->count_unemployed_ferries()));
		txt_waterways_lacking_->set_text(as_string(fleet->count_unattended_waterways()));
		txt_waterways_total_->set_text(as_string(fleet->count_total_waterways()));
	}

	if (can_act_) {
		if (infinite) {
			spinbox_.set_interval(previous_target_, previous_target_);
			spinbox_.set_value(previous_target_);
		} else {
			spinbox_.set_interval(0, kMaxTarget);
			spinbox_.set_value(current_target);
		}
	} else {
		infinite_target_.set_perm_pressed(infinite);
		Widelands::Quantity show = infinite ? previous_target_ : current_target;
		spinbox_.set_interval(show, show);
		spinbox_.set_value(show);
	}
	infinite_target_.set_pic(g_image_cache->get(infinite ? kIconEndInfinity : kIconInfinity));

	is_updating_ = false;
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window&
FleetOptionsWindow::load(FileRead& fr, InteractiveBase& ib, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			return create(&ib, ib, &mol.get<Widelands::Bob>(fr.unsigned_32()));
		}
		throw Widelands::UnhandledVersionError(
		   "Fleet Options Window", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("fleet options window: %s", e.what());
	}
}

void FleetOptionsWindow::save(FileWrite& fw, Widelands::MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_32(mos.get_object_file_index(*interface_.get(ibase_.egbase())));
}
