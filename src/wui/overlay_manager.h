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

#ifndef OVERLAY_MANAGER_H
#define OVERLAY_MANAGER_H

#include <map>
#include <set>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "logic/field.h"
#include "logic/widelands_geometry.h"

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
#define MAX_OVERLAYS_PER_NODE 6
#define MAX_OVERLAYS_PER_TRIANGLE 3
struct Overlay_Manager {
	struct Job_Id { //  Boxing

		/// Constant value for no job.
		static Job_Id Null() {Job_Id result; result.id = 0; return result;}

		operator bool() const {return id;}
		bool operator<(const Job_Id other) const {return id < other.id;}
	private:
		friend struct Overlay_Manager;
		Job_Id operator++() {++id; return *this;}
		bool operator== (Job_Id const other) const {
			return id == other.id;
		}
		uint32_t id;
	};

	struct Overlay_Info {
		const Image* pic;
		Point hotspot;
	};

	typedef boost::function<int32_t(const Widelands::TCoords<Widelands::FCoords>& coordinates)>
	CallbackFunction;

	Overlay_Manager();

	//  register callback function (see data below for description)
	void register_overlay_callback_function(CallbackFunction function);
	void remove_overlay_callback_function();

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
		 const Image* pic,
		 int32_t level,
		 Point   hotspot = Point::invalid(),
		 Job_Id          = Job_Id::Null());

	// removes all overlays when pic is zero
	void remove_overlay(Widelands::TCoords<>, const Image* pic);
	void remove_overlay(Job_Id);

	uint8_t get_overlays(Widelands::FCoords c, Overlay_Info *) const;
	uint8_t get_overlays(Widelands::TCoords<>, Overlay_Info *) const;

	boost::function<void(bool)> onBuildHelpToggle;
	bool buildhelp() {return m_showbuildhelp;}
	void show_buildhelp(bool const t) {
		if (m_showbuildhelp != t) {
			m_showbuildhelp = t;
			if (onBuildHelpToggle) onBuildHelpToggle(m_showbuildhelp);
		}
	}
	void toggle_buildhelp() {
		m_showbuildhelp = !m_showbuildhelp;
		if (onBuildHelpToggle) onBuildHelpToggle(m_showbuildhelp);
	}

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
			(const Job_Id    Jobid,
			 const Image* Picid,
			 const Point     Hotspot,
			 const int32_t   Level)
			:
			pic(Picid),
			hotspot(Hotspot),
			level(Level)
		{
			jobids.insert(Jobid);
		}
		std::set<Job_Id> jobids;
		const Image*  pic;
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

	// this callback is used to define we're overlays are set and were not
	CallbackFunction m_callback;

	//  No need to initialize m_current_job_id (see comment for get_a_job_id).
	Job_Id m_current_job_id;

};

#endif
