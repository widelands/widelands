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

#ifndef OVERLAY_MANAGER_H
#define OVERLAY_MANAGER_H

#include "field.h"
#include "widelands_geometry.h"

#include <map>
#include <set>

/*
 * The Overlay Manager is responsible for the map overlays. He
 * manages overlays in the following way:
 *   - When someone registered one (or more) special overlays
 *     for a field he draws them accordingly
 *   - If nothing was registered for this field, the Overlay Manager
 *     automatically returns
 *        - buildhelp (if activated)
 *
 *   Advantages are:
 *    - buildcaps do not need to be recalculated all the time
 *    - flexible system
 *
 *    about the level variable:
 *     the level describe when the overlay should be drawn
 *     buildhelp graphics have a level of 5, lower means the
 *     buildhelp will cover the old overlays, otherways the new
 *     overlay will cover the buildhelp.
 *
 *    about jobid:
 *     the jobid can be given to the register function, whenever
 *     the job is finished or canceld, a simple remove_overlay
 *     with the jobid can be called and all overlays created in the
 *     job are removed. This is useful for interactive road building.
 */
#define MAX_OVERLAYS_PER_NODE 5
#define MAX_OVERLAYS_PER_TRIANGLE 3
typedef int32_t (*Overlay_Callback_Function)
	(Widelands::TCoords<Widelands::FCoords>, void *, int32_t);
struct Overlay_Manager {
	struct Job_Id { //  Boxing
		static Job_Id Null() throw () //  Constant value for no job.
		{Job_Id result; result.id = 0; return result;}
		operator bool() const throw () {return id;}
		bool operator<(const Job_Id other) const throw () {return id < other.id;}
	private:
		friend class Overlay_Manager;
		Job_Id operator++() throw () {++id; return *this;}
		bool operator== (Job_Id const other) const throw () {
			return id == other.id;
		}
		uint32_t id;
	};
	struct Overlay_Info {
		uint32_t picid;
		Point hotspot;
	};

	Overlay_Manager();

	void reset();

	//  register callback function (see data below for description)
	void register_overlay_callback_function
		(Overlay_Callback_Function const func,
		 void                    * const data,
		 int32_t                   const iparam1 = 0)
	{
		m_callback = func; m_callback_data = data; m_callback_data_i = iparam1;
	}

	/// Get a job id that is hopefully unused. This function is guaranteed to
	/// never return Job_Id::Null(). All other values are valid and may be
	/// returned. The first call can return any valid value. Other calls
	/// increment the value that was returned in the last call. If the result is
	/// Job_Id::Null() it is incremented again. Then the result is returned.
	/// Since Job_Id is modular, it can wrap around. This means that the
	/// function can return a value that it has returned before. If that job id
	/// is still in use, the logic will fail, since the overlay manager will
	/// consider all jobs with a certain job id as the same job. Therefore a
	/// call to remove_overlay(Job_Id) will remove all overlays that were added
	/// during any job with that id. But the range of Job_Id should be large
	/// enough to ensure that this will not be a problem in practice.
	Job_Id get_a_job_id() {
		++m_current_job_id;
		if (m_current_job_id == Job_Id::Null())
			++m_current_job_id;
		return m_current_job_id;
	}

	void load_graphics();

	/**
	 * Register an overlay at a location (node or triangle). hotspot is the point
	 * of the picture that will be exactly over the location. If hotspot is
	 * Point::invalid(), the center of the picture will be used as hotspot.
	 */
	void register_overlay
		(Widelands::TCoords<>,
		 int32_t picid,
		 int32_t level,
		 Point   hotspot = Point::invalid(),
		 Job_Id          = Job_Id::Null());

	//  if picid == -1 remove all overlays
	void remove_overlay(Widelands::TCoords<>, int32_t picid = -1);

	void remove_overlay(Job_Id);

	uint8_t get_overlays(Widelands::FCoords c, Overlay_Info *) const;
	uint8_t get_overlays(Widelands::TCoords<>, Overlay_Info *) const;

	void show_buildhelp(bool const t) {m_showbuildhelp = t;}
	void toggle_buildhelp() {m_showbuildhelp = !m_showbuildhelp;}

	void recalc_field_overlays(Widelands::FCoords);

	//  Road overlays are registered like normal overlays and removed like
	//  normal overlays but they use are handled internally completely
	//  different. When a road overlay information is requested the same data a
	//  s for a field is returned (a uint8_t which needs to be ANDed).
	void register_road_overlay
		(Widelands::Coords, uint8_t where, Job_Id = Job_Id::Null());
	void remove_road_overlay(Widelands::Coords);
	void remove_road_overlay(Job_Id);
	uint8_t get_road_overlay(const Widelands::Coords c) const {
		Registered_Road_Overlays_Map::const_iterator const it =
			m_road_overlays.find(c);
		if (it != m_road_overlays.end())
			return it->second.where;
		return 0;
	}

private:
	struct Registered_Overlays {
		Registered_Overlays
			(const Job_Id Jobid,
			 const int32_t Picid,
			 const Point  Hotspot,
			 const int32_t Level)
			:
			picid(Picid),
			hotspot(Hotspot),
			level(Level)
		{jobids.insert(Jobid);}
		std::set<Job_Id> jobids;
		int32_t picid;
		Point            hotspot;
		int32_t level;
	};

	struct Registered_Road_Overlays {
		Job_Id jobid;
		uint8_t where;
	};

	typedef
		std::map
		<const Widelands::Coords,
		Registered_Road_Overlays,
		Widelands::Coords::ordering_functor>
		Registered_Road_Overlays_Map;

	Registered_Road_Overlays_Map m_road_overlays;

	typedef
		std::multimap
		<const Widelands::Coords,
		Registered_Overlays,
		Widelands::Coords::ordering_functor>
		Registered_Overlays_Map;

	//  indexed by TCoords<>::TriangleIndex
	Registered_Overlays_Map m_overlays[3];

	Overlay_Info m_buildhelp_infos[Widelands::Field::Buildhelp_None];
	bool m_are_graphics_loaded;
	bool m_showbuildhelp;
	Overlay_Callback_Function m_callback;           // this callback is used to define we're overlays are set and were not
	// since we only care for map stuff, not for player stuff or editor issues
	void * m_callback_data;
	int32_t m_callback_data_i;

	//  No need to initialize m_current_job_id (see comment for get_a_job_id).
	Job_Id m_current_job_id;

};




#endif
