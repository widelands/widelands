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

#include "../src/myfile.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "file_descr.h"
#include "ware_descr.h"
#include "pic_descr.h"
#include "bob_descr.h"
#include "fabric.h"
#include "wtfcreate.h"

/*
 * This a worker description. It is created whenever a building needs a worker
 * through the fabric, which keeps track of worker, so none is created twice
 */
class Worker_Descr : virtual public File_Descr {
   friend class Worker_Fabric;

   protected:
      Worker_Descr(const char* gname);
      int create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob);
      virtual const char* get_subdir(void) { return WORKERS_DIR; }

            
   public:
      virtual ~Worker_Descr(void);
      const char* get_name(void) { return name; }

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private: 
      char name[30];
      ushort walking_speed;
      ushort w,h;
      ushort hsx, hsy;
      bool is_enabled;
      short nneeds; // -1 for no produce
      Ware_List* needs;
      Bob_Descr bob_walk_ne;
      Bob_Descr bob_walk_e;
      Bob_Descr bob_walk_se;
      Bob_Descr bob_walk_sw;
      Bob_Descr bob_walk_w;
      Bob_Descr bob_walk_nw;
};

//
// This is a 'alibi' class to avoid lots of code reuse. it defines
// workers, which are listed in menus (with a picture)
// 
class Menu_Worker_Descr : virtual public Worker_Descr {
   protected:
      Menu_Worker_Descr(const char* gname);
      
   public:
      virtual ~Menu_Worker_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
      ushort clrkey;
      Pic_Descr *menu_pic;
};

//
// workers having a second walk bob for every direction
// 
class Has_Walk1_Worker_Descr : virtual public Worker_Descr {
   protected:
      Has_Walk1_Worker_Descr(const char*);
   
   public:
      virtual ~Has_Walk1_Worker_Descr();
      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      Bob_Descr bob_walk_ne1;
      Bob_Descr bob_walk_e1;
      Bob_Descr bob_walk_se1;
      Bob_Descr bob_walk_sw1;
      Bob_Descr bob_walk_w1;
      Bob_Descr bob_walk_nw1;
};


// 
// workers having a work bob
// 
class Has_Working_Worker_Descr : virtual public Worker_Descr {
   protected:
      Has_Working_Worker_Descr(const char*);

   public:
      virtual ~Has_Working_Worker_Descr(void);
      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      Bob_Descr bob_working;
};

// 
// workers having a second work bob
// 
class Has_Working1_Worker_Descr : virtual public Worker_Descr {
   protected:
      Has_Working1_Worker_Descr(const char*);

   public:
      virtual ~Has_Working1_Worker_Descr(void);
      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      Bob_Descr bob_working1;
};



      
//
// Sit_Dig_Base class. this is the fundament for the sitters, 
// diggers and the specials
class Sit_Dig_Base_Descr : virtual public Worker_Descr {

   protected:
      Sit_Dig_Base_Descr(const char*);
  
   public:
      virtual ~Sit_Dig_Base_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);
};

//
// Sitting workers and digging workers (same) 
// 
class Sit_Dig_Descr : virtual public Sit_Dig_Base_Descr,
   virtual public Menu_Worker_Descr {
   friend class Fabric<Sit_Dig_Base_Descr>;
   
   protected:
      Sit_Dig_Descr(const char* gname);
      
   public:
      virtual ~Sit_Dig_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
};

//
// Searcher
//
class Searcher_Descr : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
      
   friend class Fabric<Searcher_Descr>;
   
   private:
      Searcher_Descr(const char* gname);
      
   protected:
      ~Searcher_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
};

// 
// Planter
//
class Planter_Descr : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
   friend class Fabric<Planter_Descr>;
   
   private:
      Planter_Descr(const char* gname);
      
   protected:
      ~Planter_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
};

//
// grower
//
class Grower_Descr : virtual public Worker_Descr, 
   virtual public Menu_Worker_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   friend class Fabric<Grower_Descr>;
   
   protected:
      Grower_Descr(const char* gname);
      
   public:
      ~Grower_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
};

// 
// scientist
//
class Scientist_Descr : virtual public Worker_Descr,
   virtual public Menu_Worker_Descr {
   friend class Fabric<Scientist_Descr>;

   protected:
      Scientist_Descr(const char* gname);
      
   public:
      ~Scientist_Descr(void);

      virtual int construct(Profile* p, Section*s);
      virtual int write(Binary_file* f);

   private:
};

// 
// Special workers
// 
class Carrier_Descr : virtual public Sit_Dig_Base_Descr,
   virtual public Has_Walk1_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
      protected:
      Carrier_Descr(const char* gname); 
  
   public:
      virtual ~Carrier_Descr(void);

      virtual int construct(Profile *p, Section *s);
      virtual int write(Binary_file* f);
   
   private:
};
   
class Def_Carrier_Descr : virtual public Carrier_Descr {

   public: 
      Def_Carrier_Descr(const char* gname);
      virtual ~Def_Carrier_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};

class Add_Carrier_Descr : virtual public Carrier_Descr,
   virtual public Menu_Worker_Descr {
   public:
      Add_Carrier_Descr(const char* gname);
      virtual ~Add_Carrier_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};

class Builder_Descr : virtual public Sit_Dig_Base_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {   
   public:
      Builder_Descr(const char* gname);
      virtual ~Builder_Descr(void);

      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
};

class Planer_Descr : virtual public Sit_Dig_Base_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
   
   public:
      Planer_Descr(const char* gname);
      virtual ~Planer_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};
  
class Explorer_Descr : virtual public Sit_Dig_Base_Descr,
      virtual public Menu_Worker_Descr {
   public:
      Explorer_Descr(const char* gname);
      virtual ~Explorer_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};

class Geologist_Descr : virtual public Sit_Dig_Base_Descr,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   public:
      Geologist_Descr(const char* gname);
      virtual ~Geologist_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};

   
//
// This class extends the standart fabric for some more specific get functions
//
class Worker_Fabric {
   public:
      Worker_Fabric(void);
      ~Worker_Fabric(void);

      Worker_Descr*     start_enum(void);
      Worker_Descr*     get_nitem(void);

      Scientist_Descr*     get_scientist(const char* name);
      Searcher_Descr*      get_searcher(const char* name);
      Planter_Descr*       get_planter(const char* name);
      Grower_Descr*        get_grower(const char* name);
      Sit_Dig_Base_Descr*  get_sit_dig(const char* name);
 
      void              add_sit_dig(Sit_Dig_Base_Descr* sitter) {
         sit_digf.add(sitter);
      }

   private:
      Fabric<Scientist_Descr> scientistf;
      Fabric<Searcher_Descr> searcherf;
      Fabric<Planter_Descr> planterf;
      Fabric<Grower_Descr> growerf;
      Fabric<Sit_Dig_Base_Descr> sit_digf;
      int cur_fabric;

};

// Forward declaration for overwrite
Sit_Dig_Base_Descr* Fabric<Sit_Dig_Base_Descr>::get(const char* name);


// global ware fabric objekt!
extern Worker_Fabric workerf;


#endif // __S__WORKER_DESCR


