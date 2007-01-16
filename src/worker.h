/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "animation.h"
#include "bob.h"
#include "cmd_queue.h"

class Building;
class Economy;
class Flag;
class Route;
class Road;
class PlayerImmovable;
class WareInstance;
class Tribe_Descr;
class IdleWorkerSupply;

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
class WorkerProgram;
struct WorkerAction;

class Worker_Descr : public Bob_Descr {
   friend class Tribe_Descr;
   friend class Warehouse;
   friend class WorkerProgram;

   typedef std::map<std::string, WorkerProgram*> ProgramMap;

   struct CostItem {
      std::string name;   // name of ware
      int         amount; // amount

      inline CostItem(const char* iname, int iamount)
         : name(iname), amount(iamount) {}
   };
   typedef std::vector<CostItem> BuildCost;

   public:
   enum Worker_Type {
      NORMAL = 0,
      CARRIER,
      SOLDIER,
   };

   Worker_Descr(const Tribe_Descr &, const std::string & name);
   virtual ~Worker_Descr(void);

	virtual Bob * create_object() const;

   virtual void load_graphics(void);

   inline bool get_buildable() { return m_buildable; }
   inline const BuildCost* get_buildcost() { return &m_buildcost; }

	const Tribe_Descr * get_tribe() const throw () {return m_owner_tribe;}
	const std::string & descname() const throw () {return m_descname;}
   inline std::string get_helptext() const { return m_helptext; }

	uint get_menu_pic() const throw () {return m_menu_pic;}
	const DirAnimations & get_walk_anims() const throw () {return m_walk_anims;}
	const DirAnimations & get_right_walk_anims(const bool carries_ware) const
		throw ()
	{return carries_ware ? m_walkload_anims : m_walk_anims;}
   const WorkerProgram* get_program(std::string name) const;

   virtual Worker_Type get_worker_type(void) const { return NORMAL; }

   // For leveling
	int get_max_exp() const throw () {return m_max_experience;}
	int get_min_exp() const throw () {return m_min_experience;}
	const char * get_becomes() const throw () {return m_becomes.size() ? m_becomes.c_str() : 0;}

   Worker *create(Editor_Game_Base *g, Player *owner, PlayerImmovable *location, Coords coords);

   protected:
   virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	static Worker_Descr * create_from_dir
		(const Tribe_Descr &,
		 const char * const directory,
		 const EncodeData * const);

	std::string   m_descname; //  descriptive name
	std::string   m_helptext; //  short (tooltip-like) help text
	char        * m_menu_pic_fname;
	uint          m_menu_pic;
	DirAnimations m_walk_anims;
	DirAnimations m_walkload_anims;
   bool        m_buildable;
   BuildCost      m_buildcost;
   int            m_max_experience, m_min_experience;
   std::string    m_becomes;
	ProgramMap    m_programs;
};

class Worker : public Bob {
	friend class Soldier; //  allow access to m_supply
   friend class WorkerProgram;
   friend class Widelands_Map_Bobdata_Data_Packet; // Writes this to disk

   MO_DESCR(Worker_Descr);

   public:

	Worker(const Worker_Descr &);
   virtual ~Worker();

	virtual Worker_Descr::Worker_Type get_worker_type() const throw ()
	{return descr().get_worker_type();}
   virtual int get_bob_type() { return Bob::WORKER; }

	uint get_animation(const char * const str) const
	{return descr().get_animation(str);}
	uint          get_menu_pic() const throw () {return descr().get_menu_pic();}
	const char *  get_becomes () const throw () {return descr().get_becomes ();}
	const Tribe_Descr * get_tribe() const throw () {return descr().get_tribe();}
	const std::string & descname() const throw () {return descr().descname();}

   virtual uint get_movecaps();

	PlayerImmovable * get_location(Editor_Game_Base * egbase)
	{return reinterpret_cast<PlayerImmovable * const>(m_location.get(egbase));}
   inline Economy *get_economy() { return m_economy; }

   void set_location(PlayerImmovable *location);
   void set_economy(Economy *economy);

	WareInstance * get_carried_item(Editor_Game_Base * egbase)
	{return reinterpret_cast<WareInstance * const>(m_carried_item.get(egbase));}
   const WareInstance * get_carried_item(const Editor_Game_Base * game) const
   {return reinterpret_cast<const WareInstance *>(m_carried_item.get(game));}
   void set_carried_item(Game* g, WareInstance* item);
   WareInstance* fetch_carried_item(Game* g);

   void schedule_incorporate(Game *g);
   void incorporate(Game *g);

   virtual void init(Editor_Game_Base *g);
   virtual void cleanup(Editor_Game_Base *g);

   bool wakeup_flag_capacity(Game* g, Flag* flag);
   bool wakeup_leave_building(Game* g, Building* building);

   // For leveling
   void level(Game*);
   void create_needed_experience(Game*);
   // For leveling
   void gain_experience(Game*);
   int get_needed_experience() const {return m_needed_exp;}
   int get_current_experience() const {return m_current_exp;}

   // debug
   void log_general_info(Editor_Game_Base*);

   protected:
	void draw_inner
		(const Editor_Game_Base &, RenderTarget &, const Point) const;
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const Point) const;
	virtual void init_auto_task(Game* g);

   inline bool does_carry_ware(void) { return m_carried_item.is_set(); }

   public: // worker-specific tasks
   void start_task_transfer(Game* g, Transfer* t);
   void cancel_task_transfer(Game* g);

   void start_task_buildingwork();
   void update_task_buildingwork(Game* g);

   void start_task_return(Game* g, bool dropitem);
	void start_task_program(const std::string & name);

   virtual void start_task_gowarehouse();
   void start_task_dropoff(Game* g, WareInstance* item);
	void start_task_fetchfromflag();

   bool start_task_waitforcapacity(Game* g, Flag* flag);
   void start_task_leavebuilding(Game* g, bool changelocation);
   void start_task_fugitive(Game* g);

	void start_task_geologist
		(const int attempts, const int radius, const std::string & subcommand);


   private: // task details
   void transfer_update(Game* g, State* state);
   void transfer_signal(Game* g, State* state);
   void transfer_mask(Game* g, State* state);

   void buildingwork_update(Game* g, State* state);
   void buildingwork_signal(Game* g, State* state);

   void return_update(Game* g, State* state);
   void return_signal(Game* g, State* state);

   void program_update(Game* g, State* state);
   void program_signal(Game* g, State* state);

   void gowarehouse_update(Game* g, State* state);
   void gowarehouse_signal(Game* g, State* state);

   void dropoff_update(Game* g, State* state);

   void fetchfromflag_update(Game* g, State* state);

   void waitforcapacity_update(Game* g, State* state);
   void waitforcapacity_signal(Game* g, State* state);

   void leavebuilding_update(Game* g, State* state);
   void leavebuilding_signal(Game* g, State* state);

   void fugitive_update(Game* g, State* state);
   void fugitive_signal(Game* g, State* state);

   void geologist_update(Game* g, State* state);

   protected:
   static Task taskTransfer;
   static Task taskBuildingwork;
   static Task taskReturn;
   static Task taskProgram;
   static Task taskGowarehouse;
   static Task taskDropoff;
   static Task taskFetchfromflag;
   static Task taskWaitforcapacity;
   static Task taskLeavebuilding;
   static Task taskFugitive;
   static Task taskGeologist;

   private: // Program commands
   bool run_mine(Game* g, State* state, const WorkerAction* act);
   bool run_createitem(Game* g, State* state, const WorkerAction* act);
   bool run_setdescription(Game* g, State* state, const WorkerAction* act);
   bool run_setbobdescription(Game* g, State* state, const WorkerAction* act);
   bool run_findobject(Game* g, State* state, const WorkerAction* act);
   bool run_findspace(Game* g, State* state, const WorkerAction* act);
   bool run_findresource(Game* g, State* state, const WorkerAction* act);
   bool run_walk(Game* g, State* state, const WorkerAction* act);
   bool run_animation(Game* g, State* state, const WorkerAction* act);
   bool run_return(Game* g, State* state, const WorkerAction* act);
   bool run_object(Game* g, State* state, const WorkerAction* act);
   bool run_plant(Game* g, State* state, const WorkerAction* act);
   bool run_create_bob(Game* g, State* state, const WorkerAction* act);
   bool run_removeobject(Game* g, State* state, const WorkerAction* act);
   bool run_geologist(Game* g, State* state, const WorkerAction* act);
   bool run_geologist_find(Game* g, State* state, const WorkerAction* act);
   bool run_playFX(Game* g, State* state, const WorkerAction* act);

   private:
	Object_Ptr m_location; //  meta location of the worker, a PlayerImmovable
	Economy          * m_economy;      //  economy this worker is registered in
	Object_Ptr         m_carried_item; //  item we are carrying
	IdleWorkerSupply * m_supply;  // supply while gowarehouse and not transfer
	//Supply           * m_supply;         // supply while gowarehouse and not transfer
   int               m_needed_exp;     // experience for next level
   int               m_current_exp;    // current experience
};


/*
   Carrier is a worker who is employed by a Road.
   */
struct Carrier_Descr : public Worker_Descr {
	Carrier_Descr(const Tribe_Descr &, const std::string & carrier_name);
      virtual ~Carrier_Descr(void);

      virtual Worker_Type get_worker_type(void) const { return CARRIER; }

   protected:
	virtual Bob * create_object() const;
      virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
};

class Carrier : public Worker {
   friend class Widelands_Map_Bobdata_Data_Packet; // Writes this to disk

   MO_DESCR(Carrier_Descr);

   public:
	Carrier(const Carrier_Descr & descr);
   virtual ~Carrier();

   bool notify_ware(Game* g, int flag);

	virtual Worker_Descr::Worker_Type get_worker_type() const throw ()
	{return descr().get_worker_type();}

   void start_task_road();
   void update_task_road(Game* g);
	void start_task_transport(int fromflag);
   bool start_task_walktoflag(Game* g, int flag, bool offset = false);

   private:
   void find_pending_item(Game* g);
   int find_closest_flag(Game* g);

   private: // internal task stuff
   void road_update(Game* g, State* state);
   void road_signal(Game* g, State* state);

   void transport_update(Game* g, State* state);
   void transport_signal(Game* g, State* state);

   private:
   static Task taskRoad;
   static Task taskTransport;

   private:

	//  -1: no ware acked; 0/1: acked ware for start/end flag of road
	int m_acked_ware;
};

class Cmd_Incorporate:public BaseCommand {
    private:
	    Worker* worker;

    public:
	    Cmd_Incorporate(void) : BaseCommand(0) { } // For savegame loading
       Cmd_Incorporate (int t, Worker* w);

	    void execute (Game* g);
       // Write these commands to a file (for savegames)
       virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
       virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

       virtual int get_id(void) { return QUEUE_CMD_INCORPORATE; } // Get this command id

};

#endif // __S__WORKER_DESCR_H
