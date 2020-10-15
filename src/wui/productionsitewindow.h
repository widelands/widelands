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

#ifndef WL_WUI_PRODUCTIONSITEWINDOW_H
#define WL_WUI_PRODUCTIONSITEWINDOW_H

#include <memory>

#include "logic/map_objects/tribes/productionsite.h"
#include "ui_basic/table.h"
#include "wui/buildingwindow.h"

struct ProductionSiteWindow : public BuildingWindow {
	ProductionSiteWindow(InteractiveBase& parent,
	                     UI::UniqueWindow::Registry& reg,
	                     Widelands::ProductionSite&,
	                     bool avoid_fastclick,
	                     bool workarea_preview_wanted);

protected:
	void think() override;
	void init(bool avoid_fastclick, bool workarea_preview_wanted) override;
	void evict_worker();

private:
	void update_worker_table(Widelands::ProductionSite* production_site);

	Widelands::OPtr<Widelands::ProductionSite> production_site_;
	UI::Table<uintptr_t>* worker_table_;
	UI::Box* worker_caps_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteBuilding>>
	   productionsitenotes_subscriber_;

	DISALLOW_COPY_AND_ASSIGN(ProductionSiteWindow);
};

#endif  // end of include guard: WL_WUI_PRODUCTIONSITEWINDOW_H
