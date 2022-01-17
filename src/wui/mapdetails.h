/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_MAPDETAILS_H
#define WL_WUI_MAPDETAILS_H

#include <memory>

#include "logic/editor_game_base.h"
#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/panel.h"
#include "wui/mapdata.h"
#include "wui/suggested_teams_box.h"

/**
 * Show a Panel with information about a map.
 */
class MapDetails : public UI::Panel {
public:
	MapDetails(UI::Panel* parent,
	           int32_t x,
	           int32_t y,
	           int32_t w,
	           int32_t h,
	           UI::PanelStyle style,
	           Widelands::EditorGameBase& egbase);

	void clear();
	bool update(const MapData& mapdata, bool localize_mapname, bool render_minimap);
	std::string name() {
		return name_;
	}

private:
	void layout() override;
	const UI::PanelStyle style_;
	const int padding_;

	UI::Box main_box_;
	UI::Box descr_box_;
	std::string name_;
	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
	UI::Icon minimap_icon_;
	UI::SuggestedTeamsBox* suggested_teams_box_;

	// Used to render map preview
	std::string last_map_;
	std::unordered_map<std::string, std::unique_ptr<const Texture>> minimap_cache_;
	Widelands::EditorGameBase& egbase_;  // Not owned
};

#endif  // end of include guard: WL_WUI_MAPDETAILS_H
