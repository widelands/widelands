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

#ifndef WIDELANDS_GAMERENDERER_H
#define WIDELANDS_GAMERENDERER_H

#include "point.h"

namespace Widelands {
	struct Player;
	struct Editor_Game_Base;
};

struct RenderTarget;

/**
 * This abstract base class renders the main game view into an
 * arbitrary @ref RenderTarget.
 *
 * Specializations exist for SDL software rendering and for OpenGL rendering.
 *
 * Users of this class should keep instances alive for as long as possible,
 * so that target-specific optimizations (such as caching data) can
 * be effective.
 */
struct GameRenderer {
public:
	GameRenderer() {}
	virtual ~GameRenderer() {}

	/**
	 * Renders the map from a player's point of view into the
	 * given drawing window.
	 *
	 * @param viewofs is the offset of the upper left corner of
	 * the window into the map, in pixels.
	 */
	virtual void rendermap
		(RenderTarget & dst,
		 Widelands::Editor_Game_Base const &       egbase,
		 Widelands::Player           const &       player,
		 Point                                     viewofs) = 0;

	/**
	 * Renders the map from an omniscient perspective.
	 * This is used for spectators, players that see all, and in the editor.
	 */
	virtual void rendermap
		(RenderTarget & dst,
		 Widelands::Editor_Game_Base const & egbase,
		 Point                               viewofs) = 0;
};

#endif //WIDELANDS_GAMEVIEW_H
