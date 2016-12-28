/*
 * Copyright (C) 2010-2016 by the Widelands Development Team
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

#include "graphic/game_renderer_gl4.h"

#include "graphic/gl/terrain_program_gl4.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/player.h"
#include "logic/roadtype.h"
#include "wui/edge_overlay_manager.h"
#include "wui/interactive_base.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

/**
 * This is the front-end of the GL4 rendering path. See game_renderer.cc for a
 * general overview of terrain rendering.
 *
 * Only terrain rendering (terrain textures, dithering, and roads) differs
 * substantially from the GL2 rendering path. To avoid CPU work, a persistent
 * TerrainInformationGl4 object maintains texture data and other information
 * across frames.
 *
 * The GameRendererGl4 contains per-view information, but the underlying
 * TerrainInformationGl4 instance is automatically shared between different
 * views when possible.
 */

GameRendererGl4::GameRendererGl4() {
}

GameRendererGl4::~GameRendererGl4() {
}

bool GameRendererGl4::supported() {
	return TerrainProgramGl4::supported();
}

void GameRendererGl4::draw(const EditorGameBase& egbase,
                           const Vector2f& view_offset,
                           const float zoom,
                           const TextToDraw draw_text,
                           const Player* player,
                           RenderTarget* dst) {
	Surface* surface = dst->get_surface();
	if (!surface)
		return;

	// Upload map changes.
	if (!args_.terrain || &args_.terrain->egbase() != &egbase ||
	    args_.terrain->player() != player)
		args_.terrain = TerrainInformationGl4::get(egbase, player);

	args_.terrain->update();

	// Determine the set of patches to draw.
	float scale = 1.f / zoom;
	Vector2f tl_map = view_offset - dst->get_offset().cast<float>() * zoom;

	assert(tl_map.x >= 0);  // divisions involving negative numbers are bad
	assert(tl_map.y >= 0);

	args_.minfx = tl_map.x / kTriangleWidth - 1;
	args_.minfy = tl_map.y / kTriangleHeight - 1;
	args_.maxfx = (tl_map.x + dst->get_rect().w * zoom + (kTriangleWidth / 2)) / kTriangleWidth;
	args_.maxfy = (tl_map.y + dst->get_rect().h * zoom) / kTriangleHeight;

	// Fudge for triangle boundary effects, for height differences, and for
	// large immovables.
	args_.minfx -= 1;
	args_.minfy -= 1;
	args_.maxfx += 3;
	args_.maxfy += 10;

	const Recti& bounding_rect = dst->get_rect();
	const uint32_t gametime = egbase.get_gametime();

	args_.scale = scale;
	args_.surface_offset = (bounding_rect.origin() + dst->get_offset()).cast<float>() * zoom - view_offset;
	args_.surface_width = surface->width();
	args_.surface_height = surface->height();

	scan_fields(view_offset);

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainGl4;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   Rectf(bounding_rect.x, args_.surface_height - bounding_rect.y - bounding_rect.h,
	         bounding_rect.w, bounding_rect.h);
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = args_.surface_width;
	i.terrain_arguments.renderbuffer_height = args_.surface_height;
	i.terrain_gl4_arguments = &args_;
	RenderQueue::instance().enqueue(i);

	if (!args_.roads.empty()) {
		i.program_id = RenderQueue::Program::kTerrainRoadGl4;
		i.blend_mode = BlendMode::UseAlpha;
		RenderQueue::instance().enqueue(i);
	}

	draw_objects(egbase, scale, fields_to_draw_, player, draw_text, dst);
}

void GameRendererGl4::scan_fields(const Vector2f& view_offset) {
	const EditorGameBase& egbase = args_.terrain->egbase();
	const Player* player = args_.terrain->player();
	auto& map = egbase.map();
	const EdgeOverlayManager& edge_overlay_manager = egbase.get_ibase()->edge_overlay_manager();

	args_.roads.clear();
	fields_to_draw_.reset(args_.minfx, args_.maxfx, args_.minfy, args_.maxfy);

	for (auto cursor = fields_to_draw_.cursor(); cursor.valid(); cursor.next()) {
		FieldToDrawBase& f = cursor.mutable_field();

		Vector2f map_pixel =
		   MapviewPixelFunctions::to_map_pixel_ignoring_height(cursor.geometric_coords());

		Coords normalized = cursor.geometric_coords();
		map.normalize_coords(normalized);
		f.fcoords = map.get_fcoords(normalized);

		map_pixel.y -= f.fcoords.field->get_height() * kHeightFactor;

		f.rendertarget_pixel = MapviewPixelFunctions::map_to_panel(view_offset, 1. / args_.scale, map_pixel);

		PlayerNumber owned_by = f.fcoords.field->get_owned_by();
		f.owner = owned_by != 0 ? &egbase.player(owned_by) : nullptr;
		f.is_border = f.fcoords.field->is_border();
		f.vision = 2;
		if (player && !player->see_all()) {
			const Player::Field& pf = player->fields()[map.get_index(f.fcoords, map.get_width())];
			f.vision = pf.vision;
			if (pf.vision == 1) {
				f.owner = pf.owner != 0 ? &egbase.player(owned_by) : nullptr;
				f.is_border = pf.border;
			}
		}

		uint8_t roads;
		if (!player || player->see_all()) {
			roads = f.fcoords.field->get_roads();
		} else {
			const Player::Field& pf = player->fields()[map.get_index(f.fcoords, map.get_width())];
			roads = pf.roads;
		}
		if (player)
			roads |= edge_overlay_manager.get_overlay(f.fcoords);

		uint8_t type = (roads >> RoadType::kEast) & RoadType::kMask;
		if (type) {
			args_.roads.emplace_back(cursor.geometric_coords(), type,
			                         WalkingDir::WALK_E, f.fcoords.field->get_owned_by());
		}

		type = (roads >> RoadType::kSouthEast) & RoadType::kMask;
		if (type) {
			args_.roads.emplace_back(cursor.geometric_coords(), type,
			                         WalkingDir::WALK_SE, f.fcoords.field->get_owned_by());
		}

		type = (roads >> RoadType::kSouthWest) & RoadType::kMask;
		if (type) {
			args_.roads.emplace_back(cursor.geometric_coords(), type,
			                         WalkingDir::WALK_SW, f.fcoords.field->get_owned_by());
		}
	}
}
