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

#ifndef __S__WORKER_DESCR_H
#define __S__WORKER_DESCR_H

#include "ware.h"

class Worker_Descr {
   friend class Tribe_Descr;
   
   public:
      Worker_Descr(void) {  }
      virtual ~Worker_Descr(void) { }
       
   protected:
      virtual int read(Binary_file* f);

      char name[30];
      bool is_enabled;
      ushort walking_speed;
      short nneeds;
      Need_List needs;   
      Animation walk_ne;
      Animation walk_nw;
      Animation walk_e;
      Animation walk_w;
      Animation walk_se;
      Animation walk_sw;
}; 

class Menu_Worker_Descr : virtual public Worker_Descr {
   friend class Tribe_Descr;

   public: 
      Menu_Worker_Descr() : Worker_Descr() { }
      virtual ~Menu_Worker_Descr() { };

      virtual int read(Binary_file* f) {
            ushort clrkey;
            f->read(&clrkey, sizeof(ushort));

            menu_pic.set_clrkey(clrkey);
            char buf[1152];
            f->read(buf,1152);
            menu_pic.create(24, 24, (ushort*) buf);
            
            return RET_OK;
      }

   private:
      Pic menu_pic;
};

class Soldier_Descr : virtual public Menu_Worker_Descr {
   friend class Tribe_Descr;

   public:
      Soldier_Descr(void) : Worker_Descr() { };
      ~Soldier_Descr(void) { };

   protected:
      uint energy;

      Animation attack_l;
      Animation attack1_l;
      Animation evade_l;
      Animation evade1_l;
      Animation attack_r;
      Animation attack1_r;
      Animation evade_r;
      Animation evade1_r;

      virtual int read(Binary_file* f);
};

//
// workers having a second walk bob for every direction
// 
class Has_Walk1_Worker_Descr : virtual public Worker_Descr {
   public:
      Has_Walk1_Worker_Descr(void) { }
      virtual ~Has_Walk1_Worker_Descr() { } 
      virtual int read(Binary_file* f);

   private:
      Animation walk_ne1;
      Animation walk_e1;
      Animation walk_se1;
      Animation walk_sw1;
      Animation walk_w1;
      Animation walk_nw1;
};


// 
// workers having a work bob
// 
class Has_Working_Worker_Descr : virtual public Worker_Descr {
   public:
      Has_Working_Worker_Descr(void) { }
      virtual ~Has_Working_Worker_Descr(void) { }
      virtual int read(Binary_file* f);

   private:
      Animation working;
};

// 
// workers having a second work bob
// 
class Has_Working1_Worker_Descr : virtual public Worker_Descr {
   public:
      Has_Working1_Worker_Descr(void) { }
      virtual ~Has_Working1_Worker_Descr(void) { }
      virtual int read(Binary_file* f);

   private:
      Animation working1;
};

//
// Sit_Dig_Base class. this is the fundament for the sitters, 
// diggers and the specials
class SitDigger_Base : virtual public Worker_Descr {
   public:
      SitDigger_Base(void) { };
      virtual ~SitDigger_Base(void) { }

      virtual int read(Binary_file* f);

   private:
};

//
// Sitting workers and digging workers (same) 
// 
class SitDigger : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr {
   public:
      SitDigger(void) { }
      virtual ~SitDigger(void) { }

      virtual int read(Binary_file* f);

   private:
};

//
// Searcher
//
class Searcher : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
      
   public:
      Searcher(void) { }
      ~Searcher(void) { }

      virtual int read(Binary_file* f);

   private:
};

// 
// Planter
//
class Planter : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
   
   public:
      Planter(void) { }
      ~Planter(void) { }

      virtual int read(Binary_file* f);

   private:
};

//
// grower
//
class Grower : virtual public Worker_Descr, 
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   
   public:
      Grower(void) { }
      ~Grower(void) { }

      virtual int read(Binary_file* f);

   private:
};

// 
// scientist
//
class Scientist : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr {

   public:
      Scientist(void) { }
      ~Scientist(void) { }

      virtual int read(Binary_file* f);

   private:
};

// 
// Special workers
// 
class Carrier : virtual public SitDigger_Base,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   public:
      Carrier(void) { }
      virtual ~Carrier(void) { }

      virtual int read(Binary_file* f);
   
   private:
};
   
class Def_Carrier : virtual public Carrier {

   public: 
      Def_Carrier(void) { }
      virtual ~Def_Carrier(void) { }

      virtual int read(Binary_file* f);

   private:
};

class Add_Carrier : virtual public Carrier,
   virtual public Menu_Worker_Descr {
   public:
      Add_Carrier(void) { }
      virtual ~Add_Carrier(void) { }

      virtual int read(Binary_file* f);

   private:
};

class Builder : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {   
   public:
      Builder(void) { }
      virtual ~Builder(void) { }

      virtual int read(Binary_file* f);

   private:
};

class Planer : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
   
   public:
      Planer(void) { }
      virtual ~Planer(void) { }

      virtual int read(Binary_file* f);

   private:
};
  
class Explorer : virtual public SitDigger_Base,
      virtual public Menu_Worker_Descr {
   public:
      Explorer(void) { }
      virtual ~Explorer(void) { }

      virtual int read(Binary_file* f); 

   private:
};

class Geologist : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   public:
      Geologist(void) { }
      virtual ~Geologist(void) { }

      virtual int read(Binary_file* f);

   private:
};



#endif // __S__WORKER_DESCR_H
