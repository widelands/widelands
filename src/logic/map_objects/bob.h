/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_BOB_H
#define WL_LOGIC_MAP_OBJECTS_BOB_H

#include "base/macros.h"
#include "base/vector.h"
#include "economy/route.h"
#include "graphic/animation/diranimations.h"
#include "logic/map_objects/info_to_draw.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/map_object_program.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/path.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

class Bob;

/**
 * Implement MapObjectDescr for the following \ref Bob class.
 */
class BobDescr : public MapObjectDescr {
public:
	friend struct MapBobdataPacket;

	BobDescr(const std::string& init_descname,
	         const MapObjectType type,
	         MapObjectDescr::OwnerType owner_type,
	         const LuaTable& table);

	~BobDescr() override {
	}

	Bob& create(EditorGameBase&, Player* owner, const Coords&) const;

	MapObjectDescr::OwnerType get_owner_type() const {
		return owner_type_;
	}

	virtual uint32_t movecaps() const {
		return 0;
	}
	uint32_t vision_range() const;

protected:
	virtual Bob& create_object() const = 0;

private:
	const MapObjectDescr::OwnerType owner_type_;
	const uint32_t vision_range_;
	DISALLOW_COPY_AND_ASSIGN(BobDescr);
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
class Bob : public MapObject {
public:
	friend class Map;
	friend struct MapBobdataPacket;
	friend struct MapBobPacket;

	struct State;
	using Ptr = void (Bob::*)(Game&, State&);
	using PtrSignal = void (Bob::*)(Game&, State&, const std::string&);

	/// \see struct Bob for in-depth explanation
	struct Task {
		char const* name;

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

		bool unique;  /// At most 1 of this task type can be on the stack.
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
		explicit State(const Task* const the_task = nullptr)
		   : task(the_task),
		     ivar1(0),
		     ivar2(0),
		     ivar3(0),
		     coords(Coords::null()),
		     path(nullptr),
		     route(nullptr),
		     program(nullptr) {
		}

		const Task* task;
		int32_t ivar1;
		int32_t ivar2;
		int32_t ivar3;
		ObjectPointer objvar1;
		std::string svar1;

		Coords coords;
		DirAnimations diranims;
		Path* path;
		Route* route;
		const MapObjectProgram* program;  ///< pointer to current program
	};

	MO_DESCR(BobDescr)

	uint32_t get_current_anim() const {
		return anim_;
	}
	const Time& get_animstart() const {
		return animstart_;
	}

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;
	void schedule_destroy(Game&);
	void schedule_act(Game&, const Duration& tdelta);
	void skip_act();
	Vector2f calc_drawpos(const EditorGameBase&, const Vector2f& field_on_dst, float scale) const;
	void set_owner(Player*);

	void set_position(EditorGameBase&, const Coords&);
	const FCoords& get_position() const {
		return position_;
	}
	Bob* get_next_bob() const {
		return linknext_;
	}

	/// Check whether this bob should be able to move onto the given node.
	///
	/// \param commit indicates whether this function is called from the
	///    \ref start_walk function, i.e. whether the bob will actually move
	///    onto the \p to node if this function allows it to.
	virtual bool check_node_blocked(Game&, const FCoords&, bool commit);

	// Draws the bob onto the screen with 'field_on_dst' being the position of
	// the field associated with this bob (if it is walking, that is its
	// starting field) in pixel space of 'dst' (including scale). The 'scale' is
	// required to draw the bob in the right size.
	virtual void draw(const EditorGameBase&,
	                  const InfoToDraw& info_to_draw,
	                  const Vector2f& field_on_dst,
	                  const Coords& coords,
	                  float scale,
	                  RenderTarget* dst) const;

	// For debug
	void log_general_info(const EditorGameBase&) const override;

	// default tasks
	void reset_tasks(Game&);

	// TODO(feature-Hasi50): correct (?) Send a signal that may switch to some other \ref Task
	void send_signal(Game&, char const*);
	void start_task_idle(Game&, uint32_t anim, int32_t timeout, Vector2i offset = Vector2i::zero());
	bool is_idle() const;

	/// This can fail (and return false). Therefore the caller must check the
	/// result and find something else for the bob to do. Otherwise there will
	/// be a "failed to act" error.
	bool start_task_movepath(Game&,
	                         const Coords& dest,
	                         const int32_t persist,
	                         const DirAnimations&,
	                         const bool forceonlast = false,
	                         const int32_t only_step = -1,
	                         const bool forceall = false) __attribute__((warn_unused_result));

	/// This can fail (and return false). Therefore the caller must check the
	/// result and find something else for the bob to do. Otherwise there will
	/// be a "failed to act" error.
	void start_task_movepath(Game&,
	                         const Path&,
	                         const DirAnimations&,
	                         const bool forceonlast = false,
	                         const int32_t only_step = -1);

	bool start_task_movepath(Game&,
	                         const Path&,
	                         const int32_t index,
	                         const DirAnimations&,
	                         const bool forceonlast = false,
	                         const int32_t only_step = -1) __attribute__((warn_unused_result));

	void start_task_move(Game& game, int32_t dir, const DirAnimations&, bool);

	// higher level handling (task-based)
	State& top_state() {
		assert(stack_.size());
		return *stack_.rbegin();
	}
	State* get_state() {
		return stack_.size() ? &*stack_.rbegin() : nullptr;
	}

	std::string get_signal() {
		return signal_;
	}
	State* get_state(const Task&);
	State const* get_state(const Task&) const;
	void push_task(Game& game, const Task& task, const Duration& tdelta = Duration(10));
	void pop_task(Game&);

	void signal_handled();

	/// Automatically select a task.
	virtual void init_auto_task(Game&) {
	}

	// low level animation and walking handling
	void set_animation(const EditorGameBase&, uint32_t anim);

	/// \return true if we're currently walking
	bool is_walking() {
		return walking_ != IDLE;
	}

	/**
	 * This is a hack that should not be used, if possible.
	 * It is only introduced here because profiling showed
	 * that soldiers spend a lot of time in the node blocked check.
	 */
	Bob* get_next_on_field() const {
		return linknext_;
	}

protected:
	explicit Bob(const BobDescr& descr);
	~Bob() override;

private:
	void do_act(Game&);
	void do_pop_task(Game&);
	void idle_update(Game&, State&);
	void movepath_update(Game&, State&);
	void move_update(Game&, State&);

	int32_t start_walk(Game& game, WalkingDir, uint32_t anim, bool force = false);

	/**
	 * Call this from your task_act() function that was scheduled after
	 * start_walk().
	 */
	void end_walk() {
		walking_ = IDLE;
	}

	static Task const taskIdle;
	static Task const taskMovepath;
	static Task const taskMove;

	FCoords position_;  ///< where are we right now?
	Bob* linknext_;     ///< next object on this node
	Bob** linkpprev_;
	uint32_t anim_;
	Time animstart_;  ///< gametime when the animation was started
	WalkingDir walking_;
	Time walkstart_;  ///< start time (used for interpolation)
	Time walkend_;    ///< end time (used for interpolation)

	// Task framework variables
	std::vector<State> stack_;

	/**
	 * Every time a Bob acts, this counter is incremented.
	 *
	 * All scheduled \ref Cmd_Act are given this ID as data, so that
	 * only the earliest \ref Cmd_Act issued during one act phase is actually
	 * executed. Subsequent \ref Cmd_Act could interfere and are eliminated.
	 */
	uint32_t actid_;

	/**
	 * Whether something was scheduled during this act phase.
	 *
	 * The only purpose of this variable is to act as an integrity check to avoid
	 * Bobs that hang themselves up. So e.g. \ref skip_act() also sets this
	 * to \c true, even though it technically doesn't schedule anything.
	 */
	bool actscheduled_;
	bool in_act_;  ///< if do_act is currently running
	std::string signal_;

	// saving and loading
protected:
	struct Loader : public MapObject::Loader {
	public:
		Loader() = default;

		void load(FileRead&);
		void load_pointers() override;
		void load_finish() override;

	protected:
		virtual const Task* get_task(const std::string& name);
		virtual const MapObjectProgram* get_program(const std::string& name);

	private:
		struct LoadState {
			uint32_t objvar1;
			Route::LoadData route;
		};

		std::vector<LoadState> states;
	};

public:
	bool has_new_save_support() override {
		return true;
	}

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	// Pure Bobs cannot be loaded
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_BOB_H
