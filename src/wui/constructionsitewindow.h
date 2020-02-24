/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_WUI_CONSTRUCTIONSITEWINDOW_H
#define WL_WUI_CONSTRUCTIONSITEWINDOW_H

#include <memory>
#include <vector>

#include "logic/map_objects/tribes/constructionsite.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wui/buildingwindow.h"
#include "wui/inputqueuedisplay.h"
#include "wui/waresdisplay.h"

/**
 * Status window for construction sites.
 */
struct ConstructionSiteWindow : public BuildingWindow {
	ConstructionSiteWindow(InteractiveGameBase& parent,
	                       UI::UniqueWindow::Registry& reg,
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

	protected:
		void draw_ware(RenderTarget& dst, Widelands::DescriptionIndex ware) override;

	private:
		Widelands::WarehouseSettings& settings_;
		const Widelands::TribeDescr& tribe_;
	};

	Widelands::OPtr<Widelands::ConstructionSite> construction_site_;
	UI::ProgressBar* progress_;

	// BuildingSettings-related UI elements
	UI::Checkbox* cs_launch_expedition_;
	std::unique_ptr<UI::Radiogroup> cs_prefer_heroes_rookies_;
	UI::Button* cs_soldier_capacity_decrease_;
	UI::Button* cs_soldier_capacity_increase_;
	UI::Textarea* cs_soldier_capacity_display_;
	std::vector<InputQueueDisplay*> cs_ware_queues_;
	std::vector<InputQueueDisplay*> cs_worker_queues_;
	UI::Checkbox* cs_stopped_;
	FakeWaresDisplay* cs_warehouse_wares_;
	FakeWaresDisplay* cs_warehouse_workers_;
	void change_policy(Widelands::WareWorker, Widelands::StockPolicy);

	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteWindow);
};

#endif  // end of include guard: WL_WUI_CONSTRUCTIONSITEWINDOW_H
