/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/warehousewindow.h"

#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wui/buildingwindow.h"
#include "wui/economy_options_window.h"
#include "wui/portdockwaresdisplay.h"
#include "wui/soldier_statistics_menu.h"
#include "wui/waresdisplay.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";
static const char pic_tab_dock_wares[] = "images/wui/buildings/menu_tab_wares_dock.png";
static const char pic_tab_dock_workers[] = "images/wui/buildings/menu_tab_workers_dock.png";
static const char pic_tab_expedition[] = "images/wui/buildings/start_expedition.png";
static const char pic_tab_soldiers[] = "images/wui/buildings/menu_tab_military.png";

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
	                      bool selectable);

protected:
	void draw_ware(RenderTarget& dst, Widelands::DescriptionIndex ware) override;

private:
	Widelands::Warehouse& warehouse_;
};

WarehouseWaresDisplay::WarehouseWaresDisplay(UI::Panel* parent,
                                             uint32_t width,
                                             Widelands::Warehouse& wh,
                                             Widelands::WareWorker type,
                                             bool selectable)
   : WaresDisplay(parent, 0, 0, wh.owner().tribe(), type, selectable), warehouse_(wh) {
	set_inner_size(width, 0);
	add_warelist(type == Widelands::wwWORKER ? warehouse_.get_workers() : warehouse_.get_wares());
	if (type == Widelands::wwWORKER) {
		const std::vector<Widelands::DescriptionIndex>& worker_types_without_cost =
		   warehouse_.owner().tribe().worker_types_without_cost();
		for (const Widelands::DescriptionIndex& workertype : worker_types_without_cost) {
			hide_ware(workertype);
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
	                    InteractiveBase& /*ib*/,
	                    Widelands::Warehouse& /*wh*/,
	                    Widelands::WareWorker type);

	void set_policy(Widelands::StockPolicy /*newpolicy*/);
	void change_real_fill(int32_t delta);

private:
	InteractiveBase& interactive_base_;
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
   : UI::Box(parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     interactive_base_(ib),
     wh_(wh),
     can_act_(interactive_base_.can_act(wh_.owner().player_number())),
     type_(type),
     display_(this, width, wh_, type_, can_act_) {
	add(&display_, Resizing::kFullSize);

	UI::Box* buttons = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	add(buttons, UI::Box::Resizing::kFullSize);
	UI::Button* b;

	if (can_act_) {
		add_space(15);

#define ADD_POLICY_BUTTON(policy, policyname, tooltip)                                             \
	b = new UI::Button(                                                                             \
	   buttons, #policy, 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,                                   \
	   g_image_cache->get("images/wui/buildings/stock_policy_button_" #policy ".png"), tooltip),    \
	b->sigclicked.connect([this]() { set_policy(Widelands::StockPolicy::k##policyname); }),         \
	buttons->add(b);

		ADD_POLICY_BUTTON(normal, Normal, _("Normal policy"))
		ADD_POLICY_BUTTON(prefer, Prefer, _("Preferably store selected wares here"))
		ADD_POLICY_BUTTON(dontstock, DontStock, _("Do not store selected wares here"))
		ADD_POLICY_BUTTON(remove, Remove, _("Remove selected wares from here"))

		if (interactive_base_.omnipotent()) {
			b = new UI::Button(buttons, "cheat_decrease_10", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                   g_image_cache->get("images/ui_basic/scrollbar_down_fast.png"),
			                   _("Remove 10 wares"));
			b->set_repeating(true);
			b->sigclicked.connect([this]() { change_real_fill(-10); });
			buttons->add(b);

			b = new UI::Button(buttons, "cheat_decrease_1", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                   g_image_cache->get("images/ui_basic/scrollbar_down.png"),
			                   _("Remove a ware"));
			b->set_repeating(true);
			b->sigclicked.connect([this]() { change_real_fill(-1); });
			buttons->add(b);

			b =
			   new UI::Button(buttons, "cheat_increase_1", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get("images/ui_basic/scrollbar_up.png"), _("Add a ware"));
			b->set_repeating(true);
			b->sigclicked.connect([this]() { change_real_fill(1); });
			buttons->add(b);

			b = new UI::Button(buttons, "cheat_increase_10", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                   g_image_cache->get("images/ui_basic/scrollbar_up_fast.png"),
			                   _("Add 10 wares"));
			b->set_repeating(true);
			b->sigclicked.connect([this]() { change_real_fill(10); });
			buttons->add(b);
		}
	}

	buttons->add_inf_space();

	b = new UI::Button(buttons, "configure_economy", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
	                   g_image_cache->get("images/wui/stats/genstats_nrwares.png"),
	                   _("Configure this building’s economy"));
	buttons->add(b);

	b->sigclicked.connect([this, &ib, &wh, type]() {
		EconomyOptionsWindow::create(
		   &ib, ib.egbase().mutable_descriptions(), wh.base_flag(), type, can_act_);
	});
}

void WarehouseWaresPanel::set_policy(Widelands::StockPolicy newpolicy) {
	if (interactive_base_.can_act(wh_.owner().player_number())) {
		bool is_workers = type_ == Widelands::wwWORKER;
		const std::set<Widelands::DescriptionIndex>& indices =
		   is_workers ? wh_.owner().tribe().workers() : wh_.owner().tribe().wares();

		for (const Widelands::DescriptionIndex& index : indices) {
			if (display_.ware_selected(index)) {
				if (Widelands::Game* game = interactive_base_.get_game()) {
					game->send_player_command(new Widelands::CmdSetStockPolicy(
					   game->get_gametime(), wh_.owner().player_number(), wh_, is_workers, index,
					   newpolicy));
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			}
		}
	}
}

void WarehouseWaresPanel::change_real_fill(const int32_t delta) {
	if (delta != 0 && interactive_base_.omnipotent()) {
		const bool is_workers = type_ == Widelands::wwWORKER;
		const std::set<Widelands::DescriptionIndex>& indices =
		   is_workers ? wh_.owner().tribe().workers() : wh_.owner().tribe().wares();

		for (const Widelands::DescriptionIndex& index : indices) {
			if (display_.ware_selected(index)) {
				if (is_workers) {
					if (delta > 0) {
						wh_.insert_workers(index, delta);
					} else {
						wh_.remove_workers(index, std::min<int>(-delta, wh_.get_workers().stock(index)));
					}
				} else {
					if (delta > 0) {
						wh_.insert_wares(index, delta);
					} else {
						wh_.remove_wares(index, std::min<int>(-delta, wh_.get_wares().stock(index)));
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
                                 BuildingWindow::Registry& reg,
                                 Widelands::Warehouse& wh,
                                 bool avoid_fastclick,
                                 bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, wh, avoid_fastclick), warehouse_(&wh) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void WarehouseWindow::setup_name_field_editbox(UI::Box& vbox) {
	Widelands::Warehouse* warehouse = warehouse_.get(ibase()->egbase());
	if (warehouse == nullptr || !ibase()->can_act(warehouse->owner().player_number())) {
		return BuildingWindow::setup_name_field_editbox(vbox);
	}

	UI::EditBox* name_field = new UI::EditBox(&vbox, 0, 0, 0, UI::PanelStyle::kWui);
	name_field->set_text(warehouse->get_warehouse_name());
	name_field->changed.connect([this, name_field]() {
		Widelands::Warehouse* wh = warehouse_.get(ibase()->egbase());
		if (wh == nullptr) {
			return;
		}
		if (Widelands::Game* game = ibase()->get_game(); game != nullptr) {
			game->send_player_ship_port_name(
			   wh->owner().player_number(), wh->serial(), name_field->text());
		} else {
			wh->set_warehouse_name(name_field->text());
		}
	});
	vbox.add(name_field, UI::Box::Resizing::kFullSize);
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

	get_tabs()->add(
	   "soldiers", g_image_cache->get(pic_tab_soldiers),
	   new SoldierStatisticsPanel(
	      *get_tabs(), warehouse->owner(),
	      [this](uint32_t h, uint32_t a, uint32_t d, uint32_t e) {
		      uint32_t n = 0;
		      if (Widelands::Warehouse* wh = warehouse_.get(ibase()->egbase())) {
			      assert(wh->soldier_control() != nullptr);
			      for (const Widelands::Soldier* s : wh->soldier_control()->present_soldiers()) {
				      if (s->get_health_level() == h && s->get_attack_level() == a &&
				          s->get_defense_level() == d && s->get_evade_level() == e) {
					      ++n;
				      }
			      }
		      }
		      return n;
	      }),
	   _("Soldiers"));

	if (const Widelands::PortDock* pd = warehouse->get_portdock()) {
		get_tabs()->add("dock_wares", g_image_cache->get(pic_tab_dock_wares),
		                create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWARE),
		                _("Wares waiting to be shipped"));
		get_tabs()->add("dock_workers", g_image_cache->get(pic_tab_dock_workers),
		                create_portdock_wares_display(get_tabs(), Width, *pd, Widelands::wwWORKER),
		                _("Workers waiting to embark"));
		if (pd->expedition_started()) {
			if (upcast(InteractiveGameBase, igb, ibase())) {
				get_tabs()->add("expedition_wares_queue", g_image_cache->get(pic_tab_expedition),
				                create_portdock_expedition_display(
				                   get_tabs(), *warehouse, *igb, priority_collapsed()),
				                _("Expedition"));
			}
		}
	}

	think();
	initialization_complete();
}
