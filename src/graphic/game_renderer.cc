/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#include "graphic/game_renderer.h"

#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/player.h"

void draw_border_markers(const FieldsToDraw::Field& field,
                         const float scale,
                         const FieldsToDraw& fields_to_draw,
                         RenderTarget* dst) {
	if (!field.all_neighbors_valid() || !field.is_border) {
		return;
	}
	assert(field.owner != nullptr);

	uint32_t const anim_idx = field.owner->tribe().frontier_animation();
	if (field.seeing != Widelands::VisibleState::kUnexplored) {
		dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale, anim_idx, Time(0),
		                    &field.owner->get_playercolor());
	}
	for (const auto& nf : {fields_to_draw.at(field.rn_index), fields_to_draw.at(field.bln_index),
	                       fields_to_draw.at(field.brn_index)}) {
		if ((field.seeing != Widelands::VisibleState::kUnexplored ||
		     nf.seeing != Widelands::VisibleState::kUnexplored) &&
		    nf.is_border && (field.owner == nf.owner || nf.owner == nullptr)) {
			dst->blit_animation(middle(field.rendertarget_pixel, nf.rendertarget_pixel),
			                    Widelands::Coords::null(), scale, anim_idx, Time(0),
			                    &field.owner->get_playercolor());
		}
	}
}

void draw_terrain(uint32_t gametime,
                  const Widelands::Descriptions& descriptions,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  const Workareas& workarea,
                  bool grid,
                  const Widelands::Player* player,
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
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &descriptions.terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw;
	i.terrain_arguments.scale = scale;
	i.terrain_arguments.player = player;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::kTerrainDither;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	if (!workarea.empty()) {
		// Enqueue the drawing of the workarea overlay layer.
		i.program_id = RenderQueue::Program::kTerrainWorkarea;
		i.terrain_arguments.workareas = workarea;
		RenderQueue::instance().enqueue(i);
	}

	if (grid) {
		// Enqueue the drawing of the grid layer.
		i.program_id = RenderQueue::Program::kTerrainGrid;
		i.blend_mode = BlendMode::UseAlpha;
		RenderQueue::instance().enqueue(i);
	}

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::kTerrainRoad;
	RenderQueue::instance().enqueue(i);
}
