/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "graphic/game_renderer.h"

#include <memory>

#include "graphic/gl/coordinate_conversion.h"
#include "graphic/graphic.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "wui/edge_overlay_manager.h"
#include "wui/field_overlay_manager.h"
#include "wui/interactive_base.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

/*
 * Explanation of how drawing works:
 * Schematic of triangle neighborhood:
 *
 *               *
 *              / \
 *             / u \
 *         (f)/     \
 *    *------*------* (r)
 *     \  l / \  r / \
 *      \  /   \  /   \
 *       \/  d  \/ rr  \
 *       *------*------* (br)
 *        \ dd /
 *         \  /
 *          \/
 *          *
 *
 * Each field (f) owns two triangles: (r)ight & (d)own. When we look at the
 * field, we have to make sure to schedule drawing the triangles. This is done
 * by TerrainProgram.
 *
 * To draw dithered edges, we have to look at the neighboring triangles for the
 * two triangles too: If a neighboring triangle has another texture and our
 * dither layer is smaller, we have to draw a dithering triangle too - this lets
 * the neighboring texture bleed into our triangle.
 *
 * The dither triangle is the triangle that should be partially drawn (either r or
 * d). Example: if r and d have different textures and r.dither_layer >
 * d.dither_layer, then we will repaint d with the dither texture as mask.
 */

namespace {

using namespace Widelands;

// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'player' (which can be nullptr).
float field_brightness(const FCoords& fcoords,
                       const uint32_t gametime,
                       const Map& map,
                       const Player* const player) {
	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(fcoords, map.get_width())];
		if (pf.vision == 0) {
			return 0.;
		} else if (pf.vision == 1) {
			static const uint32_t kDecayTimeInMs = 20000;
			const Duration time_ago = gametime - pf.time_node_last_unseen;
			if (time_ago < kDecayTimeInMs) {
				brightness = (brightness * (2 * kDecayTimeInMs - time_ago)) / (2 * kDecayTimeInMs);
			} else {
				brightness = brightness / 2;
			}
		}
	}
	return brightness / 255.;
}

// Returns the road that should be rendered here. The format is like in field,
// but this is not the physically present road, but the one that should be
// drawn (i.e. taking into account if there is fog of war involved or road
// building overlays enabled).
uint8_t field_roads(const FCoords& coords,
                    const Map& map,
                    const EdgeOverlayManager& edge_overlay_manager,
                    const Player* const player) {
	uint8_t roads;
	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | edge_overlay_manager.get_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= edge_overlay_manager.get_overlay(coords);
	return roads;
}

// Draws the objects (animations & overlays).
void draw_objects(const EditorGameBase& egbase,
                  const Transform2f& mappixel_to_screen,
                  const Player* player,
                  const int minfx,
                  const int maxfx,
                  const int minfy,
                  const int maxfy,
                  RenderTarget* dst) {
	// TODO(sirver): this should use FieldsToDraw. Would simplify this function a lot.
	static const uint32_t F = 0;
	static const uint32_t R = 1;
	static const uint32_t BL = 2;
	static const uint32_t BR = 3;
	const Map& map = egbase.map();

	const float zoom = mappixel_to_screen.zoom();
	std::vector<FieldOverlayManager::OverlayInfo> overlay_info;
	for (int32_t fy = minfy; fy <= maxfy; ++fy) {
		for (int32_t fx = minfx; fx <= maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords coords[4];
			coords[F] = map.get_fcoords(ncoords);
			coords[R] = map.r_n(coords[F]);
			coords[BL] = map.bl_n(coords[F]);
			coords[BR] = map.br_n(coords[F]);
			FloatPoint pos[4] = {
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(Coords(fx, fy)),
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(Coords(fx + 1, fy)),
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(Coords(fx + (fy & 1) - 1, fy + 1)),
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(Coords(fx + (fy & 1), fy + 1)),
			};
			for (uint32_t i = 0; i < 4; ++i) {
				pos[i].y -= coords[i].field->get_height() * kHeightFactor;
				pos[i] = mappixel_to_screen.apply(pos[i]);
			}

			PlayerNumber owner_number[4];
			bool isborder[4];
			Vision vision[4] = {2, 2, 2, 2};
			for (uint32_t d = 0; d < 4; ++d)
				owner_number[d] = coords[d].field->get_owned_by();
			for (uint32_t d = 0; d < 4; ++d)
				isborder[d] = coords[d].field->is_border();

			if (player && !player->see_all()) {
				for (uint32_t d = 0; d < 4; ++d) {
					const Player::Field& pf =
					   player->fields()[map.get_index(coords[d], map.get_width())];
					vision[d] = pf.vision;
					if (pf.vision == 1) {
						owner_number[d] = pf.owner;
						isborder[d] = pf.border;
					}
				}
			}

			if (isborder[F]) {
				const Player& owner = egbase.player(owner_number[F]);
				uint32_t const anim_idx = owner.tribe().frontier_animation();
				if (vision[F])
					dst->blit_animation(pos[F], zoom, anim_idx, 0, owner.get_playercolor());
				for (uint32_t d = 1; d < 4; ++d) {
					if ((vision[F] || vision[d]) && isborder[d] &&
					    (owner_number[d] == owner_number[F] || !owner_number[d])) {
						dst->blit_animation(middle(pos[F], pos[d]), zoom, anim_idx, 0, owner.get_playercolor());
					}
				}
			}

			// NOCOM(#sirver): figure out census and statistics here.
			if (1 < vision[F]) {  // Render stuff that belongs to the node.
				if (BaseImmovable* const imm = coords[F].field->get_immovable()) {
					imm->draw(egbase.get_gametime(), BaseImmovable::ShowText::kNone, coords[F], pos[F],
					          zoom, dst);
				}
				for (Bob* bob = coords[F].field->get_first_bob(); bob; bob = bob->get_next_bob()) {
					bob->draw(egbase, pos[F], zoom, dst);
				}
			} else if (vision[F] == 1) {
				const Player::Field& f_pl = player->fields()[map.get_index(coords[F], map.get_width())];
				const Player* owner = owner_number[F] ? egbase.get_player(owner_number[F]) : nullptr;
				if (const MapObjectDescr* const map_object_descr =
				       f_pl.map_object_descr[TCoords<>::None]) {
					if (f_pl.constructionsite.becomes) {
						assert(owner != nullptr);
						const ConstructionsiteInformation& csinf = f_pl.constructionsite;
						// draw the partly finished constructionsite
						uint32_t anim_idx;
						try {
							anim_idx = csinf.becomes->get_animation("build");
						} catch (MapObjectDescr::AnimationNonexistent&) {
							try {
								anim_idx = csinf.becomes->get_animation("unoccupied");
							} catch (MapObjectDescr::AnimationNonexistent) {
								anim_idx = csinf.becomes->get_animation("idle");
							}
						}
						const Animation& anim = g_gr->animations().get_animation(anim_idx);
						const size_t nr_frames = anim.nr_frames();
						uint32_t cur_frame =
						   csinf.totaltime ? csinf.completedtime * nr_frames / csinf.totaltime : 0;
						uint32_t tanim = cur_frame * FRAME_LENGTH;

						const uint16_t w = anim.width();
						const uint16_t h = anim.height();
						uint32_t lines = h * csinf.completedtime * nr_frames;
						if (csinf.totaltime)
							lines /= csinf.totaltime;
						assert(h * cur_frame <= lines);
						lines -= h * cur_frame;

						if (cur_frame) {  // not the first frame
							// draw the prev frame from top to where next image will be drawing
							dst->blit_animation(pos[F], zoom, anim_idx, tanim - FRAME_LENGTH,
							                   owner->get_playercolor(), Rect(Point(0, 0), w, h - lines));
						} else if (csinf.was) {
							// Is the first frame, but there was another building here before,
							// get its last build picture and draw it instead.
							uint32_t a;
							try {
								a = csinf.was->get_animation("unoccupied");
							} catch (MapObjectDescr::AnimationNonexistent&) {
								a = csinf.was->get_animation("idle");
							}
							dst->blit_animation(pos[F], zoom, a, tanim - FRAME_LENGTH, owner->get_playercolor(),
							                   Rect(Point(0, 0), w, h - lines));
						}
						assert(lines <= h);
						dst->blit_animation(pos[F], zoom, anim_idx, tanim, owner->get_playercolor(),
						                   Rect(Point(0, h - lines), w, lines));
					} else if (upcast(const BuildingDescr, building, map_object_descr)) {
						assert(owner != nullptr);
						// this is a building therefore we either draw unoccupied or idle animation
						uint32_t pic;
						try {
							pic = building->get_animation("unoccupied");
						} catch (MapObjectDescr::AnimationNonexistent&) {
							pic = building->get_animation("idle");
						}
						dst->blit_animation(pos[F], zoom, pic, 0, owner->get_playercolor());
					} else if (map_object_descr->type() == MapObjectType::FLAG) {
						assert(owner != nullptr);
						dst->blit_animation(
						   pos[F], zoom, owner->tribe().flag_animation(), 0, owner->get_playercolor());
					} else if (const uint32_t pic = map_object_descr->main_animation()) {
						if (owner != nullptr) {
							dst->blit_animation(pos[F], zoom, pic, 0, owner->get_playercolor());
						} else {
							dst->blit_animation(pos[F], zoom, pic, 0);
						}
					}
				}
			}

			const FieldOverlayManager& overlay_manager = egbase.get_ibase()->field_overlay_manager();
			{
				overlay_info.clear();
				overlay_manager.get_overlays(coords[F], &overlay_info);
				for (const auto& overlay : overlay_info) {
					// NOCOM(#sirver): this also requires zoom and FloatPoint
					dst->blit((pos[F] - overlay.hotspot.cast<float>()).cast<int>(), overlay.pic);
				}
			}

			{
				// Render overlays on the R triangle
				overlay_info.clear();
				overlay_manager.get_overlays(TCoords<>(coords[F], TCoords<>::R), &overlay_info);

				Point tripos(
				   (pos[F].x + pos[R].x + pos[BR].x) / 3, (pos[F].y + pos[R].y + pos[BR].y) / 3);
				for (const auto& overlay : overlay_info) {
					dst->blit(tripos - overlay.hotspot, overlay.pic);
				}
			}

			{
				// Render overlays on the D triangle
				overlay_info.clear();
				overlay_manager.get_overlays(TCoords<>(coords[F], TCoords<>::D), &overlay_info);

				Point tripos(
				   (pos[F].x + pos[BL].x + pos[BR].x) / 3, (pos[F].y + pos[BL].y + pos[BR].y) / 3);
				for (const auto& overlay : overlay_info) {
					dst->blit(tripos - overlay.hotspot, overlay.pic);
				}
			}
		}
	}
}

}  // namespace

GameRenderer::GameRenderer() {
}

GameRenderer::~GameRenderer() {
}

void GameRenderer::rendermap(const Widelands::EditorGameBase& egbase,
                             const Transform2f& screen_to_mappixel,
                             const Widelands::Player& player,
                             RenderTarget* dst) {
	draw(egbase, screen_to_mappixel, &player, dst);
}

void GameRenderer::rendermap(const Widelands::EditorGameBase& egbase,
                             const Transform2f& screen_to_mappixel,
                             RenderTarget* dst) {
	draw(egbase, screen_to_mappixel, nullptr, dst);
}

// NOCOM(#sirver): screen_to_mappixel is not correct - it needs to add dst->offset for mapviews
void GameRenderer::draw(const EditorGameBase& egbase,
                        const Transform2f& screen_to_mappixel,
                        const Player* player,
                        RenderTarget* dst) {
	FloatPoint tl_map = screen_to_mappixel.apply(dst->get_offset().cast<float>());
	FloatPoint br_map = screen_to_mappixel.apply(dst->get_rect().opposite_of_origin().cast<float>());

	assert(tl_map.x >= 0);  // divisions involving negative numbers are bad
	assert(tl_map.y >= 0);

	int minfx = std::floor(tl_map.x / kTriangleWidth) - 1;
	int minfy = std::floor(tl_map.y / kTriangleHeight) - 1;
	int maxfx = std::ceil(br_map.x / kTriangleWidth) + 1;
	int maxfy = std::ceil(br_map.y / kTriangleHeight) + 1;

	// NOCOM(#sirver): weird? correct!
	// fudge for triangle boundary effects and for height differences
	minfx -= 1;
	minfy -= 1;
	maxfx += 1;
	maxfy += 10;

	Surface* surface = dst->get_surface();
	if (!surface)
		return;

	const Rect& bounding_rect = dst->get_rect();
	const int surface_width = surface->width();
	const int surface_height = surface->height();

	Map& map = egbase.map();
	const EdgeOverlayManager& edge_overlay_manager = egbase.get_ibase()->edge_overlay_manager();
	const uint32_t gametime = egbase.get_gametime();

	fields_to_draw_.reset(minfx, maxfx, minfy, maxfy, screen_to_mappixel);
	for (int32_t fy = minfy; fy <= maxfy; ++fy) {
		for (int32_t fx = minfx; fx <= maxfx; ++fx) {
			FieldsToDraw::Field& f =
			   *fields_to_draw_.mutable_field(fields_to_draw_.calculate_index(fx, fy));

			f.fx = fx;
			f.fy = fy;

			Coords coords(fx, fy);

			// Texture coordinates for pseudo random tiling of terrain and road
			// graphics. Since screen space X increases top-to-bottom and OpenGL
			// increases bottom-to-top we flip the y coordinate to not have
			// terrains and road graphics vertically mirrorerd.
			FloatPoint map_pixel = MapviewPixelFunctions::to_map_pixel_ignoring_height(coords);
			f.texture_x = map_pixel.x / kTextureSideLength;
			f.texture_y = -map_pixel.y / kTextureSideLength;

			map.normalize_coords(coords);
			const FCoords& fcoords = map.get_fcoords(coords);

			map_pixel.y -= fcoords.field->get_height() * kHeightFactor;

			// NOCOM(#sirver): pull out inverse?
			const FloatPoint pixel = screen_to_mappixel.inverse().apply(map_pixel);
			f.gl_x = f.pixel_x = pixel.x;
			f.gl_y = f.pixel_y = pixel.y;
			pixel_to_gl_renderbuffer(surface_width, surface_height, &f.gl_x, &f.gl_y);

			f.ter_d = fcoords.field->terrain_d();
			f.ter_r = fcoords.field->terrain_r();

			f.brightness = field_brightness(fcoords, gametime, map, player);

			const PlayerNumber owner_number = fcoords.field->get_owned_by();
			if (owner_number > 0) {
				f.road_textures = &egbase.player(owner_number).tribe().road_textures();
			} else {
				f.road_textures = nullptr;
			}

			f.roads = field_roads(fcoords, map, edge_overlay_manager, player);
		}
	}

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainBase;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   FloatRect(bounding_rect.x, surface_height - bounding_rect.y - bounding_rect.h,
	             bounding_rect.w, bounding_rect.h);
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &egbase.world().terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw_;
	i.terrain_arguments.zoom = screen_to_mappixel.inverse().zoom();
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::kTerrainDither;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::kTerrainRoad;
	RenderQueue::instance().enqueue(i);

	draw_objects(egbase, screen_to_mappixel.inverse(), player, minfx, maxfx, minfy, maxfy, dst);
}

