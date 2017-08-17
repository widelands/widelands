/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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
#include <map>

#include "base/macros.h"
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
	struct Overlays {
		TextToDraw text_to_draw;
		std::map<Widelands::Coords, uint8_t> road_building_preview;
	};

	enum class DrawImmovables { kNo, kYes };
	enum class DrawBobs { kNo, kYes };

	GameRenderer();
	~GameRenderer();

	// Renders the map from a player's point of view into the given drawing
	// window. The 'viewpoint' is the top left screens pixel map pixel and
	// 'scale' is the magnification of the view.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Vector2f& viewpoint,
	               float scale,
	               const Widelands::Player& player,
	               const Overlays& overlays,
	               RenderTarget* dst);

	// Renders the map from an omniscient perspective. This is used
	// for spectators, players that see all, and in the editor. Only in the editor we allow toggling
	// of immovables and bobs.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Vector2f& viewpoint,
	               float scale,
	               const Overlays& overlays,
	               const DrawImmovables& draw_immovables,
	               const DrawBobs& draw_bobs,
	               RenderTarget* dst);

private:
	// Draw the map for the given parameters (see rendermap). 'player'
	// can be nullptr in which case the whole map is drawn.
	void draw(const Widelands::EditorGameBase& egbase,
	          const Vector2f& viewpoint,
	          float scale,
	          const Overlays& overlays,
	          const DrawImmovables& draw_immovables,
	          const DrawBobs& draw_bobs,
	          const Widelands::Player* player,
	          RenderTarget* dst);

	// This is owned and handled by us, but handed to the RenderQueue, so we
	// basically promise that this stays valid for one frame.
	FieldsToDraw fields_to_draw_;

	DISALLOW_COPY_AND_ASSIGN(GameRenderer);
};

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
