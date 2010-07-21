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
#include "graphic/rendertarget.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/warehouse.h"
#include "ui_basic/tabpanel.h"
#include "waresdisplay.h"

using Widelands::Warehouse;

static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";

static const char pic_policy_prefer[] = "pics/stock_policy_prefer.png";
static const char pic_policy_dontstock[] = "pics/stock_policy_dontstock.png";
static const char pic_policy_remove[] = "pics/stock_policy_remove.png";

/**
 * Extends the wares display to show and modify stock policy of items.
 */
struct WarehouseWaresDisplay : WaresDisplay {
	WarehouseWaresDisplay(UI::Panel * parent, uint32_t width, Interactive_GameBase &, Warehouse &, wdType type);

protected:
	virtual void draw_ware(RenderTarget& dst, Point pt, Widelands::Ware_Index ware, uint32_t stock, bool is_worker);
	virtual bool handle_mousepress(Uint8 btn, int32_t x, int32_t y);

private:
	Interactive_GameBase & m_igbase;
	Warehouse & m_warehouse;
};

WarehouseWaresDisplay::WarehouseWaresDisplay
	(UI::Panel* parent, uint32_t width, Interactive_GameBase & igbase, Warehouse& wh, wdType type)
:
WaresDisplay(parent, 0, 0, wh.owner().tribe()),
m_igbase(igbase),
m_warehouse(wh)
{
	set_inner_size(width, 0);
	add_warelist
		(type == WORKER ? m_warehouse.get_workers() : m_warehouse.get_wares(),
		 type);
}

void WarehouseWaresDisplay::draw_ware(RenderTarget& dst, Point pt, Widelands::Ware_Index ware, uint32_t stock, bool is_worker)
{
	WaresDisplay::draw_ware(dst, pt, ware, stock, is_worker);

	Warehouse::StockPolicy policy = m_warehouse.get_stock_policy(is_worker, ware);
	PictureID picid;

	switch (policy) {
	case Warehouse::SP_Prefer: picid = g_gr->get_picture(PicMod_UI, pic_policy_prefer); break;
	case Warehouse::SP_DontStock: picid = g_gr->get_picture(PicMod_UI, pic_policy_dontstock); break;
	case Warehouse::SP_Remove: picid = g_gr->get_picture(PicMod_UI, pic_policy_remove); break;
	default:
		// don't draw anything for the normal policy
		return;
	}

	dst.blit(pt, picid);
}

bool WarehouseWaresDisplay::handle_mousepress(Uint8 btn, int32_t x, int32_t y)
{
	if (btn == SDL_BUTTON_LEFT) {
		Widelands::Ware_Index ware = ware_at_point(x, y);
		if (!ware)
			return false;

		if (m_igbase.can_act(m_warehouse.owner().player_number())) {
			Warehouse::StockPolicy policy = m_warehouse.get_stock_policy(get_type() == WORKER, ware);
			Warehouse::StockPolicy newpolicy;

			switch(policy) {
			case Warehouse::SP_Normal: newpolicy = Warehouse::SP_Prefer; break;
			case Warehouse::SP_Prefer: newpolicy = Warehouse::SP_DontStock; break;
			case Warehouse::SP_DontStock: newpolicy = Warehouse::SP_Remove; break;
			default: newpolicy = Warehouse::SP_Normal; break;
			}

			m_igbase.game().send_player_command
				(*new Widelands::Cmd_SetStockPolicy
					(m_igbase.game().get_gametime(), m_warehouse.owner().player_number(),
					 m_warehouse, get_type() == WORKER, ware, newpolicy));
		}

		return true;
	}

	return WaresDisplay::handle_mousepress(btn, x, y);
}


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
	WaresDisplay * display = new WarehouseWaresDisplay(get_tabs(), Width, igbase(), warehouse(), type);
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
