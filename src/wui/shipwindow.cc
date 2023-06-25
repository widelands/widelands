/*
 * Copyright (C) 2011-2023 by the Widelands Development Team
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

#include "wui/shipwindow.h"

#include "base/macros.h"
#include "economy/portdock.h"
#include "economy/ware_instance.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "ui_basic/box.h"
#include "ui_basic/messagebox.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"

namespace {
constexpr const char* const kImgGoTo = "images/wui/ship/menu_ship_goto.png";
constexpr const char* const kImgDestination = "images/wui/ship/menu_ship_destination.png";
constexpr const char* const kImgSink = "images/wui/ship/menu_ship_sink.png";
constexpr const char* const kImgDebug = "images/wui/fieldaction/menu_debug.png";
constexpr const char* const kImgCancelExpedition =
   "images/wui/ship/menu_ship_cancel_expedition.png";
constexpr const char* const kImgExploreCW = "images/wui/ship/ship_explore_island_cw.png";
constexpr const char* const kImgExploreCCW = "images/wui/ship/ship_explore_island_ccw.png";
constexpr const char* const kImgScoutNW = "images/wui/ship/ship_scout_nw.png";
constexpr const char* const kImgScoutNE = "images/wui/ship/ship_scout_ne.png";
constexpr const char* const kImgScoutW = "images/wui/ship/ship_scout_w.png";
constexpr const char* const kImgScoutE = "images/wui/ship/ship_scout_e.png";
constexpr const char* const kImgScoutSW = "images/wui/ship/ship_scout_sw.png";
constexpr const char* const kImgScoutSE = "images/wui/ship/ship_scout_se.png";
constexpr const char* const kImgConstructPort = "images/wui/ship/ship_construct_port_space.png";

constexpr int kPadding = 5;
}  // namespace

ShipWindow::ShipWindow(InteractiveBase& ib, UniqueWindow::Registry& reg, Widelands::Ship* ship)
   : UniqueWindow(&ib, UI::WindowStyle::kWui, "shipwindow", &reg, 0, 0, ship->get_shipname()),
     ibase_(ib),
     ship_(ship),
     vbox_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     navigation_box_(&vbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical) {
	vbox_.set_inner_spacing(kPadding);
	assert(ship->get_owner());

	if (ibase_.can_act(ship->owner().player_number())) {
		name_field_ = new UI::EditBox(&vbox_, 0, 0, 0, UI::PanelStyle::kWui);
		name_field_->set_text(ship->get_shipname());
		name_field_->changed.connect([this]() { act_rename(); });
		vbox_.add(name_field_, UI::Box::Resizing::kFullSize);
	} else {
		name_field_ = nullptr;
	}

	display_ = new ItemWaresDisplay(&vbox_, ship->owner());
	display_->set_capacity(ship->get_capacity());
	vbox_.add(display_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	// Expedition buttons
	UI::Box* exp_top =
	   new UI::Box(&navigation_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_top, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	UI::Box* exp_mid =
	   new UI::Box(&navigation_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_mid, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	UI::Box* exp_bot =
	   new UI::Box(&navigation_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_bot, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	btn_scout_[Widelands::WALK_NW - 1] =
	   make_button(exp_top, "scnw", _("Scout towards the north west"), kImgScoutNW,
	               [this]() { act_scout_towards(Widelands::WALK_NW); });
	exp_top->add(btn_scout_[Widelands::WALK_NW - 1]);

	btn_explore_island_cw_ =
	   make_button(exp_top, "expcw", _("Explore the island’s coast clockwise"), kImgExploreCW,
	               [this]() { act_explore_island(Widelands::IslandExploreDirection::kClockwise); });
	exp_top->add(btn_explore_island_cw_);

	btn_scout_[Widelands::WALK_NE - 1] =
	   make_button(exp_top, "scne", _("Scout towards the north east"), kImgScoutNE,
	               [this]() { act_scout_towards(Widelands::WALK_NE); });
	exp_top->add(btn_scout_[Widelands::WALK_NE - 1]);

	btn_scout_[Widelands::WALK_W - 1] =
	   make_button(exp_mid, "scw", _("Scout towards the west"), kImgScoutW,
	               [this]() { act_scout_towards(Widelands::WALK_W); });
	exp_mid->add(btn_scout_[Widelands::WALK_W - 1]);

	btn_construct_port_ =
	   make_button(exp_mid, "buildport", _("Construct a port at the current location"),
	               kImgConstructPort, [this]() { act_construct_port(); });
	exp_mid->add(btn_construct_port_);

	btn_scout_[Widelands::WALK_E - 1] =
	   make_button(exp_mid, "sce", _("Scout towards the east"), kImgScoutE,
	               [this]() { act_scout_towards(Widelands::WALK_E); });
	exp_mid->add(btn_scout_[Widelands::WALK_E - 1]);

	btn_scout_[Widelands::WALK_SW - 1] =
	   make_button(exp_bot, "scsw", _("Scout towards the south west"), kImgScoutSW,
	               [this]() { act_scout_towards(Widelands::WALK_SW); });
	exp_bot->add(btn_scout_[Widelands::WALK_SW - 1]);

	btn_explore_island_ccw_ = make_button(
	   exp_bot, "expccw", _("Explore the island’s coast counter clockwise"), kImgExploreCCW,
	   [this]() { act_explore_island(Widelands::IslandExploreDirection::kCounterClockwise); });
	exp_bot->add(btn_explore_island_ccw_);

	btn_scout_[Widelands::WALK_SE - 1] =
	   make_button(exp_bot, "scse", _("Scout towards the south east"), kImgScoutSE,
	               [this]() { act_scout_towards(Widelands::WALK_SE); });
	exp_bot->add(btn_scout_[Widelands::WALK_SE - 1]);

	vbox_.add(&navigation_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	// Bottom buttons
	UI::Box* buttons = new UI::Box(&vbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	vbox_.add(buttons, UI::Box::Resizing::kFullSize);

	btn_sink_ = make_button(buttons, "sink", _("Sink the ship"), kImgSink, [this]() { act_sink(); });
	buttons->add(btn_sink_);

	btn_cancel_expedition_ =
	   make_button(buttons, "cancel_expedition", _("Cancel the Expedition"), kImgCancelExpedition,
	               [this]() { act_cancel_expedition(); });
	buttons->add(btn_cancel_expedition_);

	buttons->add_inf_space();

	if (ibase_.get_display_flag(InteractiveBase::dfDebug)) {
		btn_debug_ = make_button(
		   buttons, "debug", _("Show Debug Window"), kImgDebug, [this]() { act_debug(); });
		btn_debug_->set_enabled(true);
		buttons->add(btn_debug_);
	}

	btn_destination_ =
	   make_button(buttons, "destination", "", kImgDestination, [this]() { act_destination(); });
	buttons->add(btn_destination_);

	btn_goto_ = make_button(buttons, "goto", _("Go to ship"), kImgGoTo, [this]() { act_goto(); });
	buttons->add(btn_goto_);

	set_center_panel(&vbox_);
	set_thinks(true);

	shipnotes_subscriber_ =
	   Notifications::subscribe<Widelands::NoteShip>([this](const Widelands::NoteShip& note) {
		   if (note.ship->serial() == ship_.serial()) {
			   switch (note.action) {
				// Unable to cancel the expedition
			   case Widelands::NoteShip::Action::kNoPortLeft:
				   no_port_error_message();
				   break;
				// The ship is no more
			   case Widelands::NoteShip::Action::kLost:
				   // Stop this from thinking to avoid segfaults
				   set_thinks(false);
				   die();
				   break;
				// If the ship state has changed, e.g. expedition started or scouting direction changed,
				// think() will take care of it.
			   case Widelands::NoteShip::Action::kDestinationChanged:
			   case Widelands::NoteShip::Action::kWaitingForCommand:
			   case Widelands::NoteShip::Action::kGained:
				   break;
			   }
		   }
	   });

	// Init button visibility
	navigation_box_.set_visible(false);
	btn_cancel_expedition_->set_enabled(false);
	update_destination_button(ship);
	set_button_visibility();

	move_out_of_the_way();
	set_fastclick_panel(btn_goto_);
	warp_mouse_to_fastclick_panel();

	initialization_complete();
}

void ShipWindow::set_button_visibility() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}

	if (navigation_box_.is_visible() != ship->state_is_expedition()) {
		navigation_box_.set_visible(ship->state_is_expedition());
		layout();
	}
	if (btn_cancel_expedition_->is_visible() != btn_cancel_expedition_->enabled()) {
		btn_cancel_expedition_->set_visible(btn_cancel_expedition_->enabled());
	}
}

void ShipWindow::no_port_error_message() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (ibase_.can_act(ship->owner().player_number())) {
		UI::WLMessageBox messagebox(
		   get_parent(), UI::WindowStyle::kWui,
		   /** TRANSLATORS: Window label when an expedition can't be canceled */
		   _("Cancel Expedition"),
		   _("This expedition can’t be canceled, because the "
		     "ship has no port to return to."),
		   UI::WLMessageBox::MBoxType::kOk);
		messagebox.run<UI::Panel::Returncodes>();
	}
}

void ShipWindow::update_destination_button(const Widelands::Ship* ship) {
	btn_destination_->set_enabled(ship->get_destination() != nullptr);
	btn_destination_->set_tooltip(
	   ship->get_destination() == nullptr ?
         _("Go to destination") :
         /** TRANSLATORS: Placeholder is the name of the destination port */
         format(_("Go to destination (%s)"),
	             ship->get_destination()->get_warehouse()->get_warehouse_name()));
}

void ShipWindow::think() {
	UI::Window::think();
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}

	set_title(ship->get_shipname());
	const bool can_act = ibase_.can_act(ship->owner().player_number());

	update_destination_button(ship);
	btn_sink_->set_enabled(can_act);

	display_->clear();
	for (uint32_t idx = 0; idx < ship->get_nritems(); ++idx) {
		Widelands::ShippingItem item = ship->get_item(idx);
		Widelands::WareInstance* ware;
		Widelands::Worker* worker;
		item.get(ibase_.egbase(), &ware, &worker);

		if (ware != nullptr) {
			display_->add(Widelands::wwWARE, ware->descr_index());
		}
		if (worker != nullptr) {
			display_->add(Widelands::wwWORKER, worker->descr().worker_index());
		}
	}

	Widelands::Ship::ShipStates state = ship->get_ship_state();
	if (ship->state_is_expedition()) {
		/* The following rules apply:
		 * - The "construct port" button is only active, if the ship is waiting for commands and found
		 * a port
		 *   buildspace
		 * - The "scout towards a direction" buttons are only active, if the ship can move at least
		 * one field
		 *   in that direction without reaching the coast.
		 * - The "explore island's coast" buttons are only active, if a coast is in vision range (no
		 * matter if
		 *   in waiting or already expedition/scouting mode)
		 */
		btn_construct_port_->set_enabled(
		   can_act && (state == Widelands::Ship::ShipStates::kExpeditionPortspaceFound));
		bool coast_nearby = false;
		for (Widelands::Direction dir = 1; dir <= Widelands::LAST_DIRECTION; ++dir) {
			// NOTE buttons are saved in the format DIRECTION - 1
			btn_scout_[dir - 1]->set_enabled(
			   can_act && ship->exp_dir_swimmable(dir) &&
			   (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
			coast_nearby |= !ship->exp_dir_swimmable(dir);
		}
		btn_explore_island_cw_->set_enabled(
		   can_act && coast_nearby && (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
		btn_explore_island_ccw_->set_enabled(
		   can_act && coast_nearby && (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
		btn_sink_->set_enabled(can_act &&
		                       (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
	}
	btn_cancel_expedition_->set_enabled(
	   ship->state_is_expedition() && can_act &&
	   (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
	// Expedition specific buttons
	set_button_visibility();
}

UI::Button* ShipWindow::make_button(UI::Panel* parent,
                                    const std::string& name,
                                    const std::string& title,
                                    const std::string& picname,
                                    const std::function<void()>& callback) {
	UI::Button* btn = new UI::Button(
	   parent, name, 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, g_image_cache->get(picname), title);
	btn->sigclicked.connect(callback);
	return btn;
}

void ShipWindow::act_rename() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_ship_port_name(
		   ship->owner().player_number(), ship->serial(), name_field_->get_text());
	} else {
		ship->set_shipname(name_field_->get_text());
	}
}

/// Move the main view towards the current ship location
void ShipWindow::act_goto() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	ibase_.map_view()->scroll_to_field(ship->get_position(), MapView::Transition::Smooth);
}

/// Move the main view towards the current destination of the ship
void ShipWindow::act_destination() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (Widelands::PortDock* destination = ship->get_destination()) {
		ibase_.map_view()->scroll_to_field(
		   destination->get_warehouse()->get_position(), MapView::Transition::Smooth);
	}
}

/// Sink the ship if confirmed
void ShipWindow::act_sink() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		if (Widelands::Game* game = ibase_.get_game()) {
			game->send_player_sink_ship(*ship);
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	} else if (upcast(InteractivePlayer, ipl, &ibase_)) {
		show_ship_sink_confirm(*ipl, *ship);
	}
}

/// Show debug info
void ShipWindow::act_debug() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	show_mapobject_debug(ibase_, *ship);
}

/// Cancel expedition if confirmed
void ShipWindow::act_cancel_expedition() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		if (Widelands::Game* game = ibase_.get_game()) {
			game->send_player_cancel_expedition_ship(*ship);
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	} else if (upcast(InteractivePlayer, ipl, &ibase_)) {
		show_ship_cancel_expedition_confirm(*ipl, *ship);
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(Widelands::WalkingDir direction) {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	// ignore request if the direction is not swimmable at all
	if (!ship->exp_dir_swimmable(static_cast<Widelands::Direction>(direction))) {
		return;
	}
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_ship_scouting_direction(*ship, direction);
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

/// Constructs a port at the port build space in vision range
void ShipWindow::act_construct_port() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (ship->exp_port_spaces().empty()) {
		return;
	}
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_ship_construct_port(*ship, ship->exp_port_spaces().front());
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(Widelands::IslandExploreDirection direction) {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	bool coast_nearby = false;
	bool moveable = false;
	for (Widelands::Direction dir = 1;
	     (dir <= Widelands::LAST_DIRECTION) && (!coast_nearby || !moveable); ++dir) {
		if (!ship->exp_dir_swimmable(dir)) {
			coast_nearby = true;
		} else {
			moveable = true;
		}
	}
	if (!coast_nearby || !moveable) {
		return;
	}
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_ship_explore_island(*ship, direction);
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& ShipWindow::load(FileRead& fr, InteractiveBase& ib, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			return ib.show_ship_window(&mol.get<Widelands::Ship>(fr.unsigned_32()));
		}
		throw Widelands::UnhandledVersionError("Ship Window", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("ship window: %s", e.what());
	}
}
void ShipWindow::save(FileWrite& fw, Widelands::MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_32(mos.get_object_file_index(*ship_.get(ibase_.egbase())));
}
