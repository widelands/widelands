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

#ifndef WL_GRAPHIC_RENDER_MINIMAPRENDERER_H
#define WL_GRAPHIC_RENDER_MINIMAPRENDERER_H

#include <memory>

#include "graphic/rendertarget.h"

class StreamWrite;

namespace Widelands {
	class Player;
	class Editor_Game_Base;
}

// NOCOM(#sirver): rename everything in here
enum class MiniMapLayers {
	Terrains = 1,
	Owners = 2,
	Flags = 4,
	Roads = 8,
	Buildings = 16,
	Zoom2 = 32,
	ViewWindow = 64,
};

inline MiniMapLayers operator|(MiniMapLayers left, MiniMapLayers right) {
	return MiniMapLayers(static_cast<int>(left) | static_cast<int>(right));
}
inline int operator&(MiniMapLayers left, MiniMapLayers right) {
	return static_cast<int>(left) & static_cast<int>(right);
}
inline MiniMapLayers operator ^ (MiniMapLayers left, MiniMapLayers right) {
	return MiniMapLayers(static_cast<int>(left) ^ static_cast<int>(right));
}

/// Render the minimap. If player is not nullptr, it renders from that player's
/// point of view.
/// \param viewpoint: top left corner in map coordinates
std::unique_ptr<Surface> draw_minimap
	(const Widelands::Editor_Game_Base& egbase, const Widelands::Player* player,
	 const Point& viewpoint, MiniMapLayers layers);

/// Render the minimap to a file. 1 pixel will be used for each fields.
/// \param viewpoint : The game point of view as returned by interactive_base.get_viewpoint();
void write_minimap_image
	(const Widelands::Editor_Game_Base& egbase, Widelands::Player const* player,
	 const Point& viewpoint, MiniMapLayers layers, StreamWrite* const streamwrite);

#endif  // end of include guard: WL_GRAPHIC_RENDER_MINIMAPRENDERER_H
