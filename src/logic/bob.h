/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef BOB_H
#define BOB_H

#include "economy/route.h"
#include "graphic/animation.h"
#include "graphic/diranimations.h"
#include "logic/instances.h"
#include "point.h"
#include "port.h"
#include "logic/walkingdir.h"

struct Profile;

namespace Widelands {
class Map;
struct Route;
struct Transfer;
struct Tribe_Descr;


/**
 * BobProgramBase is only used that
 * get_name always works
 */

struct BobProgramBase {
	virtual ~BobProgramBase() {}
	virtual std::string get_name() const = 0;
};

class Bob;

// Description for the Bob class.
class BobDescr : public Map_Object_Descr {
public:
	friend struct Map_Bobdata_Data_Packet;

	BobDescr(char const* name,
	      char const* descname,
	      const std::string& directory,
	      Profile&,
	      Section& global_s,
	      Tribe_Descr const*);

	virtual ~BobDescr() {};
	Bob& create(Editor_Game_Base&, Player* owner, const Coords&) const;
	bool is_world_bob() const {
		return not m_owner_tribe;
	}

	Tribe_Descr const* get_owner_tribe() const {
		return m_owner_tribe;
	}

	virtual uint32_t movecaps() const {
		return 0;
	}
	uint32_t vision_range() const;

protected:
	virtual Bob& create_object() const = 0;

	const Tribe_Descr* const m_owner_tribe;  //  0 if world bob
};

/**
 * Bobs are moving map objects: Animals, humans, ships...
 *
 * The name comes from original Settlers2 terminology.
 *
 * \par Bobs, Tasks and their signalling
 *
 * Bobs have a call-stack of "tasks". The top-most \ref Task is the one that is
 * currently being executed.
 *
 * Upon initialization, an object has no Task at all. A CMD_ACT will be
 * scheduled automatically. When it is executed, \ref init_auto_task() is
 * called to automatically select a fallback Task.
 *
 * However, the creator of the Bob can choose to push a specific Task
 * immediately after creating the Bob. This will override the fallback
 * behaviour. init_auto_task() is also called when the final task is popped
 * from the stack.
 *
 * All state information that a Task uses must be stored in the State structure
 * returned by get_state(). Every Task on the Task stack has its own
 * State structure, i.e. push_task() does not destroy the current Task's State.
 *
 * In order to start a new sub-task, you have to call \ref push_task(), and then
 * fill the State structure returned by get_state() with any parameters that the
 * Task may need.
 *
 * A Task is ended by \ref pop_task(). Note, however, that pop_task() can only
 * be called from a Task's update() function. If you want to interrupt the
 * current \ref Task for some reason, you should call \ref send_signal().
 * The signal semantics are explained below in more detail.
 *
 * To implement a new Task, you need to create a new Task object with an
 * update() function. This update() function is called in one of the following
 * situations:
 * \li a timeout set by \ref schedule_act() has occurred
 * \li the task has just been started via \ref push_task()
 * \li the child task has ended via \ref pop_task()
 * \li a signal has been sent via \ref send_signal()
 * It is the responsibility of the update() function to distinguish between
 * these situations as appropriate.
 *
 * One of the following things must happen during update():
 * \li Call schedule_act() to schedule the next call to update()
 * \li Call skip_act() if you really don't want to act until a signal occurs.
 * \li Call pop_task() to end the current task
 * \li Send a new signal via \ref send_signal(). Note that in this case,
 *     the update() function will be called again after some delay, and it
 *     remains the responsibility of the update() function to deal with the
 *     signal.
 * The last case is mostly useful when signals are sent from functions that
 * can be called at any time, such as \ref set_location().
 *
 * Whenever \ref send_signal() is called, any current signal is overwritten
 * by the new signal and the signal_immediate() functions of all Tasks on the
 * stack are called if available. Note that these functions are not supposed
 * to perform any actions besides bookkeeping actions that must be performed
 * in all situations (for example, one might zero some pointer in
 * signal_immediate() to avoid dangling pointers).
 *
 * Then, \ref send_signal() schedules a future call to the top-most task's
 * update() function. Often, update() functions will just call \ref pop_task()
 * and leave the signal handling to their parent tasks. To ultimately handle
 * a signal, the update() function must call \ref signal_handled().
 *
 * If a task maintains state outside of its \ref State structure, it may have
 * to do certain bookkeeping tasks whenever the task is popped from the stack.
 * To this end, a task may have a \ref Task::pop method. If this method
 * exists, it is always called just before the task is popped from the stack.
 */
class Bob : public Map_Object {
public:
	friend class Map;
	friend struct Map_Bobdata_Data_Packet;
	friend struct Map_Bob_Data_Packet;

	struct State;
	typedef void (Bob::*Ptr)(Game &, State &);
	typedef void (Bob::*PtrSignal)(Game &, State &, const std::string &);
	enum Type {CRITTER, WORKER, SHIP};

	/// \see struct Bob for in-depth explanation
	struct Task {
		char const * name;

		/**
		 * Called to update the current task and schedule the next
		 * actions etc.
		 *
		 * \see Bob for in-depth explanation
		 */
		Ptr update;

		/**
		 * Called by \ref send_signal() to perform bookkeeping tasks that
		 * must be performed immediately. May be zero.
		 */
		PtrSignal signal_immediate;

		/**
		 * Called by \ref pop_task() just before the task is popped from
		 * the task. Must only perform bookkeeping tasks. May be zero.
		 */
		Ptr pop;

		bool unique; /// At most 1 of this task type can be on the stack.
	};

	/**
	 * The current state of a task on the stack.
	 *
	 * If you think in terms of functions, \ref Task represents the code
	 * of a function, while \ref State represents the stackframe of an
	 * actual execution of the function.
	 *
	 * \see class Bob for in-depth explanation
	 */
	struct State {
		State(const Task * const the_task = nullptr) :
			task    (the_task),
			ivar1   (0),
			ivar2   (0),
			ivar3   (0),
			coords  (Coords::Null()),
			path    (nullptr),
			route   (nullptr),
			program (nullptr)
		{}

		const Task           * task;
		int32_t                ivar1;
		int32_t                ivar2;
		union                  {int32_t ivar3; uint32_t ui32var3;};
		Object_Ptr             objvar1;
		std::string            svar1;

		Coords                 coords;
		DirAnimations          diranims;
		Path                 * path;
		Route                * route;
		const BobProgramBase * program; ///< pointer to current program
	};

	MO_DESCR(BobDescr);

	uint32_t get_current_anim() const {return m_anim;}
	int32_t get_animstart() const {return m_animstart;}

	virtual int32_t get_type() const override {return BOB;}
	virtual char const * type_name() const override {return "bob";}
	virtual Type get_bob_type() const = 0;
	const std::string & name() const {return descr().name();}

	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;
	virtual void act(Game &, uint32_t data) override;
	void schedule_destroy(Game &);
	void schedule_act(Game &, uint32_t tdelta);
	void skip_act();
	Point calc_drawpos(const Editor_Game_Base &, Point) const;
	void set_owner(Player *);
	Player * get_owner() const {return m_owner;}
	void set_position(Editor_Game_Base &, const Coords &);
	const FCoords & get_position() const {return m_position;}
	Bob * get_next_bob() const {return m_linknext;}
	bool is_world_bob() const {return descr().is_world_bob();}

	uint32_t vision_range() const {return descr().vision_range();}

	/// Check whether this bob should be able to move onto the given node.
	///
	/// \param commit indicates whether this function is called from the
	///    \ref start_walk function, i.e. whether the bob will actually move
	///    onto the \p to node if this function allows it to.
	virtual bool checkNodeBlocked(Game &, const FCoords &, bool commit);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const Point&) const;

	// For debug
	virtual void log_general_info(const Editor_Game_Base &) override;

	// default tasks
	void reset_tasks(Game &);
	void send_signal(Game &, char const *);
	void start_task_idle(Game &, uint32_t anim, int32_t timeout);

	/// This can fail (and return false). Therefore the caller must check the
	/// result and find something else for the bob to do. Otherwise there will
	/// be a "failed to act" error.
	bool start_task_movepath
		(Game                &,
		 const Coords        & dest,
		 const int32_t         persist,
		 const DirAnimations &,
		 const bool            forceonlast = false,
		 const int32_t         only_step = -1)
		__attribute__((warn_unused_result));

	/// This can fail (and return false). Therefore the caller must check the
	/// result and find something else for the bob to do. Otherwise there will
	/// be a "failed to act" error.
	void start_task_movepath
		(Game                &,
		 const Path          &,
		 const DirAnimations &,
		 const bool            forceonlast  = false,
		 const int32_t         only_step = -1);

	bool start_task_movepath
		(Game                &,
		 const Path          &,
		 const int32_t         index,
		 const DirAnimations &,
		 const bool            forceonlast = false,
		 const int32_t         only_step = -1)
		__attribute__((warn_unused_result));

	void start_task_move(Game & game, int32_t dir, const DirAnimations &, bool);

	// higher level handling (task-based)
	State & top_state() {assert(m_stack.size()); return *m_stack.rbegin();}
	State * get_state() {return m_stack.size() ? &*m_stack.rbegin() : nullptr;}


	std::string get_signal() {return m_signal;}
	State       * get_state(const Task &);
	State const * get_state(const Task &) const;
	void push_task(Game & game, const Task & task, uint32_t tdelta = 10);
	void pop_task(Game &);

	void signal_handled();

	/// Automatically select a task.
	virtual void init_auto_task(Game &) {};

	// low level animation and walking handling
	void set_animation(Editor_Game_Base &, uint32_t anim);

	/// \return true if we're currently walking
	bool is_walking() {return m_walking != IDLE;}


	/**
	 * This is a hack that should not be used, if possible.
	 * It is only introduced here because profiling showed
	 * that soldiers spend a lot of time in the node blocked check.
	 */
	Bob * get_next_on_field() const {return m_linknext;}

protected:
	Bob(const BobDescr & descr);
	virtual ~Bob();

private:
	void do_act(Game &);
	void do_pop_task(Game &);
	void idle_update(Game &, State &);
	void movepath_update(Game &, State &);
	void move_update(Game &, State &);

	int32_t start_walk
		(Game & game, WalkingDir, uint32_t anim, bool force = false);

	/**
	 * Call this from your task_act() function that was scheduled after
	 * start_walk().
	 */
	void end_walk() {m_walking = IDLE;}


	static Task const taskIdle;
	static Task const taskMovepath;
	static Task const taskMove;

	Player   * m_owner; ///< can be 0
	FCoords    m_position; ///< where are we right now?
	Bob      * m_linknext; ///< next object on this node
	Bob    * * m_linkpprev;
	uint32_t       m_anim;
	int32_t        m_animstart; ///< gametime when the animation was started
	WalkingDir m_walking;
	int32_t        m_walkstart; ///< start time (used for interpolation)
	int32_t        m_walkend;   ///< end time (used for interpolation)

	// Task framework variables
	std::vector<State> m_stack;

	/**
	 * Every time a Bob acts, this counter is incremented.
	 *
	 * All scheduled \ref Cmd_Act are given this ID as data, so that
	 * only the earliest \ref Cmd_Act issued during one act phase is actually
	 * executed. Subsequent \ref Cmd_Act could interfere and are eliminated.
	 */
	uint32_t m_actid;

	/**
	 * Whether something was scheduled during this act phase.
	 *
	 * The only purpose of this variable is to act as an integrity check to avoid
	 * Bobs that hang themselves up. So e.g. \ref skip_act() also sets this
	 * to \c true, even though it technically doesn't schedule anything.
	 */
	bool m_actscheduled;
	bool m_in_act; ///< if do_act is currently running
	std::string m_signal;

	// saving and loading
protected:
	struct Loader : public Map_Object::Loader {
	public:
		Loader();

		void load(FileRead &);
		virtual void load_pointers() override;
		virtual void load_finish() override;

	protected:
		virtual const Task * get_task(const std::string & name);
		virtual const BobProgramBase * get_program(const std::string & name);

	private:
		struct LoadState {
			uint32_t objvar1;
			Route::LoadData route;
		};

		std::vector<LoadState> states;
	};

public:
	virtual bool has_new_save_support() override {return true;}

	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
	// Pure Bobs cannot be loaded
};

}

#endif
