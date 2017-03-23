/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_WUI_FIELD_OVERLAY_MANAGER_H
#define WL_WUI_FIELD_OVERLAY_MANAGER_H

#include <functional>
#include <map>
#include <set>
#include <vector>

#include "base/vector.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"

class Image;

/*
 * The Overlay Manager is responsible for the map overlays. He
 * manages overlays in the following way:
 *   - When someone registered one (or more) special overlays
 *     for a field he draws them accordingly
 *
 *    about the level variable:
 *     the level describe when the overlay should be drawn, lower means drawn
 *     earlier.
 *
 *    about overlay_id:
 *     the overlay_id can be given to the register function, whenever
 *     the job is finished or canceled, a simple remove_overlay
 *     with the overlay_id can be called and all overlays created in the
 *     job are removed.
 */
constexpr int kLevelForBuildHelp = 5;
struct FieldOverlayManager {
	/// A unique id identifying a registered overlay.
	using OverlayId = uint32_t;

	/// A overlay as drawn onto the screen.
	struct OverlayInfo {
		OverlayInfo() = default;
		OverlayInfo(const Image* init_pic, const Vector2i& init_hotspot)
		   : pic(init_pic), hotspot(init_hotspot) {
		}

		const Image* pic;
		Vector2i hotspot;
	};

	/// A function returning Field::nodecaps() for the build overlay. This can be
	/// registered to hide or change some of the nodecaps during rendering.
	using CallbackFn =
	   std::function<int32_t(const Widelands::TCoords<Widelands::FCoords>& coordinates)>;

	FieldOverlayManager();

	/// Returns true if the buildhelp is currently shown.
	bool buildhelp() const;

	/// Defines if the buildhelp should be shown.
	void show_buildhelp(bool t);

	/// Register callback function.
	void register_overlay_callback_function(CallbackFn function);

	/// Get a unique, unused id that can be passed to register_overlay.
	OverlayId next_overlay_id();

	/// Register an overlay at a location (node or triangle). hotspot is the point
	/// of the picture that will be exactly over the location. If hotspot is
	/// Vector2i::invalid(), the center of the picture will be used as hotspot.
	void register_overlay(const Widelands::TCoords<>& coords,
	                      const Image* pic,
	                      int32_t level,
	                      Vector2i hotspot = Vector2i::invalid(),
	                      OverlayId overlay_id = 0);

	/// removes all overlays when pic is nullptr.
	void remove_overlay(Widelands::TCoords<>, const Image* pic);

	/// remove all overlays with this overlay_id
	void remove_overlay(OverlayId overlay_id);

	/// Removes all overlays.
	// TODO(sirver): It would be preferable to just delete and recreate the object.
	void remove_all_overlays();

	/// Returns the currently registered overlays and the buildhelp for a node.
	void get_overlays(const Widelands::Map& map, Widelands::FCoords c, std::vector<OverlayInfo>* result) const;

	/// Returns the currently registered overlays for a triangle.
	void get_overlays(Widelands::TCoords<>, std::vector<OverlayInfo>* result) const;

private:
	struct RegisteredOverlays {
		RegisteredOverlays(const OverlayId init_overlay_id,
		                   const Image* init_pic,
		                   const Vector2i init_hotspot,
		                   const int init_level)
		   : pic(init_pic), hotspot(init_hotspot), level(init_level) {
			overlay_ids.insert(init_overlay_id);
		}
		std::set<OverlayId> overlay_ids;
		const Image* pic;
		Vector2i hotspot;
		int level;
	};

	using RegisteredOverlaysMap = std::multimap<const Widelands::Coords,
	                                            RegisteredOverlays,
	                                            Widelands::Coords::OrderingFunctor>;

	// Returns the index into buildhelp_infos_ for the correct fieldcaps for
	// 'fc' according to the current 'callback_'.
	int get_buildhelp_overlay(const Widelands::Map& map, const Widelands::FCoords& fc) const;

	//  indexed by TCoords<>::TriangleIndex
	RegisteredOverlaysMap overlays_[3];

	OverlayInfo buildhelp_infos_[static_cast<int>(Widelands::Field::Buildhelp::kNone)];
	bool buildhelp_;

	// this callback is used to define where overlays are drawn.
	CallbackFn callback_;
	OverlayId current_overlay_id_;
};

#endif  // end of include guard: WL_WUI_FIELD_OVERLAY_MANAGER_H
