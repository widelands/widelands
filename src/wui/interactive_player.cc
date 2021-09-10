/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "wui/interactive_player.h"

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/multithreading.h"
#include "economy/flag.h"
#include "game_io/game_loader.h"
#include "graphic/game_renderer.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text_layout.h"
#include "logic/cmd_queue.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "ui_basic/unique_window.h"
#include "wlapplication_options.h"
#include "wui/building_statistics_menu.h"
#include "wui/debugconsole.h"
#include "wui/fieldaction.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/seafaring_statistics_menu.h"
#include "wui/soldier_statistics_menu.h"
#include "wui/stock_menu.h"
#include "wui/toolbar.h"
#include "wui/tribal_encyclopedia.h"
#include "wui/ware_statistics_menu.h"

using Widelands::Building;
using Widelands::Map;

namespace {

// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'pf'. See 'field_brightness' in fields_to_draw.cc for scale of values.
float adjusted_field_brightness(const Widelands::FCoords& fcoords,
                                const Time& gametime,
                                const Widelands::Player::Field& pf) {
	if (pf.vision == Widelands::VisibleState::kUnexplored) {
		return 0.;
	}

	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (pf.vision == Widelands::VisibleState::kPreviouslySeen) {
		static const Duration kDecayTimeInMs = Duration(20000);
		const Duration time_ago = gametime - pf.time_node_last_unseen;
		if (time_ago < kDecayTimeInMs) {
			brightness =
			   (brightness * (2 * kDecayTimeInMs.get() - time_ago.get())) / (2 * kDecayTimeInMs.get());
		} else {
			brightness = brightness / 2;
		}
	}
	return brightness / 255.;
}
// Remove statistics from the text to draw if the player does not match the map object's owner
InfoToDraw filter_info_to_draw(InfoToDraw info_to_draw,
                               const Widelands::MapObject* object,
                               const Widelands::Player& player) {
	InfoToDraw result = info_to_draw;
	const Widelands::Player* owner = object->get_owner();
	if (owner != nullptr && !player.see_all() && player.is_hostile(*owner)) {
		result = static_cast<InfoToDraw>(result & ~InfoToDraw::kStatistics);
	}
	return result;
}

void draw_bobs_for_visible_field(const Widelands::EditorGameBase& egbase,
                                 const FieldsToDraw::Field& field,
                                 const float scale,
                                 const InfoToDraw info_to_draw,
                                 const Widelands::Player& player,
                                 RenderTarget* dst) {
	for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob;
	     bob = bob->get_next_bob()) {
		bob->draw(egbase, filter_info_to_draw(info_to_draw, bob, player), field.rendertarget_pixel,
		          field.fcoords, scale, dst);
	}
}

void draw_immovable_for_formerly_visible_field(const FieldsToDraw::Field& field,
                                               const InfoToDraw info_to_draw,
                                               const Widelands::Player::Field& player_field,
                                               const float scale,
                                               RenderTarget* dst) {
	if (player_field.map_object_descr == nullptr) {
		return;
	}

	if (upcast(const Widelands::BuildingDescr, building, player_field.map_object_descr)) {
		assert(field.owner != nullptr);
		// this is a building therefore we either draw unoccupied or idle animation
		if (building->type() == Widelands::MapObjectType::CONSTRUCTIONSITE) {
			player_field.constructionsite->draw(field.rendertarget_pixel, field.fcoords, scale,
			                                    (info_to_draw & InfoToDraw::kShowBuildings),
			                                    field.owner->get_playercolor(), dst);
		} else {
			const RGBColor* player_color;
			float opacity;
			if (info_to_draw & InfoToDraw::kShowBuildings) {
				player_color = &field.owner->get_playercolor();
				opacity = 1.0f;
			} else {
				player_color = nullptr;
				opacity = Widelands::kBuildingSilhouetteOpacity;
			}
			if (building->type() == Widelands::MapObjectType::DISMANTLESITE &&
			    // TODO(Nordfriese): `building` can only be nullptr in savegame
			    // compatibility cases – remove that check after v1.0
			    player_field.dismantlesite.building) {
				dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale,
				                    player_field.dismantlesite.building->get_unoccupied_animation(),
				                    Time(0), player_color, opacity,
				                    100 - ((player_field.dismantlesite.progress * 100) >> 16));
			} else {
				dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale,
				                    building->get_unoccupied_animation(), Time(0), player_color,
				                    opacity);
			}
		}
	} else if (player_field.map_object_descr->type() == Widelands::MapObjectType::FLAG) {
		assert(field.owner != nullptr);
		dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale,
		                    field.owner->tribe().flag_animation(), Time(0),
		                    &field.owner->get_playercolor());
	} else if (const uint32_t pic = player_field.map_object_descr->main_animation()) {
		dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale, pic, Time(0),
		                    (field.owner == nullptr) ? nullptr : &field.owner->get_playercolor());
	}
}

}  // namespace

InteractivePlayer::InteractivePlayer(Widelands::Game& g,
                                     Section& global_s,
                                     Widelands::PlayerNumber const plyn,
                                     bool const multiplayer,
                                     ChatProvider* chat_provider)
   : InteractiveGameBase(g, global_s, multiplayer, chat_provider),
     auto_roadbuild_mode_(global_s.get_bool("auto_roadbuild_mode", true)),
     flag_to_connect_(Widelands::Coords::null()),
     statisticsmenu_(toolbar(),
                     "dropdown_menu_statistics",
                     0,
                     0,
                     MainToolbar::kButtonSize,
                     10,
                     MainToolbar::kButtonSize,
                     /** TRANSLATORS: Title for the statistics menu button in the game */
                     _("Statistics"),
                     UI::DropdownType::kPictorialMenu,
                     UI::PanelStyle::kWui,
                     UI::ButtonStyle::kWuiPrimary),
     grid_marker_pic_(g_image_cache->get("images/wui/overlays/grid_marker.png"))
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
     , training_wheel_indicator_pic_(g_image_cache->get("images/wui/training_wheels_arrow.png")),
     training_wheel_indicator_field_(Widelands::FCoords::null(), nullptr)
#endif
{
	add_main_menu();

	toolbar()->add_space(15);

	add_mapview_menu(MiniMapType::kStaticViewWindow);
	add_showhide_menu();
	add_gamespeed_menu();

	toolbar()->add_space(15);
	if (multiplayer) {
		add_chat_ui();
		toolbar()->add_space(15);
	}

	add_statistics_menu();

	add_toolbar_button("wui/menus/objectives", "objectives",
	                   as_tooltip_text_with_hotkey(
	                      _("Objectives"), shortcut_string_for(KeyboardShortcut::kInGameObjectives),
	                      UI::PanelStyle::kWui),
	                   &objectives_, true);
	objectives_.open_window = [this] { new GameObjectivesMenu(this, objectives_); };

	toggle_message_menu_ =
	   add_toolbar_button("wui/menus/message_old", "messages",
	                      as_tooltip_text_with_hotkey(
	                         _("Messages"), shortcut_string_for(KeyboardShortcut::kInGameMessages),
	                         UI::PanelStyle::kWui),
	                      &message_menu_, true);
	message_menu_.open_window = [this] { new GameMessageMenu(*this, message_menu_); };

	toolbar()->add_space(15);

	add_toolbar_button("ui_basic/menu_help", "help",
	                   as_tooltip_text_with_hotkey(
	                      _("Help"), shortcut_string_for(KeyboardShortcut::kCommonEncyclopedia),
	                      UI::PanelStyle::kWui),
	                   &encyclopedia_, true);
	encyclopedia_.open_window = [this] {
		new TribalEncyclopedia(*this, encyclopedia_, &game().lua());
	};

	set_player_number(plyn);
	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		node_action(node_and_triangle);
	});

	finalize_toolbar();

#ifndef NDEBUG  //  only in debug builds
	addCommand(
	   "switchplayer", [this](const std::vector<std::string>& str) { cmdSwitchPlayer(str); });
#endif

	map_options_subscriber_ = Notifications::subscribe<NoteMapOptions>(
	   [this](const NoteMapOptions&) { rebuild_statistics_menu(); });
	shipnotes_subscriber_ =
	   Notifications::subscribe<Widelands::NoteShip>([this](const Widelands::NoteShip& note) {
		   if (note.ship->owner().player_number() == player_number() &&
		       note.action == Widelands::NoteShip::Action::kWaitingForCommand &&
		       note.ship->get_ship_state() ==
		          Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
			   expedition_port_spaces_.emplace(note.ship, note.ship->exp_port_spaces().front());
		   }
	   });

	initialization_complete();
}

void InteractivePlayer::add_statistics_menu() {
	statisticsmenu_.set_image(g_image_cache->get("images/wui/menus/statistics.png"));
	toolbar()->add(&statisticsmenu_);

	menu_windows_.stats_seafaring.open_window = [this] {
		new SeafaringStatisticsMenu(*this, menu_windows_.stats_seafaring);
	};

	menu_windows_.stats_stock.open_window = [this] {
		new StockMenu(*this, menu_windows_.stats_stock);
	};

	menu_windows_.stats_buildings.open_window = [this] {
		new BuildingStatisticsMenu(*this, menu_windows_.stats_buildings);
	};

	menu_windows_.stats_soldiers.open_window = [this] {
		new SoldierStatisticsMenu(*this, menu_windows_.stats_soldiers);
	};

	menu_windows_.stats_wares.open_window = [this] {
		new WareStatisticsMenu(*this, menu_windows_.stats_wares);
	};

	menu_windows_.stats_general.open_window = [this] {
		new GeneralStatisticsMenu(*this, menu_windows_.stats_general);
	};

	// NoteMapOptions takes care of the rebuilding

	statisticsmenu_.selected.connect(
	   [this] { statistics_menu_selected(statisticsmenu_.get_selected()); });
}

void InteractivePlayer::rebuild_statistics_menu() {
	const StatisticsMenuEntry last_selection = statisticsmenu_.has_selection() ?
                                                 statisticsmenu_.get_selected() :
                                                 StatisticsMenuEntry::kSoldiers;

	statisticsmenu_.clear();

	if (egbase().map().allows_seafaring()) {
		/** TRANSLATORS: An entry in the game's statistics menu */
		statisticsmenu_.add(_("Seafaring"), StatisticsMenuEntry::kSeafaring,
		                    g_image_cache->get("images/wui/menus/statistics_seafaring.png"), false,
		                    "", shortcut_string_for(KeyboardShortcut::kInGameStatsSeafaring));
	}

	/** TRANSLATORS: An entry in the game's statistics menu */
	statisticsmenu_.add(_("Soldiers"), StatisticsMenuEntry::kSoldiers,
	                    g_image_cache->get("images/wui/menus/toggle_soldier_levels.png"), false, "",
	                    shortcut_string_for(KeyboardShortcut::kInGameStatsSoldiers));

	/** TRANSLATORS: An entry in the game's statistics menu */
	statisticsmenu_.add(_("Stock"), StatisticsMenuEntry::kStock,
	                    g_image_cache->get("images/wui/menus/statistics_stock.png"), false, "",
	                    shortcut_string_for(KeyboardShortcut::kInGameStatsStock));

	/** TRANSLATORS: An entry in the game's statistics menu */
	statisticsmenu_.add(_("Buildings"), StatisticsMenuEntry::kBuildings,
	                    g_image_cache->get("images/wui/menus/statistics_buildings.png"), false, "",
	                    shortcut_string_for(KeyboardShortcut::kInGameStatsBuildings));

	/** TRANSLATORS: An entry in the game's statistics menu */
	statisticsmenu_.add(_("Wares"), StatisticsMenuEntry::kWare,
	                    g_image_cache->get("images/wui/menus/statistics_wares.png"), false, "",
	                    shortcut_string_for(KeyboardShortcut::kInGameStatsWares));

	/** TRANSLATORS: An entry in the game's statistics menu */
	statisticsmenu_.add(_("General"), StatisticsMenuEntry::kGeneral,
	                    g_image_cache->get("images/wui/menus/statistics_general.png"), false, "",
	                    shortcut_string_for(KeyboardShortcut::kInGameStatsGeneral));

	statisticsmenu_.select(last_selection);
}

void InteractivePlayer::statistics_menu_selected(StatisticsMenuEntry entry) {
	switch (entry) {
	case StatisticsMenuEntry::kGeneral: {
		menu_windows_.stats_general.toggle();
	} break;
	case StatisticsMenuEntry::kWare: {
		menu_windows_.stats_wares.toggle();
	} break;
	case StatisticsMenuEntry::kBuildings: {
		menu_windows_.stats_buildings.toggle();
	} break;
	case StatisticsMenuEntry::kSoldiers: {
		menu_windows_.stats_soldiers.toggle();
	} break;
	case StatisticsMenuEntry::kStock: {
		menu_windows_.stats_stock.toggle();
	} break;
	case StatisticsMenuEntry::kSeafaring: {
		if (egbase().map().allows_seafaring()) {
			menu_windows_.stats_seafaring.toggle();
		}
	} break;
	}
	statisticsmenu_.toggle();
}

void InteractivePlayer::rebuild_showhide_menu() {
	const ShowHideEntry last_selection =
	   showhidemenu_.has_selection() ? showhidemenu_.get_selected() : ShowHideEntry::kBuildingSpaces;

	InteractiveGameBase::rebuild_showhide_menu();

	showhidemenu_.add(
	   get_display_flag(dfShowWorkareaOverlap) ?
          /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether workarea overlaps
           * are highlighted */
          _("Hide Workarea Overlaps") :
          /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether workarea overlaps
           * are highlighted */
          _("Show Workarea Overlaps"),
	   ShowHideEntry::kWorkareaOverlap,
	   g_image_cache->get("images/wui/menus/show_workarea_overlap.png"), false,
	   _("Toggle whether overlapping workareas are indicated when placing a constructionsite"),
	   shortcut_string_for(KeyboardShortcut::kInGameShowhideWorkareas));

	showhidemenu_.select(last_selection);
}

bool InteractivePlayer::has_expedition_port_space(const Widelands::Coords& coords) const {
	for (const auto& pair : expedition_port_spaces_) {
		if (pair.second == coords) {
			return true;
		}
	}
	return false;
}

void InteractivePlayer::draw_immovables_for_visible_field(
   const Widelands::EditorGameBase& egbase,
   const FieldsToDraw::Field& field,
   const float scale,
   const InfoToDraw info_to_draw,
   const Widelands::Player& player,
   RenderTarget* dst,
   std::set<Widelands::Coords>& deferred_coords) {
	MutexLock m(MutexLock::ID::kObjects);

	Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
	if (imm == nullptr) {
		return;
	}
	if (imm->get_positions(egbase).front() == field.fcoords) {
		imm->draw(egbase.get_gametime(), filter_info_to_draw(info_to_draw, imm, player),
		          field.rendertarget_pixel, field.fcoords, scale, dst);
		if (upcast(const Widelands::Immovable, i, imm)) {
			if (i->is_marked_for_removal(player_number())) {
				const Image* img = g_image_cache->get("images/wui/overlays/targeted.png");
				blit_field_overlay(dst, field, img, Vector2i(img->width() / 2, img->height()), scale);
			}
		}
	} else {
		// This is not the building's main position so we can't draw it now.
		// We remember it so we can draw it later.
		deferred_coords.insert(imm->get_positions(egbase).front());
	}
}

void InteractivePlayer::think() {
	InteractiveBase::think();

	if (player().is_picking_custom_starting_position()) {
		set_sel_picture(
		   playercolor_image(player().get_playercolor(), "images/players/player_position_menu.png"));
	}

	if (flag_to_connect_) {
		Widelands::Field& field = egbase().map()[flag_to_connect_];
		if (upcast(Widelands::Flag const, flag, field.get_immovable())) {
			if (!flag->has_road() && !in_road_building_mode()) {
				if (auto_roadbuild_mode_) {
					//  There might be a fieldaction window open, showing a button
					//  for roadbuilding. If that dialog remains open so that the
					//  button is clicked, we would enter roadbuilding mode while
					//  we are already in roadbuilding mode from the call below.
					//  That is not allowed. Therefore we must delete the
					//  fieldaction window before entering roadbuilding mode here.
					fieldaction_.destroy();
					map_view()->mouse_to_field(flag_to_connect_, MapView::Transition::Jump);
					set_sel_pos(Widelands::NodeAndTriangle<>{
					   flag_to_connect_,
					   Widelands::TCoords<>(flag_to_connect_, Widelands::TriangleIndex::D)});
					start_build_road(flag_to_connect_, field.get_owned_by(), RoadBuildingType::kRoad);
				}
			}
			flag_to_connect_ = Widelands::Coords::null();
		}
	}
	{
		char const* msg_icon = "images/wui/menus/message_old.png";
		std::string msg_tooltip = _("Messages");
		if (uint32_t const nr_new_messages =
		       player().messages().nr_messages(Widelands::Message::Status::kNew)) {
			msg_icon = "images/wui/menus/message_new.png";
			msg_tooltip =
			   (boost::format(ngettext("%u new message", "%u new messages", nr_new_messages)) %
			    nr_new_messages)
			      .str();
		}
		toggle_message_menu_->set_pic(g_image_cache->get(msg_icon));
		toggle_message_menu_->set_tooltip(as_tooltip_text_with_hotkey(
		   msg_tooltip, shortcut_string_for(KeyboardShortcut::kInGameMessages),
		   UI::PanelStyle::kWui));
	}

	// Cleanup found port spaces if the ship sailed on or was destroyed
	for (auto it = expedition_port_spaces_.begin(); it != expedition_port_spaces_.end(); ++it) {
		Widelands::Ship* ship = it->first.get(egbase());
		if (!ship ||
		    ship->get_ship_state() != Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
			expedition_port_spaces_.erase(it);
			// If another port space also needs removing, we'll take care of it in the next frame
			return;
		}
	}
}

void InteractivePlayer::draw(RenderTarget& dst) {
	// Bail out if the game isn't actually loaded.
	// This fixes a crash with displaying an error dialog during loading.
	if (!game().is_loaded()) {
		return;
	}

	draw_map_view(map_view(), &dst);
}

constexpr float kBuildhelpOpacity = 0.3f;

void InteractivePlayer::draw_map_view(MapView* given_map_view, RenderTarget* dst) {
	// In-game, selection can never be on triangles or have a radius.
	assert(get_sel_radius() == 0);
	assert(!get_sel_triangles());

	const Widelands::Player& plr = player();
	const Widelands::EditorGameBase& gbase = egbase();
	const Widelands::Map& map = gbase.map();
	const Time& gametime = gbase.get_gametime();

	Workareas workareas = get_workarea_overlays(map);
	FieldsToDraw* fields_to_draw = given_map_view->draw_terrain(gbase, &plr, workareas, false, dst);
	const auto& road_building_s = road_building_steepness_overlays();
	const bool picking_starting_pos = plr.is_picking_custom_starting_position();

	const float scale = 1.f / given_map_view->view().zoom;

	// Store the coords of partially visible buildings
	// so we can draw them later when we get to their main position.
	std::set<Widelands::Coords> deferred_coords;

	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		FieldsToDraw::Field* f = fields_to_draw->mutable_field(idx);

		const Widelands::Player::Field& player_field =
		   plr.fields()[map.get_index(f->fcoords, map.get_width())];

		// Adjust this field for visibility for this player.
		if (!plr.see_all()) {
			f->brightness = adjusted_field_brightness(f->fcoords, gametime, player_field);
			f->road_e = player_field.r_e;
			f->road_se = player_field.r_se;
			f->road_sw = player_field.r_sw;
			f->seeing = player_field.vision;
			if (player_field.vision == Widelands::VisibleState::kPreviouslySeen) {
				f->owner = player_field.owner != 0 ? gbase.get_player(player_field.owner) : nullptr;
				f->is_border = player_field.border;
			}
		}

		// Add road building overlays if applicable.
		if (f->seeing != Widelands::VisibleState::kUnexplored) {
			draw_road_building(*f);

			draw_bridges(
			   dst, f, f->seeing == Widelands::VisibleState::kVisible ? gametime : Time(0), scale);
			draw_border_markers(*f, scale, *fields_to_draw, dst);

			// Draw immovables and bobs.
			const InfoToDraw info_to_draw = get_info_to_draw(!given_map_view->is_animating());

			if (f->seeing == Widelands::VisibleState::kVisible) {
				draw_immovables_for_visible_field(
				   gbase, *f, scale, info_to_draw, plr, dst, deferred_coords);
				draw_bobs_for_visible_field(gbase, *f, scale, info_to_draw, plr, dst);
			} else if (deferred_coords.count(f->fcoords) > 0) {
				// This is the main position of a building that is visible on another field
				// so although this field isn't visible we draw the building as if it was.
				draw_immovables_for_visible_field(
				   gbase, *f, scale, info_to_draw, plr, dst, deferred_coords);
			} else {
				// We never show census or statistics for objects in the fog.
				draw_immovable_for_formerly_visible_field(*f, info_to_draw, player_field, scale, dst);
			}
		}

		// Draw the player starting position overlays.
		const bool suited_as_starting_pos =
		   picking_starting_pos && plr.get_starting_position_suitability(f->fcoords);
		if (suited_as_starting_pos) {
			for (unsigned p = map.get_nrplayers(); p; --p) {
				if (map.get_starting_pos(p) == f->fcoords) {
					const Image* player_image =
					   playercolor_image(p - 1, "images/players/player_position.png");
					static constexpr int kStartingPosHotspotY = 55;
					blit_field_overlay(dst, *f, player_image,
					                   Vector2i(player_image->width() / 2, kStartingPosHotspotY), scale);
					break;
				}
			}
		}

		// Draw work area markers.
		if (has_workarea_preview(f->fcoords, &map)) {
			blit_field_overlay(dst, *f, grid_marker_pic_,
			                   Vector2i(grid_marker_pic_->width() / 2, grid_marker_pic_->height() / 2),
			                   scale);
		}

		if (f->seeing != Widelands::VisibleState::kUnexplored) {
			// Draw build help.
			const bool show_port_space = has_expedition_port_space(f->fcoords);
			if (show_port_space || suited_as_starting_pos || buildhelp()) {
				Widelands::NodeCaps caps;
				float opacity =
				   f->seeing == Widelands::VisibleState::kVisible ? 1.f : kBuildhelpOpacity;
				if (picking_starting_pos) {
					caps = suited_as_starting_pos || buildhelp() ? f->fcoords.field->nodecaps() :
                                                              Widelands::CAPS_NONE;
				} else if (show_port_space) {
					caps = f->fcoords.field->maxcaps();
				} else {
					caps = plr.get_buildcaps(f->fcoords);
					if (!(caps & Widelands::BUILDCAPS_SIZEMASK)) {
						for (const Widelands::BuildingDescr* b :
						     plr.tribe().buildings_built_over_immovables()) {
							if (plr.check_can_build(*b, f->fcoords)) {
								caps = f->fcoords.field->maxcaps();
								opacity *= 2 * kBuildhelpOpacity;
								break;
							}
						}
					}
				}

				if (const auto* overlay = get_buildhelp_overlay(caps)) {
					blit_field_overlay(dst, *f, overlay->pic, overlay->hotspot, scale, opacity);
				}
			}

			// Blit the selection marker.
			if (g_mouse_cursor->is_visible() && f->fcoords == get_sel_pos().node) {
				const Image* pic = get_sel_picture();
				blit_field_overlay(dst, *f, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
			}

			// Draw road building slopes.
			{
				const auto itb = road_building_s.find(f->fcoords);
				if (itb != road_building_s.end()) {
					blit_field_overlay(dst, *f, itb->second,
					                   Vector2i(itb->second->width() / 2, itb->second->height() / 2),
					                   scale);
				}
			}
		}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
		// Blit arrow for training wheel instructions
		if (training_wheel_indicator_field_ == f->fcoords) {
			constexpr int kTrainingWheelArrowOffset = 5;
			blit_field_overlay(
			   dst, *f, training_wheel_indicator_pic_,
			   Vector2i(-kTrainingWheelArrowOffset,
			            training_wheel_indicator_pic_->height() + kTrainingWheelArrowOffset),
			   scale);
		}
#endif
	}
}

void InteractivePlayer::popup_message(Widelands::MessageId const id,
                                      const Widelands::Message& message) {
	// Fix a race condition that happens only in the testsuite
	MutexLock m(MutexLock::ID::kObjects);

	message_menu_.create();
	dynamic_cast<GameMessageMenu&>(*message_menu_.window).show_new_message(id, message);
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
void InteractivePlayer::set_training_wheel_indicator_pos(const Vector2i& pos) {
	constexpr int kTrainingWheelArrowOffset = 5;
	if (pos == Vector2i::invalid()) {
		training_wheel_indicator_icon_.reset(nullptr);
	} else {
		// We create a new icon every time to paint it on top of the other child panels
		training_wheel_indicator_icon_.reset(
		   new UI::Icon(this, UI::PanelStyle::kWui, training_wheel_indicator_pic_));
		training_wheel_indicator_icon_->set_no_frame();
		training_wheel_indicator_icon_->set_pos(
		   Vector2i(pos.x - kTrainingWheelArrowOffset,
		            pos.y - training_wheel_indicator_icon_->get_h() + kTrainingWheelArrowOffset));
	}
}
void InteractivePlayer::set_training_wheel_indicator_field(const Widelands::FCoords& field) {
	training_wheel_indicator_field_ = field;
}
#endif

bool InteractivePlayer::can_see(Widelands::PlayerNumber const p) const {
	return p == player_number() || player().see_all();
}
bool InteractivePlayer::can_act(Widelands::PlayerNumber const p) const {
	return p == player_number();
}
Widelands::PlayerNumber InteractivePlayer::player_number() const {
	return player_number_;
}

/// Player has clicked on the given node; bring up the context menu.
void InteractivePlayer::node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) {
	if (player().is_picking_custom_starting_position()) {
		if (get_player()->pick_custom_starting_position(node_and_triangle.node)) {
			unset_sel_picture();
		}
		return;
	}

	const Map& map = egbase().map();
	if (player().is_seeing(Map::get_index(node_and_triangle.node, map.get_width()))) {
		// Special case for buildings
		if (upcast(Building, building, map.get_immovable(node_and_triangle.node))) {
			if (can_see(building->owner().player_number())) {
				show_building_window(node_and_triangle.node, false, false);
				return;
			}
		}

		if (!in_road_building_mode()) {
			if (try_show_ship_window()) {
				return;
			}
		}

		// everything else can bring up the temporary dialog
		show_field_action(this, get_player(), &fieldaction_);
	}
}

/**
 * Global in-game keypresses:
 * \li Space: toggles buildhelp
 * \li i: show stock (inventory)
 * \li m: show minimap
 * \li o: show objectives window
 * \li c: toggle census
 * \li s: toggle building statistics
 * \li Home: go to starting position
 * \li PageUp/PageDown: change game speed
 * \li Pause: pauses the game
 * \li Return: write chat message
 */
bool InteractivePlayer::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonEncyclopedia, code)) {
			encyclopedia_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameStatsStock, code)) {
			menu_windows_.stats_stock.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameStatsWares, code)) {
			menu_windows_.stats_wares.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessages, code)) {
			message_menu_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameObjectives, code)) {
			objectives_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameStatsBuildings, code)) {
			if (menu_windows_.stats_buildings.window == nullptr) {
				new BuildingStatisticsMenu(*this, menu_windows_.stats_buildings);
			} else {
				menu_windows_.stats_buildings.toggle();
			}
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameStatsSoldiers, code)) {
			if (menu_windows_.stats_soldiers.window == nullptr) {
				new SoldierStatisticsMenu(*this, menu_windows_.stats_soldiers);
			} else {
				menu_windows_.stats_soldiers.toggle();
			}
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameStatsSeafaring, code) &&
		    game().map().allows_seafaring()) {
			if (menu_windows_.stats_seafaring.window == nullptr) {
				new SeafaringStatisticsMenu(*this, menu_windows_.stats_seafaring);
			} else {
				menu_windows_.stats_seafaring.toggle();
			}
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameShowhideWorkareas, code)) {
			set_display_flag(dfShowWorkareaOverlap, !get_display_flag(dfShowWorkareaOverlap));
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameScrollToHQ, code) ||
		    (get_config_bool("numpad_diagonalscrolling", false) && code.sym == SDLK_KP_5 &&
		     matches_keymod(code.mod, KMOD_NONE))) {
			map_view()->scroll_to_field(
			   game().map().get_starting_pos(player_number_), MapView::Transition::Smooth);
			return true;
		}

		const Widelands::DescriptionIndex fastplace =
		   egbase().descriptions().building_index(matching_fastplace_shortcut(code));
		if (player().tribe().has_building(fastplace)) {
			game().send_player_build(player_number(), get_sel_pos().node, fastplace);
			set_flag_to_connect(game().map().br_n(get_sel_pos().node));
		}
	}

	return InteractiveGameBase::handle_key(down, code);
}

/**
 * Set the player and the visibility to this
 * player
 */
void InteractivePlayer::set_player_number(uint32_t const n) {
	player_number_ = n;
}

void InteractivePlayer::postload() {
	InteractiveGameBase::postload();

	ToolbarImageset* imageset = player().tribe().toolbar_image_set();
	if (imageset != nullptr) {
		set_toolbar_imageset(*imageset);
	}
}

bool InteractivePlayer::player_hears_field(const Widelands::Coords& coords) const {
	const Widelands::Player& plr = player();
	if (plr.see_all()) {
		return true;
	}
	const Widelands::Map& map = egbase().map();
	const Widelands::Player::Field& player_field =
	   plr.fields()[map.get_index(coords, map.get_width())];
	return player_field.vision == Widelands::VisibleState::kVisible;
}

void InteractivePlayer::cmdSwitchPlayer(const std::vector<std::string>& args) {
	if (args.size() != 2) {
		DebugConsole::write("Usage: switchplayer <nr>");
		return;
	}

	int const n = boost::lexical_cast<int>(args[1]);
	if (n < 1 || n > kMaxPlayers || !game().get_player(n)) {
		DebugConsole::write(str(boost::format("Player #%1% does not exist.") % n));
		return;
	}

	DebugConsole::write(
	   str(boost::format("Switching from #%1% to #%2%.") % static_cast<int>(player_number_) % n));
	player_number_ = n;

	if (UI::UniqueWindow* const building_statistics_window = menu_windows_.stats_buildings.window) {
		dynamic_cast<BuildingStatisticsMenu&>(*building_statistics_window).update();
	}
	menu_windows_.stats_soldiers.destroy();
}
