/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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
// only to be included by building*.cc; if you need anything from this
// file somewhere else, move the declarations to building.h

#ifndef included_building_int_h
#define included_building_int_h


class Request;
class WaresQueue;


/*
ConstructionSite
----------------
A "building" that acts as a placeholder during construction work.
Once the construction phase is finished, the ConstructionSite is removed
and replaced by the actual building.

A construction site can have a worker.
A construction site has one (or more) input wares types, each with an
  associated store.

Note that the ConstructionSite_Descr class is mostly a dummy class.
The ConstructionSite is derived from Building so that it fits in more cleanly
with the transport and Flag code.

Every tribe has exactly one ConstructionSite_Descr.
The ConstructionSite's idling animation is the basic construction site marker.
*/
class ConstructionSite_Descr : public Building_Descr {
public:
	ConstructionSite_Descr(Tribe_Descr* tribe, const char* name);

	virtual void parse(const char* directoy, Profile* prof, const EncodeData* encdata);
	virtual Building* create_object();
};

class ConstructionSite : public Building {
	MO_DESCR(ConstructionSite_Descr);

public:
	ConstructionSite(ConstructionSite_Descr* descr);
	virtual ~ConstructionSite();

	virtual int get_size();
	virtual uint get_playercaps();
	virtual uint get_ui_anim();
	virtual std::string get_census_string();
	virtual std::string get_statistics_string();
	uint get_built_per64k();

	void set_building(Building_Descr* descr);
	inline Building_Descr* get_building() const { return m_building; }

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game *g, uint data);

	virtual bool burn_on_destroy();

	virtual void set_economy(Economy* e);

	uint get_nrwaresqueues() { return m_wares.size(); }
	WaresQueue* get_waresqueue(uint idx) { return m_wares[idx]; }

protected:
	virtual UIWindow *create_options_window(Interactive_Player *plr, UIWindow **registry);

	void check_work(Game* g);

	void request_builder(Game* g);
	static void request_builder_callback(Game* g, Request* rq, int ware, Worker* w, void* data);
	static void wares_queue_callback(Game* g, WaresQueue* wq, int ware, void* data);

	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos);

private:
	Building_Descr*	m_building; // type of building that is being built

	Request*				m_builder_request;
	Worker*				m_builder;

	std::vector<WaresQueue*>	m_wares;

	bool					m_working;		// true if the builder is currently working
	uint					m_work_steptime;	// time when next step is completed
	uint					m_work_completed;	// how many steps have we done so far?
	uint					m_work_steps;		// how many steps (= items) until we're done?
};


/*
ProductionSite
--------------
Every building that is part of the economics system is a production site.

A production site has a worker.
A production site can have one (or more) output wares types (in theory it should
  be possible to burn wares for some virtual result such as "mana", or maybe
  even just for the fun of it, although that's not planned).
A production site can have one (or more) input wares types. Every input
  wares type has an associated store.
*/
class ProductionProgram;
class Input;

class ProductionSite_Descr : public Building_Descr {
	typedef std::map<std::string, ProductionProgram*> ProgramMap;

public:
	ProductionSite_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~ProductionSite_Descr();

	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	virtual Building *create_object();

	std::string get_worker_name() const { return m_worker_name; }
	bool is_output(std::string name) const { return m_output.find(name) != m_output.end(); }
	const std::set<std::string>* get_outputs() const { return &m_output; }
	const std::vector<Input>* get_inputs() const { return &m_inputs; }
   const ProductionProgram* get_program(std::string name) const;

   virtual bool is_only_production_site(void) { return true; }

private:
	std::string					m_worker_name;	// name of worker type
   std::vector<Input>      m_inputs;
   std::set<std::string>	m_output;	// output wares type names
	ProgramMap					m_programs;
};

class ProductionSite : public Building {
	MO_DESCR(ProductionSite_Descr);

public:
	ProductionSite(ProductionSite_Descr* descr);
	virtual ~ProductionSite();

	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);
	virtual void act(Game *g, uint data);

	virtual void remove_worker(Worker *w);

	virtual bool get_building_work(Game* g, Worker* w, bool success);

   virtual void set_economy(Economy* e);


   inline std::vector<WaresQueue*>* get_warequeues(void) { return &m_input_queues; }

protected:
	virtual UIWindow *create_options_window(Interactive_Player *plr, UIWindow **registry);

private:
	void request_worker(Game* g);
	static void request_worker_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

	void program_step();
   void program_restart();

private:
	Request*		m_worker_request;
	Worker*		m_worker;

	const ProductionProgram*	m_program;			// currently running program
	int								m_program_ip;		// instruction pointer
	int								m_program_phase;	// micro-step index (instruction dependent)
	bool								m_program_timer;	// execute next instruction based on pointer
	int								m_program_time;	// timer time
   bool                       m_program_needs_restart; // program shall be restarted on next act()
   std::vector<WaresQueue*>   m_input_queues;   //  input queues for all inputs
};

class MilitarySite_Descr : public ProductionSite_Descr {
   public:
      MilitarySite_Descr(Tribe_Descr *tribe, const char *name); 
      virtual ~MilitarySite_Descr();
  
      virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
      virtual Building *create_object();

      virtual bool is_only_production_site(void) { return false; }

      virtual int get_conquers(void) const { return m_conquer_radius; }
      inline int get_max_number_of_soldiers(void) { return m_num_soldiers; }
      inline int get_max_number_of_medics(void) { return m_num_medics; }
      inline int get_heal_per_second(void) { return m_heal_per_second; }
      inline int get_heal_increase_per_medic(void) { return m_heal_incr_per_medic; }

   private:
      int                     m_conquer_radius;
      int                     m_num_soldiers;
      int                     m_num_medics;
      int                     m_heal_per_second;
      int                     m_heal_incr_per_medic;
};

class MilitarySite : public ProductionSite {
	MO_DESCR(MilitarySite_Descr);
   
   public:
      MilitarySite(MilitarySite_Descr* descr);
      virtual ~MilitarySite();

		virtual std::string get_statistics_string();

      virtual void init(Editor_Game_Base *g);
      virtual void cleanup(Editor_Game_Base *g);
      virtual void act(Game *g, uint data);

      virtual void set_economy(Economy* e);

      /*
      get_soldiersqueue()
      release_soldier()
      request_soldier()
      */

protected:
	virtual UIWindow *create_options_window(Interactive_Player *plr, UIWindow **registry);

private:
	void request_soldier(Game* g);
   static void request_soldier_callback(Game* g, Request* rq, int ware, Worker* w, void* data);

   private:
	Request*		m_soldier_request;
	Worker*		m_soldier;
//     Soldier*
//  Soldier_Queue
};

/*
=============================

class Input

This class descripes, how many items of a certain
ware can be stored in a house.
This class will be extended to support ordering of
certain wares directly or releasing some wares
out of a building

=============================
*/
class Input {
   public:
      Input(Ware_Descr* ware, int max) { m_ware=ware; m_max=max; }
      ~Input(void) { }

      inline void set_max(int n) { m_max=n; }
      inline int get_max(void) const { return m_max; }
      inline Ware_Descr* get_ware() const { return m_ware; }

   private:
      Ware_Descr* m_ware;
      int m_max;
};



#endif // included_building_int_h
