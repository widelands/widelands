/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef __S__WORKER_H
#define __S__WORKER_H

#include "idleworkersupply.h"
#include "worker_descr.h"

namespace Widelands {

/**
 * Worker is the base class for all humans (and actually potential non-humans,
 * too) that belong to a tribe.
 *
 * Every worker can carry one (item) ware.
 *
 * Workers can be in one of the following meta states:
 *  - Request: the worker is walking to his job somewhere
 *  - Idle: the worker is at his job but idling
 *  - Work: the worker is running his working schedule
 */
class Worker : public Bob {
	friend class Soldier; //  allow access to m_supply
	friend class WorkerProgram;
	friend struct Map_Bobdata_Data_Packet;

	MO_DESCR(Worker_Descr);

	struct Action {
		typedef bool (Worker::*execute_t)(Game* g, Bob::State* state,
		                                  const Action* act);

		enum {
			walkObject, //  walk to objvar1
			walkCoords, //  walk to coords
		};

		execute_t                function;
		int32_t                      iparam1;
		int32_t                      iparam2;
		std::string              sparam1;

		std::vector<std::string> sparamv;
	};


public:
	Worker(const Worker_Descr &);
	virtual ~Worker();

	virtual Worker_Descr::Worker_Type get_worker_type() const throw ()
	{return descr().get_worker_type();}
	virtual Bob::Type get_bob_type() const throw () {return Bob::WORKER;}

	uint32_t get_animation(const char * const str) const {return descr().get_animation(str);}
	uint32_t get_menu_pic() const throw () {return descr().get_menu_pic();}
	const char * get_becomes() const throw () {return descr().get_becomes ();}
	const Tribe_Descr * get_tribe() const throw () {return descr().get_tribe();}
	Tribe_Descr const & tribe() const throw () {return descr().tribe();}
	const std::string & descname() const throw () {return descr().descname();}

	virtual uint32_t get_movecaps() const throw ();

	PlayerImmovable * get_location(Editor_Game_Base * egbase) {
		return reinterpret_cast<PlayerImmovable *>(m_location.get(egbase));
	}
	Economy * get_economy() const throw () {return m_economy;}

	void set_location(PlayerImmovable *location);
	void set_economy(Economy *economy);

	WareInstance * get_carried_item(Editor_Game_Base * egbase) {
		return reinterpret_cast<WareInstance *>(m_carried_item.get(egbase));
	}
	const WareInstance * get_carried_item(const Editor_Game_Base * game) const {
		return reinterpret_cast<const WareInstance *>(m_carried_item.get(game));
	}
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
	int32_t get_needed_experience() const {return m_needed_exp;}
	int32_t get_current_experience() const {return m_current_exp;}

	// debug
	void log_general_info(Editor_Game_Base*);

	// worker-specific tasks
	void start_task_transfer(Game* g, Transfer* t);
	void cancel_task_transfer(Game* g);

	void start_task_buildingwork();
	void update_task_buildingwork(Game* g);

	void start_task_return(Game* g, bool dropitem);
	void start_task_program(const std::string & programname);

	virtual void start_task_gowarehouse();
	void start_task_dropoff(Game* g, WareInstance* item);
	void start_task_fetchfromflag();

	bool start_task_waitforcapacity(Game* g, Flag* flag);
	void start_task_leavebuilding(Game* g, bool changelocation);
	void start_task_fugitive(Game* g);

	void start_task_geologist
			(const int32_t attempts, const int32_t radius,
			 const std::string & subcommand);


protected:
	void draw_inner(const Editor_Game_Base &, RenderTarget &,
	                const Point) const;
	virtual void draw(const Editor_Game_Base &, RenderTarget &,
	                  const Point) const;
	virtual void init_auto_task(Game* g);

	bool does_carry_ware() {return m_carried_item.is_set();}

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

private:
	// task details
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

	// Program commands
	bool run_mine(Game* g, State* state, const Action* act);
	bool run_createitem(Game* g, State* state, const Action* act);
	bool run_setdescription(Game* g, State* state, const Action* act);
	bool run_setbobdescription(Game* g, State* state, const Action* act);
	bool run_findobject(Game* g, State* state, const Action* act);
	bool run_findspace(Game* g, State* state, const Action* act);
	bool run_findresource(Game* g, State* state, const Action* act);
	bool run_walk(Game* g, State* state, const Action* act);
	bool run_animation(Game* g, State* state, const Action* act);
	bool run_return(Game* g, State* state, const Action* act);
	bool run_object(Game* g, State* state, const Action* act);
	bool run_plant(Game* g, State* state, const Action* act);
	bool run_create_bob(Game* g, State* state, const Action* act);
	bool run_removeobject(Game* g, State* state, const Action* act);
	bool run_geologist(Game* g, State* state, const Action* act);
	bool run_geologist_find(Game* g, State* state, const Action* act);
	bool run_playFX(Game* g, State* state, const Action* act);

	Object_Ptr         m_location;     ///< meta location of the worker, a PlayerImmovable
	Economy          * m_economy;      ///< economy this worker is registered in
	Object_Ptr         m_carried_item; ///< item we are carrying
	IdleWorkerSupply * m_supply;       ///< supply while gowarehouse and not transfer
	int32_t                m_needed_exp;   ///< experience for next level
	int32_t                m_current_exp;  ///< current experience
};

};

#endif // __S__WORKER_H
