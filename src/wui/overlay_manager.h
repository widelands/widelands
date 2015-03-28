/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef WL_WUI_OVERLAY_MANAGER_H
#define WL_WUI_OVERLAY_MANAGER_H

#include <map>
#include <set>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "logic/field.h"
#include "logic/widelands_geometry.h"
#include "base/point.h"

class Image;

/*
 * The Overlay Manager is responsible for the map overlays. He
 * manages overlays in the following way:
 *   - When someone registered one (or more) special overlays
 *     for a field he draws them accordingly
 *   - If nothing was registered for this field, the Overlay Manager
 *     automatically returns
 *        - buildhelp (if activated)
 *
 *    about the level variable:
 *     the level describe when the overlay should be drawn
 *     buildhelp graphics have a level of 5, lower means the
 *     buildhelp will cover the old overlays, otherways the new
 *     overlay will cover the buildhelp.
 *
 *    about overlay_id:
 *     the overlay_id can be given to the register function, whenever
 *     the job is finished or canceled, a simple remove_overlay
 *     with the overlay_id can be called and all overlays created in the
 *     job are removed. This is useful for interactive road building.
 */
#define MAX_OVERLAYS_PER_NODE 6
#define MAX_OVERLAYS_PER_TRIANGLE 3

// A unique id identifying a registered field or road overlay.
using OverlayId = uint32_t;

struct OverlayManager {
	struct OverlayInfo {
		const Image* pic;
		Point hotspot;
	};

	using CallbackFn =
		boost::function<int32_t(const Widelands::TCoords<Widelands::FCoords>& coordinates)>;

	OverlayManager();

	//  register callback function (see data below for description)
	void register_overlay_callback_function(CallbackFn function);
	void remove_overlay_callback_function();

	/// Get a unique, unused job id.
	// NOCOM(#sirver): who uses this?
	OverlayId get_a_job_id();

	void load_graphics();

	/// Register an overlay at a location (node or triangle). hotspot is the point
	/// of the picture that will be exactly over the location. If hotspot is
	/// Point::invalid(), the center of the picture will be used as hotspot.
	void register_overlay
		(Widelands::TCoords<>,
		 const Image* pic,
		 int32_t level,
		 Point   hotspot = Point::invalid(),
		 OverlayId overlay_id = 0);

	// removes all overlays when pic is zero
	void remove_overlay(Widelands::TCoords<>, const Image* pic);
	void remove_overlay(OverlayId overlay_id);

	uint8_t get_overlays(Widelands::FCoords c, OverlayInfo *) const;
	uint8_t get_overlays(Widelands::TCoords<>, OverlayInfo *) const;

	boost::function<void(bool)> onBuildHelpToggle;
	bool buildhelp() const;
	void show_buildhelp(bool t);
	void toggle_buildhelp();

	void recalc_field_overlays(Widelands::FCoords);

private:
	struct RegisteredOverlays {
		RegisteredOverlays(const OverlayId init_overlay_id,
		                    const Image* init_pic,
		                    const Point init_hotspot,
		                    const int init_level)
		   : pic(init_pic), hotspot(init_hotspot), level(init_level) {
			overlay_ids.insert(init_overlay_id);
		}
		std::set<OverlayId> overlay_ids;
		const Image* pic;
		Point hotspot;
		int level;
	};


	using RegisteredOverlaysMap =
		std::multimap<const Widelands::Coords, RegisteredOverlays, Widelands::Coords::OrderingFunctor>;

	//  indexed by TCoords<>::TriangleIndex
	RegisteredOverlaysMap m_overlays[3];

	OverlayInfo m_buildhelp_infos[Widelands::Field::Buildhelp_None];
	bool m_are_graphics_loaded;
	bool m_showbuildhelp;

	// this callback is used to define where overlays are drawn.
	CallbackFn m_callback;
	OverlayId m_current_job_id;
};


// NOCOM(#sirver): move into separate file
// NOCOM(#sirver): rename OverlayManager to FieldOverlayManager
// NOCOM(#sirver): maybe rename the other to edgeOverlayManager?
class RoadOverlayManager {
public:
	RoadOverlayManager() = default;

	//  Road overlays are registered like normal overlays and removed like
	//  normal overlays but they use are handled internally completely
	//  different. When a road overlay information is requested the same data a
	//  s for a field is returned (a uint8_t which needs to be ANDed).
	void register_road_overlay
		(Widelands::Coords, uint8_t where, OverlayId overlay_id = 0);
	void remove_road_overlay(Widelands::Coords);
	void remove_road_overlay(OverlayId overlay_id);
	uint8_t get_road_overlay(Widelands::Coords c) const;

private:
	struct RegisteredRoadOverlays {
		OverlayId overlay_id;
		uint8_t where;
	};

	using RegisteredRoadOverlaysMap =
		std::map<const Widelands::Coords, RegisteredRoadOverlays, Widelands::Coords::OrderingFunctor>;
	RegisteredRoadOverlaysMap m_road_overlays;

	DISALLOW_COPY_AND_ASSIGN(RoadOverlayManager);
};

#endif  // end of include guard: WL_WUI_OVERLAY_MANAGER_H
