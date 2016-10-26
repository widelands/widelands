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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_H

#include "economy/idleworkersupply.h"
#include "economy/portdock.h"
#include "economy/transfer.h"
#include "economy/ware_instance.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "map_io/tribes_legacy_lookup_table.h"

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
	friend class Soldier;  //  allow access to supply_
	friend struct WorkerProgram;
	friend struct MapBobdataPacket;

	MO_DESCR(WorkerDescr)

	struct Action {
		using WorkerExecuteActionFn = bool (Worker::*)(Game&, Bob::State&, const Action&);

		enum {
			walkObject = 1,  //  walk to objvar1
			walkCoords = 2,  //  walk to coords
		};

		enum { plantAlways, plantUnlessObject };

		WorkerExecuteActionFn function;
		int32_t iparam1;
		int32_t iparam2;
		int32_t iparam3;
		int32_t iparam4;
		int32_t iparam5;
		std::string sparam1;

		std::vector<std::string> sparamv;
	};

public:
	Worker(const WorkerDescr&);
	virtual ~Worker();

	Player& owner() const {
		assert(get_owner());
		return *get_owner();
	}
	PlayerImmovable* get_location(EditorGameBase& egbase) {
		return location_.get(egbase);
	}
	OPtr<PlayerImmovable> get_location() const {
		return location_;
	}
	Economy* get_economy() const {
		return economy_;
	}

	/// Sets the location of the worker initially. It may not have a previous
	/// location. Does not add the worker to the location's set of workers (it
	/// should be there already). The worker must already be in the same economy
	/// as the location.
	void set_location_initially(PlayerImmovable& location) {
		assert(!location_.is_set());
		assert(location.serial());
		assert(economy_);
		assert(economy_ == location.get_economy());
		location_ = &location;
	}

	void set_location(PlayerImmovable*);
	void set_economy(Economy*);

	WareInstance* get_carried_ware(EditorGameBase& egbase) {
		return carried_ware_.get(egbase);
	}
	WareInstance const* get_carried_ware(const EditorGameBase& egbase) const {
		return carried_ware_.get(egbase);
	}
	void set_carried_ware(EditorGameBase&, WareInstance*);
	WareInstance* fetch_carried_ware(EditorGameBase&);

	void schedule_incorporate(Game&);
	void incorporate(Game&);

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	bool wakeup_flag_capacity(Game&, Flag&);
	bool wakeup_leave_building(Game&, Building&);

	/// This should be called whenever the worker has done work that he gains
	/// experience from. It may cause him to change his type so that he becomes
	/// overqualified for his current working position and can be replaced.
	/// If so, his old DescriptionIndex is returned so that the calling code can
	/// request a new worker of his old type. Otherwise INVALID_INDEX is
	/// returned.
	DescriptionIndex gain_experience(Game&);

	void create_needed_experience(Game&);
	DescriptionIndex level(Game&);

	int32_t get_current_experience() const {
		return current_exp_;
	}
	bool needs_experience() const {
		return descr().get_needed_experience() != INVALID_INDEX;
	}

	// debug
	void log_general_info(const EditorGameBase&) override;

	// worker-specific tasks
	void start_task_transfer(Game&, Transfer*);
	void cancel_task_transfer(Game&);
	Transfer* get_transfer() const {
		return transfer_;
	}

	void start_task_shipping(Game&, PortDock*);
	void end_shipping(Game&);
	bool is_shipping();

	void start_task_buildingwork(Game&);
	void update_task_buildingwork(Game&);
	void evict(Game&);

	void start_task_return(Game& game, bool dropware);
	void start_task_program(Game& game, const std::string& programname);

	void start_task_gowarehouse(Game&);
	void start_task_dropoff(Game&, WareInstance&);
	void start_task_releaserecruit(Game&, Worker&);
	void start_task_fetchfromflag(Game&);

	bool start_task_waitforcapacity(Game&, Flag&);
	void start_task_leavebuilding(Game&, bool changelocation);
	void start_task_fugitive(Game&);

	void
	start_task_geologist(Game&, uint8_t attempts, uint8_t radius, const std::string& subcommand);

	void start_task_scout(Game&, uint16_t, uint32_t);

protected:
	virtual bool is_evict_allowed();
	void draw_inner(const EditorGameBase& game,
	                const Vector2f& point_on_dst,
	                const float scale,
	                RenderTarget* dst) const;
	void draw(const EditorGameBase&,
	          const WLDrawText& draw_text,
	          const Vector2f& field_on_dst,
	          float scale,
	          RenderTarget* dst) const override;
	void init_auto_task(Game&) override;

	bool does_carry_ware() {
		return carried_ware_.is_set();
	}

	void set_program_objvar(Game&, State&, MapObject* obj);

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
	void transfer_update(Game&, State&);
	void transfer_pop(Game&, State&);
	void shipping_update(Game&, State&);
	void shipping_pop(Game&, State&);
	void buildingwork_update(Game&, State&);
	void return_update(Game&, State&);
	void program_update(Game&, State&);
	void program_pop(Game&, State&);
	void gowarehouse_update(Game&, State&);
	void gowarehouse_signalimmediate(Game&, State&, const std::string& signal);
	void gowarehouse_pop(Game& game, State& state);
	void dropoff_update(Game&, State&);
	void releaserecruit_update(Game&, State&);
	void fetchfromflag_update(Game&, State&);
	void waitforcapacity_update(Game&, State&);
	void waitforcapacity_pop(Game& game, State& state);
	void leavebuilding_update(Game&, State&);
	void leavebuilding_pop(Game& game, State& state);
	void fugitive_update(Game&, State&);
	void geologist_update(Game&, State&);
	void scout_update(Game&, State&);

	// Program commands
	bool run_mine(Game&, State&, const Action&);
	bool run_breed(Game&, State&, const Action&);
	bool run_createware(Game&, State&, const Action&);
	bool run_setbobdescription(Game&, State&, const Action&);
	bool run_findobject(Game&, State&, const Action&);
	bool run_findspace(Game&, State&, const Action&);
	bool run_walk(Game&, State&, const Action&);
	bool run_animation(Game&, State&, const Action&);
	bool run_return(Game&, State&, const Action&);
	bool run_object(Game&, State&, const Action&);
	bool run_plant(Game&, State&, const Action&);
	bool run_create_bob(Game&, State&, const Action&);
	bool run_removeobject(Game&, State&, const Action&);
	bool run_geologist(Game&, State&, const Action&);
	bool run_geologist_find(Game&, State&, const Action&);
	bool run_scout(Game&, State&, const Action&);
	bool run_play_sound(Game&, State&, const Action&);
	bool run_construct(Game&, State&, const Action&);

	OPtr<PlayerImmovable> location_;   ///< meta location of the worker
	Economy* economy_;                 ///< economy this worker is registered in
	OPtr<WareInstance> carried_ware_;  ///< ware we are carrying
	IdleWorkerSupply* supply_;         ///< supply while gowarehouse and not transfer
	Transfer* transfer_;               ///< where we are currently being sent
	int32_t current_exp_;              ///< current experience

	// saving and loading
protected:
	struct Loader : public Bob::Loader {
	public:
		Loader();

		virtual void load(FileRead&);
		void load_pointers() override;
		void load_finish() override;

	protected:
		const Task* get_task(const std::string& name) override;
		const BobProgramBase* get_program(const std::string& name) override;

	private:
		uint32_t location_;
		uint32_t carried_ware_;
		Transfer::ReadData transfer_;
	};

	virtual Loader* create_loader();

public:
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	virtual void do_save(EditorGameBase&, MapObjectSaver&, FileWrite&);

	static MapObject::Loader* load(EditorGameBase&,
	                               MapObjectLoader&,
	                               FileRead&,
	                               const TribesLegacyLookupTable& lookup_table,
	                               uint8_t packet_version);
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_H
