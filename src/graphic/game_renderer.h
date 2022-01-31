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

#ifndef WL_GRAPHIC_GAME_RENDERER_H
#define WL_GRAPHIC_GAME_RENDERER_H

#include "graphic/gl/fields_to_draw.h"
#include "logic/map_objects/descriptions.h"

// Draw the terrain only.
void draw_terrain(uint32_t gametime,
                  const Widelands::Descriptions& descriptions,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  const Workareas& workarea,
                  bool grid,
                  const Widelands::Player*,
                  RenderTarget* dst);

// Draw the border stones for 'field' if it is a border and 'visibility' is
// correct.
void draw_border_markers(const FieldsToDraw::Field& field,
                         const float scale,
                         const FieldsToDraw& fields_to_draw,
                         RenderTarget* dst);

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
