/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include "logic/map_objects/tribes/constructionsite.h"
#include "ui_basic/progressbar.h"
#include "wui/buildingwindow.h"

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
	Widelands::OPtr<Widelands::ConstructionSite> construction_site_;
	UI::ProgressBar* progress_;
	DISALLOW_COPY_AND_ASSIGN(ConstructionSiteWindow);
};

#endif  // end of include guard: WL_WUI_CONSTRUCTIONSITEWINDOW_H
