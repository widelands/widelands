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

#include <string>
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
typedef int (*Overlay_Callback_Function)(FCoords&, void*, int);
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
      void register_overlay_callback_function(Overlay_Callback_Function func, void* data, int iparam1 = 0) {
         m_callback=func; m_callback_data=data;  m_callback_data_i=iparam1;
      }

      inline int get_a_job_id(void) { ++m_cur_jobid; if(m_cur_jobid>=std::numeric_limits<int>::max()) m_cur_jobid=1000; return m_cur_jobid; }

      void register_overlay(Coords c, int picid, int level, Coords hot_spot = Coords(-1,-1), int jobid=0);

      void remove_overlay(Coords c, int picid=-1); // if picid == -1 remove all overlays
      void remove_overlay(int jobid);              // remove by jobid

      int get_overlays(FCoords& c, Overlay_Info* overlays);

      void show_buildhelp(bool t) { m_showbuildhelp= t; }
      void toggle_buildhelp(void) { m_showbuildhelp=!m_showbuildhelp; }

      void recalc_field_overlays(FCoords& f, FCoords* neighbours);

      // Road overlays are registered like normal overlays and removed like normal overlays
      // but they use are handled internally completly different. When a road overlay information is requested
      // the same data as for a field is returned (a uchar which needs to be ANDed)
      void register_road_overlay(Coords c, uchar where, int jobid = -1);
      void remove_road_overlay(Coords c);
      void remove_road_overlay(int jobid);
      inline uchar get_road_overlay(Coords& c) {
         std::map<int, Registered_Road_Overlays>::iterator i=m_road_overlays.find((c.y<<8)+c.x);
         if(i!=m_road_overlays.end()) return i->second.where;
         return 0;
      }

      // functions for border. This could be generalized with "in between overlays"
      // but there do not exist any more (yet?). For now, handling borders for themself
      // is ok. this functions are inlined for speed and must be called in a row.
      // calling them in a different order results in undefined behaviour
      inline int is_frontier_field(Coords& fc) {
         // index pointer and m_overlay_basic are invalid
         m_index_pointer=fc.y*m_w + fc.x;
         m_overlay_basic = m_overlay_fields[m_index_pointer];
         if (m_overlay_basic > Overlay_Frontier_Base && m_overlay_basic <= Overlay_Frontier_Max)
            return (m_overlay_basic - Overlay_Frontier_Base);
         return 0;
      }
      inline uchar draw_border_to_right(Coords& fc) {
         // index pointer points to field, m_overlay_basic should be valid
         int index=m_index_pointer;
         if(fc.x==m_w-1) { index-=fc.x; } else { ++index; } // right neighbour
         if(m_overlay_fields[index] == m_overlay_basic) return true;
         return false;
      }
      inline uchar draw_border_to_bottom_left(Coords& fc) {
         // index pointer points to field, m_overlay_basic should be valid
         m_index_pointer+=m_w; // increase by one row
         if(fc.y == m_h-1) m_index_pointer=fc.x;
         if(!(fc.y & 1)) {
            m_index_pointer-=1;
            if(fc.x==0) m_index_pointer+=m_w;
         }
         if(m_overlay_fields[m_index_pointer] == m_overlay_basic) return true;
         return false;
      }

   private:
      // this is always sorted by (y<<8)+x. a unique coordinate which is sortable
      struct Registered_Overlays {
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

      // data
      std::map<int, Registered_Road_Overlays> m_road_overlays;           // for road overlays
      std::multimap<int, Registered_Overlays> m_overlays;
      Overlay_Info m_buildhelp_infos[5];              // flag, small, medium, big, mine
      bool m_are_graphics_loaded;
      bool m_showbuildhelp;
      uchar*	m_overlay_fields;                      // build help
      int m_w, m_h;
      Overlay_Callback_Function m_callback;           // this callback is used to define we're overlays are set and were not
      // since we only care for map stuff, not for player stuff or editor issues
      void *m_callback_data;
      int m_callback_data_i;
      int m_cur_jobid;                                // current job id
      int m_index_pointer;                            // used for frontier drawing
      uchar m_overlay_basic;                          // used for frontier drawing too

      // functions
      void load_graphics();
      inline int get_build_overlay(FCoords& c, Overlay_Info* overlays, int i);
};




#endif // __S__OVERLAY_HANDLER_H
