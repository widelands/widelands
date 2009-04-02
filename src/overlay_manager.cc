/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "overlay_manager.h"

#include "field.h"
#include "graphic.h"

#include <algorithm>


Overlay_Manager::Overlay_Manager() :
m_are_graphics_loaded(false),
m_showbuildhelp(false),
m_callback(0)
#ifndef NDEBUG
//  No need to initialize (see comment for get_a_job_id) other than to shut up
//  Valgrind.
/**/,
m_current_job_id(Job_Id::Null())
#endif
{}


/**
 * Returns the currently registered overlays and the buildhelp for a node.
 */
uint8_t Overlay_Manager::get_overlays
	(Widelands::FCoords const c, Overlay_Info * const overlays) const
{
	assert(m_are_graphics_loaded);

	uint8_t num_ret = 0;

	const Registered_Overlays_Map & overlay_map =
		m_overlays[Widelands::TCoords<>::None];
	Registered_Overlays_Map::const_iterator it = overlay_map.lower_bound(c);
	while (it != overlay_map.end() and it->first == c and it->second.level <= 5)
	{
		overlays[num_ret].picid = it->second.picid;
		overlays[num_ret].hotspot = it->second.hotspot;
		if (++num_ret == MAX_OVERLAYS_PER_NODE) goto end;
		++it;
	}
	if (m_showbuildhelp) {
		const uint8_t buildhelp_overlay_index =
			c.field->get_buildhelp_overlay_index();
		if (buildhelp_overlay_index < Widelands::Field::Buildhelp_None) {
			overlays[num_ret] = m_buildhelp_infos[buildhelp_overlay_index];
			if (++num_ret == MAX_OVERLAYS_PER_NODE) goto end;
		}
	}
	while (it != overlay_map.end() and it->first == c) {
		overlays[num_ret].picid = it->second.picid;
		overlays[num_ret].hotspot = it->second.hotspot;
		if (++num_ret == MAX_OVERLAYS_PER_NODE) goto end;
		++it;
	}
end:
	return num_ret;
}

/**
 * Returns the currently registered overlays for a triangle.
 */
uint8_t Overlay_Manager::get_overlays
	(Widelands::TCoords<> const c, Overlay_Info * const overlays) const
{
	assert(m_are_graphics_loaded);
	assert(c.t == Widelands::TCoords<>::D or c.t == Widelands::TCoords<>::R);

	uint8_t num_ret = 0;

	const Registered_Overlays_Map & overlay_map = m_overlays[c.t];
	Registered_Overlays_Map::const_iterator it = overlay_map.lower_bound(c);
	while
		(it != overlay_map.end()
		 and
		 it->first == c
		 and
		 num_ret < MAX_OVERLAYS_PER_TRIANGLE)
	{
		overlays[num_ret].picid = it->second.picid;
		overlays[num_ret].hotspot = it->second.hotspot;
		++num_ret;
		++it;
	}
	return num_ret;
}


/*
 * remove all registered overlays. The Overlay_Manager
 * can than be reused without needing to be delete()ed
 */
void Overlay_Manager::reset() {
	m_are_graphics_loaded = false;
	m_callback = 0;

	const Registered_Overlays_Map * const overlays_end = m_overlays + 3;
	for (Registered_Overlays_Map * it = m_overlays; it != overlays_end; ++it)
		it->clear();
	m_road_overlays.clear();
}


/*
 * Recalculates all calculatable overlays for fields
 */
void Overlay_Manager::recalc_field_overlays(const Widelands::FCoords fc) {
	Widelands::FieldCaps const caps =
		m_callback ?
		static_cast<Widelands::FieldCaps>
		(m_callback(fc, m_callback_data, m_callback_data_i))
		:
		fc.field->get_caps();

	fc.field->set_buildhelp_overlay_index
		(caps & Widelands::BUILDCAPS_MINE                                      ?
		 Widelands::Field::Buildhelp_Mine                                      :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG    ?
		 Widelands::Field::Buildhelp_Big                                       :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_MEDIUM ?
		 Widelands::Field::Buildhelp_Medium                                    :
		 (caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_SMALL  ?
		 Widelands::Field::Buildhelp_Small                                     :
		 caps & Widelands::BUILDCAPS_FLAG                                      ?
		 Widelands::Field::Buildhelp_Flag : Widelands::Field::Buildhelp_None);
}

/*
 * finally, register a new overlay
 */
void Overlay_Manager::register_overlay
	(Widelands::TCoords<> const c,
	 int32_t              const picid,
	 int32_t              const level,
	 Point                      hotspot,
	 Job_Id               const jobid)
{
	assert(c.t <= 2);
	assert(level != 5); //  level == 5 is undefined behavior

	if (hotspot == Point::invalid()) {
		uint32_t picture_width, picture_height;
		g_gr->get_picture_size(picid, picture_width, picture_height);
		hotspot = Point(picture_width / 2, picture_height / 2);
	}

	Registered_Overlays_Map & overlay_map = m_overlays[c.t];
	for
		(Registered_Overlays_Map::iterator it = overlay_map.find(c);
		 it != overlay_map.end() and it->first == c;
		 ++it)
		if
			(it->second.picid   == picid
			 and
			 it->second.hotspot == hotspot
			 and
			 it->second.level   == level)
		{
			it->second.jobids.insert(jobid);
			return;
		}

	overlay_map.insert
		(std::pair<Widelands::Coords const, Registered_Overlays>
		 	(c, Registered_Overlays(jobid, picid, hotspot, level)));

	//  Now manually sort, so that they are ordered
	//    * first by c (done by std::multimap)
	//    * second by levels (done manually here)

	// there is at least one registered
	Registered_Overlays_Map::iterator it = overlay_map.lower_bound(c), jt;

	do {
		jt = it;
		++jt;
		if (jt == overlay_map.end()) break;
		if (jt->first == it->first) {
			// There are several overlays registered for this location.
			if (jt->second.level < it->second.level) {
				std::swap<Overlay_Manager::Registered_Overlays>
					(it->second, jt->second);
				it = overlay_map.lower_bound(c);
			} else ++it;
		} else break; // it is the last element, break this loop.
	} while (it->first == c);
}

/*
 * remove one (or many) overlays from a node or triangle
 */
void Overlay_Manager::remove_overlay
	(Widelands::TCoords<> const c, int32_t const picid)
{
	assert(c.t <= 2);

	Registered_Overlays_Map & overlay_map = m_overlays[c.t];

	if (overlay_map.count(c)) {
		Registered_Overlays_Map::iterator it = overlay_map.lower_bound(c);
		do {
			if (it->second.picid == picid or picid == -1) {
				overlay_map.erase(it);
				it = overlay_map.lower_bound(c);
			} else {
				++it;
			}
		} while (it != overlay_map.end() and it->first == c);
	}
}

/*
 * remove all overlays with this jobid
 */
void Overlay_Manager::remove_overlay(const Job_Id jobid) {
	const Registered_Overlays_Map * const overlays_end = m_overlays + 3;
	for (Registered_Overlays_Map * j = m_overlays; j != overlays_end; ++j)
		for (Registered_Overlays_Map::iterator it = j->begin(); it != j->end();) {
			it->second.jobids.erase(jobid);
			if (it->second.jobids.empty())
				j->erase(it++); //  This is necessary!
			else
				++it;
		}
}

/*
 * Register road overlays
 */
void Overlay_Manager::register_road_overlay
	(Widelands::Coords const c, uint8_t const where, Job_Id const jobid)
{
	const Registered_Road_Overlays overlay = {jobid, where};
	Registered_Road_Overlays_Map::iterator it = m_road_overlays.find(c);
	if (it == m_road_overlays.end())
		m_road_overlays.insert
		(std::pair<const Widelands::Coords,
		 Registered_Road_Overlays>(c, overlay));
	else it->second = overlay;
}

/*
 * Remove road overlay
 */
void Overlay_Manager::remove_road_overlay(const Widelands::Coords c) {
	const Registered_Road_Overlays_Map::iterator it = m_road_overlays.find(c);
	if (it != m_road_overlays.end()) m_road_overlays.erase(it);
}

/*
 * remove all overlays with this jobid
 */
void Overlay_Manager::remove_road_overlay(Job_Id const jobid) {
	Registered_Road_Overlays_Map::iterator it = m_road_overlays.begin();
	const Registered_Road_Overlays_Map::const_iterator end =
		m_road_overlays.end();
	while (it != end) {
		if (it->second.jobid == jobid) m_road_overlays.erase(it++); //  Necessary!
		else ++it;
	}
}

/*
 * call cleanup and then, when graphic is reloaded
 * overlay_manager calls this for himself and everything should be fine
 *
 * Load all the needed graphics
 */
void Overlay_Manager::load_graphics() {
	if (m_are_graphics_loaded) return;

	Overlay_Info * buildhelp_info = m_buildhelp_infos;
	static const char * filenames[] = {
		"pics/set_flag.png",
		"pics/small.png",
		"pics/medium.png",
		"pics/big.png",
		"pics/mine.png"
	};
	const char * const * filename = filenames;

	//  Special case for flag, which has a different formula for hotspot_y.
	buildhelp_info->picid = g_gr->get_picture(PicMod_Game, *filename);
	{
		uint32_t hotspot_x, hotspot_y;
		g_gr->get_picture_size(buildhelp_info->picid, hotspot_x, hotspot_y);
		buildhelp_info->hotspot = Point(hotspot_x / 2, hotspot_y - 1);
	}

	const Overlay_Info * const buildhelp_infos_end =
		buildhelp_info + Widelands::Field::Buildhelp_None;
	for (;;) { // The other buildhelp overlays.
		++buildhelp_info, ++filename;
		if (buildhelp_info == buildhelp_infos_end) break;
		buildhelp_info->picid = g_gr->get_picture(PicMod_Game, *filename);
		uint32_t hotspot_x, hotspot_y;
		g_gr->get_picture_size(buildhelp_info->picid, hotspot_x, hotspot_y);
		buildhelp_info->hotspot = Point(hotspot_x / 2, hotspot_y / 2);
	}

	m_are_graphics_loaded = true;
}
