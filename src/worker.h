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

#include "bob.h"

class Economy;
class Request;
class Route;
class Road;
class PlayerImmovable;
class Pic;
class Tribe_Descr;

/*
Worker is the base class for all humans (and actually potential non-humans, too)
that belong to a tribe.

Every worker can carry one (item) ware.

Workers can be in one of the following meta states:
- Request: the worker is walking to his job somewhere
- Idle: the worker is at his job but idling
- Work: the worker is running his working schedule
*/
class Worker;

class Worker_Descr : public Bob_Descr {
   friend class Tribe_Descr;

public:
	Worker_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Worker_Descr(void);

	void load_graphics();

	inline Tribe_Descr *get_tribe() { return m_tribe; }
	inline uint get_menu_pic() { return m_menu_pic; }
	inline DirAnimations *get_walk_anims() { return &m_walk_anims; }
	inline DirAnimations *get_walkload_anims() { return &m_walkload_anims; }
	inline int get_ware_id() const { return m_ware_id; }

	void set_ware_id(int idx);

	Worker *create(Editor_Game_Base *g, Player *owner, PlayerImmovable *location, Coords coords, bool logic);

protected:
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	static Worker_Descr *create_from_dir(Tribe_Descr *tribe, const char *directory, const EncodeData *encdata);

	Tribe_Descr*	m_tribe;
	char*				m_menu_pic_fname;
	uint				m_menu_pic;
	DirAnimations	m_walk_anims;
	DirAnimations	m_walkload_anims;

	int				m_ware_id;
};

class Worker : public Bob {
	MO_DESCR(Worker_Descr);

public:
	enum {
		State_None = 0,
		State_Request,			// fulfilling a ware request
		State_Fugitive,		// lost our location, trying to get back to warehouse
		State_GoWarehouse,	// return to warehouse

		State_Worker_Last		// must be last
	};

	Worker(Worker_Descr *descr, bool ingamelogic);
	virtual ~Worker();

	inline int get_ware_id() const { return get_descr()->get_ware_id(); }

	virtual uint get_movecaps();

	inline PlayerImmovable *get_location(Editor_Game_Base *g) { return (PlayerImmovable*)m_location.get(g); }
	inline Economy *get_economy() { return m_economy; }

	void set_location(PlayerImmovable *location);
	void set_economy(Economy *economy);

	void set_job_request(Request *req, const Route *route);
	void change_job_request(bool cancel);

	void set_job_gowarehouse();

	void schedule_incorporate(Game *g);
	void incorporate(Game *g);

	inline Route *get_route() { return m_route; }

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

protected:
	virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);

	void end_state(Game *g, bool success);
	void run_state_request(Game *g, uint prev, bool success, uint nexthint);
	void run_state_fugitive(Game *g, uint prev, bool success, uint nexthint);
	void run_state_gowarehouse(Game *g, uint prev, bool success, uint nexthint);

	int run_route(Game *g, uint prev, Route *route, PlayerImmovable *finalgoal);

private:
	Object_Ptr	m_location;			// meta location of the worker, a PlayerImmovable
	Economy*		m_economy;			// Economy this worker is registered in
	int			m_state;				// one of State_XXX
	int			m_carried_ware;	// Ware ID (-1 if none carried)
	Route			*m_route;			// used by Request, GoWarehouse

	Request		*m_request;			// the request we're supposed to fulfill

	int			m_fugitive_death;	// when are we going to die?

	Object_Ptr	m_gowarehouse;		// the warehouse we're trying to reach
};


/*
Carrier is a worker who is employed by a Road.
*/
class Carrier_Descr : public Worker_Descr {
public:
	Carrier_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Carrier_Descr(void);

protected:
	virtual Bob *create_object(bool);
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
};

class Carrier : public Worker {
	MO_DESCR(Carrier_Descr);

public:
	enum {
		State_WorkIdle = State_Worker_Last + 1,	// idling on the road
		State_WorkFetch,
	};

	Carrier(Carrier_Descr *descr, bool ingamelogic);
	virtual ~Carrier();

	//void set_job_work(Road* road);

private:
	Object_Ptr	m_road;		// the road we work on

	int			m_fetch_flag;	// fetch from start_flag if 0, end_flag if 1; drop at the other flag
};


/*
class Menu_Worker_Descr : virtual public Worker_Descr {
   friend class Tribe_Descr;

   public:
      Menu_Worker_Descr() : Worker_Descr() { }
      virtual ~Menu_Worker_Descr() { };

      virtual int read(FileRead* f) {
            ushort clrkey;
            clrkey = f->Unsigned16();

            menu_pic.set_clrkey(clrkey);
            menu_pic.create(24, 24, (ushort*)f->Data(24*24*2));

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

      virtual int read(FileRead* f);
};

//
// workers having a second walk bob for every direction
// 
class Has_Walk1_Worker_Descr : virtual public Worker_Descr {
   public:
      Has_Walk1_Worker_Descr(void) { }
      virtual ~Has_Walk1_Worker_Descr() { } 
      virtual int read(FileRead* f);

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
      virtual int read(FileRead* f);

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
      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);

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

      virtual int read(FileRead* f);
   
   private:
};
   
class Def_Carrier : virtual public Carrier {

   public: 
      Def_Carrier(void) { }
      virtual ~Def_Carrier(void) { }

      virtual int read(FileRead* f);

   private:
};

class Add_Carrier : virtual public Carrier,
   virtual public Menu_Worker_Descr {
   public:
      Add_Carrier(void) { }
      virtual ~Add_Carrier(void) { }

      virtual int read(FileRead* f);

   private:
};

class Builder : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {   
   public:
      Builder(void) { }
      virtual ~Builder(void) { }

      virtual int read(FileRead* f);

   private:
};

class Planer : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr {
   
   public:
      Planer(void) { }
      virtual ~Planer(void) { }

      virtual int read(FileRead* f);

   private:
};
  
class Explorer : virtual public SitDigger_Base,
      virtual public Menu_Worker_Descr {
   public:
      Explorer(void) { }
      virtual ~Explorer(void) { }

      virtual int read(FileRead* f); 

   private:
};

class Geologist : virtual public SitDigger_Base,
   virtual public Menu_Worker_Descr,
   virtual public Has_Working_Worker_Descr,
   virtual public Has_Working1_Worker_Descr {
   public:
      Geologist(void) { }
      virtual ~Geologist(void) { }

      virtual int read(FileRead* f);

   private:
};
*/


#endif // __S__WORKER_DESCR_H
