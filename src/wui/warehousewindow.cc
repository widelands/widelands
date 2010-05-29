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
#include "logic/player.h"
#include "logic/warehouse.h"
#include "ui_basic/tabpanel.h"
#include "waresdisplay.h"

using Widelands::Warehouse;

static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";

/**
 * Status window for warehouses
 */
struct Warehouse_Window : public Building_Window {
	Warehouse_Window
		(Interactive_GameBase & parent, Warehouse &, UI::Window * & registry);

	Warehouse & warehouse() {
		return ref_cast<Warehouse, Widelands::Building>(building());
	}

private:
	void make_wares_tab
		(WaresDisplay::wdType type,
		 PictureID tabicon, const std::string & tooltip);
};

/**
 * Create the tabs of a warehouse window.
 */
Warehouse_Window::Warehouse_Window
	(Interactive_GameBase & parent,
	 Warehouse            & wh,
	 UI::Window *         & registry)
	: Building_Window(parent, wh, registry)
{
	make_wares_tab
		(WaresDisplay::WARE, g_gr->get_picture(PicMod_UI, pic_tab_wares),
		 _("Wares"));
	make_wares_tab
		(WaresDisplay::WORKER, g_gr->get_picture(PicMod_UI, pic_tab_workers),
		 _("Workers"));
}

void Warehouse_Window::make_wares_tab
	(WaresDisplay::wdType type, PictureID tabicon, const std::string & tooltip)
{
	WaresDisplay * display =
		new WaresDisplay(get_tabs(), 0, 0, warehouse().owner().tribe());
	display->set_inner_size(Width, 0);
	display->add_warelist
		(type == WaresDisplay::WARE ?
		 warehouse().get_wares() : warehouse().get_workers(),
		 type);

	get_tabs()->add("wares", tabicon, display, tooltip);
}

/**
 * Create the status window describing the warehouse.
 */
void Widelands::Warehouse::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	new Warehouse_Window(parent, *this, registry);
}
