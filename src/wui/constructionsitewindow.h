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

#ifndef WL_WUI_CONSTRUCTIONSITEWINDOW_H
#define WL_WUI_CONSTRUCTIONSITEWINDOW_H

#include <memory>

#include "logic/map_objects/tribes/constructionsite.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wui/buildingwindow.h"
#include "wui/inputqueuedisplay.h"
#include "wui/waresdisplay.h"

// Soldier capacity for training or military site
struct ConstructionSoldierCapacityBox : public UI::Box {
private:
	uint32_t current_, min_, max_;
	bool enabled_;
	UI::Button cs_soldier_capacity_decrease_;
	UI::Button cs_soldier_capacity_increase_;
	UI::Textarea cs_soldier_capacity_display_;

public:
	ConstructionSoldierCapacityBox(
	   Panel* parent, uint32_t current, uint32_t min, uint32_t max, bool enabled);

	void refresh(uint32_t current, uint32_t max, bool enabled);
	void set_current(uint32_t value);
	void change_current(int32_t delta);
	Notifications::Signal<> changed;
	uint32_t get_current() const {
		return current_;
	}
	bool handle_key(bool down, SDL_Keysym code) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

private:
	void update();
};

/**
 * Status window for construction sites.
 */
struct ConstructionSiteWindow : public BuildingWindow {
	ConstructionSiteWindow(InteractiveBase& parent,
	                       BuildingWindow::Registry& reg,
	                       Widelands::ConstructionSite&,
	                       bool avoid_fastclick,
	                       bool workarea_preview_wanted);

	void think() override;

protected:
	void init(bool avoid_fastclick, bool workarea_preview_wanted) override;

private:
	class FakeWaresDisplay : public WaresDisplay {
	public:
		FakeWaresDisplay(UI::Panel* parent,
		                 bool can_act,
		                 Widelands::ConstructionSite& cs,
		                 Widelands::WareWorker type);
		~FakeWaresDisplay() override;

	protected:
		void draw_ware(RenderTarget& dst, Widelands::DescriptionIndex ware) override;

	private:
		Widelands::WarehouseSettings& settings_;
		const Widelands::TribeDescr& tribe_;
		std::unique_ptr<Widelands::WareList> warelist_;
	};

	Widelands::OPtr<Widelands::ConstructionSite> construction_site_;
	UI::ProgressBar* progress_{nullptr};

	// BuildingSettings-related UI elements
	UI::Checkbox* cs_launch_expedition_{nullptr};
	std::unique_ptr<UI::Radiogroup> cs_prefer_heroes_rookies_;
	ConstructionSoldierCapacityBox* cs_soldier_capacity_{nullptr};
	std::vector<InputQueueDisplay*> cs_ware_queues_;
	std::vector<InputQueueDisplay*> cs_worker_queues_;
	UI::Checkbox* cs_stopped_{nullptr};
	FakeWaresDisplay* cs_warehouse_wares_{nullptr};
	FakeWaresDisplay* cs_warehouse_workers_{nullptr};
	void change_policy(Widelands::WareWorker, Widelands::StockPolicy);
	void add_wares_queues(Widelands::ConstructionSite* construction_site, UI::Box& box) const;
	void add_progress_bar(UI::Box& box);
	void build_wares_tab(Widelands::ConstructionSite* construction_site);
	void build_settings_tab(Widelands::ConstructionSite* construction_site);
	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteWindow);
};

#endif  // end of include guard: WL_WUI_CONSTRUCTIONSITEWINDOW_H
