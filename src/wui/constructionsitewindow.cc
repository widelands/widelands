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

#include "wui/constructionsitewindow.h"

#include <memory>

#include "logic/map_objects/tribes/militarysite.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"
#include "wui/actionconfirm.h"
#include "wui/inputqueuedisplay.h"
#include "wui/interactive_player.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_settings[] = "images/wui/menus/statistics_stock.png";
static const char pic_tab_settings_wares[] = "images/wui/stats/menu_tab_wares_warehouse.png";
static const char pic_tab_settings_workers[] = "images/wui/stats/menu_tab_workers_warehouse.png";
static const char pic_stock_policy_prefer[] = "images/wui/buildings/stock_policy_prefer.png";
static const char pic_stock_policy_dontstock[] = "images/wui/buildings/stock_policy_dontstock.png";
static const char pic_stock_policy_remove[] = "images/wui/buildings/stock_policy_remove.png";
static const char pic_stock_policy_button_normal[] =
   "images/wui/buildings/stock_policy_button_normal.png";
static const char pic_stock_policy_button_prefer[] =
   "images/wui/buildings/stock_policy_button_prefer.png";
static const char pic_stock_policy_button_dontstock[] =
   "images/wui/buildings/stock_policy_button_dontstock.png";
static const char pic_stock_policy_button_remove[] =
   "images/wui/buildings/stock_policy_button_remove.png";
static const char pic_decrease_capacity[] = "images/wui/buildings/menu_down_train.png";
static const char pic_increase_capacity[] = "images/wui/buildings/menu_up_train.png";
constexpr uint16_t kSoldierCapacityDisplayWidth = 145;

ConstructionSiteWindow::FakeWaresDisplay::FakeWaresDisplay(UI::Panel* parent,
                                                           bool can_act,
                                                           Widelands::ConstructionSite& cs,
                                                           Widelands::WareWorker type)
   : WaresDisplay(parent, 0, 0, cs.owner().tribe(), type, can_act),
     settings_(*dynamic_cast<Widelands::WarehouseSettings*>(cs.get_settings())),
     tribe_(cs.owner().tribe()),
     warelist_(new Widelands::WareList()) {
	if (type == Widelands::wwWARE) {
		for (const auto& pair : cs.get_additional_wares()) {
			warelist_->add(pair.first, pair.second);
		}
	} else {
		for (const Widelands::Worker* w : cs.get_additional_workers()) {
			warelist_->add(w->descr().worker_index(), 1);
		}
	}
	add_warelist(*warelist_);
}

ConstructionSiteWindow::FakeWaresDisplay::~FakeWaresDisplay() {
	warelist_->clear();  // Avoid annoying warnings
}

void ConstructionSiteWindow::FakeWaresDisplay::draw_ware(RenderTarget& dst,
                                                         Widelands::DescriptionIndex ware) {
	if (get_type() == Widelands::wwWORKER &&
	    std::find(tribe_.worker_types_without_cost().begin(),
	              tribe_.worker_types_without_cost().end(),
	              ware) != tribe_.worker_types_without_cost().end()) {
		return;
	}
	WaresDisplay::draw_ware(dst, ware);

	const auto& map =
	   get_type() == Widelands::wwWARE ? settings_.ware_preferences : settings_.worker_preferences;
	const auto it = map.find(ware);
	if (it == map.end()) {
		return;
	}
	const Image* pic = nullptr;
	switch (it->second) {
	case Widelands::StockPolicy::kPrefer:
		pic = g_image_cache->get(pic_stock_policy_prefer);
		break;
	case Widelands::StockPolicy::kDontStock:
		pic = g_image_cache->get(pic_stock_policy_dontstock);
		break;
	case Widelands::StockPolicy::kRemove:
		pic = g_image_cache->get(pic_stock_policy_remove);
		break;
	case Widelands::StockPolicy::kNormal:
		// No icon for the normal policy
		return;
	}
	assert(pic);
	dst.blit(ware_position(ware), pic);
}

ConstructionSiteWindow::ConstructionSiteWindow(InteractiveBase& parent,
                                               BuildingWindow::Registry& reg,
                                               Widelands::ConstructionSite& cs,
                                               bool avoid_fastclick,
                                               bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, cs, cs.building(), avoid_fastclick),
     construction_site_(&cs),

     cs_prefer_heroes_rookies_(nullptr) {
	init(avoid_fastclick, workarea_preview_wanted);
}

void ConstructionSiteWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::ConstructionSite* construction_site = construction_site_.get(ibase()->egbase());
	assert(construction_site != nullptr);
	set_building_descr_for_help(&construction_site->building());

	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);

	build_wares_tab(construction_site);

	if (construction_site->get_settings() != nullptr) {
		build_settings_tab(construction_site);
	}

	set_title(format(_("(%s)"), construction_site->building().descname()));

	think();
	initialization_complete();
}

void ConstructionSiteWindow::build_wares_tab(Widelands::ConstructionSite* construction_site) {
	UI::Box& box = *new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);

	add_progress_bar(box);

	box.add_space(8);

	ensure_box_can_hold_input_queues(box);
	add_wares_queues(construction_site, box);

	get_tabs()->add("wares", g_image_cache->get(pic_tab_wares), &box, _("Building materials"));
}

void ConstructionSiteWindow::add_progress_bar(UI::Box& box) {
	progress_ = new UI::ProgressBar(&box, UI::PanelStyle::kWui, 0, 0, UI::ProgressBar::DefaultWidth,
	                                UI::ProgressBar::DefaultHeight, UI::ProgressBar::Horizontal);
	progress_->set_total(1 << 16);
	box.add(progress_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
}

void ConstructionSiteWindow::add_wares_queues(Widelands::ConstructionSite* construction_site,
                                              UI::Box& box) const {  // Add the wares queue
	for (uint32_t i = 0; i < construction_site->nr_dropout_waresqueues(); ++i) {
		box.add(new InputQueueDisplay(&box, *ibase(), *construction_site,
		                              *construction_site->get_dropout_waresqueue(i), true, false,
		                              priority_collapsed()),
		        UI::Box::Resizing::kFullSize);
	}
	for (uint32_t i = 0; i < construction_site->nr_consume_waresqueues(); ++i) {
		Widelands::WaresQueue* ware = construction_site->get_consume_waresqueue(i);
		// only display the queue if it requests at least one item and is not yet completed
		if (ware->get_max_size() > 0 && ware->get_filled() <= ware->get_max_size()) {
			box.add(new InputQueueDisplay(
			           &box, *ibase(), *construction_site, *ware, false, true, priority_collapsed()),
			        UI::Box::Resizing::kFullSize);
		}
	}
}
void ConstructionSiteWindow::build_settings_tab(Widelands::ConstructionSite* construction_site) {
	const bool can_act = ibase()->can_act(construction_site->owner().player_number());
	// Create the settings. Since we don't access an actual building, we create
	// a simplified faksimile of the later building window that contains only
	// the relevant options.
	bool nothing_added = false;
	std::unique_ptr<UI::Box> settings_box(
	   new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical));
	switch (construction_site->building().type()) {
	case Widelands::MapObjectType::PRODUCTIONSITE:
	case Widelands::MapObjectType::TRAININGSITE: {
		upcast(Widelands::ProductionsiteSettings, ps, construction_site->get_settings());
		// We use the ProductionSiteDescr to get the correct order for the input queues
		upcast(const Widelands::ProductionSiteDescr, prodsite, &construction_site->building());
		assert(prodsite != nullptr);
		assert(ps->ware_queues.size() == prodsite->input_wares().size());
		ensure_box_can_hold_input_queues(*settings_box);
		for (const auto& pair : prodsite->input_wares()) {
			InputQueueDisplay* queue =
			   new InputQueueDisplay(settings_box.get(), *ibase(), *construction_site,
			                         Widelands::wwWARE, pair.first, priority_collapsed());
			settings_box->add(queue, UI::Box::Resizing::kFullSize);
			cs_ware_queues_.push_back(queue);
		}
		assert(ps->worker_queues.size() == prodsite->input_workers().size());
		for (const auto& pair : prodsite->input_workers()) {
			InputQueueDisplay* queue =
			   new InputQueueDisplay(settings_box.get(), *ibase(), *construction_site,
			                         Widelands::wwWORKER, pair.first, priority_collapsed());
			settings_box->add(queue, UI::Box::Resizing::kFullSize);
			cs_ware_queues_.push_back(queue);
		}
		if (upcast(Widelands::TrainingsiteSettings, ts, ps)) {
			cs_soldier_capacity_ = new ConstructionSoldierCapacityBox(
			   settings_box.get(), ts->desired_capacity, 0, ts->max_capacity, can_act);
			settings_box->add(cs_soldier_capacity_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
			settings_box->add_space(8);
			cs_soldier_capacity_->changed.connect([this]() {
				if (game_ != nullptr) {
					game_->send_player_change_soldier_capacity(
					   *construction_site_.get(ibase()->egbase()), cs_soldier_capacity_->get_current());
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			});
		}
		cs_stopped_ = new UI::Checkbox(settings_box.get(), UI::PanelStyle::kWui, Vector2i::zero(),
		                               _("Stopped"), _("Stop this building’s work after completion"));
		cs_stopped_->clickedto.connect([this, ps](bool stop) {
			if (stop != ps->stopped) {
				if (game_ != nullptr) {
					game_->send_player_start_stop_building(*construction_site_.get(ibase()->egbase()));
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			}
		});
		settings_box->add(cs_stopped_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		settings_box->add_space(6);
		cs_stopped_->set_enabled(can_act);
	} break;
	case Widelands::MapObjectType::MILITARYSITE: {
		upcast(Widelands::MilitarysiteSettings, ms, construction_site->get_settings());
		cs_soldier_capacity_ = new ConstructionSoldierCapacityBox(
		   settings_box.get(), ms->desired_capacity, 1, ms->max_capacity, can_act);
		settings_box->add(cs_soldier_capacity_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		settings_box->add_space(8);
		cs_soldier_capacity_->changed.connect([this]() {
			if (game_ != nullptr) {
				game_->send_player_change_soldier_capacity(
				   *construction_site_.get(ibase()->egbase()), cs_soldier_capacity_->get_current());
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
		});

		UI::Box& soldier_preference_box =
		   *new UI::Box(settings_box.get(), UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
		settings_box->add(&soldier_preference_box, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		Panel& soldier_preference_panel =
		   *new Panel(&soldier_preference_box, UI::PanelStyle::kWui, 0, 0, 64, 32);
		soldier_preference_box.add(&soldier_preference_panel);
		cs_prefer_heroes_rookies_.reset(new UI::Radiogroup());
		cs_prefer_heroes_rookies_->add_button(
		   &soldier_preference_panel, UI::PanelStyle::kWui, Vector2i::zero(),
		   g_image_cache->get("images/wui/buildings/prefer_heroes.png"), _("Prefer heroes"));
		cs_prefer_heroes_rookies_->add_button(
		   &soldier_preference_panel, UI::PanelStyle::kWui, Vector2i(32, 0),
		   g_image_cache->get("images/wui/buildings/prefer_rookies.png"), _("Prefer rookies"));
		cs_prefer_heroes_rookies_->set_state(ms->prefer_heroes ? 0 : 1, false);
		if (can_act) {
			cs_prefer_heroes_rookies_->changedto.connect([this](int32_t state) {
				if (game_ != nullptr) {
					game_->send_player_militarysite_set_soldier_preference(
					   *construction_site_.get(ibase()->egbase()),
					   state != 0 ? Widelands::SoldierPreference::kRookies :
                               Widelands::SoldierPreference::kHeroes);
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			});
		}
		settings_box->add_space(8);
	} break;
	case Widelands::MapObjectType::WAREHOUSE: {
		upcast(Widelands::WarehouseSettings, ws, construction_site->get_settings());
		auto add_tab = [this, construction_site, can_act](
		                  Widelands::WareWorker ww, FakeWaresDisplay** display) {
			UI::Box& mainbox = *new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
			*display = new FakeWaresDisplay(&mainbox, can_act, *construction_site, ww);
			mainbox.add(*display, UI::Box::Resizing::kFullSize);
			UI::Box& buttonsbox =
			   *new UI::Box(&mainbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
			mainbox.add(&buttonsbox, UI::Box::Resizing::kAlign, UI::Align::kCenter);
			mainbox.add_space(15);
			UI::Button& sp_normal = *new UI::Button(
			   &buttonsbox, "stock_policy_normal", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get(pic_stock_policy_button_normal), _("Normal policy"));
			UI::Button& sp_prefer = *new UI::Button(&buttonsbox, "stock_policy_prefer", 0, 0, 34, 34,
			                                        UI::ButtonStyle::kWuiMenu,
			                                        g_image_cache->get(pic_stock_policy_button_prefer),
			                                        _("Preferably store selected wares here"));
			UI::Button& sp_dont = *new UI::Button(
			   &buttonsbox, "stock_policy_dontstock", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get(pic_stock_policy_button_dontstock),
			   _("Do not store selected wares here"));
			UI::Button& sp_remove = *new UI::Button(&buttonsbox, "stock_policy_remove", 0, 0, 34, 34,
			                                        UI::ButtonStyle::kWuiMenu,
			                                        g_image_cache->get(pic_stock_policy_button_remove),
			                                        _("Remove selected wares from here"));
			sp_remove.sigclicked.connect(
			   [this, ww]() { change_policy(ww, Widelands::StockPolicy::kRemove); });
			sp_dont.sigclicked.connect(
			   [this, ww]() { change_policy(ww, Widelands::StockPolicy::kDontStock); });
			sp_prefer.sigclicked.connect(
			   [this, ww]() { change_policy(ww, Widelands::StockPolicy::kPrefer); });
			sp_normal.sigclicked.connect(
			   [this, ww]() { change_policy(ww, Widelands::StockPolicy::kNormal); });
			sp_normal.set_enabled(can_act);
			sp_dont.set_enabled(can_act);
			sp_remove.set_enabled(can_act);
			sp_prefer.set_enabled(can_act);
			buttonsbox.add(&sp_normal);
			buttonsbox.add(&sp_prefer);
			buttonsbox.add(&sp_dont);
			buttonsbox.add(&sp_remove);
			if (ww == Widelands::wwWARE) {
				get_tabs()->add("warehouse_wares", g_image_cache->get(pic_tab_settings_wares), &mainbox,
				                _("Ware settings to apply after construction"));
			} else {
				get_tabs()->add("warehouse_workers", g_image_cache->get(pic_tab_settings_workers),
				                &mainbox, _("Worker settings to apply after construction"));
			}
		};
		add_tab(Widelands::wwWARE, &cs_warehouse_wares_);
		add_tab(Widelands::wwWORKER, &cs_warehouse_workers_);
		if (construction_site->get_info().becomes->get_isport()) {
			cs_launch_expedition_ = new UI::Checkbox(
			   settings_box.get(), UI::PanelStyle::kWui, Vector2i::zero(), _("Start an expedition"),
			   _("Start an expedition from this port after completion"));
			cs_launch_expedition_->clickedto.connect([this, ws](bool launch) {
				if (launch != ws->launch_expedition) {
					if (game_ != nullptr) {
						game_->send_player_start_or_cancel_expedition(
						   *construction_site_.get(ibase()->egbase()));
					} else {
						NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
					}
				}
			});
			settings_box->add(cs_launch_expedition_, UI::Box::Resizing::kFullSize);
			settings_box->add_space(6);
			cs_launch_expedition_->set_enabled(can_act);
		} else {
			nothing_added = true;
		}
	} break;
	default:
		NEVER_HERE();
	}

	if (!nothing_added) {
		get_tabs()->add("settings", g_image_cache->get(pic_tab_settings), settings_box.release(),
		                _("Settings to apply after construction"));
	}
}

void ConstructionSiteWindow::change_policy(Widelands::WareWorker ww, Widelands::StockPolicy p) {
	Widelands::ConstructionSite* construction_site = construction_site_.get(ibase()->egbase());
	assert(construction_site);
	upcast(Widelands::WarehouseSettings, ws, construction_site->get_settings());
	assert(ws);
	if (ww == Widelands::wwWARE) {
		for (const auto& pair : ws->ware_preferences) {
			if (cs_warehouse_wares_->ware_selected(pair.first)) {
				if (game_ != nullptr) {
					game_->send_player_set_stock_policy(
					   *construction_site, Widelands::wwWARE, pair.first, p);
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			}
		}
	} else {
		for (const auto& pair : ws->worker_preferences) {
			if (cs_warehouse_workers_->ware_selected(pair.first)) {
				if (game_ != nullptr) {
					game_->send_player_set_stock_policy(
					   *construction_site, Widelands::wwWORKER, pair.first, p);
				} else {
					NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
				}
			}
		}
	}
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void ConstructionSiteWindow::think() {
	// BuildingWindow::think() will call die in case we are no longer in
	// existance.
	BuildingWindow::think();

	Widelands::ConstructionSite* construction_site = construction_site_.get(ibase()->egbase());
	if (construction_site == nullptr) {
		return;
	}

	progress_->set_state(construction_site->get_built_per64k());

	const bool can_act = ibase()->can_act(construction_site->owner().player_number());
	// InputQueueDisplay and FakeWaresDisplay update themselves – we need to refresh the other
	// settings
	if (upcast(Widelands::ProductionsiteSettings, ps, construction_site->get_settings())) {
		assert(cs_stopped_);
		cs_stopped_->set_state(ps->stopped);
	}
	if (upcast(Widelands::TrainingsiteSettings, ts, construction_site->get_settings())) {
		assert(cs_soldier_capacity_);
		cs_soldier_capacity_->refresh(ts->desired_capacity, ts->max_capacity, can_act);
	} else if (upcast(Widelands::MilitarysiteSettings, ms, construction_site->get_settings())) {
		assert(cs_soldier_capacity_);
		assert(cs_prefer_heroes_rookies_);
		cs_soldier_capacity_->refresh(ms->desired_capacity, ms->max_capacity, can_act);
		cs_prefer_heroes_rookies_->set_state(ms->prefer_heroes ? 0 : 1, false);
	} else if (upcast(Widelands::WarehouseSettings, ws, construction_site->get_settings())) {
		if (cs_launch_expedition_ != nullptr) {
			cs_launch_expedition_->set_state(ws->launch_expedition);
		}
#ifndef NDEBUG
		else {
			assert(!ws->launch_expedition);
		}
#endif
	}
}

ConstructionSoldierCapacityBox::ConstructionSoldierCapacityBox(
   Panel* parent, uint32_t current, uint32_t min, uint32_t max, bool enabled)
   : Box(parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     current_(current),
     min_(min),
     max_(max),
     enabled_(enabled),
     cs_soldier_capacity_decrease_(
        this,
        "decrease",
        0,
        0,
        32,
        32,
        UI::ButtonStyle::kWuiMenu,
        g_image_cache->get(pic_decrease_capacity),
        _("Decrease capacity. Hold down Ctrl to set the capacity to the lowest value")),
     cs_soldier_capacity_increase_(
        this,
        "increase",
        0,
        0,
        32,
        32,
        UI::ButtonStyle::kWuiMenu,
        g_image_cache->get(pic_increase_capacity),
        _("Increase capacity. Hold down Ctrl to set the capacity to the highest value")),
     cs_soldier_capacity_display_(
        this, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kCenter) {
	cs_soldier_capacity_decrease_.set_repeating(true);
	cs_soldier_capacity_increase_.set_repeating(true);
	add(&cs_soldier_capacity_decrease_);
	add(&cs_soldier_capacity_display_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	add(&cs_soldier_capacity_increase_);
	cs_soldier_capacity_display_.set_fixed_width(kSoldierCapacityDisplayWidth);
	cs_soldier_capacity_decrease_.sigclicked.connect([this]() {
		if (matches_keymod(SDL_GetModState(), KMOD_CTRL)) {
			set_current(min_);
		} else {
			change_current(-1);
		}
	});
	cs_soldier_capacity_increase_.sigclicked.connect([this]() {
		if (matches_keymod(SDL_GetModState(), KMOD_CTRL)) {
			set_current(max_);
		} else {
			change_current(1);
		}
	});
	set_can_focus(true);
	layout();
	update();
}
void ConstructionSoldierCapacityBox::refresh(uint32_t current, uint32_t max, bool enabled) {
	if (current_ == current && max_ == max && enabled_ == enabled) {
		return;
	}
	assert(max >= current);
	current_ = current;
	max_ = max;
	enabled_ = enabled;
	update();
}
void ConstructionSoldierCapacityBox::set_current(uint32_t value) {
	if (!enabled_ || value == current_) {
		return;
	}
	current_ = value;
	update();
	changed();
}
void ConstructionSoldierCapacityBox::change_current(int32_t delta) {
	if (!enabled_ || delta == 0) {
		return;
	}
	int32_t new_val = current_ + delta;
	if (delta < 0 && static_cast<int32_t>(min_) >= new_val) {
		set_current(min_);
	} else if (delta > 0 && max_ <= static_cast<uint32_t>(new_val)) {
		set_current(max_);
	} else {
		set_current(new_val);
	}
}
void ConstructionSoldierCapacityBox::update() {
	cs_soldier_capacity_display_.set_text(
	   format(ngettext("%u soldier", "%u soldiers", current_), current_));
	cs_soldier_capacity_decrease_.set_enabled(enabled_ && current_ > min_);
	cs_soldier_capacity_increase_.set_enabled(enabled_ && current_ < max_);
}
bool ConstructionSoldierCapacityBox::handle_key(bool down, SDL_Keysym code) {
	if (enabled_ && down) {
		switch (get_keyboard_change(code)) {
		case ChangeType::kNone:
			break;
		case ChangeType::kPlus:
			change_current(1);
			return true;
		case ChangeType::kMinus:
			change_current(-1);
			return true;
		case ChangeType::kBigPlus:
			change_current(ChangeBigStep::kSmallRange);
			return true;
		case ChangeType::kBigMinus:
			change_current(-ChangeBigStep::kSmallRange);
			return true;
		case ChangeType::kSetMax:
			set_current(max_);
			return true;
		case ChangeType::kSetMin:
			set_current(min_);
			return true;
		}
	}
	return UI::Box::handle_key(down, code);
}
bool ConstructionSoldierCapacityBox::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	if (!enabled_) {
		return false;
	}
	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValue, x, y, modstate);
	if (change == 0) {
		// Try big step
		change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValueBig, x, y, modstate);
		if (change == 0) {
			return false;
		}
		change *= ChangeBigStep::kSmallRange;
	}
	change_current(change);
	return true;
}
