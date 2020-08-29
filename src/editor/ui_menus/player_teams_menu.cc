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

#include <SDL_mouse.h>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/map.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/panel.h"

constexpr int16_t kPlayerRelationsCellSize = 48;
constexpr uint8_t kPlayerColorAlpha = 0x7f;

EditorPlayerTeamsMenu::PlayerRelationsPanel::PlayerRelationsPanel(UI::Panel* parent,
                                                                  EditorInteractive& e,
                                                                  uint8_t n)
   : UI::Panel(
        parent, 0, 0, (n + 2) * kPlayerRelationsCellSize, (n + 2) * kPlayerRelationsCellSize),
     eia_(e),
     nr_players_(n) {
	assert(eia_.finalized());
	for (uint8_t i = 0; i < nr_players_; ++i) {
		UI::Dropdown<uint8_t>* dd = new UI::Dropdown<uint8_t>(
		   this, "team_" + std::to_string(i + 1), kPlayerRelationsCellSize * (i + 1),
		   kPlayerRelationsCellSize * (i + 1), kPlayerRelationsCellSize, nr_players_,
		   kPlayerRelationsCellSize,
		   (boost::format(_("Team for %s")) % eia_.egbase().map().get_scenario_player_name(i + 1))
		      .str(),
		   UI::DropdownType::kPictorial, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
		const unsigned team = eia_.egbase().player(i + 1).team_number();
		dd->add("-", 0, g_image_cache->get("images/players/no_team.png"), team == 0, _("No team"));
		for (unsigned j = 1; j <= nr_players_ / 2; ++j) {
			dd->add(std::to_string(j), j, playercolor_image(j - 1, "images/players/team.png"),
			        team == j, (boost::format(_("Team %u")) % j).str());
		}
		dd->selected.connect(
		   [this, i, dd]() { eia_.egbase().get_player(i + 1)->set_team_number(dd->get_selected()); });
		assert(teams_.size() == i);
		teams_.push_back(std::unique_ptr<UI::Dropdown<uint8_t>>(dd));

		assert(buttons_.size() == i * 2);
		UI::Button* b = new UI::Button(
		   this, "allowed_buildings_" + std::to_string(static_cast<unsigned>(i)),
		   (n + 1) * kPlayerRelationsCellSize, (i + 1) * kPlayerRelationsCellSize,
		   kPlayerRelationsCellSize, kPlayerRelationsCellSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/stats/genstats_nrbuildings.png"),
		   (boost::format(_("Configure allowed building types for %s")) %
		    eia_.egbase().map().get_scenario_player_name(i + 1))
		      .str());
		b->sigclicked.connect([this, i]() { eia_.show_allowed_buildings_window(i + 1); });
		buttons_.push_back(std::unique_ptr<UI::Button>(b));
		b = new UI::Button(this, "goto_start_" + std::to_string(static_cast<unsigned>(i)),
		                   (i + 1) * kPlayerRelationsCellSize, (n + 1) * kPlayerRelationsCellSize,
		                   kPlayerRelationsCellSize, kPlayerRelationsCellSize,
		                   UI::ButtonStyle::kWuiSecondary,
		                   g_image_cache->get("images/wui/fieldaction/menu_tab_attack.png"),
		                   /** TRANSLATORS: %s is the name of a player */
		                   (boost::format(_("Go to %s’s starting position")) %
		                    eia_.egbase().map().get_scenario_player_name(i + 1))
		                      .str());
		b->sigclicked.connect([this, i]() {
			eia_.map_view()->scroll_to_field(
			   eia_.egbase().map().get_starting_pos(i + 1), MapView::Transition::Smooth);
		});
		buttons_.push_back(std::unique_ptr<UI::Button>(b));
	}
}

static void
draw_image(RenderTarget& r, const std::string& i, int32_t x, int32_t y, int player = -1) {
	const Image* img = player >= 0 ? playercolor_image(player, i) : g_image_cache->get(i);
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
		const Widelands::Player& pl = eia_.egbase().player(p1 + 1);
		for (uint8_t p2 = 0; p2 < nr_players_; ++p2) {
			if (p1 != p2) {
				draw_image(r,
				           pl.is_attack_forbidden(p2 + 1) ?
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
	set_tooltip((boost::format(eia_.egbase().player(att).is_attack_forbidden(def) ?
	                              _("Click to allow %1$s to attack %2$s") :
	                              _("Click to forbid %1$s to attack %2$s")) %
	             eia_.egbase().map().get_scenario_player_name(att) %
	             eia_.egbase().map().get_scenario_player_name(def))
	               .str());
	return true;
}

bool EditorPlayerTeamsMenu::PlayerRelationsPanel::handle_mousepress(uint8_t b,
                                                                    int32_t x,
                                                                    int32_t y) {
	if (b != SDL_BUTTON_LEFT) {
		return false;
	}
	const int8_t att = player_at(x);
	if (att < 0) {
		return false;
	}
	const int8_t def = player_at(y);
	if (def < 0 || (att == 0 && def == 0)) {
		return false;
	}
	if (att == 0) {
		for (uint8_t a = 1; a <= nr_players_; ++a) {
			if (a != def) {
				eia_.egbase().get_player(a)->set_attack_forbidden(
				   def, !eia_.egbase().player(a).is_attack_forbidden(def));
			}
		}
	} else if (def == 0) {
		for (uint8_t d = 1; d <= nr_players_; ++d) {
			if (d != att) {
				eia_.egbase().get_player(att)->set_attack_forbidden(
				   d, !eia_.egbase().player(att).is_attack_forbidden(d));
			}
		}
	} else {
		eia_.egbase().get_player(att)->set_attack_forbidden(
		   def, !eia_.egbase().player(att).is_attack_forbidden(def));
	}
	return true;
}

inline int8_t EditorPlayerTeamsMenu::PlayerRelationsPanel::player_at(int32_t xy) const {
	if (xy < 0) {
		return -1;
	}
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

EditorPlayerAllowedBuildingsWindow::EditorPlayerAllowedBuildingsWindow(
   EditorInteractive* eia, Widelands::PlayerNumber p, UI::UniqueWindow::Registry& r)
   : UI::UniqueWindow(eia,
                      "allowed_buildings_" + std::to_string(static_cast<unsigned>(p)),
                      &r,
                      300,
                      250,
                      (boost::format(_("Allowed Building Types for %s")) %
                       eia->egbase().map().get_scenario_player_name(p))
                         .str()),
     box_(this, 0, 0, UI::Box::Vertical) {
	Widelands::Player* player = eia->egbase().get_player(p);
	for (Widelands::DescriptionIndex di : player->tribe().buildings()) {
		const Widelands::BuildingDescr& d = *eia->egbase().tribes().get_building_descr(di);
		UI::Checkbox& c =
		   *new UI::Checkbox(&box_, Vector2i(0, 0),
		                     (boost::format(
		                         /** TRANSLATORS: Building Name (internal_building_name) */
		                         _("%1$s (%2$s)")) %
		                      d.descname() % d.name())
		                        .str());
		c.set_state(player->is_building_type_allowed(di));
		c.changedto.connect([player, p, di](bool a) { player->allow_building_type(di, a); });
		box_.add(&c);
	}
	box_.set_scrolling(true);
	box_.set_max_size(700, 400);
	set_center_panel(&box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
}
