/*
 * Copyright (C) 2001 by the Widelands Development Team
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

#ifndef __S__FIELD_H
#define __S__FIELD_H

// TODO: delete the Instance_Link list of each field on game shutdown
// TODO; think, if we shouldn't call for each field a new() in map::set_size
// and a delete, for proper garbage collection, since here is a memory hole, at
// the moment (it would also spare the if(!obj) call in hook_instance

class Terrain_Descr;
class Map_Object;
class Instance;

struct Instance_Link {
   Instance_Link *prev, *next;
   Instance* inst;
};

class Field {
   friend class Map;
   private:
   uchar height;
   char brightness;
   Terrain_Descr *terr, *terd;
   Instance_Link* inst_first; 
   uchar seen_by;
   uchar owned_by;

   public:
   enum Build_Symbol {
      NOTHING,
      FLAG,
      SMALL,
      MEDIUM,
      BIG,
      MINE,
      PORT
   };

   inline uchar get_height() const { return height; }

   inline Terrain_Descr *get_terr() const { return terr; }
   inline Terrain_Descr *get_terd() const { return terd; }
   inline void set_terrainr(Terrain_Descr *p) { assert(p); terr = p; }
   inline void set_terraind(Terrain_Descr *p) { assert(p); terd = p; }
   bool has_hooked(uchar);
     
   inline void hook_instance(Instance* obj) { 
      if(!obj) {
         // This function gets called once to bring the field in 
         // a good shape with obj==0
         inst_first=0;
         return;
      }

      assert(obj);

      if(!inst_first) {
         inst_first=new Instance_Link();
         inst_first->next=0;
         inst_first->prev=0;
         inst_first->inst=obj;
         return;
      } else {
         Instance_Link* i=inst_first;
         while(i->next) i=i->next;
         i->next=new Instance_Link();
         i->next->prev=i;
         i=i->next;
         i->next=0;
         i->inst=obj;
      }
   }
   

     
   inline Instance_Link* get_first_inst(void) { return inst_first; }

   void set_brightness(int l, int r, int tl, int tr, int bl, int br);
   inline char get_brightness() const { return brightness; }

   inline void set_seen_by(uint pln, bool seen) { 
      assert(pln<9); 
      if(seen) {
         seen_by &= (pln-1);   
      } else {
         seen_by &= ~(pln-1);   
      }
   }
   inline bool get_seen_by(uint pln) { assert(pln<9); return (seen_by & (pln-1)); }
   inline void set_owned_by(uint pln) { owned_by=pln; }
   inline uchar get_owned_by(void) { return owned_by; }

   // functions to move one instance from a field to another
   Instance_Link* unhook_inst_link(Map_Object* obj);
   void hook_inst_link(Instance_Link* link); 

   private:
   // note: you must reset this field's + neighbor's brightness when you change the height
   // Map's set_height does this
   inline void set_height(uchar h) { height = h; }
};


#endif
