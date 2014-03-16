/*
 * Copyright (C) 2011, 2013 by the Widelands Development Team
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

#include "logic/ship.h"

#include "economy/portdock.h"
#include "economy/ware_instance.h"
#include "graphic/graphic.h"
#include "logic/player.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "ui_basic/box.h"
#include "upcast.h"
#include "wui/actionconfirm.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"
#include "wui/itemwaresdisplay.h"

static const char pic_goto[] = "pics/menu_ship_goto.png";
static const char pic_destination[] = "pics/menu_ship_destination.png";
static const char pic_sink[]     = "pics/menu_ship_sink.png";
static const char pic_cancel_expedition[] = "pics/menu_ship_cancel_expedition.png";
static const char pic_explore_cw[]  = "pics/ship_explore_island_cw.png";
static const char pic_explore_ccw[] = "pics/ship_explore_island_ccw.png";
static const char pic_scout_nw[] = "pics/ship_scout_nw.png";
static const char pic_scout_ne[] = "pics/ship_scout_ne.png";
static const char pic_scout_w[]  = "pics/ship_scout_w.png";
static const char pic_scout_e[]  = "pics/ship_scout_e.png";
static const char pic_scout_sw[] = "pics/ship_scout_sw.png";
static const char pic_scout_se[] = "pics/ship_scout_se.png";
static const char pic_construct_port[] = "pics/fsel_editor_set_port_space.png";

namespace Widelands {

/**
 * Display information about a ship.
 */
struct ShipWindow : UI::Window {
	ShipWindow(Interactive_GameBase & igb, Ship & ship);
	virtual ~ShipWindow();

	virtual void think() override;

	UI::Button * make_button
		(UI::Panel * parent,
		 const std::string & name,
		 const std::string & title,
		 const std::string & picname,
		 boost::function<void()> callback);

	void act_goto();
	void act_destination();
	void act_sink();
	void act_cancel_expedition();
	void act_scout_towards(uint8_t);
	void act_construct_port();
	void act_explore_island(bool);

private:
	Interactive_GameBase & m_igbase;
	Ship & m_ship;

	UI::Button * m_btn_goto;
	UI::Button * m_btn_destination;
	UI::Button * m_btn_sink;
	UI::Button * m_btn_cancel_expedition;
	UI::Button * m_btn_explore_island_cw;
	UI::Button * m_btn_explore_island_ccw;
	UI::Button * m_btn_scout[LAST_DIRECTION]; // format: DIRECTION - 1, as 0 is normally the current location.
	UI::Button * m_btn_construct_port;
	ItemWaresDisplay * m_display;
};

ShipWindow::ShipWindow(Interactive_GameBase & igb, Ship & ship) :
	Window(&igb, "shipwindow", 0, 0, 0, 0, _("Ship")),
	m_igbase(igb),
	m_ship(ship)
{
	assert(!m_ship.m_window);
	assert(m_ship.get_owner());
	m_ship.m_window = this;

	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	m_display = new ItemWaresDisplay(vbox, *ship.get_owner());
	m_display->set_capacity(ship.get_capacity());
	vbox->add(m_display, UI::Box::AlignCenter, false);

	// Expedition buttons
	if (m_ship.state_is_expedition()) {
		UI::Box * exp_top = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_top, UI::Box::AlignCenter, false);
		UI::Box * exp_mid = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_mid, UI::Box::AlignCenter, false);
		UI::Box * exp_bot = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
		vbox->add(exp_bot, UI::Box::AlignCenter, false);

		m_btn_scout[WALK_NW - 1] =
			make_button
				(exp_top, "scnw", _("Scout towards the north west"), pic_scout_nw,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_NW));
		exp_top->add(m_btn_scout[WALK_NW - 1], 0, false);

		m_btn_explore_island_cw =
			make_button
				(exp_top, "expcw", _("Explore the island’s coast clockwise"), pic_explore_cw,
				 boost::bind(&ShipWindow::act_explore_island, this, true));
		exp_top->add(m_btn_explore_island_cw, 0, false);

		m_btn_scout[WALK_NE - 1] =
			make_button
				(exp_top, "scne", _("Scout towards the north east"), pic_scout_ne,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_NE));
		exp_top->add(m_btn_scout[WALK_NE - 1], 0, false);

		m_btn_scout[WALK_W - 1] =
			make_button
				(exp_mid, "scw", _("Scout towards the west"), pic_scout_w,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_W));
		exp_mid->add(m_btn_scout[WALK_W - 1], 0, false);

		m_btn_construct_port =
			make_button
				(exp_mid, "buildport", _("Construct a port at the current location"), pic_construct_port,
				 boost::bind(&ShipWindow::act_construct_port, this));
		exp_mid->add(m_btn_construct_port, 0, false);

		m_btn_scout[WALK_E - 1] =
			make_button
				(exp_mid, "sce", _("Scout towards the east"), pic_scout_e,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_E));
		exp_mid->add(m_btn_scout[WALK_E - 1], 0, false);

		m_btn_scout[WALK_SW - 1] =
			make_button
				(exp_bot, "scsw", _("Scout towards the south west"), pic_scout_sw,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_SW));
		exp_bot->add(m_btn_scout[WALK_SW - 1], 0, false);

		m_btn_explore_island_ccw =
			make_button
				(exp_bot, "expccw", _("Explore the island’s coast counter clockwise"), pic_explore_ccw,
				 boost::bind(&ShipWindow::act_explore_island, this, false));
		exp_bot->add(m_btn_explore_island_ccw, 0, false);

		m_btn_scout[WALK_SE - 1] =
			make_button
				(exp_bot, "scse", _("Scout towards the south east"), pic_scout_se,
				 boost::bind(&ShipWindow::act_scout_towards, this, WALK_SE));
		exp_bot->add(m_btn_scout[WALK_SE - 1], 0, false);

	}

	// Bottom buttons
	UI::Box * buttons = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	vbox->add(buttons, UI::Box::AlignLeft, false);

	m_btn_goto =
		make_button
			(buttons, "goto", _("Go to ship"), pic_goto,
			 boost::bind(&ShipWindow::act_goto, this));
	buttons->add(m_btn_goto, 0, false);
	m_btn_destination =
		make_button
			(buttons, "destination", _("Go to destination"), pic_destination,
			 boost::bind(&ShipWindow::act_destination, this));
	m_btn_destination->set_enabled(false);
	buttons->add(m_btn_destination, 0, false);

	m_btn_sink =
		make_button
			(buttons, "sink", _("Sink the ship"), pic_sink, boost::bind(&ShipWindow::act_sink, this));
	buttons->add(m_btn_sink, 0, false);
	if (m_ship.state_is_expedition()) {
		m_btn_cancel_expedition =
			make_button
				(buttons, "cancel_expedition", _("Cancel the Expedition"), pic_cancel_expedition,
				boost::bind(&ShipWindow::act_cancel_expedition, this));
		buttons->add(m_btn_cancel_expedition, 0, false);
	}
	set_center_panel(vbox);
	set_think(true);

	center_to_parent();
	move_out_of_the_way();
	set_fastclick_panel(m_btn_goto);
}

ShipWindow::~ShipWindow()
{
	assert(m_ship.m_window == this);
	m_ship.m_window = nullptr;
}

void ShipWindow::think()
{
	UI::Window::think();
	Interactive_Base * ib = m_ship.get_owner()->egbase().get_ibase();
	bool can_act = false;
	if (upcast(Interactive_GameBase, igb, ib))
		can_act = igb->can_act(m_ship.get_owner()->player_number());

	m_btn_destination->set_enabled(m_ship.get_destination(m_igbase.egbase()));
	m_btn_sink->set_enabled(can_act);

	m_display->clear();
	for (uint32_t idx = 0; idx < m_ship.get_nritems(); ++idx) {
		Widelands::ShippingItem item = m_ship.get_item(idx);
		Widelands::WareInstance * ware;
		Widelands::Worker * worker;
		item.get(m_igbase.egbase(), &ware, &worker);

		if (ware) {
			m_display->add(false, ware->descr_index());
		}
		if (worker) {
			m_display->add(true, worker->descr().worker_index());
		}
	}

	// Expedition specific buttons
	uint8_t state = m_ship.get_ship_state();
	if (m_ship.state_is_expedition()) {
		/* The following rules apply:
		 * - The "construct port" button is only active, if the ship is waiting for commands and found a port
		 *   buildspace
		 * - The "scout towards a direction" buttons are only active, if the ship can move at least one field
		 *   in that direction without reaching the coast.
		 * - The "explore island's coast" buttons are only active, if a coast is in vision range (no matter if
		 *   in waiting or already expedition/scouting mode)
		 */
		m_btn_construct_port->set_enabled(can_act && (state == Ship::EXP_FOUNDPORTSPACE));
		bool coast_nearby = false;
		bool moveable = false;
		for (Direction dir = 1; dir <= LAST_DIRECTION; ++dir) {
			// NOTE buttons are saved in the format DIRECTION - 1
			m_btn_scout[dir - 1]->set_enabled
				(can_act && m_ship.exp_dir_swimable(dir) && (state != Ship::EXP_COLONIZING));
			coast_nearby |= !m_ship.exp_dir_swimable(dir);
			moveable |= m_ship.exp_dir_swimable(dir);
		}
		m_btn_explore_island_cw ->set_enabled(can_act && coast_nearby && (state != Ship::EXP_COLONIZING));
		m_btn_explore_island_ccw->set_enabled(can_act && coast_nearby && (state != Ship::EXP_COLONIZING));
		m_btn_sink              ->set_enabled(can_act && (state != Ship::EXP_COLONIZING));
		m_btn_cancel_expedition ->set_enabled(can_act && (state != Ship::EXP_COLONIZING));
	}
}

UI::Button * ShipWindow::make_button
	(UI::Panel * parent, const std::string & name, const std::string & title,
	 const std::string & picname, boost::function<void()> callback)
{
	UI::Button * btn =
		new UI::Button
			(parent, name, 0, 0, 34, 34,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get(picname),
			 title);
	btn->sigclicked.connect(callback);
	return btn;
}

/// Move the main view towards the current ship location
void ShipWindow::act_goto()
{
	m_igbase.move_view_to(m_ship.get_position());
}

/// Move the main view towards the current destination of the ship
void ShipWindow::act_destination()
{
	if (PortDock * destination = m_ship.get_destination(m_igbase.egbase())) {
		m_igbase.move_view_to(destination->get_warehouse()->get_position());
	}
}

/// Sink the ship if confirmed
void ShipWindow::act_sink()
{
	if (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)) {
		m_igbase.game().send_player_sink_ship(m_ship);
	}
	else {
		show_ship_sink_confirm(ref_cast<Interactive_Player, Interactive_GameBase>(m_igbase), m_ship);
	}
}

/// Cancel expedition if confirmed
void ShipWindow::act_cancel_expedition()
{
	if (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)) {
		m_igbase.game().send_player_cancel_expedition_ship(m_ship);
	}
	else {
		show_ship_cancel_expedition_confirm
			(ref_cast<Interactive_Player, Interactive_GameBase>(m_igbase), m_ship);
	}
}

/// Sends a player command to the ship to scout towards a specific direction
void ShipWindow::act_scout_towards(uint8_t direction) {
	// ignore request if the direction is not swimable at all
	if (!m_ship.exp_dir_swimable(direction))
		return;
	m_igbase.game().send_player_ship_scout_direction(m_ship, direction);
}

/// Constructs a port at the port build space in vision range
void ShipWindow::act_construct_port() {
	if (!m_ship.exp_port_spaces() || m_ship.exp_port_spaces()->empty())
		return;
	m_igbase.game().send_player_ship_construct_port(m_ship, m_ship.exp_port_spaces()->front());
}

/// Explores the island cw or ccw
void ShipWindow::act_explore_island(bool cw) {
	bool coast_nearby = false;
	bool moveable = false;
	for (Direction dir = 1; (dir <= LAST_DIRECTION) && (!coast_nearby || !moveable); ++dir) {
		if (!m_ship.exp_dir_swimable(dir))
			coast_nearby = true;
		else
			moveable = true;
	}
	if (!coast_nearby || !moveable)
		return;
	m_igbase.game().send_player_ship_explore_island(m_ship, cw);
}


/**
 * Show the window for this ship as long as it is not sinking:
 * either bring it to the front, or create it.
 */
void Ship::show_window(Interactive_GameBase & igb, bool avoid_fastclick)
{
	// No window, if ship is sinking
	if (m_ship_state == SINK_REQUEST || m_ship_state == SINK_ANIMATION)
		return;

	if (m_window) {
		if (m_window->is_minimal())
			m_window->restore();
		m_window->move_to_top();
	} else {
		new ShipWindow(igb, *this);
		if (!avoid_fastclick)
			m_window->warp_mouse_to_fastclick_panel();
	}
}

/**
 * Close the window for this ship.
 */
void Ship::close_window()
{
	if (m_window) {
		delete m_window;
		m_window = nullptr;
	}
}

/**
 * refreshes the window of this ship - useful if some ui elements have to be removed or added
 */
void Ship::refresh_window(Interactive_GameBase & igb) {
	// Only do something if there is actually a window
	if (m_window) {
		Point window_position = m_window->get_pos();
		close_window();
		show_window(igb, true);
		// show window could theoretically fail if refresh_window was called at the very same moment
		// as the ship begins to sink
		if (m_window)
			m_window->set_pos(window_position);
	}
}

} // namespace Widelands
