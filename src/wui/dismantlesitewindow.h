/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#ifndef WL_WUI_DISMANTLESITEWINDOW_H
#define WL_WUI_DISMANTLESITEWINDOW_H

#include <vector>

#include "logic/map_objects/tribes/dismantlesite.h"
#include "ui_basic/progressbar.h"
#include "wui/buildingwindow.h"

class InputQueueDisplay;

/**
 * Status window for dismantle sites.
 */
struct DismantleSiteWindow : public BuildingWindow {
	DismantleSiteWindow(InteractiveBase& parent,
	                    BuildingWindow::Registry& reg,
	                    Widelands::DismantleSite&,
	                    bool avoid_fastclick);

	void think() override;
	void draw(RenderTarget& rt) override;

private:
	void init(bool avoid_fastclick, bool workarea_preview_wanted) override;

	Widelands::OPtr<Widelands::DismantleSite> dismantle_site_;
	UI::ProgressBar* progress_{nullptr};
	std::vector<InputQueueDisplay*> dropout_queues_;
	DISALLOW_COPY_AND_ASSIGN(DismantleSiteWindow);
};

#endif  // end of include guard: WL_WUI_DISMANTLESITEWINDOW_H
