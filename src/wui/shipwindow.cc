/*
 * Copyright (C) 2011-2020 by the Widelands Development Team
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

#include <memory>

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
static const char pic_editorcfg[] = "images/wui/buildings/menu_tab_wares_dock.png";
static const char pic_cancel_expedition[] = "images/wui/ship/menu_ship_cancel_expedition.png";
static const char pic_explore_cw[] = "images/wui/ship/ship_explore_island_cw.png";
static const char pic_explore_ccw[] = "images/wui/ship/ship_explore_island_ccw.png";
static const char pic_scout_nw[] = "images/wui/ship/ship_scout_nw.png";
static const char pic_scout_ne[] = "images/wui/ship/ship_scout_ne.png";
static const char pic_scout_w[] = "images/wui/ship/ship_scout_w.png";
static const char pic_scout_e[] = "images/wui/ship/ship_scout_e.png";
static const char pic_scout_sw[] = "images/wui/ship/ship_scout_sw.png";
static const char pic_scout_se[] = "images/wui/ship/ship_scout_se.png";
static const char pic_construct_port[] = "images/wui/ship/ship_construct_port_space.png";

constexpr int kPadding = 5;
}  // namespace

constexpr uint16_t kShipCfgIconSize = 32;
constexpr uint16_t kShipCfgMaxColumns = 10;
ShipCfg::ShipCfg(InteractiveBase& ib, Widelands::Ship& s)
   : UI::Window(&ib, "shipcfg", 0, 0, 240, 200, _("Configure Ship")),
     ibase_(ib),
     ship_(&s),
     main_box_(this, 0, 0, UI::Box::Vertical),
     shipname_(&main_box_, 0, 0, 200, UI::PanelStyle::kWui),
     capacity_(&main_box_,
               0,
               0,
               200,
               100,
               s.get_capacity(),
               1,
               Widelands::INVALID_INDEX,
               UI::PanelStyle::kWui,
               _("Capacity"),
               UI::SpinBox::Units::kNone,
               UI::SpinBox::Type::kBig),
     ok_(&main_box_,
         "ok",
         0,
         0,
         200,
         30,
         UI::ButtonStyle::kWuiPrimary,
         _("OK"),
         _("Close and apply changes")),
     cancel_(&main_box_,
             "cancel",
             0,
             0,
             200,
             30,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel"),
             _("Close and discard changes")) {
	shipname_.set_text(s.get_shipname());

	{
		const uint32_t capacity = s.get_capacity();
		const uint32_t carried_items = s.get_nritems();
		uint32_t rows = capacity / kShipCfgMaxColumns;
		if (kShipCfgMaxColumns * rows < capacity) {
			++rows;
		}
		assert(kShipCfgMaxColumns * rows >= capacity);

		row_boxes_.resize(rows);
		for (uint32_t i = 0; i < rows; ++i) {
			row_boxes_[i].reset(new UI::Box(&main_box_, 0, 0, UI::Box::Horizontal));
			main_box_.add(row_boxes_[i].get());
		}

		uint32_t rows_index = 0;
		for (uint32_t i = 0; i < capacity; ++i) {
			UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>* dd =
			   new UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>(
			      row_boxes_[rows_index].get(), "wareworker_" + std::to_string(i), 0, 0,
			      kShipCfgIconSize, 8, kShipCfgIconSize, _("Ware/Worker"),
			      UI::DropdownType::kPictorial, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
			Widelands::WareInstance* shipping_ware = nullptr;
			Widelands::Worker* shipping_worker = nullptr;
			if (i < carried_items) {
				s.get_item(i).get(ib.egbase(), &shipping_ware, &shipping_worker);
			}
			dd->add(_("(Empty)"), std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX),
			        g_gr->images().get("images/wui/editor/no_ware.png"),
			        !shipping_ware && !shipping_worker, _("Empty slot"));
			for (Widelands::DescriptionIndex di : s.owner().tribe().wares()) {
				const Widelands::WareDescr& w = *s.owner().tribe().get_ware_descr(di);
				dd->add(w.descname(), std::make_pair(Widelands::wwWARE, di), w.icon(),
				        shipping_ware && shipping_ware->descr_index() == di, w.descname());
			}
			for (Widelands::DescriptionIndex di : s.owner().tribe().workers()) {
				const Widelands::WorkerDescr& w = *s.owner().tribe().get_worker_descr(di);
				dd->add(w.descname(), std::make_pair(Widelands::wwWORKER, di), w.icon(),
				        shipping_worker && shipping_worker->descr().worker_index() == di, w.descname());
			}

			row_boxes_[rows_index]->add(dd);
			items_.push_back(
			   std::unique_ptr<
			      UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>>(dd));
			rows_index = (rows_index + 1) % rows;
		}
	}

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	cancel_.sigclicked.connect([this]() { die(); });

	main_box_.add(&capacity_, UI::Box::Resizing::kFullSize);
	main_box_.add(&shipname_, UI::Box::Resizing::kFullSize);
	main_box_.add(&cancel_, UI::Box::Resizing::kFullSize);
	main_box_.add(&ok_, UI::Box::Resizing::kFullSize);
	set_center_panel(&main_box_);
	center_to_parent();
}
const Widelands::Ship* ShipCfg::ship() const {
	return ship_.get(ibase_.egbase());
}
Widelands::Ship* ShipCfg::ship() {
	return ship_.get(ibase_.egbase());
}
void ShipCfg::think() {
	if (!ship()) {
		die();
	}
	UI::Window::think();
}
void ShipCfg::clicked_ok() {
	if (Widelands::Ship* s = ship()) {
		Widelands::EditorGameBase& egbase = ibase_.egbase();
		if (!shipname_.text().empty()) {
			s->set_shipname(shipname_.text());
		}
		while (!s->items_.empty()) {
			s->items_.begin()->remove(egbase);
			s->items_.erase(s->items_.begin());
		}
		uint32_t capacity = capacity_.get_value();
		s->set_capacity(capacity);
		for (const auto& dd : items_) {
			if (!capacity) {
				break;
			}
			if (dd && dd->get_selected().second != Widelands::INVALID_INDEX) {
				if (dd->get_selected().first == Widelands::wwWARE) {
					Widelands::WareInstance& w = *new Widelands::WareInstance(
					   dd->get_selected().second,
					   egbase.tribes().get_ware_descr(dd->get_selected().second));
					w.init(egbase);
					w.set_location(egbase, s);
					s->items_.push_back(Widelands::ShippingItem(w));
				} else {
					Widelands::Worker& w =
					   egbase.tribes()
					      .get_worker_descr(dd->get_selected().second)
					      ->create(egbase, s->get_owner(), nullptr, s->get_position());
					s->items_.push_back(Widelands::ShippingItem(w));
				}
				--capacity;
			}
		}
	}
	die();
}

ShipWindow::ShipWindow(InteractiveBase& ib, UniqueWindow::Registry& reg, Widelands::Ship* ship)
   : UniqueWindow(&ib, "shipwindow", &reg, 0, 0, ship->get_shipname()),
     ibase_(ib),
     ship_(ship),
     vbox_(this, 0, 0, UI::Box::Vertical),
     navigation_box_(&vbox_, 0, 0, UI::Box::Vertical),
     navigation_box_height_(0) {
	vbox_.set_inner_spacing(kPadding);
	assert(ship->get_owner());

	display_ = new ItemWaresDisplay(&vbox_, ship->owner());
	display_->set_capacity(ship->get_capacity());
	vbox_.add(display_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	// Expedition buttons
	UI::Box* exp_top = new UI::Box(&navigation_box_, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_top, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	UI::Box* exp_mid = new UI::Box(&navigation_box_, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_mid, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	UI::Box* exp_bot = new UI::Box(&navigation_box_, 0, 0, UI::Box::Horizontal);
	navigation_box_.add(exp_bot, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	btn_scout_[Widelands::WALK_NW - 1] =
	   make_button(exp_top, "scnw", _("Scout towards the north west"), pic_scout_nw,
	               [this]() { act_scout_towards(Widelands::WALK_NW); });
	exp_top->add(btn_scout_[Widelands::WALK_NW - 1]);

	btn_explore_island_cw_ =
	   make_button(exp_top, "expcw", _("Explore the island’s coast clockwise"), pic_explore_cw,
	               [this]() { act_explore_island(Widelands::IslandExploreDirection::kClockwise); });
	exp_top->add(btn_explore_island_cw_);

	btn_scout_[Widelands::WALK_NE - 1] =
	   make_button(exp_top, "scne", _("Scout towards the north east"), pic_scout_ne,
	               [this]() { act_scout_towards(Widelands::WALK_NE); });
	exp_top->add(btn_scout_[Widelands::WALK_NE - 1]);

	btn_scout_[Widelands::WALK_W - 1] =
	   make_button(exp_mid, "scw", _("Scout towards the west"), pic_scout_w,
	               [this]() { act_scout_towards(Widelands::WALK_W); });
	exp_mid->add(btn_scout_[Widelands::WALK_W - 1]);

	btn_construct_port_ =
	   make_button(exp_mid, "buildport", _("Construct a port at the current location"),
	               pic_construct_port, [this]() { act_construct_port(); });
	exp_mid->add(btn_construct_port_);

	btn_scout_[Widelands::WALK_E - 1] =
	   make_button(exp_mid, "sce", _("Scout towards the east"), pic_scout_e,
	               [this]() { act_scout_towards(Widelands::WALK_E); });
	exp_mid->add(btn_scout_[Widelands::WALK_E - 1]);

	btn_scout_[Widelands::WALK_SW - 1] =
	   make_button(exp_bot, "scsw", _("Scout towards the south west"), pic_scout_sw,
	               [this]() { act_scout_towards(Widelands::WALK_SW); });
	exp_bot->add(btn_scout_[Widelands::WALK_SW - 1]);

	btn_explore_island_ccw_ = make_button(
	   exp_bot, "expccw", _("Explore the island’s coast counter clockwise"), pic_explore_ccw,
	   [this]() { act_explore_island(Widelands::IslandExploreDirection::kCounterClockwise); });
	exp_bot->add(btn_explore_island_ccw_);

	btn_scout_[Widelands::WALK_SE - 1] =
	   make_button(exp_bot, "scse", _("Scout towards the south east"), pic_scout_se,
	               [this]() { act_scout_towards(Widelands::WALK_SE); });
	exp_bot->add(btn_scout_[Widelands::WALK_SE - 1]);

	vbox_.add(&navigation_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	// Bottom buttons
	UI::Box* buttons = new UI::Box(&vbox_, 0, 0, UI::Box::Horizontal);
	vbox_.add(buttons, UI::Box::Resizing::kFullSize);

	if (ibase_.get_game()) {
		btn_sink_ =
		   make_button(buttons, "sink", _("Sink the ship"), pic_sink, [this]() { act_sink(); });
		buttons->add(btn_sink_);
	} else {
		// Use the Delete Workers tool to remove the ship
		btn_sink_ = nullptr;
	}

	btn_cancel_expedition_ =
	   make_button(buttons, "cancel_expedition", _("Cancel the Expedition"), pic_cancel_expedition,
	               [this]() { act_cancel_expedition(); });
	buttons->add(btn_cancel_expedition_);

	buttons->add_inf_space();

	if (ibase_.get_display_flag(InteractiveBase::dfDebug)) {
		btn_debug_ = make_button(
		   buttons, "debug", _("Show Debug Window"), pic_debug, [this]() { act_debug(); });
		btn_debug_->set_enabled(true);
		buttons->add(btn_debug_);
	} else {
		btn_debug_ = nullptr;
	}
	if (ibase_.omnipotent()) {
		btn_editorcfg_ =
		   make_button(buttons, "editorcfg", _("Configure wares and workers and rename the ship"),
		               pic_editorcfg, [this]() { act_editorcfg(); });
		btn_editorcfg_->set_enabled(true);
		buttons->add(btn_editorcfg_);
	} else {
		btn_editorcfg_ = nullptr;
	}

	if (ibase_.get_game()) {
		btn_destination_ = make_button(buttons, "destination", _("Go to destination"),
		                               pic_destination, [this]() { act_destination(); });
		btn_destination_->set_enabled(false);
		buttons->add(btn_destination_);
	} else {
		btn_destination_ = nullptr;
	}

	btn_goto_ = make_button(buttons, "goto", _("Go to ship"), pic_goto, [this]() { act_goto(); });
	buttons->add(btn_goto_);

	set_center_panel(&vbox_);
	set_thinks(true);
	set_fastclick_panel(btn_goto_);
	move_out_of_the_way();
	warp_mouse_to_fastclick_panel();

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
	navigation_box_height_ = navigation_box_.get_h();
	navigation_box_.set_visible(false);
	navigation_box_.set_desired_size(navigation_box_.get_w(), 0);
	btn_cancel_expedition_->set_enabled(false);
	think();
}

void ShipWindow::set_button_visibility() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (navigation_box_.is_visible() != ship->state_is_expedition()) {
		navigation_box_.set_visible(ship->state_is_expedition());
		navigation_box_.set_desired_size(
		   navigation_box_.get_w(), ship->state_is_expedition() ? navigation_box_height_ : 0);
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
	if (upcast(InteractiveBase, ibase, ship->get_owner()->egbase().get_ibase())) {
		if (ibase->can_act(ship->owner().player_number()) || ibase_.omnipotent()) {
			UI::WLMessageBox messagebox(
			   get_parent(),
			   /** TRANSLATORS: Window label when an expedition can't be canceled */
			   _("Cancel Expedition"),
			   _("This expedition can’t be canceled, because the "
			     "ship has no port to return to."),
			   UI::WLMessageBox::MBoxType::kOk);
			messagebox.run<UI::Panel::Returncodes>();
		}
	}
}

void ShipWindow::think() {
	UI::Window::think();
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	set_title(ship->get_shipname());
	bool can_act = ibase_.can_act(ship->owner().player_number()) || ibase_.omnipotent();

	if (btn_destination_) {
		btn_destination_->set_enabled(ship->get_destination());
	}
	if (btn_sink_) {
		btn_sink_->set_enabled(can_act);
	}

	display_->clear();
	for (uint32_t idx = 0; idx < ship->get_nritems(); ++idx) {
		Widelands::ShippingItem item = ship->get_item(idx);
		Widelands::WareInstance* ware;
		Widelands::Worker* worker;
		item.get(ibase_.egbase(), &ware, &worker);

		if (ware) {
			display_->add(false, ware->descr_index());
		}
		if (worker) {
			display_->add(true, worker->descr().worker_index());
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
		if (btn_sink_) {
			btn_sink_->set_enabled(can_act &&
			                       (state != Widelands::Ship::ShipStates::kExpeditionColonizing));
		}
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
	   parent, name, 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu, g_gr->images().get(picname), title);
	btn->sigclicked.connect(callback);
	return btn;
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
	if (SDL_GetModState() & KMOD_CTRL) {
		ibase_.game().send_player_sink_ship(*ship);
	} else {
		show_ship_sink_confirm(dynamic_cast<InteractivePlayer&>(ibase_), *ship);
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

void ShipWindow::act_editorcfg() {
	if (Widelands::Ship* ship = ship_.get(ibase_.egbase())) {
		new ShipCfg(ibase_, *ship);
	}
}

/// Cancel expedition if confirmed
void ShipWindow::act_cancel_expedition() {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	if (SDL_GetModState() & KMOD_CTRL) {
		ibase_.game().send_player_cancel_expedition_ship(*ship);
	} else {
		show_ship_cancel_expedition_confirm(dynamic_cast<InteractivePlayer&>(ibase_), *ship);
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(const Widelands::WalkingDir direction) {
	Widelands::Ship* ship = ship_.get(ibase_.egbase());
	if (ship == nullptr) {
		return;
	}
	// ignore request if the direction is not swimmable at all
	if (!ship->exp_dir_swimmable(static_cast<Widelands::Direction>(direction))) {
		return;
	}
	ibase_.game().send_player_ship_scouting_direction(*ship, direction);
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
	ibase_.game().send_player_ship_construct_port(*ship, ship->exp_port_spaces().front());
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(const Widelands::IslandExploreDirection direction) {
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
	ibase_.game().send_player_ship_explore_island(*ship, direction);
}
