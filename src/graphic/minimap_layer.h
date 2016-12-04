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

#ifndef WL_GRAPHIC_MINIMAP_LAYER_H
#define WL_GRAPHIC_MINIMAP_LAYER_H

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

#endif  // end of include guard: WL_GRAPHIC_MINIMAP_LAYER_H
