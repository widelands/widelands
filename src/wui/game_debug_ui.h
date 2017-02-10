/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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
// UI classes for real-time game debugging

#ifndef WL_WUI_GAME_DEBUG_UI_H
#define WL_WUI_GAME_DEBUG_UI_H

#include "logic/widelands_geometry.h"

class InteractiveBase;
namespace Widelands {
class MapObject;
}

// Open debug window for the given coordinates
void show_mapobject_debug(InteractiveBase& parent, Widelands::MapObject&);
void show_field_debug(InteractiveBase& parent, const Widelands::Coords& coords);

#endif  // end of include guard: WL_WUI_GAME_DEBUG_UI_H
