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

#ifndef WL_GRAPHIC_MINIMAP_RENDERER_H
#define WL_GRAPHIC_MINIMAP_RENDERER_H

#include <memory>

#include "base/rect.h"
#include "base/vector.h"
#include "graphic/texture.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"

// Layers for selecting what do display on the minimap.
enum class MiniMapLayer {
	Terrain = 1,
	Owner = 2,
	Flag = 4,
	Road = 8,
	Building = 16,
	Zoom2 = 32,
	ViewWindow = 64,
};

// A bunch of operators that turn MiniMapLayer into a bitwise combinable flag class.
inline MiniMapLayer operator|(MiniMapLayer left, MiniMapLayer right) {
	return MiniMapLayer(static_cast<int>(left) | static_cast<int>(right));
}
inline int operator&(MiniMapLayer left, MiniMapLayer right) {
	return static_cast<int>(left) & static_cast<int>(right);
}
inline MiniMapLayer operator^(MiniMapLayer left, MiniMapLayer right) {
	return MiniMapLayer(static_cast<int>(left) ^ static_cast<int>(right));
}

enum class MiniMapType {
	// Keep the view window always in the center of the minimap and pan the underlying map.
	kStaticViewWindow,

	// Always align the map at (0, 0) and move the view window instead.
	// NOCOM(#sirver): implement this.
	kStaticMap,
};

// Converts between minimap pixel and map pixel.
// Remember to call 'normalize_pix' after applying the transformation.
Vector2i minimap_pixel_to_mappixel(const Widelands::Map& map,
                                const Vector2i& minimap_pixel,
                                const Rectf& view_area,
                                MiniMapType minimap_type,
										  const bool zoom);

// Render the minimap. If player is not nullptr, it renders from that player's
// point of view. The 'view_area' designates the currently visible area in the
// main view in map pixel coordinates and is used to draw the wire frame view
// window. The 'view_point' is map pixel that will be drawn as the top-left
// point in the resulting minimap.
std::unique_ptr<Texture> draw_minimap(const Widelands::EditorGameBase& egbase,
                                      const Widelands::Player* player,
                                      const Rectf& view_area,
                                      const MiniMapType& map_draw_type,
                                      MiniMapLayer layers);

#endif  // end of include guard: WL_GRAPHIC_MINIMAP_RENDERER_H
