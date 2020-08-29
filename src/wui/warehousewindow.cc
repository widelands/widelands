/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "wui/warehousewindow.h"

#include "graphic/rendertarget.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wui/buildingwindow.h"
#include "wui/portdockwaresdisplay.h"
#include "wui/waresdisplay.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";
static const char pic_tab_dock_wares[] = "images/wui/buildings/menu_tab_wares_dock.png";
static const char pic_tab_dock_workers[] = "images/wui/buildings/menu_tab_workers_dock.png";
static const char pic_tab_expedition[] = "images/wui/buildings/start_expedition.png";

static const char pic_policy_prefer[] = "images/wui/buildings/stock_policy_prefer.png";
static const char pic_policy_dontstock[] = "images/wui/buildings/stock_policy_dontstock.png";
static const char pic_policy_remove[] = "images/wui/buildings/stock_policy_remove.png";

/**
 * Extends the wares display to show and modify stock policy of items.
 */
class WarehouseWaresDisplay : public WaresDisplay {
public:
	WarehouseWaresDisplay(UI::Panel* parent,
	                      uint32_t width,
	                      Widelands::Warehouse& wh,
	                      Widelands::WareWorker type,
	                      bool selectable,
	                      bool show_all);

protected:
	void draw_ware(RenderTarget& dst, Widelands::DescriptionIndex ware) override;

private:
	Widelands::Warehouse& warehouse_;
};

WarehouseWaresDisplay::WarehouseWaresDisplay(UI::Panel* parent,
                                             uint32_t width,
                                             Widelands::Warehouse& wh,
                                             Widelands::WareWorker type,
                                             bool selectable,
                                             bool show_all)
   : WaresDisplay(parent, 0, 0, wh.owner().tribe(), type, selectable), warehouse_(wh) {
	set_inner_size(width, 0);
	add_warelist(type == Widelands::wwWORKER ? warehouse_.get_workers() : warehouse_.get_wares());
	if (!show_all && type == Widelands::wwWORKER) {
		const std::vector<Widelands::DescriptionIndex>& worker_types_without_cost =
		   warehouse_.owner().tribe().worker_types_without_cost();
		for (size_t i = 0; i < worker_types_without_cost.size(); ++i) {
			hide_ware(worker_types_without_cost.at(i));
		}
	}
}

void WarehouseWaresDisplay::draw_ware(RenderTarget& dst, Widelands::DescriptionIndex ware) {
	WaresDisplay::draw_ware(dst, ware);

	Widelands::StockPolicy policy = warehouse_.get_stock_policy(get_type(), ware);
	const Image* pic = nullptr;
	switch (policy) {
	case Widelands::StockPolicy::kPrefer:
		pic = g_image_cache->get(pic_policy_prefer);
		break;
	case Widelands::StockPolicy::kDontStock:
		pic = g_image_cache->get(pic_policy_dontstock);
		break;
	case Widelands::StockPolicy::kRemove:
		pic = g_image_cache->get(pic_policy_remove);
		break;
	case Widelands::StockPolicy::kNormal:
		// don't draw anything for the normal policy
		return;
	}
	assert(pic != nullptr);

	dst.blit(ware_position(ware), pic);
}

/**
 * Wraps the wares display together with some buttons
 */
struct WarehouseWaresPanel : UI::Box {
	WarehouseWaresPanel(UI::Panel* parent,
	                    uint32_t width,
	                    InteractiveBase&,
	                    Widelands::Warehouse&,
	                    Widelands::WareWorker type);

	void set_policy(Widelands::StockPolicy);

private:
	void change_real_amount(int32_t delta);

	InteractiveBase& ib_;
	Widelands::Warehouse& wh_;
	bool can_act_;
	Widelands::WareWorker type_;
	WarehouseWaresDisplay display_;
};

WarehouseWaresPanel::WarehouseWaresPanel(UI::Panel* parent,
                                         uint32_t width,
                                         InteractiveBase& ib,
                                         Widelands::Warehouse& wh,
                                         Widelands::WareWorker type)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     ib_(ib),
     wh_(wh),
     can_act_(ib.omnipotent() || ib.can_act(wh_.owner().player_number())),
     type_(type),
     display_(this, width, wh_, type_, can_act_, ib.omnipotent()) {
	add(&display_, Resizing::kFullSize);

	if (can_act_) {
		UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
		UI::Button* b;
		add(buttons, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		add_space(15);

#define ADD_POLICY_BUTTON(policy, policyname, tooltip)                                             \
	b = new UI::Button(                                                                             \
	   buttons, #policy, 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,                                   \
	   g_image_cache->get("images/wui/buildings/stock_policy_button_" #policy ".png"), tooltip),    \
	b->sigclicked.connect([this]() { set_policy(Widelands::StockPolicy::k##policyname); }),         \
	buttons->add(b);

#define ADD_REAL_STORAGE_BUTTON(delta, img, tt)                                                    \
	b = new UI::Button(buttons, "real_storage_" img, 0, 0, 44, 28, UI::ButtonStyle::kWuiSecondary,  \
	                   g_image_cache->get("images/ui_basic/scrollbar_" img ".png"), tt);            \
	b->set_repeating(true);                                                                         \
	b->sigclicked.connect([this]() { change_real_amount(delta); });                                 \
	buttons->add(b);

		ADD_POLICY_BUTTON(normal, Normal, _("Normal policy"))
		ADD_POLICY_BUTTON(prefer, Prefer, _("Preferably store selected wares here"))
		ADD_POLICY_BUTTON(dontstock, DontStock, _("Do not store selected wares here"))
		ADD_POLICY_BUTTON(remove, Remove, _("Remove selected wares from here"))

		if (ib_.omnipotent()) {
			buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
			add(buttons, UI::Box::Resizing::kAlign, UI::Align::kCenter);
			ADD_REAL_STORAGE_BUTTON(-10, "down_fast", _("Decrease storage by 10"))
			ADD_REAL_STORAGE_BUTTON(-1, "down", _("Decrease storage by 1"))
			ADD_REAL_STORAGE_BUTTON(1, "up", _("Increase storage by 1"))
			ADD_REAL_STORAGE_BUTTON(10, "up_fast", _("Increase storage by 10"))
		}
	}
}

void WarehouseWaresPanel::change_real_amount(int32_t delta) {
	if (delta == 0) {
		return;
	}
	assert(ib_.omnipotent());
	const bool is_workers = type_ == Widelands::wwWORKER;
	for (const Widelands::DescriptionIndex& index :
	     is_workers ? wh_.owner().tribe().workers() : wh_.owner().tribe().wares()) {
		if (display_.ware_selected(index)) {
			if (is_workers) {
				if (delta > 0) {
					wh_.insert_workers(index, delta);
				} else {
					const int32_t stock = wh_.get_workers().stock(index);
					wh_.remove_workers(index, std::min(stock, -delta));
				}
			} else {
				if (delta > 0) {
					wh_.insert_wares(index, delta);
				} else {
					const int32_t stock = wh_.get_wares().stock(index);
					wh_.remove_wares(index, std::min(stock, -delta));
				}
			}
		}
	}
}

/**
 * Add Buttons policy buttons
 */
void WarehouseWaresPanel::set_policy(Widelands::StockPolicy newpolicy) {
	if (ib_.omnipotent() || ib_.can_act(wh_.owner().player_number())) {
		const bool is_workers = type_ == Widelands::wwWORKER;
		const std::set<Widelands::DescriptionIndex>& indices =
		   is_workers ? wh_.owner().tribe().workers() : wh_.owner().tribe().wares();

		for (const Widelands::DescriptionIndex& index : indices) {
			if (display_.ware_selected(index)) {
				if (ib_.get_game()) {
					ib_.game().send_player_command(new Widelands::CmdSetStockPolicy(
					   ib_.game().get_gametime(), wh_.owner().player_number(), wh_, is_workers, index,
					   newpolicy));
				} else {
					if (is_workers) {
						wh_.set_worker_policy(index, newpolicy);
					} else {
						wh_.set_ware_policy(index, newpolicy);
					}
				}
			}
		}
	}
}

/**
 * Create the tabs of a warehouse window.
 */
WarehouseWindow::WarehouseWindow(InteractiveBase& parent,
                                 UI::UniqueWindow::Registry& reg,
                                 Widelands::Warehouse& wh,
                                 bool avoid_fastclick,
                                 bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, wh, avoid_fastclick), warehouse_(&wh) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void WarehouseWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::Warehouse* warehouse = warehouse_.get(ibase()->egbase());
	assert(warehouse != nullptr);
	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);
	get_tabs()->add(
	   "wares", g_image_cache->get(pic_tab_wares),
	   new WarehouseWaresPanel(get_tabs(), Width, *ibase(), *warehouse, Widelands::wwWARE),
	   _("Wares"));
	get_tabs()->add(
	   "workers", g_image_cache->get(pic_tab_workers),
	   new WarehouseWaresPanel(get_tabs(), Width, *ibase(), *warehouse, Widelands::wwWORKER),
	   _("Workers"));

	if (const Widelands::PortDock* pd = warehouse->get_portdock()) {
		get_tabs()->add("dock_wares", g_image_cache->get(pic_tab_dock_wares),
		                create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWARE),
		                _("Wares waiting to be shipped"));
		get_tabs()->add("dock_workers", g_image_cache->get(pic_tab_dock_workers),
		                create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWORKER),
		                _("Workers waiting to embark"));
		if (pd->expedition_started()) {
			get_tabs()->add("expedition_wares_queue", g_image_cache->get(pic_tab_expedition),
			                create_portdock_expedition_display(get_tabs(), *warehouse, *ibase()),
			                _("Expedition"));
		}
	}
	think();
}
