/*
 * Copyright (C) 2024-2025 by the Widelands Development Team
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

#ifndef WL_UI_GAME_MARKETWINDOW_H
#define WL_UI_GAME_MARKETWINDOW_H

#include "logic/map_objects/tribes/market.h"
#include "ui/game/buildingwindow.h"

/**
 * Status window for \ref Market
 */
struct MarketWindow : public BuildingWindow {
	MarketWindow(InteractiveBase& parent,
	             BuildingWindow::Registry& reg,
	             Widelands::Market&,
	             bool avoid_fastclick,
	             bool workarea_preview_wanted);

	void update_proposals_tooltip(uint32_t count);
	void update_offers_tooltip(uint32_t count);

private:
	void init(bool avoid_fastclick, bool workarea_preview_wanted) override;
	void setup_name_field_editbox(UI::Box& vbox) override;

	Widelands::OPtr<Widelands::Market> market_;

	UI::Tab* tab_proposals_ = nullptr;
	UI::Tab* tab_offers_ = nullptr;

	DISALLOW_COPY_AND_ASSIGN(MarketWindow);
};

#endif  // end of include guard: WL_UI_GAME_MARKETWINDOW_H
