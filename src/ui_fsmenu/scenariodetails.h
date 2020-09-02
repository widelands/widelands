/*
 * Copyright (C) 2017-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_SCENARIODETAILS_H
#define WL_UI_FSMENU_SCENARIODETAILS_H

#include <memory>

#include "logic/editor_game_base.h"
#include "map_io/map_loader.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/panel.h"
#include "ui_fsmenu/campaigns.h"

/**
 * Show a Box with information about a campaign or tutorial scenario.
 */
class ScenarioDetails : public UI::Panel {
public:
	explicit ScenarioDetails(Panel* parent);

	void update(const ScenarioData& scenariodata);

private:
	void layout() override;
	const int padding_;

	UI::Box main_box_;
	UI::Box descr_box_;
	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
	UI::Icon minimap_icon_;

	// Used to render map preview
	std::unique_ptr<Texture> minimap_image_;
	std::unique_ptr<Widelands::MapLoader> map_loader_;
	Widelands::EditorGameBase egbase_;
};

#endif  // end of include guard: WL_UI_FSMENU_SCENARIODETAILS_H
