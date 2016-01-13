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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WUI_PRODUCTIONSITEWINDOW_H
#define WL_WUI_PRODUCTIONSITEWINDOW_H

#include "wui/buildingwindow.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "ui_basic/table.h"

struct ProductionSiteWindow : public BuildingWindow {
	ProductionSiteWindow
		(InteractiveGameBase & parent,
		 Widelands::ProductionSite &,
		 UI::Window *         & registry);

	Widelands::ProductionSite & productionsite() {
		return dynamic_cast<Widelands::ProductionSite&>(building());
	}
	void update_worker_table();
protected:
	void think() override;
	void evict_worker();

private:
	UI::Table<uintptr_t> * m_worker_table;
	UI::Box * m_worker_caps;
};

#endif  // end of include guard: WL_WUI_PRODUCTIONSITEWINDOW_H
