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

#ifndef __S__WORKER_DESCR_H
#define __S__WORKER_DESCR_H

#include "animation.h"
#include "bob.h"

class Building;
class Economy;
class Flag;
class Request;
class Route;
class Road;
class PlayerImmovable;
class Pic;
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

	typedef std::map<std::string, WorkerProgram*> ProgramMap;

public:
	Worker_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Worker_Descr(void);

	virtual Bob *create_object();

	void load_graphics();

	inline Tribe_Descr *get_tribe() { return m_tribe; }
	inline std::string get_descname() const { return m_descname; }
	inline std::string get_helptext() const { return m_helptext; }
	inline uint get_menu_pic() { return m_menu_pic; }
	inline DirAnimations *get_walk_anims() { return &m_walk_anims; }
	inline DirAnimations *get_right_walk_anims(bool carries_ware) { if(carries_ware) return &m_walkload_anims; return &m_walk_anims; }
	inline int get_ware_id() const { return m_ware_id; }
	const WorkerProgram* get_program(std::string name) const;

	void set_ware_id(int idx);

	Worker *create(Editor_Game_Base *g, Player *owner, PlayerImmovable *location, Coords coords);

protected:
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
	static Worker_Descr *create_from_dir(Tribe_Descr *tribe, const char *directory, const EncodeData *encdata);

	Tribe_Descr*	m_tribe;
	std::string		m_descname;			// Descriptive name
	std::string		m_helptext;			// Short (tooltip-like) help text
	char*				m_menu_pic_fname;
	uint				m_menu_pic;
	DirAnimations	m_walk_anims;
	DirAnimations	m_walkload_anims;
	int				m_ware_id;

	ProgramMap		m_programs;
};

class Worker : public Bob {
	friend class WorkerProgram;

	MO_DESCR(Worker_Descr);

public:
	Worker(Worker_Descr *descr);
	virtual ~Worker();

	inline int get_ware_id() const { return get_descr()->get_ware_id(); }
	inline uint get_idle_anim() const { return get_descr()->get_idle_anim(); }

	virtual uint get_movecaps();

	inline PlayerImmovable *get_location(Editor_Game_Base *g) { return (PlayerImmovable*)m_location.get(g); }
	inline Economy *get_economy() { return m_economy; }

	void set_location(PlayerImmovable *location);
	void set_economy(Economy *economy);

	WareInstance* get_carried_item(Editor_Game_Base* g) { return (WareInstance*)m_carried_item.get(g); }
	void set_carried_item(Game* g, WareInstance* item);
	WareInstance* fetch_carried_item(Game* g);

	void schedule_incorporate(Game *g);
	void incorporate(Game *g);

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

	bool wakeup_flag_capacity(Game* g, Flag* flag);
	bool wakeup_leave_building(Game* g, Building* building);

protected:
	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, Point pos);
	virtual void init_auto_task(Game* g);

	inline bool does_carry_ware(void) { return m_carried_item.is_set(); }

public: // worker-specific tasks
	void start_task_transfer(Game* g, Transfer* t);
	void cancel_task_transfer(Game* g);

	void start_task_buildingwork(Game* g);
	void update_task_buildingwork(Game* g);

	void start_task_return(Game* g, bool dropitem);
	void start_task_program(Game* g, std::string name);

	void start_task_gowarehouse(Game* g);
	void start_task_dropoff(Game* g, WareInstance* item);
	void start_task_fetchfromflag(Game* g);

	bool start_task_waitforcapacity(Game* g, Flag* flag);
	void start_task_leavebuilding(Game* g, bool changelocation);
	void start_task_fugitive(Game* g);

	void start_task_geologist(Game* g, int attempts, int radius, std::string subcommand);

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

private:
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
	bool run_createitem(Game* g, State* state, const WorkerAction* act);
	bool run_setdescription(Game* g, State* state, const WorkerAction* act);
	bool run_findobject(Game* g, State* state, const WorkerAction* act);
	bool run_findspace(Game* g, State* state, const WorkerAction* act);
	bool run_walk(Game* g, State* state, const WorkerAction* act);
	bool run_animation(Game* g, State* state, const WorkerAction* act);
	bool run_return(Game* g, State* state, const WorkerAction* act);
	bool run_object(Game* g, State* state, const WorkerAction* act);
	bool run_plant(Game* g, State* state, const WorkerAction* act);
	bool run_removeobject(Game* g, State* state, const WorkerAction* act);
	bool run_geologist(Game* g, State* state, const WorkerAction* act);
	bool run_geologist_find(Game* g, State* state, const WorkerAction* act);

private:
	Object_Ptr			m_location;			// meta location of the worker, a PlayerImmovable
	Economy*				m_economy;			// Economy this worker is registered in
	Object_Ptr			m_carried_item;	// Item we are carrying
	IdleWorkerSupply*	m_supply;			// supply while gowarehouse and not transfer
};


/*
Carrier is a worker who is employed by a Road.
*/
class Carrier_Descr : public Worker_Descr {
public:
	Carrier_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Carrier_Descr(void);

protected:
	virtual Bob *create_object();
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
};

class Carrier : public Worker {
	MO_DESCR(Carrier_Descr);

public:
	Carrier(Carrier_Descr *descr);
	virtual ~Carrier();

	bool notify_ware(Game* g, int flag);

public:
	void start_task_road(Game* g, Road* road);
	void update_task_road(Game* g);
	void start_task_transport(Game* g, int fromflag);
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
	int	m_acked_ware;	// -1: no ware acked; 0/1: acked ware for start/end flag of road
};


#endif // __S__WORKER_DESCR_H
