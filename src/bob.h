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

#ifndef BOB_H
#define BOB_H

#include "animation.h"

#include <cassert>

namespace Widelands {

/// \todo (Antonio Trueba#1#): Get rid of forward declarations (cleanup of other headers needed)
class Map;
class Route;
class Transfer;
class Tribe_Descr;


/**
 * BobProgramBase is only used that
 * get_name always works
 */

struct BobProgramBase {
	virtual ~BobProgramBase() {}
	virtual std::string get_name() const = 0;
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
 * scheduled automatically. When it is executed, init_auto_task() is called to
 * automatically select a fallback Task.
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
 * In order to start a new sub-task, you have to call push_task(), and then fill
 * the State structure returned by get_state() with any parameters that the Task
 * may need.
 *
 * A Task is ended by \ref pop_task(). Note, however, that pop_task() can only
 * be called from a Task's update() function. If you want to interrupt the
 * current \ref Task for some reason, you should call \ref send_signal().
 * The signal semantics are explained below in more detail.
 *
 * To implement a new Task, you need to create a new Task object with an
 * update() function. This update() function is called in one of the following
 * situations:
 * \li a timeout set by \ref schedule_act() has occured
 * \li the task has just been started via \ref push_task()
 * \li the child task has ended via \ref pop_task()
 * \li a signal has been sent via \ref send_signal()
 * It is the responsibility of the update() function to distinguish between
 * these situations as appropriate.
 *
 * One of the following things must happen during update():
 * \li Call \ref schedule_act() to schedule the next call to update()
 * \li Call \ref skip_act() if you really don't want to act until a signal occurs.
 * \li Call \ref pop_task() to end the current task
 * \li Send a new signal via \ref send_signal(). Note that in this case,
 *     the update() function will be called again after some delay, and it remains
 *     the responsibility of the update() function to deal with the signal.
 * The last case is mostly useful when signals are sent from functions that
 * can be called at any time, such as \ref set_location().
 *
 * Whenever \ref send_signal() is called, any current signal is overwritten
 * by the new signal and the signal_immediate() functions of all Tasks on the
 * stack are called if available. Note that these functions are not supposed
 * to perform any actions besides bookkeeping actions that must be performed
 * in all situations (for example, one might zero some pointer in signal_immediate()
 * to avoid dangling pointers).
 *
 * Then, \ref send_signal() schedules a future call to the top-most task's
 * update() function. Often, update() functions will just call \ref pop_task()
 * and leave the signal handling to their parent tasks. To ultimately handle
 * a signal, the update() function must call \ref signal_handled().
 */
struct Bob : public Map_Object {
	friend struct Map_Bobdata_Data_Packet;
	friend struct Map_Bob_Data_Packet;

	struct State;
	typedef void (Bob::*Ptr)(Game*, State*);
	typedef void (Bob::*PtrSignal)(Game*, State*, const std::string&);
	enum Type {CRITTER, WORKER};

	/// \see class Bob for in-depth explanation
	struct Task {
		const char* name;

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
		State(const Task * const the_task = 0) :
			task    (the_task),
			ivar1   (0),
			ivar2   (0),
			ivar3   (0),
			coords  (Coords::Null()),
			diranims(0),
			path    (0),
			transfer(0),
			route   (0),
			program (0)
		{}

		const Task           * task;
		int32_t                ivar1;
		int32_t                ivar2;
		union                  {int32_t ivar3; uint32_t ui32var3;};
		Object_Ptr             objvar1;
		std::string            svar1;

		Coords                 coords;
		const DirAnimations  * diranims;
		Path                 * path;
		Transfer             * transfer;
		Route                * route;
		const BobProgramBase * program; ///< pointer to current program
	};

	struct Descr: public Map_Object_Descr {
		friend struct Map_Bobdata_Data_Packet;

		Descr(const Tribe_Descr * const tribe, const std::string & bob_name)
			: m_name(bob_name), m_owner_tribe(tribe)
		{
			m_default_encodedata.clear();
		}

		virtual ~Descr() {};
		const std::string & name() const throw () {return m_name;}
		Bob *create(Editor_Game_Base *g, Player *owner, Coords coords);
		bool is_world_bob() const {return not m_owner_tribe;}

		const char* get_picture() const {return m_picture.c_str();}

		const EncodeData& get_default_encodedata() const {return m_default_encodedata;}

		const Tribe_Descr * const get_owner_tribe() const throw () {return m_owner_tribe;}

		static Descr *create_from_dir
				(const char *name,
				 const char *directory,
				 Profile *prof,
				 Tribe_Descr* tribe);

		uint32_t vision_range() const;

	protected:
		virtual Bob * create_object() const = 0;
		virtual void parse
				(const char *directory, Profile *prof,
				 const EncodeData *encdata);

		const std::string   m_name;
		std::string         m_picture;
		EncodeData          m_default_encodedata;
		const Tribe_Descr * const m_owner_tribe; //  0 if world bob
	};

	MO_DESCR(Descr);

	uint32_t get_current_anim() const {return m_anim;}
	int32_t get_animstart() const {return m_animstart;}

	virtual int32_t get_type() const throw () {return BOB;}
	virtual char const * type_name() const throw () {return "bob";}
	virtual Type get_bob_type() const throw () = 0;
	virtual uint32_t get_movecaps() const throw () {return 0;}
	const std::string & name() const throw () {return descr().name();}

	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void act(Game*, uint32_t data);
	void schedule_destroy(Game* g);
	void schedule_act(Game* g, uint32_t tdelta);
	void skip_act();
	Point calc_drawpos(Editor_Game_Base const &, Point) const;
	void set_owner(Player *player);
	Player * get_owner() const {return m_owner;}
	void set_position(Editor_Game_Base* g, Coords f);
	const FCoords& get_position() const {return m_position;}
	Bob * get_next_bob() const throw () {return m_linknext;}
	bool is_world_bob() const throw () {return descr().is_world_bob();}

	uint32_t vision_range() const {return descr().vision_range();}

	virtual void draw
		(Editor_Game_Base const &, RenderTarget &, Point) const;


	// For debug
	virtual void log_general_info(Editor_Game_Base* egbase);

	// default tasks
	void reset_tasks(Game*);
	void send_signal(Game*, std::string sig);
	void start_task_idle(Game*, uint32_t anim, int32_t timeout);

	bool start_task_movepath
		(Game                *,
		 const Coords          dest,
		 const int32_t         persist,
		 const DirAnimations &,
		 const bool            forceonlast = false,
		 const int32_t         only_step = -1);

	void start_task_movepath
		(Game*,
		 const Path          &,
		 const DirAnimations &,
		 const bool            forceonlast  = false,
		 const int32_t         only_step = -1);

	bool start_task_movepath
		(Game*,
		 const Map           &,
		 const Path          &,
		 const int32_t         index,
		 const DirAnimations &,
		 const bool            forceonlast = false,
		 const int32_t         only_step = -1);

	void start_task_move(Game* g, int32_t dir, DirAnimations const *, bool);

	// higher level handling (task-based)
	State* get_state()
	{return m_stack.size() ? &m_stack[m_stack.size() - 1] : 0;}

	State & top_state()
	{assert(m_stack.size()); return m_stack[m_stack.size() - 1];}

	std::string get_signal() {return m_signal;}
	State* get_state(Task* task);
	void push_task(Game* g, const Task & task);
	void pop_task(Game* g);

	void signal_handled();

	/// Automatically select a task.
	virtual void init_auto_task(Game*) {};

	// low level animation and walking handling
	void set_animation(Editor_Game_Base* g, uint32_t anim);

	/// \return true if we're currently walking
	bool is_walking() {return m_walking != IDLE;}


protected:
	Bob(const Descr & descr);
	virtual ~Bob();


private:
	void do_act(Game* g);
	void do_pop_task();
	void idle_update(Game* g, State* state);
	void movepath_update(Game* g, State* state);
	void move_update(Game* g, State* state);

	int32_t start_walk(Game* g, WalkingDir dir, uint32_t anim, bool force = false);

	/**
	 * Call this from your task_act() function that was scheduled after
	 * start_walk().
	 */
	void end_walk() {m_walking = IDLE;}


	static Task taskIdle;
	static Task taskMovepath;
	static Task taskMove;

	Player   * m_owner; ///< can be 0
	FCoords    m_position; ///< where are we right now?
	Bob      * m_linknext; ///< next object on this field
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
};

};

#endif
