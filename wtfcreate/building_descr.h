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

#ifndef __S__BUILDING_DESCR_H
#define __S__BUILDING_DESCR_H

#include "../src/myfile.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "wtfcreate.h"
#include "file_descr.h"
#include "worker_descr.h"
#include "bob_descr.h"
#include "ware_descr.h"

/*
 * Common to all buildings!
 */
class Building_Descr : virtual public File_Descr {
   public:
      Building_Descr(void);
      virtual ~Building_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

      const char* get_name(void) { return name; }
      virtual Worker_Descr* get_worker(void)=0;

   protected:
     int create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob, ushort* ew=0, ushort* eh=0);

   private: 
      char name[30];
      ushort see_area;
      ushort w, h;
      ushort hsx, hsy;
      bool is_enabled;
      Bob_Descr bob_idle;
};

/*
 * Buildings, that have some need wares (means also stock ist valid)
 */
class Has_Needs_Building_Descr : virtual public Building_Descr {
   public:
      Has_Needs_Building_Descr(void);
      virtual ~Has_Needs_Building_Descr(void);

      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      bool needs_or; // or needs_and?
      ushort nneeds;
      Need_List* needs;
};

/*
 * Buildings, that produce something
 */
class Has_Products_Building_Descr : virtual public Building_Descr {
   public:
      Has_Products_Building_Descr(void);
      virtual ~Has_Products_Building_Descr(void);

      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   protected:
      bool products_ored(void) { return products_or; }

   private:
      bool products_or;
      ushort nproducts;
      Product_List* products;      
};

/*
 * Buildings, that have a is_a argument. This is a workaround for
 * Ports
 */
class Has_Is_A_Building_Descr : virtual public Building_Descr {
   public:
      enum {
         IS_A_SMALL = 1,
         IS_A_MEDIUM = 2,
         IS_A_BIG = 3,
         IS_A_MINE = 4,
         IS_A_PORT = 5
      };

      Has_Is_A_Building_Descr();
      ~Has_Is_A_Building_Descr();

      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      ushort is_a; // size of building
};

/*
 * Buildings, that can be build somewhere
 */
class Buildable_Building_Descr : virtual public Building_Descr {
   public:
      Buildable_Building_Descr(void);
      virtual ~Buildable_Building_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
      char category[30];
      ushort build_time;
      ushort ncost;
      Ware_List* cost;
      Bob_Descr bob_build;
};

/*
 * Buildings that are 'working' (only valid for 
 * workers type=sit) at the moment
 */
class Working_Building_Descr : virtual public Building_Descr {
   public:
      Working_Building_Descr(void);
      virtual ~Working_Building_Descr(void);

      virtual int construct(Profile* p, Section* s);
      virtual int write(Binary_file* f);

   private:
      Bob_Descr bob_working;
};

/* 
 * Buildings, that are only 'standing around' doing 
 * nothing. e.g military buildings or searcher buildings
 */
class Boring_Building_Descr : virtual public Building_Descr {
   public:
      Boring_Building_Descr(void);
      virtual ~Boring_Building_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
};

/*
 * Buildings, inhabitated by diggers
 */
class Dig_Building_Descr : virtual public Working_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr,
   virtual public Has_Products_Building_Descr {

      public:
         Dig_Building_Descr(void);
         ~Dig_Building_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);

         Worker_Descr* get_worker(void) { return worker; }

      private:
         Sit_Dig_Base_Descr *worker;
         char resource[30];
         uint idle_time;
         uint working_time;
   };

/*
 * searcher buildings
 */
class Search_Building_Descr : virtual public Boring_Building_Descr, 
   virtual public Buildable_Building_Descr, 
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr,
   virtual public Has_Products_Building_Descr {
      public:
         Search_Building_Descr(void);
         ~Search_Building_Descr(void);

         int construct(Profile* p, Section *s); // highest instance: no more virtual 
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         ushort working_time;
         ushort idle_time;
         ushort working_area;
         Searcher_Descr* worker;
         ushort nbobs;
         char*  bobs;
   };

/*
 * Buildings, that are inhabitated by planters
 */
class Plant_Building_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr {
      public:
         Plant_Building_Descr(void);
         ~Plant_Building_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         ushort working_time;
         ushort idle_time;
         ushort working_area;
         Planter_Descr* worker;
         ushort nbobs;
         char*  bobs;
   };

/*
 * Grow buildings
 */
class Grow_Building_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr,
   virtual public Has_Products_Building_Descr  {
      public:
         Grow_Building_Descr(void);
         ~Grow_Building_Descr(void);

         int construct(Profile* p, Section *s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         Grower_Descr* worker;
         ushort working_area;
         ushort working_time;
         ushort idle_time;
         char plant_bob[30];
         char search_bob[30];
   };

/*
 * Now the sit buildings 
 */
class Sit_Building_Descr : virtual public Working_Building_Descr, 
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr,
   virtual public Has_Products_Building_Descr {
      public:
         Sit_Building_Descr(void);
         ~Sit_Building_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         bool order_worker;
         Sit_Dig_Base_Descr* worker;
         ushort idle_time;
         ushort working_time;
   };

class Sit_Building_Produ_Worker_Descr : virtual public Working_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr {
      public:
         Sit_Building_Produ_Worker_Descr(void);
         ~Sit_Building_Produ_Worker_Descr(void);

         int construct(Profile* p, Section *s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         Sit_Dig_Base_Descr* worker;
         ushort idle_time;
         ushort working_time;
         Sit_Dig_Base_Descr* prod_worker;
   };

/*
 * Science buildings 
 */
class Science_Building_Descr : virtual public Working_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr {
      public:
         Science_Building_Descr(void);
         ~Science_Building_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         Scientist_Descr* worker; 
   };


/*
 * Military buildings
 */
class Military_Building_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr, 
   virtual public Has_Needs_Building_Descr {
      public:
         Military_Building_Descr::Military_Building_Descr(void);
         Military_Building_Descr::~Military_Building_Descr(void);

         int construct(Profile *p, Section *s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return NULL; }

      private:
         ushort beds;
         ushort conquers;
         ushort idle_time;
         ushort nupgr;
   };

/*
 * Cannon
 */
class Cannon_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr, 
   virtual Has_Needs_Building_Descr {
      public:
         Cannon_Descr::Cannon_Descr(void);
         Cannon_Descr::~Cannon_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         Sit_Dig_Base_Descr* worker;
         ushort idle_time;
         ushort projectile_speed;
         bool fires_balistic;
         ushort wproj, hproj;
         Bob_Descr bob_projectile;
         Bob_Descr bob_fire_ne;
         Bob_Descr bob_fire_e;
         Bob_Descr bob_fire_se;
         Bob_Descr bob_fire_sw;
         Bob_Descr bob_fire_w;
         Bob_Descr bob_fire_nw;
   };


// Special buildings!!

//
// Head quarters, HQ
// 
class HQ_Descr : virtual public Boring_Building_Descr {
   public:
      HQ_Descr::HQ_Descr(void);
      HQ_Descr::~HQ_Descr(void);

      int construct(Profile* p, Section *s);
      int write(Binary_file* f);
      Worker_Descr* get_worker(void) { return NULL; }

   private:
      ushort conquers;
};

//
// Store
// 
class Store_Descr : virtual public Boring_Building_Descr, 
   virtual public Buildable_Building_Descr, 
   virtual public Has_Is_A_Building_Descr {
      public:
         Store_Descr::Store_Descr(void);
         Store_Descr::~Store_Descr(void);

         int construct(Profile* p, Section *s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return NULL; }

      private:
         // nothing
   };

//
// dockyard
// 
class Dockyard_Descr :  virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr {
      public: 
         Dockyard_Descr::Dockyard_Descr(void);
         Dockyard_Descr::~Dockyard_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return worker; }

      private:
         ushort working_time;
         ushort idle_time;
         Planter_Descr* worker;
   };

//
// Port
// 
class Port_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr {
      public:
         Port_Descr::Port_Descr(void);
         Port_Descr::~Port_Descr(void);

         int construct(Profile* p, Section* s);
         int write(Binary_file* f);
         Worker_Descr* get_worker(void) { return NULL; }

      private:
         // nothing
   };

// description struct
struct Buildings_Descr {
   ushort nbuilds;
   Building_Descr** builds;
};




#endif // __S__BUILDING_DESCR_H

