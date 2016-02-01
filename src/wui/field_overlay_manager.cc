/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

FieldOverlayManager::FieldOverlayManager() : current_overlay_id_(0) {
	OverlayInfo * buildhelp_info = buildhelp_infos_;
	const char * filenames[] = {
		"images/wui/overlays/set_flag.png",
		"images/wui/overlays/small.png",
		"images/wui/overlays/medium.png",
		"images/wui/overlays/big.png",
		"images/wui/overlays/mine.png",
		"images/wui/overlays/port.png"
	};
	const char * const * filename = filenames;

	//  Special case for flag, which has a different formula for hotspot_y.
	buildhelp_info->pic = g_gr->images().get(*filename);
	buildhelp_info->hotspot = Point(buildhelp_info->pic->width() / 2, buildhelp_info->pic->height() - 1);

	const OverlayInfo * const buildhelp_infos_end =
		buildhelp_info + Widelands::Field::Buildhelp_None;
	for (;;) { // The other buildhelp overlays.
		++buildhelp_info, ++filename;
		if (buildhelp_info == buildhelp_infos_end)
			break;
		buildhelp_info->pic = g_gr->images().get(*filename);
		buildhelp_info->hotspot = Point(buildhelp_info->pic->width() / 2, buildhelp_info->pic->height() / 2);
	}

}

bool FieldOverlayManager::buildhelp() const {
	return buildhelp_;
}

void FieldOverlayManager::show_buildhelp(const bool value) {
	buildhelp_ = value;
}

void FieldOverlayManager::get_overlays(Widelands::FCoords const c,
                                          std::vector<OverlayInfo>* result) const {
	const RegisteredOverlaysMap & overlay_map = overlays_[Widelands::TCoords<>::None];
	RegisteredOverlaysMap::const_iterator it = overlay_map.lower_bound(c);

	while (it != overlay_map.end() && it->first == c && it->second.level <= kLevelForBuildHelp) {
		result->emplace_back(it->second.pic, it->second.hotspot);
		++it;
	}

	if (buildhelp_) {
		int buildhelp_overlay_index = get_buildhelp_overlay(c);
		if (buildhelp_overlay_index < Widelands::Field::Buildhelp_None) {
		   result->emplace_back(buildhelp_infos_[buildhelp_overlay_index]);
	   }
	}

	while (it != overlay_map.end() && it->first == c) {
		result->emplace_back(it->second.pic, it->second.hotspot);
		++it;
	}
}

void FieldOverlayManager::get_overlays(Widelands::TCoords<> const c,
                                       std::vector<OverlayInfo>* result) const {
	assert(c.t == Widelands::TCoords<>::D || c.t == Widelands::TCoords<>::R);


	const RegisteredOverlaysMap & overlay_map = overlays_[c.t];
	RegisteredOverlaysMap::const_iterator it = overlay_map.lower_bound(c);
	while (it != overlay_map.end() && it->first == c) {
		result->emplace_back(it->second.pic, it->second.hotspot);
		++it;
	}
}

int FieldOverlayManager::get_buildhelp_overlay(const Widelands::FCoords& fc) const {
	Widelands::NodeCaps const caps =
	   callback_ ? static_cast<Widelands::NodeCaps>(callback_(fc)) : fc.field->nodecaps();

	const int value = caps & Widelands::BUILDCAPS_MINE ?
	               Widelands::Field::Buildhelp_Mine :
	               (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG ?
	               (caps & Widelands::BUILDCAPS_PORT ? Widelands::Field::Buildhelp_Port :
	                                                   Widelands::Field::Buildhelp_Big) :
	               (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_MEDIUM ?
	               Widelands::Field::Buildhelp_Medium :
	               (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_SMALL ?
	               Widelands::Field::Buildhelp_Small :
	               caps & Widelands::BUILDCAPS_FLAG ? Widelands::Field::Buildhelp_Flag :
	                                                  Widelands::Field::Buildhelp_None;
	return value;
}

void FieldOverlayManager::register_overlay
	(Widelands::TCoords<> const c,
	 const Image* pic,
	 int32_t              const level,
	 Point                      hotspot,
	 OverlayId               const overlay_id)
{
	assert(c.t <= 2);
	assert(level != 5); //  level == 5 is undefined behavior

	if (hotspot == Point::invalid()) {
		hotspot = Point(pic->width() / 2, pic->height() / 2);
	}

	RegisteredOverlaysMap & overlay_map = overlays_[c.t];
	for
		(RegisteredOverlaysMap::iterator it = overlay_map.find(c);
		 it != overlay_map.end() && it->first == c;
		 ++it)
		if
			(it->second.pic   == pic
			 &&
			 it->second.hotspot == hotspot
			 &&
			 it->second.level   == level)
		{
			it->second.overlay_ids.insert(overlay_id);
			return;
		}

	overlay_map.insert
		(std::pair<Widelands::Coords const, RegisteredOverlays>
		 	(c, RegisteredOverlays(overlay_id, pic, hotspot, level)));

	//  Now manually sort, so that they are ordered
	//    * first by c (done by std::multimap)
	//    * second by levels (done manually here)

	// there is at least one registered
	RegisteredOverlaysMap::iterator it = overlay_map.lower_bound(c), jt;

	do {
		jt = it;
		++jt;
		if (jt == overlay_map.end())
			break;
		if (jt->first == it->first) {
			// There are several overlays registered for this location.
			if (jt->second.level < it->second.level) {
				std::swap(it->second, jt->second);
				it = overlay_map.lower_bound(c);
			} else ++it;
		} else break; // it is the last element, break this loop.
	} while (it->first == c);
}

/**
 * remove one (or many) overlays from a node or triangle
 *
 * @param pic    The overlay to remove. If null, all overlays are removed.
 */
void FieldOverlayManager::remove_overlay(Widelands::TCoords<> const c, const Image* pic) {
	assert(c.t <= 2);

	RegisteredOverlaysMap & overlay_map = overlays_[c.t];

	if (overlay_map.count(c)) {
		RegisteredOverlaysMap::iterator it = overlay_map.lower_bound(c);
		do {
			if (!pic || it->second.pic == pic) {
				overlay_map.erase(it);
				it = overlay_map.lower_bound(c);
			} else {
				++it;
			}
		} while (it != overlay_map.end() && it->first == c);
	}
}

void FieldOverlayManager::remove_overlay(const OverlayId overlay_id) {
	const RegisteredOverlaysMap * const overlays_end = overlays_ + 3;
	for (RegisteredOverlaysMap * j = overlays_; j != overlays_end; ++j)
		for (RegisteredOverlaysMap::iterator it = j->begin(); it != j->end();) {
			it->second.overlay_ids.erase(overlay_id);
			if (it->second.overlay_ids.empty())
				j->erase(it++); //  This is necessary!
			else
				++it;
		}
}

void FieldOverlayManager::remove_all_overlays() {
	overlays_[0].clear();
	overlays_[1].clear();
	overlays_[2].clear();
}

void FieldOverlayManager::register_overlay_callback_function(CallbackFn function) {
	callback_ = function;
}

FieldOverlayManager::OverlayId FieldOverlayManager::next_overlay_id() {
	++current_overlay_id_;
	return current_overlay_id_;
}
