/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#include "wui/interactive_spectator.h"

#include "base/i18n.h"
#include "base/macros.h"
#include "chat/chat.h"
#include "graphic/graphic.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_menu.h"
#include "wui/general_statistics_menu.h"

/**
 * Setup the replay UI for the given game.
 */
InteractiveSpectator::InteractiveSpectator(Widelands::Game& g,
                                           Section& global_s,
                                           bool const multiplayer)
   : InteractiveGameBase(g, global_s, OBSERVER, multiplayer) {
	if (is_multiplayer()) {
		add_toolbar_button(
		   "wui/menus/menu_options_menu", "options_menu", _("Main menu"), &options_, true);
		options_.open_window = [this] { new GameOptionsMenu(*this, options_, main_windows_); };

	} else {
		UI::Button* button =
		   add_toolbar_button("wui/menus/menu_exit_game", "exit_replay", _("Exit replay"));
		button->sigclicked.connect(boost::bind(&InteractiveSpectator::exit_btn, this));

		add_toolbar_button(
		   "wui/menus/menu_save_game", "save_game", _("Save game"), &main_windows_.savegame, true);
		main_windows_.savegame.open_window = [this] {
			new GameMainMenuSaveGame(*this, main_windows_.savegame);
		};
	}
	add_toolbar_button("wui/menus/menu_general_stats", "general_stats", _("Statistics"),
	                   &main_windows_.general_stats, true);
	main_windows_.general_stats.open_window = [this] {
		new GeneralStatisticsMenu(*this, main_windows_.general_stats);
	};

	toolbar()->add_space(15);

	add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry(), true);
	minimap_registry().open_window = [this] { toggle_minimap(); };

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show building spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&InteractiveBase::toggle_buildhelp, this));

	reset_zoom_ = add_toolbar_button("wui/menus/menu_reset_zoom", "reset_zoom", _("Reset zoom"));
	reset_zoom_->sigclicked.connect([this] {
		map_view()->zoom_around(
		   1.f, Vector2f(get_w() / 2.f, get_h() / 2.f), MapView::Transition::Smooth);
	});

	toolbar()->add_space(15);

	if (is_multiplayer()) {
		add_toolbar_button("wui/menus/menu_chat", "chat", _("Chat"), &chat_, true);
		chat_.open_window = [this] {
			if (chat_provider_) {
				GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
			}
		};
	}

	adjust_toolbar_position();

	// Setup all screen elements
	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		node_action(node_and_triangle);
	});
}

void InteractiveSpectator::draw(RenderTarget& dst) {
	// This fixes a crash with displaying an error dialog during loading.
	if (!game().is_loaded())
		return;

	draw_map_view(map_view(), &dst);
}

void InteractiveSpectator::draw_map_view(MapView* given_map_view, RenderTarget* dst) {
	// A spectator cannot build roads.
	assert(road_building_overlays().steepness_indicators.empty());
	assert(road_building_overlays().road_previews.empty());

	// In-game, selection can never be on triangles or have a radius.
	assert(get_sel_radius() == 0);
	assert(!get_sel_triangles());

	const Widelands::Game& the_game = game();
	const Widelands::Map& map = the_game.map();
	auto* fields_to_draw = given_map_view->draw_terrain(the_game, dst);
	const float scale = 1.f / given_map_view->view().zoom;
	const uint32_t gametime = the_game.get_gametime();

	const auto text_to_draw = get_text_to_draw();
	const std::map<Widelands::Coords, const Image*> work_area_overlays = get_work_area_overlays(map);
	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		const FieldsToDraw::Field& field = fields_to_draw->at(idx);

		draw_border_markers(field, scale, *fields_to_draw, dst);

		Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
		if (imm != nullptr && imm->get_positions(the_game).front() == field.fcoords) {
			imm->draw(gametime, text_to_draw, field.rendertarget_pixel, scale, dst);
		}

		for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob;
		     bob = bob->get_next_bob()) {
			bob->draw(the_game, text_to_draw, field.rendertarget_pixel, scale, dst);
		}

		const auto blit_overlay = [dst, &field, scale](const Image* pic, const Vector2i& hotspot) {
			dst->blitrect_scale(Rectf(field.rendertarget_pixel - hotspot.cast<float>() * scale,
			                          pic->width() * scale, pic->height() * scale),
			                    pic, Recti(0, 0, pic->width(), pic->height()), 1.f,
			                    BlendMode::UseAlpha);
		};

		// Draw work area previews.
		const auto it = work_area_overlays.find(field.fcoords);
		if (it != work_area_overlays.end()) {
			const Image* pic = it->second;
			blit_overlay(pic, Vector2i(pic->width() / 2, pic->height() / 2));
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
			const auto* overlay = get_buildhelp_overlay(caps);
			if (overlay != nullptr) {
				blit_overlay(overlay->pic, overlay->hotspot);
			}
		}

		// Blit the selection marker.
		if (field.fcoords == get_sel_pos().node) {
			const Image* pic = get_sel_picture();
			blit_overlay(pic, Vector2i(pic->width() / 2, pic->height() / 2));
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

// Toolbar button callback functions.
void InteractiveSpectator::exit_btn() {
	if (is_multiplayer()) {
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

bool InteractiveSpectator::can_see(Widelands::PlayerNumber) const {
	return true;
}
bool InteractiveSpectator::can_act(Widelands::PlayerNumber) const {
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
	if (is_a(Widelands::Building, egbase().map().get_immovable(node_and_triangle.node))) {
		show_building_window(node_and_triangle.node, false);
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
	if (down)
		switch (code.sym) {
		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_m:
			minimap_registry().toggle();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				new GameMainMenuSaveGame(*this, main_windows_.savegame);
			} else
				set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (chat_provider_) {
				if (!chat_.window) {
					GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
				}
				dynamic_cast<GameChatMenu*>(chat_.window)->enter_chat_message();
			}
			return true;
		default:
			break;
		}

	return InteractiveGameBase::handle_key(down, code);
}
