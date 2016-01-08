/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011, 2013 by the Widelands Development Team
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

#include "wui/overlay_manager.h"

#include <algorithm>

#include <stdint.h>

#include "graphic/graphic.h"
#include "logic/field.h"

OverlayManager::OverlayManager() :
	m_are_graphics_loaded(false), m_showbuildhelp(false), m_current_job_id(0) {
}

/**
 * \returns the currently registered overlays and the buildhelp for a node.
 */
uint8_t OverlayManager::get_overlays
	(Widelands::FCoords const c, OverlayInfo * const overlays) const
{
	assert(m_are_graphics_loaded);

	uint8_t num_ret = 0;

	const RegisteredOverlaysMap & overlay_map = m_overlays[Widelands::TCoords<>::None];
	RegisteredOverlaysMap::const_iterator it = overlay_map.lower_bound(c);
	while (it != overlay_map.end() && it->first == c && it->second.level <= MAX_OVERLAYS_PER_NODE)
	{
		assert(it->second.pic);
		overlays[num_ret].pic = it->second.pic;
		overlays[num_ret].hotspot = it->second.hotspot;
		if (++num_ret == MAX_OVERLAYS_PER_NODE)
			goto end;
		++it;
	}
	if (m_showbuildhelp) {
		const uint8_t buildhelp_overlay_index =
			c.field->get_buildhelp_overlay_index();
		if (buildhelp_overlay_index < Widelands::Field::Buildhelp_None) {
			overlays[num_ret] = m_buildhelp_infos[buildhelp_overlay_index];
			if (++num_ret == MAX_OVERLAYS_PER_NODE)
				goto end;
		}
	}
	while (it != overlay_map.end() && it->first == c) {
		overlays[num_ret].pic = it->second.pic;
		overlays[num_ret].hotspot = it->second.hotspot;
		if (++num_ret == MAX_OVERLAYS_PER_NODE)
			goto end;
		++it;
	}
end:
	return num_ret;
}

/**
 * \returns the currently registered overlays for a triangle.
 */
uint8_t OverlayManager::get_overlays
	(Widelands::TCoords<> const c, OverlayInfo * const overlays) const
{
	assert(m_are_graphics_loaded);
	assert(c.t == Widelands::TCoords<>::D || c.t == Widelands::TCoords<>::R);

	uint8_t num_ret = 0;

	const RegisteredOverlaysMap & overlay_map = m_overlays[c.t];
	RegisteredOverlaysMap::const_iterator it = overlay_map.lower_bound(c);
	while
		(it != overlay_map.end()
		 &&
		 it->first == c
		 &&
		 num_ret < MAX_OVERLAYS_PER_TRIANGLE)
	{
		overlays[num_ret].pic = it->second.pic;
		overlays[num_ret].hotspot = it->second.hotspot;
		++num_ret;
		++it;
	}
	return num_ret;
}

/**
 * Recalculates all calculatable overlays for fields
 */
void OverlayManager::recalc_field_overlays(const Widelands::FCoords fc) {
	Widelands::NodeCaps const caps =
	   m_callback ? static_cast<Widelands::NodeCaps>(m_callback(fc)) : fc.field->nodecaps();

	fc.field->set_buildhelp_overlay_index
		(caps & Widelands::BUILDCAPS_MINE                                      ?
		 Widelands::Field::Buildhelp_Mine                                      :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG    ?
			(caps & Widelands::BUILDCAPS_PORT ?
			 Widelands::Field::Buildhelp_Port :
			 Widelands::Field::Buildhelp_Big)
		 :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_MEDIUM ?
		 Widelands::Field::Buildhelp_Medium                                    :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_SMALL  ?
		 Widelands::Field::Buildhelp_Small                                     :
		 caps & Widelands::BUILDCAPS_FLAG                                      ?
		 Widelands::Field::Buildhelp_Flag : Widelands::Field::Buildhelp_None);
}

/**
 * finally, register a new overlay
 */
void OverlayManager::register_overlay
	(Widelands::TCoords<> const c,
	 const Image* pic,
	 int32_t              const level,
	 Point                      hotspot,
	 JobId               const jobid)
{
	assert(c.t <= 2);
	assert(level != 5); //  level == 5 is undefined behavior

	if (hotspot == Point::invalid()) {
		hotspot = Point(pic->width() / 2, pic->height() / 2);
	}

	RegisteredOverlaysMap & overlay_map = m_overlays[c.t];
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
			it->second.jobids.insert(jobid);
			return;
		}

	overlay_map.insert
		(std::pair<Widelands::Coords const, RegisteredOverlays>
		 	(c, RegisteredOverlays(jobid, pic, hotspot, level)));

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
void OverlayManager::remove_overlay
	(Widelands::TCoords<> const c, const Image* pic)
{
	assert(c.t <= 2);

	RegisteredOverlaysMap & overlay_map = m_overlays[c.t];

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

/**
 * remove all overlays with this jobid
 */
void OverlayManager::remove_overlay(const JobId jobid) {
	const RegisteredOverlaysMap * const overlays_end = m_overlays + 3;
	for (RegisteredOverlaysMap * j = m_overlays; j != overlays_end; ++j)
		for (RegisteredOverlaysMap::iterator it = j->begin(); it != j->end();) {
			it->second.jobids.erase(jobid);
			if (it->second.jobids.empty())
				j->erase(it++); //  This is necessary!
			else
				++it;
		}
}

/**
 * Register road overlays
 */
void OverlayManager::register_road_overlay
	(Widelands::Coords const c, uint8_t const where, JobId const jobid)
{
	const RegisteredRoadOverlays overlay = {jobid, where};
	RegisteredRoadOverlaysMap::iterator it = m_road_overlays.find(c);
	if (it == m_road_overlays.end())
		m_road_overlays.insert
			(std::pair<const Widelands::Coords,
			 RegisteredRoadOverlays>(c, overlay));
	else
		it->second = overlay;
}

/**
 * Remove road overlay
 */
void OverlayManager::remove_road_overlay(const Widelands::Coords c) {
	const RegisteredRoadOverlaysMap::iterator it = m_road_overlays.find(c);
	if (it != m_road_overlays.end())
		m_road_overlays.erase(it);
}

/**
 * remove all overlays with this jobid
 */
void OverlayManager::remove_road_overlay(JobId const jobid) {
	RegisteredRoadOverlaysMap::iterator it = m_road_overlays.begin();
	const RegisteredRoadOverlaysMap::const_iterator end =
		m_road_overlays.end();
	while (it != end)
		if (it->second.jobid == jobid)
			m_road_overlays.erase(it++); //  Necessary!
		else
			++it;
}

/**
 * call cleanup and then, when graphic is reloaded
 * overlay_manager calls this for himself and everything should be fine
 *
 * Load all the needed graphics
 */
void OverlayManager::load_graphics() {
	if (m_are_graphics_loaded)
		return;

	OverlayInfo * buildhelp_info = m_buildhelp_infos;
	static const char * filenames[] = {
		"pics/set_flag.png",
		"pics/small.png",
		"pics/medium.png",
		"pics/big.png",
		"pics/mine.png",
		"pics/port.png"
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

	m_are_graphics_loaded = true;
}

void OverlayManager::register_overlay_callback_function(CallbackFn function) {
	m_callback = function;
}

void OverlayManager::remove_overlay_callback_function() {
	m_callback.clear();
}

OverlayManager::JobId OverlayManager::get_a_job_id() {
	++m_current_job_id;
	if (m_current_job_id == 0)
		++m_current_job_id;
	return m_current_job_id;
}
