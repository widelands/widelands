/*
 * Copyright (C) 2011-2017 by the Widelands Development Team
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

#include "wui/shipwindow.h"

#include "base/macros.h"
#include "economy/portdock.h"
#include "economy/ware_instance.h"
#include "graphic/graphic.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/messagebox.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"

namespace {
static const char pic_goto[] = "images/wui/ship/menu_ship_goto.png";
static const char pic_destination[] = "images/wui/ship/menu_ship_destination.png";
static const char pic_sink[] = "images/wui/ship/menu_ship_sink.png";
static const char pic_debug[] = "images/wui/fieldaction/menu_debug.png";
static const char pic_cancel_expedition[] = "images/wui/ship/menu_ship_cancel_expedition.png";
static const char pic_explore_cw[] = "images/wui/ship/ship_explore_island_cw.png";
static const char pic_explore_ccw[] = "images/wui/ship/ship_explore_island_ccw.png";
static const char pic_scout_nw[] = "images/wui/ship/ship_scout_nw.png";
static const char pic_scout_ne[] = "images/wui/ship/ship_scout_ne.png";
static const char pic_scout_w[] = "images/wui/ship/ship_scout_w.png";
static const char pic_scout_e[] = "images/wui/ship/ship_scout_e.png";
static const char pic_scout_sw[] = "images/wui/ship/ship_scout_sw.png";
static const char pic_scout_se[] = "images/wui/ship/ship_scout_se.png";
static const char pic_construct_port[] = "images/wui/editor/fsel_editor_set_port_space.png";
}  // namespace

using namespace Widelands;

ShipWindow::ShipWindow(InteractiveGameBase& igb, UniqueWindow::Registry& reg, Ship& ship)
   : UniqueWindow(&igb, "shipwindow", &reg, 0, 0, ship.get_shipname()), igbase_(igb), ship_(ship) {
	init(false);
	shipnotes_subscriber_ = Notifications::subscribe<Widelands::NoteShipWindow>(
	   [this](const Widelands::NoteShipWindow& note) {
		   if (note.serial == ship_.serial()) {
			   switch (note.action) {
			   // Unable to cancel the expedition
			   case Widelands::NoteShipWindow::Action::kNoPortLeft:
				if (upcast(InteractiveGameBase, igamebase, ship_.get_owner()->egbase().get_ibase())) {
					if (igamebase->can_act(ship_.get_owner()->player_number())) {
						UI::WLMessageBox messagebox(
						   get_parent(),
						   /** TRANSLATORS: Window label when an expedition can't be canceled */
						   _("Cancel expedition"), _("This expedition can’t be canceled, because the "
						                             "ship has no port to return to."),
						   UI::WLMessageBox::MBoxType::kOk);
						messagebox.run<UI::Panel::Returncodes>();
					}
				}
				break;
			   // The ship state has changed, e.g. expedition canceled
			   case Widelands::NoteShipWindow::Action::kRefresh:
				   init(true);
				   break;
			   // The ship is no more
			   case Widelands::NoteShipWindow::Action::kClose:
				   // Stop this from thinking to avoid segfaults
				   set_thinks(false);
				   die();
				   break;
			   default:
				   break;
			   }
		   }
		});
}

void ShipWindow::init(bool avoid_fastclick) {
	assert(ship_.get_owner());

	vbox_.reset(new UI::Box(this, 0, 0, UI::Box::Vertical));

	display_ = new ItemWaresDisplay(vbox_.get(), *ship_.get_owner());
	display_->set_capacity(ship_.descr().get_capacity());
	vbox_->add(display_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	// Expedition buttons
	if (ship_.state_is_expedition()) {
		UI::Box* exp_top = new UI::Box(vbox_.get(), 0, 0, UI::Box::Horizontal);
		vbox_->add(exp_top, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		UI::Box* exp_mid = new UI::Box(vbox_.get(), 0, 0, UI::Box::Horizontal);
		vbox_->add(exp_mid, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		UI::Box* exp_bot = new UI::Box(vbox_.get(), 0, 0, UI::Box::Horizontal);
		vbox_->add(exp_bot, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		btn_scout_[WALK_NW - 1] =
		   make_button(exp_top, "scnw", _("Scout towards the north west"), pic_scout_nw,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_NW));
		exp_top->add(btn_scout_[WALK_NW - 1]);

		btn_explore_island_cw_ = make_button(
		   exp_top, "expcw", _("Explore the island’s coast clockwise"), pic_explore_cw,
		   boost::bind(&ShipWindow::act_explore_island, this, IslandExploreDirection::kClockwise));
		exp_top->add(btn_explore_island_cw_);

		btn_scout_[WALK_NE - 1] =
		   make_button(exp_top, "scne", _("Scout towards the north east"), pic_scout_ne,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_NE));
		exp_top->add(btn_scout_[WALK_NE - 1]);

		btn_scout_[WALK_W - 1] =
		   make_button(exp_mid, "scw", _("Scout towards the west"), pic_scout_w,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_W));
		exp_mid->add(btn_scout_[WALK_W - 1]);

		btn_construct_port_ =
		   make_button(exp_mid, "buildport", _("Construct a port at the current location"),
		               pic_construct_port, boost::bind(&ShipWindow::act_construct_port, this));
		exp_mid->add(btn_construct_port_);

		btn_scout_[WALK_E - 1] =
		   make_button(exp_mid, "sce", _("Scout towards the east"), pic_scout_e,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_E));
		exp_mid->add(btn_scout_[WALK_E - 1]);

		btn_scout_[WALK_SW - 1] =
		   make_button(exp_bot, "scsw", _("Scout towards the south west"), pic_scout_sw,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_SW));
		exp_bot->add(btn_scout_[WALK_SW - 1]);

		btn_explore_island_ccw_ =
		   make_button(exp_bot, "expccw", _("Explore the island’s coast counter clockwise"),
		               pic_explore_ccw, boost::bind(&ShipWindow::act_explore_island, this,
		                                            IslandExploreDirection::kCounterClockwise));
		exp_bot->add(btn_explore_island_ccw_);

		btn_scout_[WALK_SE - 1] =
		   make_button(exp_bot, "scse", _("Scout towards the south east"), pic_scout_se,
		               boost::bind(&ShipWindow::act_scout_towards, this, WALK_SE));
		exp_bot->add(btn_scout_[WALK_SE - 1]);
	}

	// Bottom buttons
	UI::Box* buttons = new UI::Box(vbox_.get(), 0, 0, UI::Box::Horizontal);
	vbox_->add(buttons);

	btn_goto_ = make_button(
	   buttons, "goto", _("Go to ship"), pic_goto, boost::bind(&ShipWindow::act_goto, this));
	buttons->add(btn_goto_);
	btn_destination_ = make_button(buttons, "destination", _("Go to destination"), pic_destination,
	                               boost::bind(&ShipWindow::act_destination, this));
	btn_destination_->set_enabled(false);
	buttons->add(btn_destination_);

	btn_sink_ = make_button(
	   buttons, "sink", _("Sink the ship"), pic_sink, boost::bind(&ShipWindow::act_sink, this));
	buttons->add(btn_sink_);

	if (ship_.state_is_expedition()) {
		btn_cancel_expedition_ =
		   make_button(buttons, "cancel_expedition", _("Cancel the Expedition"),
		               pic_cancel_expedition, boost::bind(&ShipWindow::act_cancel_expedition, this));
		buttons->add(btn_cancel_expedition_);
	}

	if (igbase_.get_display_flag(InteractiveBase::dfDebug)) {
		btn_debug_ = make_button(buttons, "debug", _("Show Debug Window"), pic_debug,
		                         boost::bind(&ShipWindow::act_debug, this));
		btn_debug_->set_enabled(true);
		buttons->add(btn_debug_);
	}
	set_center_panel(vbox_.get());
	set_thinks(true);
	set_fastclick_panel(btn_goto_);
	if (!avoid_fastclick) {
		move_out_of_the_way();
		warp_mouse_to_fastclick_panel();
	}
}

void ShipWindow::think() {
	UI::Window::think();
	InteractiveBase* ib = ship_.get_owner()->egbase().get_ibase();
	bool can_act = false;
	if (upcast(InteractiveGameBase, igb, ib))
		can_act = igb->can_act(ship_.get_owner()->player_number());

	btn_destination_->set_enabled(ship_.get_destination(igbase_.egbase()));
	btn_sink_->set_enabled(can_act);

	display_->clear();
	for (uint32_t idx = 0; idx < ship_.get_nritems(); ++idx) {
		Widelands::ShippingItem item = ship_.get_item(idx);
		Widelands::WareInstance* ware;
		Widelands::Worker* worker;
		item.get(igbase_.egbase(), &ware, &worker);

		if (ware) {
			display_->add(false, ware->descr_index());
		}
		if (worker) {
			display_->add(true, worker->descr().worker_index());
		}
	}

	// Expedition specific buttons
	Ship::ShipStates state = ship_.get_ship_state();
	if (ship_.state_is_expedition()) {
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
		btn_construct_port_->set_enabled(can_act &&
		                                 (state == Ship::ShipStates::kExpeditionPortspaceFound));
		bool coast_nearby = false;
		for (Direction dir = 1; dir <= LAST_DIRECTION; ++dir) {
			// NOTE buttons are saved in the format DIRECTION - 1
			btn_scout_[dir - 1]->set_enabled(can_act && ship_.exp_dir_swimmable(dir) &&
			                                 (state != Ship::ShipStates::kExpeditionColonizing));
			coast_nearby |= !ship_.exp_dir_swimmable(dir);
		}
		btn_explore_island_cw_->set_enabled(can_act && coast_nearby &&
		                                    (state != Ship::ShipStates::kExpeditionColonizing));
		btn_explore_island_ccw_->set_enabled(can_act && coast_nearby &&
		                                     (state != Ship::ShipStates::kExpeditionColonizing));
		btn_sink_->set_enabled(can_act && (state != Ship::ShipStates::kExpeditionColonizing));
		btn_cancel_expedition_->set_enabled(can_act &&
		                                    (state != Ship::ShipStates::kExpeditionColonizing));
	}
}

UI::Button* ShipWindow::make_button(UI::Panel* parent,
                                    const std::string& name,
                                    const std::string& title,
                                    const std::string& picname,
                                    boost::function<void()> callback) {
	UI::Button* btn =
	   new UI::Button(parent, name, 0, 0, 34, 34, g_gr->images().get("images/ui_basic/but4.png"),
	                  g_gr->images().get(picname), title);
	btn->sigclicked.connect(callback);
	return btn;
}

/// Move the main view towards the current ship location
void ShipWindow::act_goto() {
	igbase_.scroll_to_field(ship_.get_position(), MapView::Transition::Smooth);
}

/// Move the main view towards the current destination of the ship
void ShipWindow::act_destination() {
	if (PortDock* destination = ship_.get_destination(igbase_.egbase())) {
		igbase_.scroll_to_field(
		   destination->get_warehouse()->get_position(), MapView::Transition::Smooth);
	}
}

/// Sink the ship if confirmed
void ShipWindow::act_sink() {
	if (SDL_GetModState() & KMOD_CTRL) {
		igbase_.game().send_player_sink_ship(ship_);
	} else {
		show_ship_sink_confirm(dynamic_cast<InteractivePlayer&>(igbase_), ship_);
	}
}

/// Show debug info
void ShipWindow::act_debug() {
	show_mapobject_debug(igbase_, ship_);
}

/// Cancel expedition if confirmed
void ShipWindow::act_cancel_expedition() {
	if (SDL_GetModState() & KMOD_CTRL) {
		igbase_.game().send_player_cancel_expedition_ship(ship_);
	} else {
		show_ship_cancel_expedition_confirm(dynamic_cast<InteractivePlayer&>(igbase_), ship_);
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(WalkingDir direction) {
	// ignore request if the direction is not swimmable at all
	if (!ship_.exp_dir_swimmable(static_cast<Direction>(direction)))
		return;
	igbase_.game().send_player_ship_scouting_direction(ship_, direction);
}

/// Constructs a port at the port build space in vision range
void ShipWindow::act_construct_port() {
	if (ship_.exp_port_spaces().empty())
		return;
	igbase_.game().send_player_ship_construct_port(ship_, ship_.exp_port_spaces().front());
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(IslandExploreDirection direction) {
	bool coast_nearby = false;
	bool moveable = false;
	for (Direction dir = 1; (dir <= LAST_DIRECTION) && (!coast_nearby || !moveable); ++dir) {
		if (!ship_.exp_dir_swimmable(dir))
			coast_nearby = true;
		else
			moveable = true;
	}
	if (!coast_nearby || !moveable)
		return;
	igbase_.game().send_player_ship_explore_island(ship_, direction);
}
