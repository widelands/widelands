/*
 * Copyright (C) 2010-2019 by the Widelands Development Team
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
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "wui/interactive_base.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

void draw_border_markers(const FieldsToDraw::Field& field,
                         const float scale,
                         const FieldsToDraw& fields_to_draw,
                         RenderTarget* dst,
                         const Widelands::EditorGameBase* egbase) {
	if (!field.all_neighbors_valid() || !field.is_border) {
		return;
	}
	uint32_t anim_idx;
	const RGBColor* pc;
	if (field.owner) {
		anim_idx = field.owner->tribe().frontier_animation();
		pc = &field.owner->get_playercolor();
	} else {
		// No players in the scenario editor
		const Widelands::PlayerNumber p = field.fcoords.field->get_owned_by();
		if (p == 0) {
			return;
		}
		assert(egbase);
		const std::string tribe = egbase->map().get_scenario_player_tribe(p);
		anim_idx = egbase->tribes().get_tribe_descr(tribe.empty() ?
				// random tribe: a different marker on every field
				(field.fcoords.x + field.fcoords.y) % egbase->tribes().nrtribes() :
				egbase->tribes().safe_tribe_index(tribe))->frontier_animation();
		pc = &kPlayerColors[p - 1];
	}

	if (field.vision) {
		dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale, anim_idx, 0, pc);
	}
	auto draw_edge = [field](const FieldsToDraw::Field& nf) {
		if (field.owner) {
			return field.owner == nf.owner || nf.owner == nullptr;
		}
		const Widelands::PlayerNumber p = nf.fcoords.field->get_owned_by();
		return p == 0 || p == field.fcoords.field->get_owned_by();
	};
	for (const auto& nf : {fields_to_draw.at(field.rn_index), fields_to_draw.at(field.bln_index),
	                       fields_to_draw.at(field.brn_index)}) {
		if ((field.vision || nf.vision) && nf.is_border && draw_edge(nf)) {
			dst->blit_animation(middle(field.rendertarget_pixel, nf.rendertarget_pixel),
			                    Widelands::Coords::null(), scale, anim_idx, 0,
			                    pc);
		}
	}
}

void draw_terrain(const Widelands::EditorGameBase& egbase,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  Workareas workarea,
                  bool grid,
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
