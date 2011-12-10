/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include "ui_basic/box.h"

#include "economy/portdock.h"
#include "logic/warehouse.h"
#include "interactive_gamebase.h"
#include "itemwaresdisplay.h"
#include <economy/ware_instance.h>
#include <logic/worker.h>

static const char pic_goto[] = "pics/menu_ship_goto.png";
static const char pic_destination[] = "pics/menu_ship_destination.png";

namespace Widelands {

/**
 * Display information about a ship.
 */
struct ShipWindow : UI::Window {
	ShipWindow(Interactive_GameBase & igb, Ship & ship);
	virtual ~ShipWindow();

	virtual void think();

	UI::Button * make_button
		(UI::Panel * parent,
		 const std::string & name,
		 const std::string & title,
		 const std::string & picname,
		 boost::function<void()> callback);

	void act_goto();
	void act_destination();

private:
	Interactive_GameBase & m_igbase;
	Ship & m_ship;

	UI::Button * m_btn_goto;
	UI::Button * m_btn_destination;
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

	set_center_panel(vbox);
	set_think(true);

	center_to_parent();
	move_out_of_the_way();
	set_fastclick_panel(m_btn_goto);
}

ShipWindow::~ShipWindow()
{
	assert(m_ship.m_window == this);
	m_ship.m_window = 0;
}

void ShipWindow::think()
{
	UI::Window::think();

	m_btn_destination->set_enabled(m_ship.get_destination(m_igbase.egbase()));

	m_display->clear();
	for (uint32_t idx = 0; idx < m_ship.get_nritems(); ++idx) {
		Widelands::ShippingItem item = m_ship.get_item(idx);
		Widelands::WareInstance * ware;
		Widelands::Worker * worker;
		item.get(m_igbase.egbase(), ware, worker);

		if (ware) {
			m_display->add(false, ware->descr_index());
		}
		if (worker) {
			m_display->add(true, worker->descr().worker_index());
		}
	}
}

UI::Button * ShipWindow::make_button
	(UI::Panel * parent, const std::string & name, const std::string & title,
	 const std::string & picname, boost::function<void()> callback)
{
	UI::Button * btn =
		new UI::Button
			(parent, name, 0, 0, 34, 34,
			 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
			 g_gr->get_picture(PicMod_Game, picname),
			 title);
	btn->sigclicked.connect(callback);
	return btn;
}

void ShipWindow::act_goto()
{
	m_igbase.move_view_to(m_ship.get_position());
}

void ShipWindow::act_destination()
{
	if (PortDock * destination = m_ship.get_destination(m_igbase.egbase())) {
		m_igbase.move_view_to(destination->get_warehouse()->get_position());
	}
}


/**
 * Show the window for this ship: either bring it to the front,
 * or create it.
 */
void Ship::show_window(Interactive_GameBase & igb)
{
	if (m_window) {
		if (m_window->is_minimal())
			m_window->restore();
		m_window->move_to_top();
	} else {
		new ShipWindow(igb, *this);
		m_window->warp_mouse_to_fastclick_panel();
	}
}

/**
 * Close the window for this ship.
 */
void Ship::close_window()
{
	delete m_window;
}

} // namespace Widelands
