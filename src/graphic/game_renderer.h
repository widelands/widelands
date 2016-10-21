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

#ifndef WL_GRAPHIC_GAME_RENDERER_H
#define WL_GRAPHIC_GAME_RENDERER_H

#include <memory>

#include "base/macros.h"
#include "base/transform.h"
#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/map_objects/draw_text.h"

namespace Widelands {
class Player;
class EditorGameBase;
}

class RenderTarget;

// Renders the MapView on screen.
class GameRenderer {
public:
	GameRenderer();
	~GameRenderer();

	// Renders the map from a player's point of view into the given drawing
	// window. The 'screen_to_mappixel' transform converts a screen pixel into a
	// map pixel at zoom 1.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Transform2f& panel_to_mappixel,
	               const Widelands::Player& player,
						DrawText draw_text,
	               RenderTarget* dst);

	// Renders the map from an omniscient perspective. This is used
	// for spectators, players that see all, and in the editor.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Transform2f& panel_to_mappixel,
						DrawText draw_text,
	               RenderTarget* dst);

private:
	// Draw the map for the given parameters (see rendermap). 'player'
	// can be nullptr in which case the whole map is drawn.
	void draw(const Widelands::EditorGameBase& egbase,
	          const Transform2f& screen_to_mappixel,
	          DrawText draw_text,
	          const Widelands::Player* player,
	          RenderTarget* dst);

	// This is owned and handled by us, but handed to the RenderQueue, so we
	// basically promise that this stays valid for one frame.
	FieldsToDraw fields_to_draw_;

	DISALLOW_COPY_AND_ASSIGN(GameRenderer);
};

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
