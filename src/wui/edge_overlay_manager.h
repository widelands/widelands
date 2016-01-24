/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_WUI_EDGE_OVERLAY_MANAGER_H
#define WL_WUI_EDGE_OVERLAY_MANAGER_H

#include <map>

#include "base/macros.h"
#include "logic/widelands_geometry.h"

// Similar to FieldOverlayManager, this class cares for overlays that are drawn
// onto the edges of triangles.
class EdgeOverlayManager {
public:
	// A unique id identifying a registered overlay.
	using OverlayId = uint32_t;

	EdgeOverlayManager();

	/// Get a unique, unused job id.
	OverlayId next_overlay_id();

	//  When a road overlay information is requested the same data as for a
	//  field is returned (a uint8_t which needs to be ANDed).
	void register_overlay
		(Widelands::Coords, uint8_t where, OverlayId overlay_id = 0);
	void remove_overlay(Widelands::Coords);
	void remove_overlay(OverlayId overlay_id);
	uint8_t get_overlay(Widelands::Coords c) const;

private:
	struct RegisteredRoadOverlays {
		OverlayId overlay_id;
		uint8_t where;
	};

	using RegisteredRoadOverlaysMap =
	   std::map<const Widelands::Coords, RegisteredRoadOverlays, Widelands::Coords::OrderingFunctor>;

	OverlayId m_current_overlay_id;
	RegisteredRoadOverlaysMap m_overlays;

	DISALLOW_COPY_AND_ASSIGN(EdgeOverlayManager);
};



#endif  // end of include guard: WL_WUI_EDGE_OVERLAY_MANAGER_H
