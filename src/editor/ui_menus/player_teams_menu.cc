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

#include "editor/ui_menus/player_teams_menu.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/map.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/panel.h"

constexpr int16_t kPlayerRelationsCellSize = 32;
constexpr uint8_t kPlayerColorAlpha = 0x7f;

EditorPlayerTeamsMenu::PlayerRelationsPanel::PlayerRelationsPanel(UI::Panel* parent,
                                                                  EditorInteractive& e,
                                                                  uint8_t n)
   : UI::Panel(
        parent, 0, 0, (n + 1) * kPlayerRelationsCellSize, (n + 1) * kPlayerRelationsCellSize),
     eia_(e),
     nr_players_(n),
     teams_(new UI::Dropdown<uint8_t>*[n]) {
	assert(eia_.finalized());
	for (uint8_t i = 0; i < nr_players_; ++i) {
		teams_[i] = new UI::Dropdown<uint8_t>(
		   this, "team_" + std::to_string(i + 1), kPlayerRelationsCellSize * (i + 1),
		   kPlayerRelationsCellSize * (i + 1), kPlayerRelationsCellSize, nr_players_,
		   kPlayerRelationsCellSize,
		   (boost::format(_("Team for %s")) % eia_.egbase().map().get_scenario_player_name(i + 1))
		      .str(),
		   UI::DropdownType::kPictorial, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
		const unsigned team = eia_.player_relations()[i * nr_players_ + i];
		teams_[i]->add(
		   "-", 0, g_gr->images().get("images/players/no_team.png"), team == 0, _("No team"));
		for (unsigned j = 1; j <= nr_players_ / 2; ++j) {
			teams_[i]->add(std::to_string(j), j, playercolor_image(j - 1, "images/players/team.png"),
			               team == j, (boost::format(_("Team %u")) % j).str());
		}
		teams_[i]->selected.connect(
		   [this, i]() { eia_.player_relations()[i * nr_players_ + i] = teams_[i]->get_selected(); });
	}
}

static void
draw_image(RenderTarget& r, const std::string& i, int32_t x, int32_t y, int player = -1) {
	const Image* img = player >= 0 ? playercolor_image(player, i) : g_gr->images().get(i);
	assert(img);
	r.blit(Vector2i(x - img->width() / 2, y - img->height() / 2), img, BlendMode::UseAlpha);
}

void EditorPlayerTeamsMenu::PlayerRelationsPanel::draw(RenderTarget& r) {
	UI::Panel::draw(r);

	for (uint8_t n = 0; n < nr_players_; ++n) {
		RGBAColor col(kPlayerColors[n].r, kPlayerColors[n].g, kPlayerColors[n].b, kPlayerColorAlpha);
		r.fill_rect(Recti(kPlayerRelationsCellSize * (n + 1), 0, kPlayerRelationsCellSize,
		                  (nr_players_ + 1) * kPlayerRelationsCellSize),
		            col, BlendMode::Default);
		r.fill_rect(Recti(0, kPlayerRelationsCellSize * (n + 1),
		                  kPlayerRelationsCellSize * (nr_players_ + 1), kPlayerRelationsCellSize),
		            col, BlendMode::Default);
	}
	for (uint8_t p1 = 0; p1 < nr_players_; ++p1) {
		draw_image(r, "images/players/player_position_menu.png",
		           (p1 + 1) * kPlayerRelationsCellSize + kPlayerRelationsCellSize / 2,
		           kPlayerRelationsCellSize / 2, p1);
		draw_image(r, "images/players/player_position_menu.png", kPlayerRelationsCellSize / 2,
		           (p1 + 1) * kPlayerRelationsCellSize + kPlayerRelationsCellSize / 2, p1);
		for (uint8_t p2 = 0; p2 < nr_players_; ++p2) {
			if (p1 != p2) {
				draw_image(r,
				           eia_.player_relations()[p1 * nr_players_ + p2] ?
				              "images/wui/menus/chat.png" :
				              "images/wui/menus/toggle_soldier_levels.png",
				           (p1 + 1) * kPlayerRelationsCellSize + kPlayerRelationsCellSize / 2,
				           (p2 + 1) * kPlayerRelationsCellSize + kPlayerRelationsCellSize / 2);
			}
		}
	}
}

bool EditorPlayerTeamsMenu::PlayerRelationsPanel::handle_mousemove(
   uint8_t, int32_t x, int32_t y, int32_t, int32_t) {
	int8_t att = player_at(x);
	int8_t def = player_at(y);
	if (att <= 0 || def <= 0 || att == def) {
		set_tooltip("");
		return false;
	}
	set_tooltip((boost::format(eia_.player_relations()[(att - 1) * nr_players_ + (def - 1)] ?
	                              _("Click to allow %1$s to attack %2$s") :
	                              _("Click to forbid %1$s to attack %2$s")) %
	             eia_.egbase().map().get_scenario_player_name(att) %
	             eia_.egbase().map().get_scenario_player_name(def))
	               .str());
	return true;
}

bool EditorPlayerTeamsMenu::PlayerRelationsPanel::handle_mousepress(uint8_t, int32_t x, int32_t y) {
	int8_t att = player_at(x);
	if (att < 0)
		return false;
	int8_t def = player_at(y);
	if (def < 0)
		return false;
	if (att == 0 && def == 0)
		return false;
	if (att == 0) {
		--def;
		for (uint8_t a = 0; a < nr_players_; ++a)
			if (a != def)
				eia_.player_relations()[a * nr_players_ + def] =
				   !eia_.player_relations()[a * nr_players_ + def];
	} else if (def == 0) {
		--att;
		for (uint8_t d = 0; d < nr_players_; ++d)
			if (d != att)
				eia_.player_relations()[att * nr_players_ + d] =
				   !eia_.player_relations()[att * nr_players_ + d];
	} else {
		--att;
		--def;
		eia_.player_relations()[att * nr_players_ + def] =
		   !eia_.player_relations()[att * nr_players_ + def];
	}
	return true;
}

inline int8_t EditorPlayerTeamsMenu::PlayerRelationsPanel::player_at(int32_t xy) const {
	if (xy < 0)
		return -1;
	xy /= kPlayerRelationsCellSize;
	return xy > nr_players_ ? -1 : xy;
}

EditorPlayerTeamsMenu::EditorPlayerTeamsMenu(EditorInteractive& parent,
                                             EditorTool& tool,
                                             UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Player Relations"), tool),
     panel_(this, parent, parent.egbase().map().get_nrplayers()) {
	set_center_panel(&panel_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
}
