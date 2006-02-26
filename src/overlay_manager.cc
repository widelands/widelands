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
unsigned char Overlay_Manager::get_overlays
(const TCoords c, Overlay_Info * const overlays)
{
	//assert(m_are_graphics_loaded);
	if (not m_are_graphics_loaded) load_graphics(); //  Should REALLY not have to check for this in every call!!! -Erik Sigra

	unsigned char num_ret = 0;

	const Registered_Overlays_Map & overlay_map = m_overlays[c.t];
	Registered_Overlays_Map::const_iterator it = overlay_map.lower_bound(c);
	while
		(it != overlay_map.end()
		 and
		 it->first == c
		 and
		 it->second.level <= 5
		 and
		 num_ret < MAX_OVERLAYS_PER_FIELD) //  Protect overlays array access.
	{
		overlays[num_ret].picid = it->second.picid;
		overlays[num_ret].hotspot_x = it->second.hotspot_x;
		overlays[num_ret].hotspot_y = it->second.hotspot_y;
		++num_ret;
		++it;
	}
	if (c.t == TCoords::None)
		// now overlays
		num_ret += get_build_overlay(c,overlays,num_ret);
	while
		(it != overlay_map.end()
		 and
		 it->first == c
		 and
		 num_ret < MAX_OVERLAYS_PER_FIELD) //  Protect overlays array access.
	{
		overlays[num_ret].picid = it->second.picid;
		overlays[num_ret].hotspot_x = it->second.hotspot_x;
		overlays[num_ret].hotspot_y = it->second.hotspot_y;
		++num_ret;
		++it;
	}
	return num_ret;
}

/*
 * get the build overlays
 *
 * returns one if a overlay was set
 */
inline unsigned char Overlay_Manager::get_build_overlay
(const Coords c, Overlay_Info * const overlays, const int i) const
{
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
      delete[] m_overlay_fields;
      m_overlay_fields=0;
   }
	const Registered_Overlays_Map * const overlays_end = m_overlays + 3;
	for (Registered_Overlays_Map * it = m_overlays; it != overlays_end; ++it)
		it->clear();
	m_road_overlays.clear();
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

   m_overlay_fields = new uchar[w*h];
}

/*
 * Recalculates all calculatable overlays for fields
 */
void Overlay_Manager::recalc_field_overlays
(const FCoords fc, const FCoords * const neighbours)
{
   assert(m_overlay_fields);

   uchar code = 0;
   int owner = fc.field->get_owned_by();

   if (owner) {
      // A border is on every field that is owned by a player and has
      // neighbouring fields that are not owned by that player
      for(int dir = 1; dir <= 6; dir++) {
         const FCoords & neighb = neighbours[dir];

         if (neighb.field->get_owned_by() != owner)
            code = Overlay_Frontier_Base + owner;
      }
   }


   if(!code) { // do not calculate further if there is a border
      // Determine the buildhelp icon for that field
      int buildcaps=0;
      if(m_callback) {
         buildcaps = m_callback(fc, m_callback_data, m_callback_data_i);
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
void Overlay_Manager::register_overlay
(const TCoords c,
 const int picid,
 const int level,
 const Coords hot_spot,
 const int jobid)
{
	assert(c.x >= 0);
	assert(c.y >= 0);
	assert(level!=5); // level == 5 is undefined behavior

	Registered_Overlays_Map & overlay_map = m_overlays[c.t];
   if (overlay_map.count(c)) {
	   Registered_Overlays_Map::const_iterator i = overlay_map.lower_bound(c);
      do {
         if(i->second.picid==picid) return;
         ++i;

      } while (i->first == c);
   }
   int hsx=hot_spot.x;
   int hsy=hot_spot.y;

   if((hot_spot.x == -1) && (hot_spot.y == -1)) {
      g_gr->get_picture_size(picid, &hsx, &hsy);
      hsx>>=1;
      hsy>>=1;
   }

	overlay_map.insert
		(std::pair<const Coords, Registered_Overlays>
		 (c, Registered_Overlays(jobid, picid, hsx, hsy, level)));

   // Now manually sort, so that they are ordered
   //  * first by c (done by std::multimap)
   //  * second by levels (done manually here)

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
void Overlay_Manager::remove_overlay(const TCoords c, const int picid) {
	assert(c.x >= 0);
	assert(c.y >= 0);
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
void Overlay_Manager::remove_overlay(int jobid) {
	const Registered_Overlays_Map * const overlays_end = m_overlays + 3;
	for (Registered_Overlays_Map * j = m_overlays; j != overlays_end; ++j)
		for (Registered_Overlays_Map::iterator it = j->begin(); it != j->end();)
		{
			if (it->second.jobid == jobid) j->erase(it++); //  This is necessary!
			else ++it;
		}
}

/*
 * Register road overlays
 */
void Overlay_Manager::register_road_overlay(Coords c, uchar where, int jobid) {
   assert(c.x<0xffff);
   assert(c.y<0xffff);

   Registered_Road_Overlays overlay = { jobid, where };

   int index=(c.y<<8) + c.x;

   std::map<int,Registered_Road_Overlays>::iterator i;
   i=m_road_overlays.find(index);
   if(i==m_road_overlays.end()) {
      m_road_overlays.insert(std::pair<int,Registered_Road_Overlays>(index,overlay));
   } else {
      i->second=overlay;
   }
}

/*
 * Remove road overlay
 */
void Overlay_Manager::remove_road_overlay(Coords c) {
   assert(static_cast<uint>(c.x)<=0xffff);
   assert(static_cast<uint>(c.y)<=0xffff);

   int fieldindex=(c.y<<8)+c.x;

   std::map<int,Registered_Road_Overlays>::iterator i;
   i=m_road_overlays.find(fieldindex);
   if(i!=m_road_overlays.end()) {
      m_road_overlays.erase(i);
   }
}

/*
 * remove all overlays with this jobid
 */
void Overlay_Manager::remove_road_overlay(int jobid) {
   std::map<int, Registered_Road_Overlays>::iterator i=m_road_overlays.begin();

   while(i!=m_road_overlays.end()) {
      if(i->second.jobid==jobid) {
         m_road_overlays.erase(i++);
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
   m_buildhelp_infos[0].picid=g_gr->get_picture( PicMod_Game,  "pics/set_flag.png" );
   g_gr->get_picture_size(m_buildhelp_infos[0].picid, &m_buildhelp_infos[0].hotspot_x, &m_buildhelp_infos[0].hotspot_y);
   m_buildhelp_infos[0].hotspot_x/=2; m_buildhelp_infos[0].hotspot_y-=1;

   m_buildhelp_infos[1].picid=g_gr->get_picture( PicMod_Game,  "pics/small.png" );
   g_gr->get_picture_size(m_buildhelp_infos[1].picid, &m_buildhelp_infos[1].hotspot_x, &m_buildhelp_infos[1].hotspot_y);
   m_buildhelp_infos[1].hotspot_x/=2; m_buildhelp_infos[1].hotspot_y/=2;

   m_buildhelp_infos[2].picid=g_gr->get_picture( PicMod_Game,  "pics/medium.png" );
   g_gr->get_picture_size(m_buildhelp_infos[2].picid, &m_buildhelp_infos[2].hotspot_x, &m_buildhelp_infos[2].hotspot_y);
   m_buildhelp_infos[2].hotspot_x/=2; m_buildhelp_infos[2].hotspot_y/=2;

   m_buildhelp_infos[3].picid=g_gr->get_picture( PicMod_Game,  "pics/big.png" );
   g_gr->get_picture_size(m_buildhelp_infos[3].picid, &m_buildhelp_infos[3].hotspot_x, &m_buildhelp_infos[3].hotspot_y);
   m_buildhelp_infos[3].hotspot_x/=2; m_buildhelp_infos[3].hotspot_y/=2;

   m_buildhelp_infos[4].picid=g_gr->get_picture( PicMod_Game,  "pics/mine.png" );
   g_gr->get_picture_size(m_buildhelp_infos[4].picid, &m_buildhelp_infos[4].hotspot_x, &m_buildhelp_infos[4].hotspot_y);
   m_buildhelp_infos[4].hotspot_x/=2; m_buildhelp_infos[4].hotspot_y/=2;

   m_are_graphics_loaded=true;
}
