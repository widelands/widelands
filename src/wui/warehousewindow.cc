/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "buildingwindow.h"
#include "logic/warehouse.h"
#include "waresdisplay.h"

using Widelands::Warehouse;

/**
 * Status window for warehouses
 */
struct Warehouse_Window : public Building_Window {
	Warehouse_Window
		(Interactive_GameBase & parent, Warehouse &, UI::Window * & registry);

	Warehouse & warehouse() {
		return ref_cast<Warehouse, Widelands::Building>(building());
	}

	virtual void think();

private:
	void clicked_help      ();
	void clicked_switchpage();
	void clicked_goto      ();

private:
	WaresDisplay       * m_waresdisplay;
	int32_t              m_curpage;
};

/*
===============
Open the window, create the window buttons and add to the registry.
===============
*/
Warehouse_Window::Warehouse_Window
	(Interactive_GameBase & parent,
	 Warehouse            & wh,
	 UI::Window *         & registry)
	: Building_Window(parent, wh, registry)
{
	// Add wares display
	m_waresdisplay = new WaresDisplay(this, 0, 0, wh.owner().tribe());
	m_waresdisplay->add_warelist(wh.get_wares(), WaresDisplay::WARE);

	set_inner_size(m_waresdisplay->get_w(), 0);

	int32_t const spacing = 5;
	int32_t const nr_buttons = 4; // one more, turn page button is bigger
	int32_t const button_w =
		(get_inner_w() - (nr_buttons + 1) * spacing) / nr_buttons;
	int32_t       posx = spacing;
	int32_t       posy = m_waresdisplay->get_h() + spacing;
	m_curpage = 0;


	new UI::Callback_Button<Warehouse_Window>
		(this,
		 posx, posy, button_w, 25,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &Warehouse_Window::clicked_help, *this);

	posx += button_w + spacing;

	new UI::Callback_Button<Warehouse_Window>
		(this,
		 posx, posy, button_w * 2 + spacing, 25,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, "pics/warehousewindow_switchpage.png"),
		 &Warehouse_Window::clicked_switchpage, *this);

	posx += button_w * 2 + 2 * spacing;

	new UI::Callback_Button<Warehouse_Window>
		(this,
		 posx, posy, button_w, 25,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, "pics/menu_goto.png"),
		 &Warehouse_Window::clicked_goto, *this);

	posx += button_w + spacing;
	posy += 25 + spacing;

	// Add caps buttons
	posx = 0;
	UI::Panel * caps = create_capsbuttons(this);
	caps->set_pos(Point(spacing, posy));
	if (caps->get_h())
		posy += caps->get_h() + spacing;

	set_inner_size(get_inner_w(), posy);
	move_inside_parent();
}


/**
 * \todo Implement help
 */
void Warehouse_Window::clicked_help() {}


void Warehouse_Window::clicked_goto() {
	igbase().move_view_to(warehouse().get_position());
}


/*
 * Switch to the next page, that is, show
 * wares -> workers -> soldier
 */
void Warehouse_Window::clicked_switchpage() {
	if        (m_curpage == 0) {
		//  Showing wares, should show workers.
		m_waresdisplay->remove_all_warelists();
		m_waresdisplay->add_warelist
			(warehouse().get_workers(), WaresDisplay::WORKER);
		m_curpage = 1;
	} else if (m_curpage == 1) {
		//  Showing workers, should show soldiers
		//  TODO currently switches back to wares
		m_waresdisplay->remove_all_warelists();
		m_waresdisplay->add_warelist
			(warehouse().get_wares(), WaresDisplay::WARE);
		m_curpage = 0;
	}
}
/*
===============
Push the current wares status to the WaresDisplay.
===============
*/
void Warehouse_Window::think() {}


/*
===============
Create the warehouse information window
===============
*/
void Warehouse::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	new Warehouse_Window(parent, *this, registry);
}
