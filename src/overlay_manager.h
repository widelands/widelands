/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include <map>
#include <limits>
#include "types.h"
#include "geometry.h"

class Map;

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
#define MAX_OVERLAYS_PER_FIELD 5    // this should be enough
typedef int (*Overlay_Callback_Function)(FCoords&, void*);
class Overlay_Manager {
   public:
      struct Overlay_Info {
         int picid;
         int hotspot_x;
         int hotspot_y;
      };

   public:
      Overlay_Manager(int w,int h);
      ~Overlay_Manager();

      void cleanup();
      void init(int w, int h);

      // register callback function (see data below for description)
      void register_overlay_callback_function(Overlay_Callback_Function func, void* data) {
         m_callback=func; m_callback_data=data; 
      }
      
      inline int get_a_job_id(void) { ++m_cur_jobid; if(m_cur_jobid>=std::numeric_limits<int>::max()) m_cur_jobid=1000; return m_cur_jobid; }

      void register_overlay(Coords c, int picid, int level, Coords hot_spot = Coords(-1,-1), int jobid=0);

      void remove_overlay(Coords c, int picid=-1); // if picid == -1 remove all overlays
      void remove_overlay(int jobid);              // remove by jobid

      int get_overlays(FCoords c, Overlay_Info* overlays);

      void show_buildhelp(bool t) { m_showbuildhelp= t; }
      void toggle_buildhelp(void) { m_showbuildhelp=!m_showbuildhelp; } 
     
      void recalc_field_overlays(FCoords& f, FCoords* neighbours);
      uchar get_overlay_fields(int i) { return m_overlay_fields[i]; } // TODO: remove me

      // void recalc_in_between_overlays(); 
      //      void register_road_overlay(); 

   private:
      // this is always sorted by (x<<8)+y. a unique coordinate which is sortable 
      struct Registered_Overlays {
         int jobid;
         int picid;
         int hotspot_x;
         int hotspot_y;
         int level;
      };

      // data
      std::multimap<int, Registered_Overlays> m_overlays;
      Overlay_Info m_buildhelp_infos[5];              // flag, small, medium, big, mine
      bool m_are_graphics_loaded;
      bool m_showbuildhelp;
	   uchar*	m_overlay_fields;                      // build help
      int m_w, m_h;
      Overlay_Callback_Function m_callback;           // this callback is used to define we're overlays are set and were not
                                                      // since we only care for map stuff, not for player stuff or editor issues
      int m_cur_jobid;                                // current job id
      
      void *m_callback_data;

      // functions
      void load_graphics();
};


// TODO: move this to cc file
// This is private to this file
enum {
	Overlay_Frontier_Base = 0,	// add the player number to mark a border field
	Overlay_Frontier_Max = 15,

	Overlay_Build_Flag = 16,
	Overlay_Build_Small,
	Overlay_Build_Medium,
	Overlay_Build_Big,
	Overlay_Build_Mine,

	Overlay_Build_Min = Overlay_Build_Flag,
	Overlay_Build_Max = Overlay_Build_Mine,
};



#endif // __S__OVERLAY_HANDLER_H
