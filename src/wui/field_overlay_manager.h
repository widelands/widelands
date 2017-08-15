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

// Levels for the overlay registers. This defines in which order they will be
// drawn. Buildhelp is special and has the value 5, i.e. every smaller will be
// drawn below the buildhelp, everything higher above.
enum class OverlayLevel {
	kWorkAreaPreview = 0,
	kResource = 4,
	kSelection = 7,
	kRoadBuildSlope = 8,
	kPlayerStartingPosition = 9,
};

struct FieldOverlayManager {
	/// A unique id identifying a registered overlay.
	using OverlayId = uint32_t;

	/// A function returning Field::nodecaps() for the build overlay. This can be
	/// registered to hide or change some of the nodecaps during rendering.
	using CallbackFn = std::function<int32_t(const Widelands::FCoords& coordinates)>;

	FieldOverlayManager();

	/// Returns true if the buildhelp is currently shown.
	bool buildhelp() const;

	/// Defines if the buildhelp should be shown.
	void show_buildhelp(bool t);

	/// Register callback function.
	void register_overlay_callback_function(CallbackFn function);

	/// Like 'buildhelp', but for an individual layer.
	bool is_enabled(const OverlayLevel& level) const;
	void set_enabled(const OverlayLevel& level, bool value);

	/// Get a unique, unused id that can be passed to register_overlay.
	OverlayId next_overlay_id();

	/// Register an overlay at a location (node or triangle). hotspot is the point
	/// of the picture that will be exactly over the location. If hotspot is
	/// Vector2i::invalid(), the center of the picture will be used as hotspot.
	void register_overlay(const Widelands::Coords& coords,
	                      const Image* pic,
	                      const OverlayLevel& overlay_level,
	                      Vector2i hotspot = Vector2i::invalid(),
	                      OverlayId overlay_id = 0);

	/// removes all overlays when pic is nullptr.
	void remove_overlay(const Widelands::Coords& coords, const Image* pic);

	/// remove all overlays with this overlay_id
	void remove_overlay(OverlayId overlay_id);

	/// Removes all overlays.
	// TODO(sirver): It would be preferable to just delete and recreate the object.
	void remove_all_overlays();

	/// Calls 'func' for each of the the currently registered and enabled
	/// overlays and the buildhelp.
	template <typename T> void foreach_overlay(const Widelands::FCoords& c, T func) const {
		auto it = overlays_.lower_bound(c);
		while (it != overlays_.end() && it->first == c &&
		       static_cast<int>(it->second.level) <= kLevelForBuildHelp) {
			if (is_enabled(it->second.level)) {
				func(it->second.pic, it->second.hotspot);
			}
			++it;
		}

		if (buildhelp_) {
			int buildhelp_overlay_index = get_buildhelp_overlay(c);
			if (buildhelp_overlay_index < Widelands::Field::Buildhelp_None) {
				auto& overlay_info = buildhelp_infos_[buildhelp_overlay_index];
				func(overlay_info.pic, overlay_info.hotspot);
			}
		}

		while (it != overlays_.end() && it->first == c) {
			if (is_enabled(it->second.level)) {
				func(it->second.pic, it->second.hotspot);
			}
			++it;
		}
	}

private:
	static constexpr int kLevelForBuildHelp = 5;

	/// A overlay as drawn onto the screen.
	struct OverlayInfo {
		const Image* pic = nullptr;
		Vector2i hotspot = Vector2i::zero();
	};

	struct RegisteredOverlays {
		RegisteredOverlays(const OverlayId init_overlay_id,
		                   const Image* init_pic,
		                   const Vector2i init_hotspot,
		                   const OverlayLevel& init_level)
		   : pic(init_pic), hotspot(init_hotspot), level(init_level) {
			overlay_ids.insert(init_overlay_id);
		}
		std::set<OverlayId> overlay_ids;
		const Image* pic;
		Vector2i hotspot = Vector2i::zero();
		OverlayLevel level;
	};

	// Returns the index into buildhelp_infos_ for the correct fieldcaps for
	// 'fc' according to the current 'callback_'.
	int get_buildhelp_overlay(const Widelands::FCoords& fc) const;

	std::multimap<const Widelands::Coords, RegisteredOverlays> overlays_;

	OverlayInfo buildhelp_infos_[Widelands::Field::Buildhelp_None];
	bool buildhelp_;
	// We are inverting the logic here, since new layers are by default enabled
	// and we only support to toggle some of them off. Otherwise whenever a new
	// layer is added in 'OverlayLevel' we would also need to add it to the
	// 'enabled_layers_' set on construction.
	std::set<OverlayLevel> disabled_layers_;

	// this callback is used to define where overlays are drawn.
	CallbackFn callback_;
	OverlayId current_overlay_id_;
};

#endif  // end of include guard: WL_WUI_FIELD_OVERLAY_MANAGER_H
