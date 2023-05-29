/*
 * Copyright (C) 2007-2023 by the Widelands Development Team
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

#include "wui/interactive_spectator.h"

#include "base/i18n.h"
#include "base/macros.h"
#include "chat/chat.h"
#include "graphic/game_renderer.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text_layout.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/fieldaction.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/general_statistics_menu.h"

/**
 * Setup the replay UI for the given game.
 */
InteractiveSpectator::InteractiveSpectator(Widelands::Game& g,
                                           Section& global_s,
                                           bool const multiplayer,
                                           ChatProvider* chat_provider)
   : InteractiveGameBase(g, global_s, multiplayer, chat_provider) {
	constexpr int kSpacing = 15;
	add_main_menu();

	add_toolbar_button(
	   "wui/menus/statistics_general", "general_stats",
	   as_tooltip_text_with_hotkey(_("Statistics"),
	                               shortcut_string_for(KeyboardShortcut::kInGameStatsGeneral, false),
	                               UI::PanelStyle::kWui),
	   &menu_windows_.stats_general, true);
	menu_windows_.stats_general.open_window = [this] {
		new GeneralStatisticsMenu(*this, menu_windows_.stats_general);
	};

	toolbar()->add_space(kSpacing);

	add_mapview_menu(MiniMapType::kStaticViewWindow);
	add_showhide_menu();
	add_gamespeed_menu();

	toolbar()->add_space(kSpacing);

	if (is_multiplayer()) {
		add_chat_ui();
		toolbar()->add_space(kSpacing);
	}

	add_diplomacy_menu();

	finalize_toolbar();

	// Setup all screen elements
	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		node_action(node_and_triangle);
	});

	initialization_complete();
}

void InteractiveSpectator::draw(RenderTarget& dst) {
	// This fixes a crash with displaying an error dialog during loading.
	if (!game().is_loaded()) {
		return;
	}

	draw_map_view(map_view(), &dst);
}

void InteractiveSpectator::draw_map_view(MapView* given_map_view, RenderTarget* dst) {
	// In-game, selection can never be on triangles or have a radius.
	assert(get_sel_radius() == 0);
	assert(!get_sel_triangles());

	const Widelands::Game& the_game = game();
	const Widelands::Map& map = the_game.map();
	auto* fields_to_draw =
	   given_map_view->draw_terrain(the_game, nullptr, get_workarea_overlays(map), false, dst);
	const float scale = 1.f / given_map_view->view().zoom;
	const Time& gametime = the_game.get_gametime();

	const auto info_to_draw = get_info_to_draw(!given_map_view->is_animating());
	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		const FieldsToDraw::Field& field = fields_to_draw->at(idx);
		if (field.obscured_by_slope) {
			continue;
		}

		draw_bridges(dst, &field, gametime, scale);
		draw_border_markers(field, scale, *fields_to_draw, dst);

		{
			MutexLock m(MutexLock::ID::kObjects);
			Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
			if (imm != nullptr && imm->get_positions(the_game).front() == field.fcoords) {
				imm->draw(gametime, info_to_draw, field.rendertarget_pixel, field.fcoords, scale, dst);
				if (upcast(const Widelands::Immovable, i, imm)) {
					if (!i->get_marked_for_removal().empty()) {
						const Image* img = g_image_cache->get("images/wui/overlays/targeted.png");
						blit_field_overlay(
						   dst, field, img, Vector2i(img->width() / 2, img->height()), scale);
					}
				}
			}

			for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob != nullptr;
			     bob = bob->get_next_bob()) {
				bob->draw(the_game, info_to_draw, field.rendertarget_pixel, field.fcoords, scale, dst);
			}
		}

		if (has_workarea_special_coords(field.fcoords)) {
			blit_field_overlay(dst, field, special_coords_marker_pic_,
			                   Vector2i(special_coords_marker_pic_->width() / 2,
			                            special_coords_marker_pic_->height() / 2),
			                   scale);
		}

		// Draw build help.
		if (buildhelp()) {
			auto caps = Widelands::NodeCaps::CAPS_NONE;
			const Widelands::PlayerNumber nr_players = map.get_nrplayers();
			iterate_players_existing(p, nr_players, the_game, player) {
				const Widelands::NodeCaps nc = player->get_buildcaps(field.fcoords);
				if (nc > Widelands::NodeCaps::CAPS_NONE) {
					caps = nc;
					break;
				}
			}
			const auto* overlay = get_buildhelp_overlay(caps, scale);
			if (overlay != nullptr) {
				blit_field_overlay(dst, field, overlay->pic, overlay->hotspot, scale / overlay->scale);
			}
		}

		// Blit the selection marker.
		if (g_mouse_cursor->is_visible() && field.fcoords == get_sel_pos().node) {
			const Image* pic = get_sel_picture();
			blit_field_overlay(dst, field, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
		}
	}
}

/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Widelands::Player* InteractiveSpectator::get_player() const {
	return nullptr;
}

bool InteractiveSpectator::player_hears_field(const Widelands::Coords& /*coords*/) const {
	return true;
}

// Toolbar button callback functions.
void InteractiveSpectator::exit_btn() {
	if (is_multiplayer()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

bool InteractiveSpectator::can_see(Widelands::PlayerNumber /* p */) const {
	return true;
}
bool InteractiveSpectator::can_act(Widelands::PlayerNumber /* p */) const {
	return false;
}
Widelands::PlayerNumber InteractiveSpectator::player_number() const {
	return 0;
}

/**
 * Observer has clicked on the given node; bring up the context menu.
 */
void InteractiveSpectator::node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) {
	// Special case for buildings
	const Widelands::MapObject* mo = egbase().map().get_immovable(node_and_triangle.node);
	if ((mo != nullptr) && mo->descr().type() >= Widelands::MapObjectType::BUILDING) {
		show_building_window(node_and_triangle.node, false, false);
		return;
	}

	if (try_show_ship_window()) {
		return;
	}

	//  everything else can bring up the temporary dialog
	show_field_action(this, nullptr, &fieldaction_);
}

/**
 * Global in-game keypresses:
 */
bool InteractiveSpectator::handle_key(bool const down, SDL_Keysym const code) {
	return InteractiveGameBase::handle_key(down, code);
}
