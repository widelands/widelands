/*
 * Copyright (C) 2002 by the Widelands Development Team  
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

#include "soldier_descr.h"
#include "../src/graphic.h"

Fabric<Soldier_Descr> soldierf;

//
// Soldier Description Class
// 
Soldier_Descr::Soldier_Descr(const char* gname) : Worker_Descr(gname), 
         Menu_Worker_Descr(gname) {
   energy=0;
}
Soldier_Descr::~Soldier_Descr(void) {
}

int Soldier_Descr::construct(Profile* p, Section* s) {
   int retval;

//    cerr << "Parsing soldier \"" << get_name() << "\"!" << endl;
  
   retval=Worker_Descr::construct(p,s);
   if(retval) return retval;
   retval=Menu_Worker_Descr::construct(p,s);
   if(retval) return retval;

   // parsing energy
   energy=s->get_int("energy", 0);
   if(!energy) {
      strcpy(err_sec,s->get_name());
      strcpy(err_key,"energy");
      return KEY_MISSING;
   }

/*   uchar r=255;
   uchar g=255;
   uchar b=255;
   Section* def= p->get_section("defaults");
   if(def) {
      r=def->get_int("clrkey_r", r);
      g=def->get_int("clrkey_g", g);
      b=def->get_int("clrkey_b", b);
   }
   r=s->get_int("clrkey_r", r);
   g=s->get_int("clrkey_g", g);
   b=s->get_int("clrkey_b", b);
   clrkey=pack_rgb(r, g, b);
  */
   
   // parsing extra bobs
   retval=create_bob(p, s, "_attackl_??.bmp", "attack_left", &attack_l);
   if(retval) return retval; 
   retval=create_bob(p, s, "_attackl1_??.bmp", "attack1_left", &attack1_l);
   if(retval) return retval; 
   retval=create_bob(p, s, "_evadel_??.bmp", "evade_left", &evade_l);
   if(retval) return retval; 
   retval=create_bob(p, s, "_evadel1_??.bmp", "evade1_left", &evade1_l);
   if(retval) return retval; 
   retval=create_bob(p, s, "_attackr_??.bmp", "attack_right", &attack_r);
   if(retval) return retval; 
   retval=create_bob(p, s, "_attackr1_??.bmp", "attack1_right", &attack1_r);
   if(retval) return retval; 
   retval=create_bob(p, s, "_evader_??.bmp", "evade_right", &evade_r);
   if(retval) return retval; 
   retval=create_bob(p, s, "_evader1_??.bmp", "evade1_right", &evade1_r);
   if(retval) return retval; 

   return OK;
}
         
int Soldier_Descr::write(Binary_file* f) {
   
   // First, write recognition
   uchar temp;
   temp=SOLDIER;
   f->write(&temp, sizeof(uchar));

   // write standart worker descr 
   Worker_Descr::write(f);
   Menu_Worker_Descr::write(f);
   
   // write soldier extras
   ushort temp1=energy;
   f->write(&temp1, sizeof(ushort));
   
   // write bobs
   attack_l.write(f);
   attack1_l.write(f);
   evade_l.write(f);
   evade1_l.write(f);
   attack_r.write(f);
   attack1_r.write(f);
   evade_r.write(f);
   evade1_r.write(f);
   
   return 0;
}

