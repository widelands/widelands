/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/warehouse.h"
#include "ui_basic/tabpanel.h"
#include "wui/buildingwindow.h"
#include "wui/portdockwaresdisplay.h"
#include "wui/waresdisplay.h"

using Widelands::Warehouse;

static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";
static const char pic_tab_dock_wares[] = "pics/menu_tab_wares_dock.png";
static const char pic_tab_dock_workers[] = "pics/menu_tab_workers_dock.png";
static const char pic_tab_expedition[] = "pics/start_expedition.png";

static const char pic_policy_prefer[] = "pics/stock_policy_prefer.png";
static const char pic_policy_dontstock[] = "pics/stock_policy_dontstock.png";
static const char pic_policy_remove[] = "pics/stock_policy_remove.png";

/**
 * Extends the wares display to show and modify stock policy of items.
 */
class WarehouseWaresDisplay : public WaresDisplay {
public:
	WarehouseWaresDisplay
		(UI::Panel * parent, uint32_t width,
		 Warehouse & wh, Widelands::WareWorker type, bool selectable);

protected:
	virtual void draw_ware(RenderTarget & dst, Widelands::Ware_Index ware) override;

private:
	Warehouse & m_warehouse;
};

WarehouseWaresDisplay::WarehouseWaresDisplay
	(UI::Panel * parent, uint32_t width,
	 Warehouse & wh, Widelands::WareWorker type, bool selectable)
:
WaresDisplay(parent, 0, 0, wh.owner().tribe(), type, selectable),
m_warehouse(wh)
{
	set_inner_size(width, 0);
	add_warelist(type == Widelands::wwWORKER ? m_warehouse.get_workers() : m_warehouse.get_wares());
	if (type == Widelands::wwWORKER) {
		Widelands::Ware_Index carrier_index =
			m_warehouse.descr().tribe().worker_index("carrier");
		hide_ware(carrier_index);
	}
}

void WarehouseWaresDisplay::draw_ware(RenderTarget & dst, Widelands::Ware_Index ware)
{
	WaresDisplay::draw_ware(dst, ware);

	Warehouse::StockPolicy policy = m_warehouse.get_stock_policy(get_type(), ware);
	const Image* pic;

	switch (policy) {
	case Warehouse::SP_Prefer: pic = g_gr->images().get(pic_policy_prefer); break;
	case Warehouse::SP_DontStock: pic = g_gr->images().get(pic_policy_dontstock); break;
	case Warehouse::SP_Remove: pic = g_gr->images().get(pic_policy_remove); break;
	default:
		// don't draw anything for the normal policy
		return;
	}

	dst.blit(ware_position(ware), pic);
}

/**
 * Wraps the wares display together with some buttons
 */
struct WarehouseWaresPanel : UI::Box {
	WarehouseWaresPanel
		(UI::Panel * parent, uint32_t width,
		 Interactive_GameBase &, Warehouse &, Widelands::WareWorker type);

	void set_policy(Warehouse::StockPolicy);
private:
	Interactive_GameBase & m_gb;
	Warehouse & m_wh;
	bool m_can_act;
	Widelands::WareWorker m_type;
	WarehouseWaresDisplay m_display;
};

WarehouseWaresPanel::WarehouseWaresPanel
	(UI::Panel * parent, uint32_t width,
	 Interactive_GameBase & gb, Warehouse & wh, Widelands::WareWorker type)
:
	UI::Box(parent, 0, 0, UI::Box::Vertical),
	m_gb(gb),
	m_wh(wh),
	m_can_act(m_gb.can_act(m_wh.owner().player_number())),
	m_type(type),
	m_display(this, width, m_wh, m_type, m_can_act)
{
	add(&m_display, UI::Box::AlignLeft, true);

	if (m_can_act) {
		UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
		UI::Button * b;
		add(buttons, UI::Box::AlignLeft);

#define ADD_POLICY_BUTTON(policy, policyname, tooltip)                                           \
		b = new UI::Button                                                             \
			(buttons, #policy, 0, 0, 34, 34,                                                  \
			 g_gr->images().get("pics/but4.png"),                                   \
			 g_gr->images().get("pics/stock_policy_button_" #policy ".png"),      \
			 tooltip),                                                                        \
		b->sigclicked.connect \
			(boost::bind(&WarehouseWaresPanel::set_policy, this, Warehouse::SP_##policyname)), \
		buttons->add(b, UI::Box::AlignCenter);

		ADD_POLICY_BUTTON(normal, Normal, _("Normal policy"))
		ADD_POLICY_BUTTON(prefer, Prefer, _("Preferably store selected wares here"))
		ADD_POLICY_BUTTON(dontstock, DontStock, _("Do not store selected wares here"))
		ADD_POLICY_BUTTON(remove, Remove, _("Remove selected wares from here"))
	}
}

/**
 * Add Buttons policy buttons
 */
void WarehouseWaresPanel::set_policy(Warehouse::StockPolicy newpolicy) {
	bool is_workers = m_type == Widelands::wwWORKER;
	Widelands::Ware_Index nritems =
	                   is_workers ? m_wh.owner().tribe().get_nrworkers() :
				        m_wh.owner().tribe().get_nrwares();
	if (m_gb.can_act(m_wh.owner().player_number())) {
		for
			(Widelands::Ware_Index id = Widelands::Ware_Index::First();
			 id < nritems; ++id)
		{
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
	get_tabs()->add
		("wares",
		 g_gr->images().get(pic_tab_wares),
		 new WarehouseWaresPanel
			(get_tabs(),
			 Width,
			 igbase(),
			 warehouse(),
			 Widelands::wwWARE),
		 _("Wares"));
	get_tabs()->add
		("workers",
		 g_gr->images().get(pic_tab_workers),
		 new WarehouseWaresPanel
			(get_tabs(),
			 Width,
			 igbase(),
			 warehouse(),
			 Widelands::wwWORKER),
		 _("Workers"));

	if (Widelands::PortDock * pd = wh.get_portdock()) {
		get_tabs()->add
			("dock_wares",
			 g_gr->images().get(pic_tab_dock_wares),
			 create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWARE),
			 _("Wares in dock"));
		get_tabs()->add
			("dock_workers",
			 g_gr->images().get(pic_tab_dock_workers),
			 create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWORKER),
			 _("Workers in dock"));
		if (pd->expedition_started()) {
			get_tabs()->add
				("expedition_wares_queue",
				 g_gr->images().get(pic_tab_expedition),
				 create_portdock_expedition_display(get_tabs(), warehouse(), igbase()),
				 _("Expedition"));
		}
	}
}

/**
 * Create the status window describing the warehouse.
 */
void Widelands::Warehouse::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	new Warehouse_Window(parent, *this, registry);
}
