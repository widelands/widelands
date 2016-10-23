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
#include "wui/mapviewpixelconstants.h"

using namespace Widelands;

/**
 * This is the front-end of the GL4 rendering path. See game_renderer.cc for a
 * general overview of terrain rendering.
 *
 * TODO(nha): minimap rendering
 *
 * Only terrain rendering (terrain textures, dithering, and roads) differs
 * substantially from the GL2 rendering path. To avoid CPU work, persistent
 * objects Terrain{Base,PlayerPerspective}Gl4 maintain texture data and other
 * information across frames.
 *
 * The GameRendererGl4 contains per-view information, but the underlying
 * Terrain*Gl4 instances are automatically shared between different views when
 * possible.
 */

GameRendererGl4::GameRendererGl4() {
}

GameRendererGl4::~GameRendererGl4() {
}

bool GameRendererGl4::supported() {
	return TerrainProgramGl4::supported();
}

void GameRendererGl4::draw(RenderTarget& dst,
                           const EditorGameBase& egbase,
                           const Point& view_offset,
                           const Player* player) {
	Surface* surface = dst.get_surface();
	if (!surface)
		return;

	// Upload map changes.
	if (!args_.terrain || &args_.terrain->egbase() != &egbase)
		args_.terrain = TerrainBaseGl4::get(egbase);

	args_.terrain->update();

	if (!args_.perspective ||
	    &args_.perspective->egbase() != &egbase ||
	    args_.perspective->player() != player)
		args_.perspective = TerrainPlayerPerspectiveGl4::get(egbase, player);

	args_.perspective->update();

	// Determine the set of patches to draw.
	Point tl_map = dst.get_offset() + view_offset;

	assert(tl_map.x >= 0);  // divisions involving negative numbers are bad
	assert(tl_map.y >= 0);

	args_.minfx = tl_map.x / kTriangleWidth - 1;
	args_.minfy = tl_map.y / kTriangleHeight - 1;
	args_.maxfx = (tl_map.x + dst.get_rect().w + (kTriangleWidth / 2)) / kTriangleWidth;
	args_.maxfy = (tl_map.y + dst.get_rect().h) / kTriangleHeight;

	// fudge for triangle boundary effects and for height differences
	args_.minfx -= 1;
	args_.minfy -= 1;
	args_.maxfx += 1;
	args_.maxfy += 10;

	const Rect& bounding_rect = dst.get_rect();
	const uint32_t gametime = egbase.get_gametime();

	args_.surface_offset = bounding_rect.origin() + dst.get_offset() - view_offset;
	args_.surface_width = surface->width();
	args_.surface_height = surface->height();

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainGl4;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   FloatRect(bounding_rect.x, args_.surface_height - bounding_rect.y - bounding_rect.h,
	             bounding_rect.w, bounding_rect.h);
	i.terrain_arguments.gametime = gametime;
	i.terrain_arguments.renderbuffer_width = args_.surface_width;
	i.terrain_arguments.renderbuffer_height = args_.surface_height;
	i.terrain_gl4_arguments = &args_;
	RenderQueue::instance().enqueue(i);

	draw_objects(dst, egbase, view_offset, player, args_.minfx, args_.maxfx, args_.minfy, args_.maxfy);
}
