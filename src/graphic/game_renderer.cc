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
#include "logic/player.h"
#include "logic/world/world.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

// Explanation of how drawing works:
// Schematic of triangle neighborhood:
//
//               *
//              / \
//             / u \
//         (f)/     \
//    *------*------* (r)
//     \  l / \  r / \
//      \  /   \  /   \
//       \/  d  \/ rr  \
//       *------*------* (br)
//        \ dd /
//         \  /
//          \/
//          *
//
// Each field (f) owns two triangles: (r)ight & (d)own. When we look at the
// field, we have to make sure to schedule drawing the triangles. This is done
// by of these triangles is done by TerrainProgram.
//
// To draw dithered edges, we have to look at the neighboring triangles for the
// two triangles too: If a neighboring triangle has another texture and our
// dither layer is smaller, we have to draw a dithering triangle too - this lets the neighboring
// texture
// bleed into our triangle.
//
// The dither triangle is the triangle that should be partially (either r or
// d). Example: if r and d have different textures and r.dither_layer >
// d.dither_layer, then we will repaint d with the dither texture as mask.

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
uint8_t field_roads(const FCoords& coords, const Map& map, const Player* const player) {
	uint8_t roads;
	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | map.overlay_manager().get_road_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= map.overlay_manager().get_road_overlay(coords);
	return roads;
}

}  // namespace

GameRenderer::GameRenderer()  {
}

GameRenderer::~GameRenderer() {
}

void GameRenderer::rendermap(RenderTarget& dst,
                             const Widelands::EditorGameBase& egbase,
                             const Point& view_offset,

                             const Widelands::Player& player) {
	draw(dst, egbase, view_offset, &player);
}

void GameRenderer::rendermap(RenderTarget& dst,
                             const Widelands::EditorGameBase& egbase,
                             const Point& view_offset) {
	draw(dst, egbase, view_offset, nullptr);
}

void GameRenderer::draw(RenderTarget& dst,
                        const EditorGameBase& egbase,
                        const Point& view_offset,
                        const Player* player) {
	Point tl_map = dst.get_offset() + view_offset;

	assert(tl_map.x >= 0); // divisions involving negative numbers are bad
	assert(tl_map.y >= 0);

	int minfx = tl_map.x / TRIANGLE_WIDTH - 1;
	int minfy = tl_map.y / TRIANGLE_HEIGHT - 1;
	int maxfx = (tl_map.x + dst.get_rect().w + (TRIANGLE_WIDTH / 2)) / TRIANGLE_WIDTH;
	int maxfy = (tl_map.y + dst.get_rect().h) / TRIANGLE_HEIGHT;

	// fudge for triangle boundary effects and for height differences
	minfx -= 1;
	minfy -= 1;
	maxfx += 1;
	maxfy += 10;


	Surface* surface = dst.get_surface();
	if (!surface)
		return;

	const Rect& bounding_rect = dst.get_rect();
	const Point surface_offset = bounding_rect.origin() + dst.get_offset() - view_offset;
	const int surface_width = surface->width();
	const int surface_height = surface->height();

	Map& map = egbase.map();
	const uint32_t gametime = egbase.get_gametime();

	fields_to_draw_.reset(minfx, maxfx, minfy, maxfy);
	for (int32_t fy = minfy; fy <= maxfy; ++fy) {
		for (int32_t fx = minfx; fx <= maxfx; ++fx) {
			FieldsToDraw::Field& f =
			   *fields_to_draw_.mutable_field(fields_to_draw_.calculate_index(fx, fy));

			f.fx = fx;
			f.fy = fy;

			Coords coords(fx, fy);
			int x, y;
			MapviewPixelFunctions::get_basepix(coords, x, y);

			map.normalize_coords(coords);
			const FCoords& fcoords = map.get_fcoords(coords);

			f.texture_x = float(x) / kTextureSideLength;
			f.texture_y = float(y) / kTextureSideLength;

			f.gl_x = f.pixel_x = x + surface_offset.x;
			f.gl_y = f.pixel_y = y + surface_offset.y - fcoords.field->get_height() * HEIGHT_FACTOR;
			pixel_to_gl_renderbuffer(surface_width, surface_height, &f.gl_x, &f.gl_y);

			f.ter_d = fcoords.field->terrain_d();
			f.ter_r = fcoords.field->terrain_r();

			f.brightness = field_brightness(fcoords, gametime, map, player);

			PlayerNumber owner_number = fcoords.field->get_owned_by();
			if (owner_number > 0) {
				f.road_textures = &egbase.player(owner_number).tribe().road_textures();
			} else {
				f.road_textures = nullptr;
			}

			f.roads = field_roads(fcoords, map, player);
		}
	}

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::TERRAIN_BASE;
	i.blend_mode = BlendMode::Copy;
	i.destination_rect =
	   FloatRect(bounding_rect.x,
	             surface_height - bounding_rect.y - bounding_rect.h,
	             bounding_rect.w,
	             bounding_rect.h);
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &egbase.world().terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw_;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::TERRAIN_DITHER;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::TERRAIN_ROAD;
	RenderQueue::instance().enqueue(i);

	draw_objects(dst, egbase, view_offset, player, minfx, maxfx, minfy, maxfy);
}

void GameRenderer::draw_objects(RenderTarget& dst,
                                const EditorGameBase& egbase,
                                const Point& view_offset,
                                const Player* player,
                                int minfx,
                                int maxfx,
                                int minfy,
                                int maxfy) {
	// TODO(sirver): this should use FieldsToDraw. Would simplify this function a lot.
	static const uint32_t F = 0;
	static const uint32_t R = 1;
	static const uint32_t BL = 2;
	static const uint32_t BR = 3;
	const Map & map = egbase.map();

	for (int32_t fy = minfy; fy <= maxfy; ++fy) {
		for (int32_t fx = minfx; fx <= maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords coords[4];
			coords[F] = map.get_fcoords(ncoords);
			coords[R] = map.r_n(coords[F]);
			coords[BL] = map.bl_n(coords[F]);
			coords[BR] = map.br_n(coords[F]);
			Point pos[4];
			MapviewPixelFunctions::get_basepix(Coords(fx, fy), pos[F].x, pos[F].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + 1, fy), pos[R].x, pos[R].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1) - 1, fy + 1), pos[BL].x, pos[BL].y);
			MapviewPixelFunctions::get_basepix(Coords(fx + (fy & 1), fy + 1), pos[BR].x, pos[BR].y);
			for (uint32_t d = 0; d < 4; ++d) {
				pos[d].y -= coords[d].field->get_height() * HEIGHT_FACTOR;
				pos[d] -= view_offset;
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
					const Player::Field & pf = player->fields()[map.get_index(coords[d], map.get_width())];
					vision[d] = pf.vision;
					if (pf.vision == 1) {
						owner_number[d] = pf.owner;
						isborder[d] = pf.border;
					}
				}
			}

			if (isborder[F]) {
				const Player & owner = egbase.player(owner_number[F]);
				uint32_t const anim_idx = owner.tribe().frontier_animation();
				if (vision[F])
					dst.drawanim(pos[F], anim_idx, 0, &owner);
				for (uint32_t d = 1; d < 4; ++d) {
					if
						((vision[F] || vision[d]) &&
						 isborder[d] &&
						 (owner_number[d] == owner_number[F] || !owner_number[d]))
					{
						dst.drawanim(middle(pos[F], pos[d]), anim_idx, 0, &owner);
					}
				}
			}

			if (1 < vision[F]) { // Render stuff that belongs to the node.
				if (BaseImmovable * const imm = coords[F].field->get_immovable())
					imm->draw(egbase, dst, coords[F], pos[F]);
				for
					(Bob * bob = coords[F].field->get_first_bob();
					 bob;
					 bob = bob->get_next_bob())
					bob->draw(egbase, dst, pos[F]);
			} else if (vision[F] == 1) {
				const Player::Field & f_pl = player->fields()[map.get_index(coords[F], map.get_width())];
				const Player * owner = owner_number[F] ? egbase.get_player(owner_number[F]) : nullptr;
				if
					(const MapObjectDescr * const map_object_descr =
						f_pl.map_object_descr[TCoords<>::None])
				{
					if
						(f_pl.constructionsite.becomes)
					{
						const ConstructionsiteInformation & csinf = f_pl.constructionsite;
						// draw the partly finished constructionsite
						uint32_t anim_idx;
						try {
							anim_idx = csinf.becomes->get_animation("build");
						} catch (MapObjectDescr::AnimationNonexistent &) {
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

						if (cur_frame) // not the first frame
							// draw the prev frame from top to where next image will be drawing
							dst.drawanimrect
								(pos[F], anim_idx, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						else if (csinf.was) {
							// Is the first frame, but there was another building here before,
							// get its last build picture and draw it instead.
							uint32_t a;
							try {
								a = csinf.was->get_animation("unoccupied");
							} catch (MapObjectDescr::AnimationNonexistent &) {
								a = csinf.was->get_animation("idle");
							}
							dst.drawanimrect
								(pos[F], a, tanim - FRAME_LENGTH, owner, Rect(Point(0, 0), w, h - lines));
						}
						assert(lines <= h);
						dst.drawanimrect(pos[F], anim_idx, tanim, owner, Rect(Point(0, h - lines), w, lines));
					} else if (upcast(const BuildingDescr, building, map_object_descr)) {
						// this is a building therefore we either draw unoccupied or idle animation
						uint32_t pic;
						try {
							pic = building->get_animation("unoccupied");
						} catch (MapObjectDescr::AnimationNonexistent &) {
							pic = building->get_animation("idle");
						}
						dst.drawanim(pos[F], pic, 0, owner);
					} else if (const uint32_t pic = map_object_descr->main_animation()) {
						dst.drawanim(pos[F], pic, 0, owner);
					} else if (map_object_descr->type() == MapObjectType::FLAG) {
						dst.drawanim(pos[F], owner->tribe().flag_animation(), 0, owner);
					}
				}
			}

			{
				// Render overlays on the node
				OverlayManager::OverlayInfo overlay_info[MAX_OVERLAYS_PER_NODE];

				const OverlayManager::OverlayInfo * const end =
					overlay_info
					+
					map.overlay_manager().get_overlays(coords[F], overlay_info);

				for
					(const OverlayManager::OverlayInfo * it = overlay_info;
					 it < end;
					 ++it)
					dst.blit(pos[F] - it->hotspot, it->pic);
			}

			{
				// Render overlays on the R triangle
				OverlayManager::OverlayInfo overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				OverlayManager::OverlayInfo const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(coords[F], TCoords<>::R), overlay_info);

				Point tripos
					((pos[F].x + pos[R].x + pos[BR].x) / 3,
					 (pos[F].y + pos[R].y + pos[BR].y) / 3);

				for
					(OverlayManager::OverlayInfo const * it = overlay_info;
					 it < end;
					 ++it)
					dst.blit(tripos - it->hotspot, it->pic);
			}

			{
				// Render overlays on the D triangle
				OverlayManager::OverlayInfo overlay_info[MAX_OVERLAYS_PER_TRIANGLE];
				OverlayManager::OverlayInfo const * end =
					overlay_info
					+
					map.overlay_manager().get_overlays
							 	(TCoords<>(coords[F], TCoords<>::D), overlay_info);

				Point tripos
					((pos[F].x + pos[BL].x + pos[BR].x) / 3,
					 (pos[F].y + pos[BL].y + pos[BR].y) / 3);

				for
					(OverlayManager::OverlayInfo const * it = overlay_info;
					 it < end;
					 ++it)
					dst.blit(tripos - it->hotspot, it->pic);
			}
		}
	}
}
