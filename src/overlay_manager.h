/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__OVERLAY_HANDLER_H
#define __S__OVERLAY_HANDLER_H

#include <string>
#include <map>
#include <limits>
#include "types.h"
#include "geometry.h"
#include "map.h"


/*
 * The Overlay Manager is responsible for the map overlays. He
 * manages overlays in the following way:
 *   - When someone registered one (or more) special overlays
 *     for a field he draws them accordingly
 *   - If nothing was registered for this field, the Overlay Manager
 *     automatically returns
 *        - borders
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
 *     job are removed. This is usefull for interactive road building.
 */
#define MAX_OVERLAYS_PER_NODE 5
#define MAX_OVERLAYS_PER_TRIANGLE 3
typedef int (*Overlay_Callback_Function)(const TCoords, void*, int);
class Overlay_Manager {
   public:
      struct Overlay_Info {
         int picid;
         int hotspot_x;
         int hotspot_y;
      };

   public:
      Overlay_Manager();

	void reset();

      // register callback function (see data below for description)
      void register_overlay_callback_function(Overlay_Callback_Function func, void* data, int iparam1 = 0) {
         m_callback=func; m_callback_data=data;  m_callback_data_i=iparam1;
      }

	int get_a_job_id() {
		if (++m_current_job_id >= std::numeric_limits<int>::max())
			m_current_job_id = 1000;
		return m_current_job_id;
	}

	void load_graphics();

	void register_overlay
		(const TCoords t,
		 const int picid,
		 const int level,
		 const Coords hot_spot = Coords(-1,-1),
		 const int jobid = 0);

	//  if picid == -1 remove all overlays
	void remove_overlay(const TCoords, const int picid = -1);

	void remove_overlay(const int jobid); //  remove by jobid

	unsigned char get_overlays(const FCoords c, Overlay_Info * const) const;
	unsigned char get_overlays
		(const TCoords, Overlay_Info * const overlays) const;

      void show_buildhelp(bool t) { m_showbuildhelp= t; }
      void toggle_buildhelp(void) { m_showbuildhelp=!m_showbuildhelp; }

	void recalc_field_overlays(const FCoords, const FCoords * const neighbours);

	//  Road overlays are registered like normal overlays and removed like normal
	//  overlays but they use are handled internally completly different. When a
	//  road overlay information is requested the same data as for a field is
	//  returned (a uchar which needs to be ANDed).
	void register_road_overlay
		(const Coords, const uchar where, const int jobid = -1);
	void remove_road_overlay(const Coords);
	void remove_road_overlay(const int jobid);
	uchar get_road_overlay(const Coords c) const {
		Registered_Road_Overlays_Map::const_iterator it = m_road_overlays.find(c);
		if (it != m_road_overlays.end()) return it->second.where;
		return 0;
	}

   private:
      // this is always sorted by (y<<8)+x. a unique coordinate which is sortable
	struct Registered_Overlays {
		Registered_Overlays
			(const int Jobid,
			 const int Picid,
			 const int Hotspot_x,
			 const int Hotspot_y,
			 const int Level)
			:
			jobid(Jobid),
			picid(Picid),
			hotspot_x(Hotspot_x),
			hotspot_y(Hotspot_y),
			level(Level)
		{}
		int jobid;
		int picid;
		int hotspot_x;
		int hotspot_y;
		int level;
	};

      struct Registered_Road_Overlays {
         int jobid;
         uchar where;
      };

	typedef
		std::map<const Coords, Registered_Road_Overlays, Coords::ordering_functor>
		Registered_Road_Overlays_Map;

	Registered_Road_Overlays_Map m_road_overlays;

	typedef
		std::multimap<const Coords, Registered_Overlays, Coords::ordering_functor>
		Registered_Overlays_Map;

	//  indexed by TCoords::TriangleIndex
	Registered_Overlays_Map m_overlays[3];

	Overlay_Info m_buildhelp_infos[Field::Buildhelp_None];
      bool m_are_graphics_loaded;
      bool m_showbuildhelp;
      Overlay_Callback_Function m_callback;           // this callback is used to define we're overlays are set and were not
      // since we only care for map stuff, not for player stuff or editor issues
      void *m_callback_data;
      int m_callback_data_i;
	int m_current_job_id;
};




#endif // __S__OVERLAY_HANDLER_H
