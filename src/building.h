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

#ifndef __S__BUILDING_H
#define __S__BUIlDING_H

#include "instances.h"


class NeedWares_List {
   public:
      NeedWares_List(void) { list=0; }
      ~NeedWares_List(void) { if(list) free(list); }

      int read(FileRead *f);

   private:
      struct List {
         ushort index;
         ushort count;
         ushort stock;
      };
      short nneeds;
      List* list;
};

/*
 * Common to all buildings!
 */
class Building_Descr : public Map_Object_Descr { 
   public:
      Building_Descr(void) { }
      virtual ~Building_Descr(void) { }

      virtual int read(FileRead* f);
     
      inline char* get_name(void) { return name; }
      inline Animation* get_idle_anim(void) { return &idle; }
      inline bool get_is_enabled(void) { return is_enabled; }
      inline ushort get_see_area(void) { return see_area; }

   protected:
      //      int create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob, ushort* ew=0, ushort* eh=0);

   private: 
      char name[30];
      ushort see_area;
      bool is_enabled;

   protected: // for size
      Animation idle;
};

/*
 * Buildings, that have some need wares (means also stock ist valid)
 */
class Has_Needs_Building_Descr : virtual public Building_Descr {
   public:
      Has_Needs_Building_Descr(void) { }
      virtual ~Has_Needs_Building_Descr(void) { }

      virtual int read(FileRead* f);
      
      // functions
      NeedWares_List* get_needs(uint* n, bool* b) { *n=nneeds; *b=needs_or; return &needs; }

   private:
      bool needs_or; // or needs_and?
      ushort nneeds;
      NeedWares_List needs;
};

/*
 * Buildings, that produce something
 */
class Has_Products_Building_Descr : virtual public Building_Descr {
   public:
      Has_Products_Building_Descr(void) { }
      virtual ~Has_Products_Building_Descr(void) { }

      virtual int read(FileRead* f);

   protected:
      Need_List* get_products(uint* n, bool* b) { *n=nproducts; *b=products_or; return &products; }

   private:
      bool products_or;
      ushort nproducts;
      Need_List products;      
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
         IS_A_PORT = 5,
         IS_A_NOTHING =6
      };

      Has_Is_A_Building_Descr(void) { }
      ~Has_Is_A_Building_Descr(void) { }

      virtual int read(FileRead* f);
      ushort get_is_a(void) { return is_a; }  

   private:
      ushort is_a; // size of building
};

/*
 * Buildings, that can be build somewhere
 */
class Buildable_Building_Descr : virtual public Building_Descr {
   public:
      Buildable_Building_Descr(void) { }
      virtual ~Buildable_Building_Descr(void) { }

      virtual int read(FileRead* f);

      char* get_category(void) { return category; }
      ushort get_build_time(void) { return build_time; }
      Need_List* get_build_cost(uint* n) { *n=ncost; return &cost; }
      Animation* get_build_anim(void) { return &build; }


   private:
      char category[30];
      ushort build_time;
      ushort ncost;
      Need_List cost;
      Animation build;
};

/*
 * Buildings that are 'working' (only valid for 
 * workers type=sit) at the moment
 */
class Working_Building_Descr : virtual public Building_Descr {
   public:
      Working_Building_Descr(void) { }
      virtual ~Working_Building_Descr(void) { }

      virtual int read(FileRead* f);
      
      Animation* get_working_anim(void) { return &working; }

   private:
      Animation working;
};

/* 
 * Buildings, that are only 'standing around' doing 
 * nothing. e.g military buildings or searcher buildings
 */
class Boring_Building_Descr : virtual public Building_Descr {
   public:
      Boring_Building_Descr(void) { }
      virtual ~Boring_Building_Descr(void) { }

      virtual int read(FileRead* f);

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
         Dig_Building_Descr(void) { }
         ~Dig_Building_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         uint worker;
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
         Search_Building_Descr(void) { bobs=0; }
         ~Search_Building_Descr(void) { if(bobs) free(bobs);  }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         ushort working_time;
         ushort idle_time;
         ushort working_area;
         uint worker;
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
         Plant_Building_Descr(void) { bobs=0;}
         ~Plant_Building_Descr(void) { if(bobs) free(bobs); }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         ushort working_time;
         ushort idle_time;
         ushort working_area;
         uint worker;
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
         Grow_Building_Descr(void) { }
         ~Grow_Building_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         uint worker;
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
         Sit_Building_Descr(void) { }
         ~Sit_Building_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         bool order_worker;
         uint worker;
         ushort idle_time;
         ushort working_time;
   };

class Sit_Building_Produ_Worker_Descr : virtual public Working_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr,
   virtual public Has_Needs_Building_Descr {
      public:
         Sit_Building_Produ_Worker_Descr(void) { }
         ~Sit_Building_Produ_Worker_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         uint worker;
         ushort idle_time;
         ushort working_time;
         ushort prod_worker;
   };

/*
 * Science buildings 
 */
class Science_Building_Descr : virtual public Working_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr {
      public:
         Science_Building_Descr(void) { }
         ~Science_Building_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         uint worker; 
   };


/*
 * Military buildings
 */
class Military_Building_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr,
   virtual public Has_Is_A_Building_Descr, 
   virtual public Has_Needs_Building_Descr {
      public:
         Military_Building_Descr::Military_Building_Descr(void) { }
         Military_Building_Descr::~Military_Building_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

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
         Cannon_Descr::Cannon_Descr(void) { }
         Cannon_Descr::~Cannon_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         uint worker;
         ushort idle_time;
         ushort projectile_speed;
         bool fires_balistic;
         Animation projectile;
         Animation fire_ne;
         Animation fire_e;
         Animation fire_se;
         Animation fire_sw;
         Animation fire_w;
         Animation fire_nw;
   };


// Special buildings!!

//
// Head quarters, HQ
// 
class HQ_Descr : virtual public Boring_Building_Descr {
   public:
      HQ_Descr::HQ_Descr(void) { }
      HQ_Descr::~HQ_Descr(void) { }

      int read(FileRead* f);
      Map_Object *create_object();
      
      ushort get_conquers(void) { return conquers; }

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
         Store_Descr::Store_Descr(void) { }
         Store_Descr::~Store_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

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
         Dockyard_Descr::Dockyard_Descr(void) { }
         Dockyard_Descr::~Dockyard_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         ushort working_time;
         ushort idle_time;
         uint worker;
   };

//
// Port
// 
class Port_Descr : virtual public Boring_Building_Descr,
   virtual public Buildable_Building_Descr {
      public:
         Port_Descr::Port_Descr(void) { }
         Port_Descr::~Port_Descr(void) { }

         int read(FileRead* f);
         Map_Object *create_object();

      private:
         // nothing
};


#endif // __S__BUILDING_H
