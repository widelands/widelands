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

#include <algorithm>
#include "overlay_manager.h"
#include "graphic.h"
#include "field.h"
#include "error.h"


/*
 * Constructor
 */
Overlay_Manager::Overlay_Manager(int w, int h) {
   m_showbuildhelp=false;
   m_are_graphics_loaded=false;
   m_w=0;
   m_h=0;
   m_callback=0;
   m_overlay_fields=0;
   m_cur_jobid=1000; // don't start with 0

   init(w,h);
}

/*
 * Destructor
 */
Overlay_Manager::~Overlay_Manager(void) {
   cleanup();
}

/*
 * return the currently registered overlays
 * or the standart help.
 */
int Overlay_Manager::get_overlays(FCoords& c, Overlay_Info* overlays) {
   if(!m_are_graphics_loaded) load_graphics();

   int num_ret=0;

   // are there any below
   int fieldindex= (c.y<<8) + c.x;
   int nov=0;
   if((nov=m_overlays.count(fieldindex))) {
      // there are overlays registered 
      std::map<int, Registered_Overlays>::iterator i=m_overlays.lower_bound(fieldindex);
      while(i->first==fieldindex && i->second.level<=5) {   
         overlays[num_ret].picid=i->second.picid;
         overlays[num_ret].hotspot_x=i->second.hotspot_x;
         overlays[num_ret].hotspot_y=i->second.hotspot_y;
         ++num_ret;
         ++i;
      } 
   
      // now overlays
      num_ret+=get_build_overlay(c,overlays,num_ret);
     
      while(i->first==fieldindex) {
         overlays[num_ret].picid=i->second.picid;
         overlays[num_ret].hotspot_x=i->second.hotspot_x;
         overlays[num_ret].hotspot_y=i->second.hotspot_y;
         ++num_ret;
         ++i;
      }
      return num_ret;
   }

   // no overlays, this will be most common
   num_ret+=get_build_overlay(c,overlays,num_ret);
   return num_ret;
}

/*
 * get the build overlays
 *
 * returns one if a overlay was set
 */
inline int Overlay_Manager::get_build_overlay(FCoords& c, Overlay_Info* overlays, int i) {
   uchar overlay_field = m_overlay_fields[c.y*m_w+c.x];  
   if(m_showbuildhelp && overlay_field >= Overlay_Build_Min &&
         overlay_field <= Overlay_Build_Max) {
      int build_overlay = overlay_field - Overlay_Build_Min;

      overlays[i]=m_buildhelp_infos[build_overlay];
      return 1;
   }
   return 0;
}
      

/*
 * remove all registered overlays. The Overlay_Manager 
 * can than be reused without needing to be delete()ed
 */
void Overlay_Manager::cleanup(void) {
   m_w=0;
   m_h=0;
   m_are_graphics_loaded=false;
   m_callback=0;

   if(m_overlay_fields) {
      delete m_overlay_fields;
      m_overlay_fields=0;
   }
}

/*
 * called when the map changes size 
 * everything should already be invalid
 */
void Overlay_Manager::init(int w, int h) {
   assert(!m_w);
   assert(!m_h);
   assert(!m_overlay_fields);
   
   m_w=w;
   m_h=h;

   m_overlay_fields = (uchar*)malloc(w*h);
}

/*
 * Recalculates all calculatable overlays for fields
 */
void Overlay_Manager::recalc_field_overlays(FCoords& fc, FCoords* neighbours) {
   assert(m_overlay_fields);
   
   uchar code = 0;
   int owner = fc.field->get_owned_by();

   if (owner) {
      // A border is on every field that is owned by a player and has
      // neighbouring fields that are not owned by that player
      for(int dir = 1; dir <= 6; dir++) {
         FCoords neighb=neighbours[dir];

         if (neighb.field->get_owned_by() != owner)
            code = Overlay_Frontier_Base + owner;
      }
   }


   if(!code) { // do not calculate further if there is a border
      // Determine the buildhelp icon for that field
      int buildcaps=0;
      if(m_callback) { 
         buildcaps = m_callback(fc, m_callback_data);
      } else {
         buildcaps = fc.field->get_caps(); 
      }

      if (buildcaps & BUILDCAPS_MINE)
         code = Overlay_Build_Mine;
      else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG)
         code = Overlay_Build_Big;
      else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_MEDIUM)
         code = Overlay_Build_Medium;
      else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_SMALL)
         code = Overlay_Build_Small;
      else if (buildcaps & BUILDCAPS_FLAG)
         code = Overlay_Build_Flag;
   }

	m_overlay_fields[fc.y*m_w + fc.x] = code;

}

/*
 * finally, register a new overlay
 */
void Overlay_Manager::register_overlay(Coords c, int picid, int level, Coords hot_spot, int jobid) {
   assert(static_cast<uint>(c.x)<=0xffff);
   assert(static_cast<uint>(c.y)<=0xffff);
   assert(level!=5); // level == 5 is undefined behavior

   int fieldindex=(c.y<<8)+c.x;
   if(m_overlays.count(fieldindex)) {
      std::map<int, Registered_Overlays>::iterator i=m_overlays.lower_bound(fieldindex);
      do {
         if(i->second.picid==picid) return;
         ++i;

      } while(i->first==fieldindex);
   }
   int hsx=hot_spot.x;
   int hsy=hot_spot.y;

   if((hot_spot.x == -1) && (hot_spot.y == -1)) {
      g_gr->get_picture_size(picid, &hsx, &hsy);
      hsx>>=1;
      hsy>>=1;
   }
      
   Registered_Overlays info= {
      jobid, 
      picid,
      hsx, hsy,
      level
   };

   m_overlays.insert(std::pair<int,Registered_Overlays>(fieldindex, info));

   // Now manually sort, so that they are ordered 
   //  * first by fieldindex (done by std::multimap)
   //  * second by levels (done manually here)
   std::map<int, Registered_Overlays>::iterator i=m_overlays.lower_bound(fieldindex); // theres at least one registered
   std::map<int, Registered_Overlays>::iterator j; 
   do {
      j=i;
      ++j;
      
      if(j->first==i->first) {
         // there are more elements registered
         if(j->second.level > i->second.level) {
            std::swap<Overlay_Manager::Registered_Overlays>(i->second,j->second);
            i=m_overlays.lower_bound(fieldindex);
         } else {
            ++i;
         }
      } else {
         // i is the last element, break this loop
         break;
      }
   } while(i->first==fieldindex);
}

/*
 * remove one (or many) overlays from a field
 */
void Overlay_Manager::remove_overlay(Coords c, int picid) {
   assert(static_cast<uint>(c.x)<=0xffff);
   assert(static_cast<uint>(c.y)<=0xffff);

   int fieldindex=(c.y<<8)+c.x;

   if(m_overlays.count(fieldindex)) {
      std::map<int, Registered_Overlays>::iterator i=m_overlays.lower_bound(fieldindex);
      do {
         if(i->second.picid==picid || picid == -1) {
            m_overlays.erase(i);
            i=m_overlays.lower_bound(fieldindex);
         } else {
            ++i;
         }
      } while(i->first==fieldindex);
   }
}

/*
 * remove all overlays with this jobid
 */
void Overlay_Manager::remove_overlay(int jobid) {
   std::map<int, Registered_Overlays>::iterator i=m_overlays.begin();

   while(i!=m_overlays.end()) {
      if(i->second.jobid==jobid) {
         m_overlays.erase(i++);
      } else {
         ++i;
      }
   }
}

/*
 * [ private function ]
 *
 * call cleanup and then, when graphic is reloaded
 * overlay_manager calls this for himself and everything should be fine
 */
void Overlay_Manager::load_graphics(void) {
   // Load all the needed graphics
   m_buildhelp_infos[0].picid=g_gr->get_picture(PicMod_Game, "pics/set_flag.png", RGBColor(0,0,255));
   g_gr->get_picture_size(m_buildhelp_infos[0].picid, &m_buildhelp_infos[0].hotspot_x, &m_buildhelp_infos[0].hotspot_y); 
   m_buildhelp_infos[0].hotspot_x/=2; m_buildhelp_infos[0].hotspot_y-=1;
   
   m_buildhelp_infos[1].picid=g_gr->get_picture(PicMod_Game, "pics/small.png", RGBColor(0,0,255));
   g_gr->get_picture_size(m_buildhelp_infos[1].picid, &m_buildhelp_infos[1].hotspot_x, &m_buildhelp_infos[1].hotspot_y); 
   m_buildhelp_infos[1].hotspot_x/=2; m_buildhelp_infos[1].hotspot_y/=2;
  
   m_buildhelp_infos[2].picid=g_gr->get_picture(PicMod_Game, "pics/medium.png", RGBColor(0,0,255));
   g_gr->get_picture_size(m_buildhelp_infos[2].picid, &m_buildhelp_infos[2].hotspot_x, &m_buildhelp_infos[2].hotspot_y); 
   m_buildhelp_infos[2].hotspot_x/=2; m_buildhelp_infos[2].hotspot_y/=2;
   
   m_buildhelp_infos[3].picid=g_gr->get_picture(PicMod_Game, "pics/big.png", RGBColor(0,0,255));
   g_gr->get_picture_size(m_buildhelp_infos[3].picid, &m_buildhelp_infos[3].hotspot_x, &m_buildhelp_infos[3].hotspot_y); 
   m_buildhelp_infos[3].hotspot_x/=2; m_buildhelp_infos[3].hotspot_y/=2;
   
   m_buildhelp_infos[4].picid=g_gr->get_picture(PicMod_Game, "pics/mine.png", RGBColor(0,0,255));
   g_gr->get_picture_size(m_buildhelp_infos[4].picid, &m_buildhelp_infos[4].hotspot_x, &m_buildhelp_infos[4].hotspot_y); 
   m_buildhelp_infos[4].hotspot_x/=2; m_buildhelp_infos[4].hotspot_y/=2;
   
   m_are_graphics_loaded=true;
}


