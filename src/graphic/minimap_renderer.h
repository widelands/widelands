/*
 * Copyright (C) 2010-2023 by the Widelands Development Team
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
	Terrain = 1 << 0,
	Owner = 1 << 1,
	Flag = 1 << 2,
	Road = 1 << 3,
	Building = 1 << 4,
	Zoom2 = 1 << 5,
	ViewWindow = 1 << 6,
	StartingPositions = 1 << 7,
	Ship = 1 << 8,
	Traffic = 1 << 9,
	Artifacts = 1 << 10,
	Attack = 1 << 11,
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
	kStaticMap,
};

// Converts between minimap pixel and map pixel.
// Remember to call 'normalize_pix' after applying the transformation.
Vector2f minimap_pixel_to_mappixel(const Widelands::Map& map,
                                   const Vector2i& minimap_pixel,
                                   const Rectf& view_area,
                                   MiniMapType minimap_type,
                                   bool zoom);

// Render the minimap. If player is not nullptr, it renders from that player's
// point of view. The 'view_area' designates the currently visible area in the
// main view in map pixel coordinates and is used to draw the wire frame view
// window. The 'view_point' is map pixel that will be drawn as the top-left
// point in the resulting minimap.
// This draws the entire minimap from start to finish. It can also be drawn in
// stages by the functions below.
std::unique_ptr<Texture> draw_minimap(const Widelands::EditorGameBase& egbase,
                                      const Widelands::Player* player,
                                      const Rectf& view_area,
                                      const MiniMapType& minimap_type,
                                      MiniMapLayer layers);

// Create an empty minimap texture.
std::unique_ptr<Texture> create_minimap_empty(const Widelands::EditorGameBase& egbase,
                                              MiniMapLayer layers);

// Draw a static (independent of the view area) version of the minimap on the
// given texture. Can update the entire texture or just a part of it,
// depending on 'draw_full'.
// If 'draw_full' is false, 'rows_drawn' has to be passed to keep track of
// the row number where we stopped drawing. We will start from that row
// (or row zero if it exceeds the map height) and the variable will be updated
// to contain the row where we stopped.
void draw_minimap_static(Texture& texture,
                         const Widelands::EditorGameBase& egbase,
                         const Widelands::Player* player,
                         MiniMapLayer layers,
                         bool draw_full = true,
                         uint16_t* rows_drawn = nullptr);

// Blit the static texture from the previous step onto a new texture.
// Optionally center it on 'view_area' and draw the view window.
std::unique_ptr<Texture> draw_minimap_final(const Texture& input_texture,
                                            const Widelands::EditorGameBase& egbase,
                                            const Rectf& view_area,
                                            const MiniMapType& minimap_type,
                                            MiniMapLayer layers);

// Find an even multiplier to fit the map into 300px
int scale_map(const Widelands::Map& map, bool zoom);

#endif  // end of include guard: WL_GRAPHIC_MINIMAP_RENDERER_H
