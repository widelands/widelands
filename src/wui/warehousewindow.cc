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
	virtual void draw_ware(RenderTarget& dst, Widelands::Ware_Index ware, uint32_t stock);

private:
	Interactive_GameBase & m_igbase;
	Warehouse & m_warehouse;
};

WarehouseWaresDisplay::WarehouseWaresDisplay
	(UI::Panel* parent, uint32_t width, Interactive_GameBase & igbase, Warehouse& wh, wdType type)
:
WaresDisplay(parent, 0, 0, wh.owner().tribe(), true),
m_igbase(igbase),
m_warehouse(wh)
{
	set_inner_size(width, 0);
	add_warelist
		(type == WORKER ? m_warehouse.get_workers() : m_warehouse.get_wares(),
		 type);
}

void WarehouseWaresDisplay::draw_ware(RenderTarget& dst, Widelands::Ware_Index ware, uint32_t stock)
{
	WaresDisplay::draw_ware(dst, ware, stock);

	Warehouse::StockPolicy policy = m_warehouse.get_stock_policy(get_type() == WORKER, ware);
	PictureID picid;

	switch (policy) {
	case Warehouse::SP_Prefer: picid = g_gr->get_picture(PicMod_UI, pic_policy_prefer); break;
	case Warehouse::SP_DontStock: picid = g_gr->get_picture(PicMod_UI, pic_policy_dontstock); break;
	case Warehouse::SP_Remove: picid = g_gr->get_picture(PicMod_UI, pic_policy_remove); break;
	default:
		// don't draw anything for the normal policy
		return;
	}

	dst.blit(ware_position(ware), picid);
}

/**
 * Wraps the wares display together with some buttons
 */
struct WarehouseWaresPanel : UI::Box {
	WarehouseWaresPanel(UI::Panel * parent, uint32_t width, Interactive_GameBase &, Warehouse &, WaresDisplay::wdType type);

	void set_policy(Warehouse::StockPolicy);
private:
	WarehouseWaresDisplay m_display;
	Interactive_GameBase & m_gb;
	Warehouse & m_wh;
	WaresDisplay::wdType m_type;

};

WarehouseWaresPanel::WarehouseWaresPanel(UI::Panel * parent, uint32_t width, Interactive_GameBase & gb, Warehouse & wh, WaresDisplay::wdType type) :
	UI::Box(parent, 0, 0, UI::Box::Vertical),
	m_display(this, width, gb, wh, type),
	m_gb(gb),
	m_wh(wh),
	m_type(type)
{
	add(&m_display, UI::Box::AlignLeft, true);

	UI::Box *buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(buttons, UI::Box::AlignLeft);
		
	if (m_gb.can_act(m_wh.owner().player_number())) {
#define ADD_POLICY_BUTTON(policy, policyname)                             \
        	buttons->add(new UI::Callback_Button(                     \
			buttons, "workarea",                              \
			0, 0, 34, 34,                                     \
			g_gr->get_picture(PicMod_UI,"pics/but4.png"),     \
			g_gr->get_picture(PicMod_Game,                    \
		              "pics/stock_policy_button_" #policy ".png"),\
			boost::bind(&WarehouseWaresPanel::set_policy,     \
			            this, Warehouse::SP_##policyname),    \
			_("Normal policy"))                               \
		, UI::Box::AlignCenter);                                  \
	
		ADD_POLICY_BUTTON(normal, Normal)
		ADD_POLICY_BUTTON(prefer, Prefer)
		ADD_POLICY_BUTTON(dontstock, DontStock)
		ADD_POLICY_BUTTON(remove, Remove)
	}
}

/**
 * Add Buttons policy buttons
 */
void WarehouseWaresPanel::set_policy(Warehouse::StockPolicy newpolicy) {
	bool is_workers = m_type == WaresDisplay::WORKER;
	Widelands::Ware_Index nritems =
	                   is_workers ? m_wh.owner().tribe().get_nrworkers() :
				        m_wh.owner().tribe().get_nrwares();
	if (m_gb.can_act(m_wh.owner().player_number())) {
	       for (Widelands::Ware_Index id = Widelands::Ware_Index::First();
	            id < nritems; ++id) {
			if (m_display.ware_selected(id)) {
				m_gb.game().send_player_command
					(*new Widelands::Cmd_SetStockPolicy
						(m_gb.game().get_gametime(),
						 m_wh.owner().player_number(),
						 m_wh, is_workers,
						 id, newpolicy));
			}
		}
	}
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
	WarehouseWaresPanel * panel = new WarehouseWaresPanel(get_tabs(), Width, igbase(), warehouse(), type);
	get_tabs()->add("wares", tabicon, panel, tooltip);
}

/**
 * Create the status window describing the warehouse.
 */
void Widelands::Warehouse::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	new Warehouse_Window(parent, *this, registry);
}
