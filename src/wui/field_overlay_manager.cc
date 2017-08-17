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

#include "wui/field_overlay_manager.h"

#include <algorithm>

#include <stdint.h>

#include "graphic/graphic.h"
#include "logic/field.h"

FieldOverlayManager::FieldOverlayManager() : buildhelp_(false), current_overlay_id_(0) {
	OverlayInfo* buildhelp_info = buildhelp_infos_;
	const char* filenames[] = {"images/wui/overlays/set_flag.png", "images/wui/overlays/small.png",
	                           "images/wui/overlays/medium.png",   "images/wui/overlays/big.png",
	                           "images/wui/overlays/mine.png",     "images/wui/overlays/port.png"};
	const char* const* filename = filenames;

	//  Special case for flag, which has a different formula for hotspot_y.
	buildhelp_info->pic = g_gr->images().get(*filename);
	buildhelp_info->hotspot =
	   Vector2i(buildhelp_info->pic->width() / 2, buildhelp_info->pic->height() - 1);

	const OverlayInfo* const buildhelp_infos_end = buildhelp_info + Widelands::Field::Buildhelp_None;
	for (;;) {  // The other buildhelp overlays.
		++buildhelp_info;
		++filename;
		if (buildhelp_info == buildhelp_infos_end)
			break;
		buildhelp_info->pic = g_gr->images().get(*filename);
		buildhelp_info->hotspot =
		   Vector2i(buildhelp_info->pic->width() / 2, buildhelp_info->pic->height() / 2);
	}
}

bool FieldOverlayManager::buildhelp() const {
	return buildhelp_;
}

void FieldOverlayManager::show_buildhelp(const bool value) {
	buildhelp_ = value;
}

int FieldOverlayManager::get_buildhelp_overlay(const Widelands::FCoords& fc) const {
	Widelands::NodeCaps const caps =
	   callback_ ? static_cast<Widelands::NodeCaps>(callback_(fc)) : fc.field->nodecaps();

	if (caps & Widelands::BUILDCAPS_MINE) {
		return Widelands::Field::Buildhelp_Mine;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		if (caps & Widelands::BUILDCAPS_PORT) {
			return Widelands::Field::Buildhelp_Port;
		}
		return Widelands::Field::Buildhelp_Big;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_MEDIUM) {
		return Widelands::Field::Buildhelp_Medium;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_SMALL) {
		return Widelands::Field::Buildhelp_Small;
	}
	if (caps & Widelands::BUILDCAPS_FLAG) {
		return Widelands::Field::Buildhelp_Flag;
	}
	return Widelands::Field::Buildhelp_None;
}

void FieldOverlayManager::register_overlay(const Widelands::Coords& c,
                                           const Image* pic,
                                           const OverlayLevel& level,
                                           Vector2i hotspot,
                                           OverlayId const overlay_id) {
	if (hotspot == Vector2i::invalid()) {
		hotspot = Vector2i(pic->width() / 2, pic->height() / 2);
	}

	for (auto it = overlays_.find(c); it != overlays_.end() && it->first == c; ++it)
		if (it->second.pic == pic && it->second.hotspot == hotspot && it->second.level == level) {
			it->second.overlay_ids.insert(overlay_id);
			return;
		}

	overlays_.insert(std::pair<Widelands::Coords const, RegisteredOverlays>(
	   c, RegisteredOverlays(overlay_id, pic, hotspot, level)));

	//  Now manually sort, so that they are ordered
	//    * first by c (done by std::multimap)
	//    * second by levels (done manually here)

	// there is at least one registered
	auto it = overlays_.lower_bound(c);
	do {
		auto jt = it;
		++jt;
		if (jt == overlays_.end())
			break;
		if (jt->first == it->first) {
			// There are several overlays registered for this location.
			if (jt->second.level < it->second.level) {
				std::swap(it->second, jt->second);
				it = overlays_.lower_bound(c);
			} else
				++it;
		} else
			break;  // it is the last element, break this loop.
	} while (it->first == c);
}

/**
 * remove one (or many) overlays from a node or triangle
 *
 * @param pic    The overlay to remove. If null, all overlays are removed.
 */
void FieldOverlayManager::remove_overlay(const Widelands::Coords& c, const Image* pic) {
	if (overlays_.count(c) == 0) {
		return;
	}
	auto it = overlays_.lower_bound(c);
	do {
		if (!pic || it->second.pic == pic) {
			overlays_.erase(it);
			it = overlays_.lower_bound(c);
		} else {
			++it;
		}
	} while (it != overlays_.end() && it->first == c);
}

void FieldOverlayManager::remove_overlay(const OverlayId overlay_id) {
	for (auto it = overlays_.begin(); it != overlays_.end();) {
		it->second.overlay_ids.erase(overlay_id);
		if (it->second.overlay_ids.empty()) {
			overlays_.erase(it++);  //  This is necessary!
		} else {
			++it;
		}
	}
}

void FieldOverlayManager::remove_all_overlays() {
	overlays_.clear();
}

void FieldOverlayManager::register_overlay_callback_function(CallbackFn function) {
	callback_ = function;
}

FieldOverlayManager::OverlayId FieldOverlayManager::next_overlay_id() {
	++current_overlay_id_;
	return current_overlay_id_;
}

bool FieldOverlayManager::is_enabled(const OverlayLevel& level) const {
	return disabled_layers_.count(level) == 0;
}

void FieldOverlayManager::set_enabled(const OverlayLevel& level, const bool enabled) {
	if (enabled) {
		disabled_layers_.erase(level);
	} else {
		disabled_layers_.insert(level);
	}
}
