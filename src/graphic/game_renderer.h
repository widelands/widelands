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
#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/map_objects/draw_text.h"

namespace Widelands {
class Player;
class EditorGameBase;
}

class RenderTarget;

/**
 * This abstract base class renders the main game view into an
 * arbitrary @ref RenderTarget.
 *
 * Specializations exist for different OpenGL rendering paths.
 *
 * Users of this class should keep instances alive for as long as possible,
 * so that target-specific optimizations (such as caching data) can
 * be effective.
 *
 * Every instance can only perform one render operation per frame. When
 * multiple views of the map are open, each needs its own instance of
 * GameRenderer.
 */
class GameRenderer {
public:
	virtual ~GameRenderer();

	// Create a game renderer instance.
	static std::unique_ptr<GameRenderer> create();

	// Renders the map from a player's point of view into the given drawing
	// window. The 'viewpoint' is the top left screens pixel map pixel and
	// 'scale' is the magnification of the view.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Vector2f& viewpoint,
	               float scale,
	               const Widelands::Player& player,
	               TextToDraw draw_text,
	               RenderTarget* dst);

	// Renders the map from an omniscient perspective. This is used
	// for spectators, players that see all, and in the editor.
	void rendermap(const Widelands::EditorGameBase& egbase,
	               const Vector2f& viewpoint,
	               float scale,
	               TextToDraw draw_text,
	               RenderTarget* dst);

protected:
	GameRenderer();

	virtual void draw(const Widelands::EditorGameBase& egbase,
	                  const Vector2f& view_offset,
	                  const float zoom,
	                  const TextToDraw draw_text,
	                  const Widelands::Player* player,
	                  RenderTarget* dst) = 0;

	// Draws the objects (animations & overlays).
	// TODO(nha): does this still exist?
	void draw_objects(const Widelands::EditorGameBase& egbase,
	                  const float zoom,
	                  const FieldsToDraw &fields_to_draw,
	                  const Widelands::Player* player,
	                  const TextToDraw draw_text,
	                  RenderTarget* dst);

	DISALLOW_COPY_AND_ASSIGN(GameRenderer);
};

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
