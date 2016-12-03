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

#ifndef WL_GRAPHIC_MINIMAP_RENDERER_H
#define WL_GRAPHIC_MINIMAP_RENDERER_H

#include <memory>

#include "base/point.h"
#include "graphic/minimap_layer.h"

class RenderTarget;
class StreamWrite;

namespace Widelands {
class Player;
class EditorGameBase;
}

/**
 * Virtual base class for mini-map renderers. Each mini-map view should own
 * an instance of this class, which must be kept alive during rendering (due
 * to commands sent to the RenderQueue).
 */
class MiniMapRenderer {
public:
	static std::unique_ptr<MiniMapRenderer>
	create(const Widelands::EditorGameBase& egbase,
	       const Widelands::Player* player);

	virtual ~MiniMapRenderer() {}

	const Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	const Widelands::Player* player() const {
		return player_;
	}

	/**
	 * Draw the minimap into the given destination. The @p viewpoint is the
	 * map field shown in the top-left corner of the minimap.
	 */
	virtual void draw(RenderTarget& dst,
	                  const Point& viewpoint,
	                  MiniMapLayer layers) = 0;

protected:
	MiniMapRenderer(const Widelands::EditorGameBase& egbase,
	                const Widelands::Player* player);

private:
	const Widelands::EditorGameBase& egbase_;
	const Widelands::Player* player_;
};

/// Render the minimap to a file. 1 pixel will be used for each fields.
/// \param viewpoint : The field at the top-left corner of the mini-map.
void write_minimap_image_field(const Widelands::EditorGameBase& egbase,
                               Widelands::Player const* player,
                               const Point& viewpoint,
                               MiniMapLayer layers,
                               StreamWrite* const streamwrite);

/// Render the minimap to a file. 1 pixel will be used for each fields.
/// \param viewpoint : The game point of view as returned by interactive_base.get_viewpoint();
void write_minimap_image(const Widelands::EditorGameBase& egbase,
                         Widelands::Player const* player,
                         const Point& viewpoint,
                         MiniMapLayer layers,
                         StreamWrite* const streamwrite);

#endif  // end of include guard: WL_GRAPHIC_MINIMAP_RENDERER_H
