/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WORKER_H
#define WORKER_H

#include "economy/idleworkersupply.h"
#include "economy/portdock.h"
#include "economy/transfer.h"
#include "economy/ware_instance.h"
#include "logic/productionsite.h"
#include "logic/worker_descr.h"

namespace Widelands {
class Building;

/**
 * Worker is the base class for all humans (and actually potential non-humans,
 * too) that belong to a tribe.
 *
 * Every worker can carry one ware.
 *
 * Workers can be in one of the following meta states:
 *  - Request: the worker is walking to his job somewhere
 *  - Idle: the worker is at his job but idling
 *  - Work: the worker is running his working schedule
 */
class Worker : public Bob {
	friend class Soldier; //  allow access to m_supply
	friend struct WorkerProgram;
	friend struct Map_Bobdata_Data_Packet;

	MO_DESCR(Worker_Descr);

	struct Action {
		typedef bool (Worker::*execute_t)(Game &, Bob::State &, const Action &);

		enum {
			walkObject = 1, //  walk to objvar1
			walkCoords = 2, //  walk to coords
		};

		enum {
			plantAlways,
			plantUnlessObject
		};

		execute_t function;
		int32_t iparam1;
		int32_t iparam2;
		int32_t iparam3;
		int32_t iparam4;
		int32_t iparam5;
		std::string sparam1;

		std::vector<std::string> sparamv;
	};


public:
	Worker(const Worker_Descr &);
	virtual ~Worker();

	virtual Worker_Descr::Worker_Type get_worker_type() const {
		return descr().get_worker_type();
	}
	char const * type_name() const override {return "worker";}
	virtual Bob::Type get_bob_type() const override {return Bob::WORKER;}

	uint32_t get_animation(char const * const str) const {
		return descr().get_animation(str);
	}
	const Image* icon() const {return descr().icon();}
	Ware_Index becomes() const {return descr().becomes();}
	Ware_Index worker_index() const {return descr().worker_index();}
	const Tribe_Descr * get_tribe() const {return descr().get_tribe();}
	const Tribe_Descr & tribe() const {return descr().tribe();}
	const std::string & descname() const {return descr().descname();}

	Player & owner() const {assert(get_owner()); return *get_owner();}
	PlayerImmovable * get_location(Editor_Game_Base & egbase) {
		return m_location.get(egbase);
	}
	OPtr<PlayerImmovable> get_location() const {return m_location;}
	Economy * get_economy() const {return m_economy;}

	/// Sets the location of the worker initially. It may not have a previous
	/// location. Does not add the worker to the location's set of workers (it
	/// should be there already). The worker must already be in the same economy
	/// as the location.
	void set_location_initially(PlayerImmovable & location) {
		assert(not m_location.is_set());
		assert(location.serial());
		assert(m_economy);
		assert(m_economy == location.get_economy());
		m_location = &location;
	}

	void set_location(PlayerImmovable *);
	void set_economy(Economy *);

	WareInstance       * get_carried_ware(Editor_Game_Base       & egbase) {
		return m_carried_ware.get(egbase);
	}
	WareInstance const * get_carried_ware(const Editor_Game_Base & egbase) const
	{
		return m_carried_ware.get(egbase);
	}
	void set_carried_ware(Editor_Game_Base &, WareInstance *);
	WareInstance * fetch_carried_ware(Editor_Game_Base &);

	void schedule_incorporate(Game &);
	void incorporate(Game &);

	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	bool wakeup_flag_capacity(Game &, Flag &);
	bool wakeup_leave_building(Game &, Building &);


	/// This should be called whenever the worker has done work that he gains
	/// experience from. It may cause him to change his type so that he becomes
	/// overqualified for his current working position and can be replaced.
	/// If so, his old Ware_Index is returned so that the calling code can
	/// request a new worker of his old type. Otherwise Ware_Index::Null is
	/// returned.
	Ware_Index gain_experience   (Game &);

	void create_needed_experience(Game &);
	Ware_Index level             (Game &);

	int32_t get_needed_experience() const {
		return descr().get_level_experience();
	}
	int32_t get_current_experience() const {return m_current_exp;}
	bool needs_experience() const {return get_needed_experience() != -1;}

	// debug
	virtual void log_general_info(const Editor_Game_Base &) override;

	// worker-specific tasks
	void start_task_transfer(Game &, Transfer *);
	void cancel_task_transfer(Game &);
	Transfer * get_transfer() const {return m_transfer;}

	void start_task_shipping(Game &, PortDock*);
	void end_shipping(Game &);
	bool is_shipping();

	void start_task_buildingwork(Game &);
	void update_task_buildingwork(Game &);
	void evict(Game &);

	void start_task_return(Game & game, bool dropware);
	void start_task_program(Game & game, const std::string & programname);

	void start_task_gowarehouse(Game &);
	void start_task_dropoff(Game &, WareInstance &);
	void start_task_releaserecruit(Game &, Worker &);
	void start_task_fetchfromflag(Game &);

	bool start_task_waitforcapacity(Game &, Flag &);
	void start_task_leavebuilding(Game &, bool changelocation);
	void start_task_fugitive(Game &);

	void start_task_geologist
		(Game &,
		 uint8_t attempts, uint8_t radius,
		 const std::string & subcommand);

	void start_task_scout(Game &, uint16_t, uint32_t);

protected:
	virtual bool is_evict_allowed();
	void draw_inner(const Editor_Game_Base &, RenderTarget &, const Point&) const;
	virtual void draw(const Editor_Game_Base &, RenderTarget &, const Point&) const override;
	virtual void init_auto_task(Game &) override;

	bool does_carry_ware() {return m_carried_ware.is_set();}

	void set_program_objvar(Game &, State &, Map_Object * obj);

public:
	static const Task taskTransfer;
	static const Task taskShipping;
	static const Task taskBuildingwork;
	static const Task taskReturn;
	static const Task taskProgram;
	static const Task taskGowarehouse;
	static const Task taskDropoff;
	static const Task taskReleaserecruit;
	static const Task taskFetchfromflag;
	static const Task taskWaitforcapacity;
	static const Task taskLeavebuilding;
	static const Task taskFugitive;
	static const Task taskGeologist;
	static const Task taskScout;

private:
	// task details
	void transfer_update(Game &, State &);
	void transfer_pop(Game &, State &);
	void shipping_update(Game &, State &);
	void shipping_pop(Game &, State &);
	void buildingwork_update(Game &, State &);
	void return_update(Game &, State &);
	void program_update(Game &, State &);
	void program_pop(Game &, State &);
	void gowarehouse_update(Game &, State &);
	void gowarehouse_signalimmediate
		(Game &,
		 State &,
		 const std::string & signal);
	void gowarehouse_pop(Game & game, State & state);
	void dropoff_update(Game &, State &);
	void releaserecruit_update(Game &, State &);
	void fetchfromflag_update(Game &, State &);
	void waitforcapacity_update(Game &, State &);
	void waitforcapacity_pop(Game & game, State & state);
	void leavebuilding_update(Game &, State &);
	void leavebuilding_pop(Game & game, State & state);
	void fugitive_update(Game &, State &);
	void geologist_update(Game &, State &);
	void scout_update(Game &, State &);

	// Program commands
	bool run_mine             (Game &, State &, const Action &);
	bool run_breed            (Game &, State &, const Action &);
	bool run_createware       (Game &, State &, const Action &);
	bool run_setdescription   (Game &, State &, const Action &);
	bool run_setbobdescription(Game &, State &, const Action &);
	bool run_findobject       (Game &, State &, const Action &);
	bool run_findspace        (Game &, State &, const Action &);
	bool run_walk             (Game &, State &, const Action &);
	bool run_animation        (Game &, State &, const Action &);
	bool run_return           (Game &, State &, const Action &);
	bool run_object           (Game &, State &, const Action &);
	bool run_plant            (Game &, State &, const Action &);
	bool run_create_bob       (Game &, State &, const Action &);
	bool run_removeobject     (Game &, State &, const Action &);
	bool run_geologist        (Game &, State &, const Action &);
	bool run_geologist_find   (Game &, State &, const Action &);
	bool run_scout            (Game &, State &, const Action &);
	bool run_playFX           (Game &, State &, const Action &);
	bool run_construct        (Game &, State &, const Action &);

	// Displays a message to the player if a find... program can't be
	// executed
	void informPlayer(Game &, Building &, std::string) const;

	OPtr<PlayerImmovable> m_location; ///< meta location of the worker
	Economy          * m_economy;      ///< economy this worker is registered in
	OPtr<WareInstance>    m_carried_ware; ///< ware we are carrying
	IdleWorkerSupply * m_supply;   ///< supply while gowarehouse and not transfer
	Transfer * m_transfer; ///< where we are currently being sent
	int32_t                m_current_exp;  ///< current experience

	// saving and loading
protected:
	struct Loader : public Bob::Loader {
	public:
		Loader();

		virtual void load(FileRead &);
		virtual void load_pointers() override;
		virtual void load_finish() override;

	protected:
		virtual const Task * get_task(const std::string & name) override;
		virtual const BobProgramBase * get_program(const std::string & name) override;

	private:
		uint32_t m_location;
		uint32_t m_carried_ware;
		Transfer::ReadData m_transfer;
	};

	virtual Loader * create_loader();

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
	virtual void do_save
		(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

}

#endif
