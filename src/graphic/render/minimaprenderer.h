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

#ifndef WIDELANDS_MINIMAPRENDERER_H
#define WIDELANDS_MINIMAPRENDERER_H

#include "graphic/rendertarget.h"
#include <memory>

class StreamWrite;
namespace Widelands {
	class Player;
	class Editor_Game_Base;
};

/**
 * This class renders the minimap.
 */
class MiniMapRenderer
{
public:
	MiniMapRenderer() {}
	virtual ~MiniMapRenderer() {}

	/// Render the minimap. If player is not 0, it renders from that player's
	/// point of view. The caller must dispose of the returned surface properly.
	/// \param viewpoint: top left corner in map coordinates
	Surface* get_minimap_image
		(const Widelands::Editor_Game_Base& egbase, const Widelands::Player* player,
		 const Point& viewpoint, uint32_t flags);

	/// Render the minimap to a file. 1 pixel will be used for each fields.
	/// \param viewpoint : The game point of view as returned by interactive_base.get_viewpoint();
	void write_minimap_image
		(const Widelands::Editor_Game_Base& egbase, Widelands::Player const* player,
		 const Point& viewpoint, uint32_t flags, StreamWrite* const streamwrite);
};

#endif //WIDELANDS_MINIMAPRENDERER_H
