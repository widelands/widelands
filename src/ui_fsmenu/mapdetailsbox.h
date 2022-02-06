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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H

#include "logic/game_settings.h"
#include "logic/map.h"
#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "wui/suggested_teams_box.h"

class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent, uint32_t padding);
	~MapDetailsBox() override = default;

	void update(GameSettingsProvider* settings, Widelands::Map& map);
	void update_from_savegame(GameSettingsProvider* settings);

	void force_new_dimensions(uint32_t width, uint32_t height);

	// TODO(jmoerschbach): only used by multiplayer screen...
	void set_map_description_text(const std::string& text);
	void show_warning(const std::string& text);

private:
	int padding_;

	UI::Textarea title_;
	UI::Box title_box_;
	UI::Box content_box_;
	UI::Textarea map_name_;
	UI::MultilineTextarea map_description_;
	UI::SuggestedTeamsBox suggested_teams_box_;

	void show_map_description(const Widelands::Map& map, GameSettingsProvider* settings);
	void show_map_description_savegame(const GameSettings& game_settings);
	void show_map_name(const GameSettings& game_settings);
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
