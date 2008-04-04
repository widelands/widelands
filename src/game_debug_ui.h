/*
 * Copyright (C) 2004, 2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
// UI classes for real-time game debugging

#ifndef GAME_DEBUG_UI_H
#define GAME_DEBUG_UI_H

#include "widelands_geometry.h"

struct Interactive_Base;
namespace Widelands {struct Map_Object;};

// Open debug window for the given coordinates
void show_mapobject_debug(Interactive_Base & parent, Widelands::Map_Object &);
void show_field_debug(Interactive_Base & parent, Widelands::Coords coords);

#endif
