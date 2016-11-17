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

void draw_objects_for_visible_field(const EditorGameBase& egbase,
                                    const FieldsToDraw::Field& field,
                                    const float zoom,
                                    const TextToDraw draw_text,
                                    const Player* player,
                                    RenderTarget* dst) {
	BaseImmovable* const imm = field.fcoords.field->get_immovable();
	if (imm != nullptr && imm->get_positions(egbase).front() == field.fcoords) {
		TextToDraw draw_text_for_this_immovable = draw_text;
		const Player* owner = imm->get_owner();
		if (player != nullptr && owner != nullptr && !player->see_all() &&
		    player->is_hostile(*owner)) {
			draw_text_for_this_immovable =
			   static_cast<TextToDraw>(draw_text_for_this_immovable & ~TextToDraw::kStatistics);
		}

		imm->draw(
		   egbase.get_gametime(), draw_text_for_this_immovable, field.rendertarget_pixel, zoom, dst);
	}
	for (Bob* bob = field.fcoords.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
		TextToDraw draw_text_for_this_bob = draw_text;
		const Player* owner = bob->get_owner();
		if (player != nullptr && owner != nullptr && !player->see_all() &&
		    player->is_hostile(*owner)) {
			draw_text_for_this_bob =
			   static_cast<TextToDraw>(draw_text_for_this_bob & ~TextToDraw::kStatistics);
		}
		bob->draw(egbase, draw_text_for_this_bob, field.rendertarget_pixel, zoom, dst);
	}
}

void draw_objets_for_formerly_visible_field(const FieldsToDraw::Field& field,
                                            const Player::Field& player_field,
                                            const float zoom,
                                            RenderTarget* dst) {
	if (const MapObjectDescr* const map_object_descr =
	       player_field.map_object_descr[TCoords<>::None]) {
		if (player_field.constructionsite.becomes) {
			assert(field.owner != nullptr);
			const ConstructionsiteInformation& csinf = player_field.constructionsite;
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

			uint32_t percent = 100 * csinf.completedtime * nr_frames;
			if (csinf.totaltime) {
				percent /= csinf.totaltime;
			}
			percent -= 100 * cur_frame;  // NOCOM test this

			if (cur_frame) {  // not the first frame
				// Draw the prev frame
				dst->blit_animation(field.rendertarget_pixel, zoom, anim_idx, tanim - FRAME_LENGTH,
				                    field.owner->get_playercolor());
			} else if (csinf.was) {
				// Is the first frame, but there was another building here before,
				// get its last build picture and draw it instead.
				uint32_t a;
				try {
					a = csinf.was->get_animation("unoccupied");
				} catch (MapObjectDescr::AnimationNonexistent&) {
					a = csinf.was->get_animation("idle");
				}
				dst->blit_animation(field.rendertarget_pixel, zoom, a, tanim - FRAME_LENGTH,
				                    field.owner->get_playercolor());
			}
			dst->blit_animation(field.rendertarget_pixel, zoom, anim_idx, tanim,
			                    field.owner->get_playercolor(), percent);
		} else if (upcast(const BuildingDescr, building, map_object_descr)) {
			assert(field.owner != nullptr);
			// this is a building therefore we either draw unoccupied or idle animation
			uint32_t pic;
			try {
				pic = building->get_animation("unoccupied");
			} catch (MapObjectDescr::AnimationNonexistent&) {
				pic = building->get_animation("idle");
			}
			dst->blit_animation(
			   field.rendertarget_pixel, zoom, pic, 0, field.owner->get_playercolor());
		} else if (map_object_descr->type() == MapObjectType::FLAG) {
			assert(field.owner != nullptr);
			dst->blit_animation(field.rendertarget_pixel, zoom, field.owner->tribe().flag_animation(),
			                    0, field.owner->get_playercolor());
		} else if (const uint32_t pic = map_object_descr->main_animation()) {
			if (field.owner != nullptr) {
				dst->blit_animation(
				   field.rendertarget_pixel, zoom, pic, 0, field.owner->get_playercolor());
			} else {
				dst->blit_animation(field.rendertarget_pixel, zoom, pic, 0);
			}
		}
	}
}

// Draws the objects (animations & overlays).
void draw_objects(const EditorGameBase& egbase,
                  const float zoom,
                  const FieldsToDraw& fields_to_draw,
                  const Player* player,
                  const TextToDraw draw_text,
                  RenderTarget* dst) {
	std::vector<FieldOverlayManager::OverlayInfo> overlay_info;
	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);
		if (!field.all_neighbors_valid()) {
			continue;
		}

		const FieldsToDraw::Field& rn = fields_to_draw.at(field.rn_index);
		const FieldsToDraw::Field& bln = fields_to_draw.at(field.bln_index);
		const FieldsToDraw::Field& brn = fields_to_draw.at(field.brn_index);

		if (field.is_border) {
			assert(field.owner != nullptr);
			uint32_t const anim_idx = field.owner->tribe().frontier_animation();
			if (field.vision) {
				dst->blit_animation(
				   field.rendertarget_pixel, zoom, anim_idx, 0, field.owner->get_playercolor());
			}
			for (const auto& nf : {rn, bln, brn}) {
				if ((field.vision || nf.vision) && nf.is_border &&
				    (field.owner == nf.owner || nf.owner == nullptr)) {
					dst->blit_animation(middle(field.rendertarget_pixel, nf.rendertarget_pixel), zoom,
					                    anim_idx, 0, field.owner->get_playercolor());
				}
			}
		}

		if (1 < field.vision) {  // Render stuff that belongs to the node.
			draw_objects_for_visible_field(egbase, field, zoom, draw_text, player, dst);
		} else if (field.vision == 1) {
			// We never show census or statistics for objects in the fog.
			assert(player != nullptr);
			const Map& map = egbase.map();
			const Player::Field& player_field =
			   player->fields()[map.get_index(field.fcoords, map.get_width())];
			draw_objets_for_formerly_visible_field(field, player_field, zoom, dst);
		}

		const FieldOverlayManager& overlay_manager = egbase.get_ibase()->field_overlay_manager();
		{
			overlay_info.clear();
			overlay_manager.get_overlays(field.fcoords, &overlay_info);
			for (const auto& overlay : overlay_info) {
				dst->blitrect_scale(
				   Rectf(field.rendertarget_pixel - overlay.hotspot.cast<float>() * zoom,
				         overlay.pic->width() * zoom, overlay.pic->height() * zoom),
				   overlay.pic, Recti(0, 0, overlay.pic->width(), overlay.pic->height()), 1.f,
				   BlendMode::UseAlpha);
			}
		}

		{
			// Render overlays on the right triangle
			overlay_info.clear();
			overlay_manager.get_overlays(TCoords<>(field.fcoords, TCoords<>::R), &overlay_info);

			Vector2f tripos(
			   (field.rendertarget_pixel.x + rn.rendertarget_pixel.x + brn.rendertarget_pixel.x) / 3.f,
			   (field.rendertarget_pixel.y + rn.rendertarget_pixel.y + brn.rendertarget_pixel.y) /
			      3.f);
			for (const auto& overlay : overlay_info) {
				dst->blitrect_scale(Rectf(tripos - overlay.hotspot.cast<float>() * zoom,
				                          overlay.pic->width() * zoom, overlay.pic->height() * zoom),
				                    overlay.pic,
				                    Recti(0, 0, overlay.pic->width(), overlay.pic->height()), 1.f,
				                    BlendMode::UseAlpha);
			}
		}

		{
			// Render overlays on the D triangle
			overlay_info.clear();
			overlay_manager.get_overlays(TCoords<>(field.fcoords, TCoords<>::D), &overlay_info);

			Vector2f tripos(
			   (field.rendertarget_pixel.x + bln.rendertarget_pixel.x + brn.rendertarget_pixel.x) /
			      3.f,
			   (field.rendertarget_pixel.y + bln.rendertarget_pixel.y + brn.rendertarget_pixel.y) /
			      3.f);
			for (const auto& overlay : overlay_info) {
				dst->blitrect_scale(Rectf(tripos - overlay.hotspot.cast<float>() * zoom,
				                          overlay.pic->width() * zoom, overlay.pic->height() * zoom),
				                    overlay.pic,
				                    Recti(0, 0, overlay.pic->width(), overlay.pic->height()), 1.f,
				                    BlendMode::UseAlpha);
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
                             const Vector2f& viewpoint,
                             const float zoom,
                             const Widelands::Player& player,
                             const TextToDraw draw_text,
                             RenderTarget* dst) {
	draw(egbase, viewpoint, zoom, draw_text, &player, dst);
}

void GameRenderer::rendermap(const Widelands::EditorGameBase& egbase,
                             const Vector2f& viewpoint,
                             const float zoom,
                             const TextToDraw draw_text,
                             RenderTarget* dst) {
	draw(egbase, viewpoint, zoom, draw_text, nullptr, dst);
}

void GameRenderer::draw(const EditorGameBase& egbase,
                        const Vector2f& viewpoint,
                        const float zoom,
                        const TextToDraw draw_text,
                        const Player* player,
                        RenderTarget* dst) {
	assert(viewpoint.x >= 0);  // divisions involving negative numbers are bad
	assert(viewpoint.y >= 0);
	assert(dst->get_offset().x <= 0);
	assert(dst->get_offset().y <= 0);

	int minfx = std::floor(viewpoint.x / kTriangleWidth);
	int minfy = std::floor(viewpoint.y / kTriangleHeight);

	// If a view window is partially moved outside of the display, its 'rect' is
	// adjusted to be fully contained on the screen - i.e. x = 0 and width is
	// made smaller. Its offset is made negative to account for this change.
	// To figure out which fields we need to draw, we have to add the absolute
	// value of 'offset' to the actual dimension of the 'rect' to get to desired
	// dimension of the 'rect'
	const Vector2f br_map = MapviewPixelFunctions::panel_to_map(
	   viewpoint, zoom, Vector2f(dst->get_rect().w + std::abs(dst->get_offset().x),
	                             dst->get_rect().h + std::abs(dst->get_offset().y)));
	int maxfx = std::ceil(br_map.x / kTriangleWidth);
	int maxfy = std::ceil(br_map.y / kTriangleHeight);

	// Adjust for triangle boundary effects and for height differences.
	minfx -= 2;
	maxfx += 2;
	minfy -= 2;
	maxfy += 10;

	Surface* surface = dst->get_surface();
	if (!surface)
		return;

	const Recti& bounding_rect = dst->get_rect();
	const int surface_width = surface->width();
	const int surface_height = surface->height();

	Map& map = egbase.map();
	const EdgeOverlayManager& edge_overlay_manager = egbase.get_ibase()->edge_overlay_manager();
	const uint32_t gametime = egbase.get_gametime();

	const float scale = 1.f / zoom;
	fields_to_draw_.reset(minfx, maxfx, minfy, maxfy);
	for (int32_t fy = minfy; fy <= maxfy; ++fy) {
		for (int32_t fx = minfx; fx <= maxfx; ++fx) {
			FieldsToDraw::Field& f =
			   *fields_to_draw_.mutable_field(fields_to_draw_.calculate_index(fx, fy));

			f.geometric_coords = Coords(fx, fy);

			f.ln_index = fields_to_draw_.calculate_index(fx - 1, fy);
			f.rn_index = fields_to_draw_.calculate_index(fx + 1, fy);
			f.trn_index = fields_to_draw_.calculate_index(fx + (fy & 1), fy - 1);
			f.bln_index = fields_to_draw_.calculate_index(fx + (fy & 1) - 1, fy + 1);
			f.brn_index = fields_to_draw_.calculate_index(fx + (fy & 1), fy + 1);

			// Texture coordinates for pseudo random tiling of terrain and road
			// graphics. Since screen space X increases top-to-bottom and OpenGL
			// increases bottom-to-top we flip the y coordinate to not have
			// terrains and road graphics vertically mirrorerd.
			Vector2f map_pixel =
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(f.geometric_coords);
			f.texture_coords.x = map_pixel.x / kTextureSideLength;
			f.texture_coords.y = -map_pixel.y / kTextureSideLength;

			Coords normalized = f.geometric_coords;
			map.normalize_coords(normalized);
			f.fcoords = map.get_fcoords(normalized);

			map_pixel.y -= f.fcoords.field->get_height() * kHeightFactor;

			f.rendertarget_pixel = MapviewPixelFunctions::map_to_panel(viewpoint, zoom, map_pixel);
			f.gl_position = f.surface_pixel = f.rendertarget_pixel +
			                                  dst->get_rect().origin().cast<float>() +
			                                  dst->get_offset().cast<float>();
			pixel_to_gl_renderbuffer(
			   surface_width, surface_height, &f.gl_position.x, &f.gl_position.y);

			f.brightness = field_brightness(f.fcoords, gametime, map, player);

			PlayerNumber owned_by = f.fcoords.field->get_owned_by();
			f.owner = owned_by != 0 ? &egbase.player(owned_by) : nullptr;
			f.is_border = f.fcoords.field->is_border();
			f.vision = 2;
			f.roads = f.fcoords.field->get_roads();
			if (player && !player->see_all()) {
				const Player::Field& pf = player->fields()[map.get_index(f.fcoords, map.get_width())];
				f.roads = pf.roads;
				f.vision = pf.vision;
				if (pf.vision == 1) {
					f.owner = pf.owner != 0 ? &egbase.player(owned_by) : nullptr;
					f.is_border = pf.border;
				}
			}
			f.roads |= edge_overlay_manager.get_overlay(f.fcoords);
		}
	}

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainBase;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   Rectf(bounding_rect.x, surface_height - bounding_rect.y - bounding_rect.h, bounding_rect.w,
	         bounding_rect.h);
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &egbase.world().terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw_;
	i.terrain_arguments.scale = scale;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::kTerrainDither;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::kTerrainRoad;
	RenderQueue::instance().enqueue(i);

	draw_objects(egbase, scale, fields_to_draw_, player, draw_text, dst);
}
