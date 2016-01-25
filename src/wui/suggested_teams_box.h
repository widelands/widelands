/*
 * Copyright (C) 2015-2016 by the Widelands Development Team
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

#ifndef WL_WUI_SUGGESTED_TEAMS_BOX_H
#define WL_WUI_SUGGESTED_TEAMS_BOX_H

#include <set>
#include <string>

#include "logic/map.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/textarea.h"


namespace UI {
	struct Box;

struct SuggestedTeamsBox : public UI::Box {
	SuggestedTeamsBox(Panel * parent,
							int32_t x, int32_t y,
							uint32_t orientation,
							int32_t padding, int32_t indent, int32_t label_height,
							int32_t max_x = 0, int32_t max_y = 0,
							uint32_t inner_spacing = 0);
	~SuggestedTeamsBox();

	void hide();
	void show(const std::vector<Widelands::Map::SuggestedTeamLineup>& suggested_teams);

private:
	int32_t const padding_;
	int32_t const indent_;
	int32_t const label_height_;
	UI::Textarea * suggested_teams_box_label_;
	UI::Box* lineup_box_;
	std::vector<UI::Icon*> player_icons_;
	std::vector<UI::Textarea*> vs_labels_;
	std::vector<Widelands::Map::SuggestedTeamLineup> suggested_teams_;
};

}

#endif  // end of include guard: WL_WUI_SUGGESTED_TEAMS_BOX_H
