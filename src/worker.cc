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

#include "widelands.h"
#include "myfile.h"
#include "pic.h"
#include "bob.h"
#include "worker.h"
#include "tribe.h"

// 
// class Worker_Descr
//
int Worker_Descr::read(Binary_file* f) {
   uchar temp;
   
   f->read(name, sizeof(name));

   f->read(&temp, sizeof(uchar));
   is_enabled=temp ? true : false;
   f->read(&walking_speed, sizeof(ushort));

   needs.read(f);

   ushort w, h, sx, sy;
   f->read(&w, sizeof(ushort));
   f->read(&h, sizeof(ushort));
   f->read(&sx, sizeof(ushort));
   f->read(&sy, sizeof(ushort));

   walk_ne.set_dimensions(w, h);
   walk_nw.set_dimensions(w, h);
   walk_w.set_dimensions(w, h);
   walk_sw.set_dimensions(w, h);
   walk_se.set_dimensions(w, h);
   walk_e.set_dimensions(w, h);
   walk_ne.set_hotspot(sx, sy);
   walk_nw.set_hotspot(sx, sy);
   walk_w.set_hotspot(sx, sy);
   walk_sw.set_hotspot(sx, sy);
   walk_se.set_hotspot(sx, sy);
   walk_e.set_hotspot(sx, sy);
   
   walk_ne.read(f);
   walk_e.read(f);
   walk_se.read(f);
   walk_sw.read(f);
   walk_w.read(f);
   walk_nw.read(f);

   return RET_OK;
}

// 
// class Soldier_Descr
// 
int Soldier_Descr::read(Binary_file* f) {
   
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   f->read(&energy, sizeof(ushort));

   attack_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_l.read(f);
   attack1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_l.read(f);
   evade_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_l.read(f);
   evade1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_l.read(f);

   attack_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_r.read(f);
   attack1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_r.read(f);
   evade_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_r.read(f);
   evade1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_r.read(f);
     
   return RET_OK;
}

// Worker class read functions
int Has_Working_Worker_Descr::read(Binary_file* f) {
   working.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working.read(f);
   return RET_OK;
}

int Has_Working1_Worker_Descr::read(Binary_file* f) {
   working1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working1.read(f);
   return RET_OK;
}
int Has_Walk1_Worker_Descr::read(Binary_file* f) {
   walk_ne1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_nw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_sw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_se1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_ne1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_nw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_sw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_se1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   
   walk_ne1.read(f);
   walk_e1.read(f);
   walk_se1.read(f);
   walk_sw1.read(f);
   walk_w1.read(f);
   walk_nw1.read(f);

   return RET_OK;
}
int Scientist::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   return RET_OK;
}

int Searcher::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);

   // nothing additional 
   return RET_OK;
}

int Grower::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Planter::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   
   return RET_OK;
}
int SitDigger_Base::read(Binary_file* f) {
   // Nothing to do
   return RET_OK;
}
int SitDigger::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   
   // Nothing of our own stuff to read
   return RET_OK;
}

int Carrier::read(Binary_file* f) {

   // nothing to do
   return RET_OK;
}

int Def_Carrier::read(Binary_file* f) {
   Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Add_Carrier::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}
int Builder::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}

int Planer::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
      
   return RET_OK;
}

int Explorer::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);

   return RET_OK;
}

int Geologist::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}
