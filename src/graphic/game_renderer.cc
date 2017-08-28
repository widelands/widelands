/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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

namespace  {

void draw_immovables_for_visible_field(const Widelands::EditorGameBase& egbase,
                                       const FieldsToDraw::Field& field,
                                       const float scale,
                                       const TextToDraw text_to_draw,
                                       const Widelands::Player& player,
                                       RenderTarget* dst) {
	Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
	if (imm != nullptr && imm->get_positions(egbase).front() == field.fcoords) {
		TextToDraw draw_text_for_this_immovable = text_to_draw;
		const Widelands::Player* owner = imm->get_owner();
		if (owner != nullptr && !player.see_all() && player.is_hostile(*owner)) {
			draw_text_for_this_immovable =
			   static_cast<TextToDraw>(draw_text_for_this_immovable & ~TextToDraw::kStatistics);
		}
		imm->draw(
		   egbase.get_gametime(), draw_text_for_this_immovable, field.rendertarget_pixel, scale, dst);
	}
}

void draw_bobs_for_visible_field(const Widelands::EditorGameBase& egbase,
                                 const FieldsToDraw::Field& field,
                                 const float scale,
                                 const TextToDraw text_to_draw,
                                 const Widelands::Player& player,
                                 RenderTarget* dst) {
	for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
		TextToDraw draw_text_for_this_bob = text_to_draw;
		const Widelands::Player* owner = bob->get_owner();
		if (owner != nullptr && !player.see_all() && player.is_hostile(*owner)) {
			draw_text_for_this_bob =
			   static_cast<TextToDraw>(draw_text_for_this_bob & ~TextToDraw::kStatistics);
		}
		bob->draw(egbase, draw_text_for_this_bob, field.rendertarget_pixel, scale, dst);
	}
}

void draw_immovables_for_formerly_visible_field(const FieldsToDraw::Field& field,
                                                const Widelands::Player::Field& player_field,
                                                const float scale,
                                                RenderTarget* dst) {
	if (const Widelands::MapObjectDescr* const map_object_descr =
	       player_field.map_object_descr[Widelands::TCoords<>::None]) {
		if (player_field.constructionsite.becomes) {
			assert(field.owner != nullptr);
			const Widelands::ConstructionsiteInformation& csinf = player_field.constructionsite;
			// draw the partly finished constructionsite
			uint32_t anim_idx;
			try {
				anim_idx = csinf.becomes->get_animation("build");
			} catch (Widelands::MapObjectDescr::AnimationNonexistent&) {
				try {
					anim_idx = csinf.becomes->get_animation("unoccupied");
				} catch (Widelands::MapObjectDescr::AnimationNonexistent) {
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
			percent -= 100 * cur_frame;

			if (cur_frame) {  // not the first frame
				// Draw the prev frame
				dst->blit_animation(field.rendertarget_pixel, scale, anim_idx, tanim - FRAME_LENGTH,
				                    field.owner->get_playercolor());
			} else if (csinf.was) {
				// Is the first frame, but there was another building here before,
				// get its last build picture and draw it instead.
				uint32_t a;
				try {
					a = csinf.was->get_animation("unoccupied");
				} catch (Widelands::MapObjectDescr::AnimationNonexistent&) {
					a = csinf.was->get_animation("idle");
				}
				dst->blit_animation(field.rendertarget_pixel, scale, a, tanim - FRAME_LENGTH,
				                    field.owner->get_playercolor());
			}
			dst->blit_animation(field.rendertarget_pixel, scale, anim_idx, tanim,
			                    field.owner->get_playercolor(), percent);
		} else if (upcast(const Widelands::BuildingDescr, building, map_object_descr)) {
			assert(field.owner != nullptr);
			// this is a building therefore we either draw unoccupied or idle animation
			uint32_t pic;
			try {
				pic = building->get_animation("unoccupied");
			} catch (Widelands::MapObjectDescr::AnimationNonexistent&) {
				pic = building->get_animation("idle");
			}
			dst->blit_animation(
			   field.rendertarget_pixel, scale, pic, 0, field.owner->get_playercolor());
		} else if (map_object_descr->type() == Widelands::MapObjectType::FLAG) {
			assert(field.owner != nullptr);
			dst->blit_animation(field.rendertarget_pixel, scale, field.owner->tribe().flag_animation(),
			                    0, field.owner->get_playercolor());
		} else if (const uint32_t pic = map_object_descr->main_animation()) {
			if (field.owner != nullptr) {
				dst->blit_animation(
				   field.rendertarget_pixel, scale, pic, 0, field.owner->get_playercolor());
			} else {
				dst->blit_animation(field.rendertarget_pixel, scale, pic, 0);
			}
		}
	}
}

}  // namespace

void draw_border(const FieldsToDraw::Field& field,
					  const float scale,
                 const FieldsToDraw& fields_to_draw,
                 RenderTarget* dst) {
	if (!field.all_neighbors_valid() || !field.is_border) {
		return;
	}
	assert(field.owner != nullptr);

	uint32_t const anim_idx = field.owner->tribe().frontier_animation();
	if (field.vision) {
		dst->blit_animation(
		   field.rendertarget_pixel, scale, anim_idx, 0, field.owner->get_playercolor());
	}
	for (const auto& nf : {fields_to_draw.at(field.rn_index), fields_to_draw.at(field.bln_index),
	                       fields_to_draw.at(field.brn_index)}) {
		if ((field.vision || nf.vision) && nf.is_border &&
		    (field.owner == nf.owner || nf.owner == nullptr)) {
			dst->blit_animation(middle(field.rendertarget_pixel, nf.rendertarget_pixel), scale,
			                    anim_idx, 0, field.owner->get_playercolor());
		}
	}
}

// NOCOM(#sirver): hoist into InteractivePlayer.
// Draws the objects (animations & overlays).
void draw_objects(const Widelands::EditorGameBase& egbase,
                  const float scale,
                  const FieldsToDraw& fields_to_draw,
                  const Widelands::Player& player,
                  const TextToDraw text_to_draw,
                  RenderTarget* dst) {
	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		draw_border(field, scale, fields_to_draw, dst);

		if (1 < field.vision) {  // Render stuff that belongs to the node.
			draw_immovables_for_visible_field(egbase, field, scale, text_to_draw, player, dst);
			draw_bobs_for_visible_field(egbase, field, scale, text_to_draw, player, dst);
		} else if (field.vision == 1) {
			// We never show census or statistics for objects in the fog.
			const Widelands::Map& map = egbase.map();
			const Widelands::Player::Field& player_field =
			   player.fields()[map.get_index(field.fcoords, map.get_width())];
			draw_immovables_for_formerly_visible_field(field, player_field, scale, dst);
		}

		egbase.get_ibase()->field_overlay_manager().foreach_overlay(
		   field.fcoords, [dst, &field, scale](const Image* pic, const Vector2i& hotspot) {
			   dst->blitrect_scale(Rectf(field.rendertarget_pixel - hotspot.cast<float>() * scale,
			                             pic->width() * scale, pic->height() * scale),
			                       pic, Recti(0, 0, pic->width(), pic->height()), 1.f,
			                       BlendMode::UseAlpha);
			});
	}
}

void draw_terrain(const Widelands::EditorGameBase& egbase,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  RenderTarget* dst) {
	const Recti& bounding_rect = dst->get_rect();
	const Surface& surface = dst->get_surface();
	const int surface_width = surface.width();
	const int surface_height = surface.height();

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainBase;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   Rectf(bounding_rect.x, surface_height - bounding_rect.y - bounding_rect.h, bounding_rect.w,
	         bounding_rect.h);
	i.terrain_arguments.gametime = egbase.get_gametime();
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &egbase.world().terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw;
	i.terrain_arguments.scale = scale;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::kTerrainDither;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::kTerrainRoad;
	RenderQueue::instance().enqueue(i);

}
