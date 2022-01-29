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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/bob.h"

#include "base/log.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/wexception.h"
#include "economy/roadbase.h"
#include "economy/route.h"
#include "economy/transfer.h"
#include "graphic/rendertarget.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/backtrace.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/critter.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/mapviewpixelconstants.h"

namespace Widelands {

BobDescr::BobDescr(const std::string& init_descname,
                   const MapObjectType init_type,
                   MapObjectDescr::OwnerType owner_type,
                   const LuaTable& table)
   : MapObjectDescr(init_type, table.get_string("name"), init_descname, table),
     owner_type_(owner_type),
     // Only tribe bobs have a vision range, since it would be irrelevant for world bobs.
     vision_range_(owner_type == MapObjectDescr::OwnerType::kTribe ? table.get_int("vision_range") :
                                                                     0) {
	if (!is_animation_known("idle")) {
		throw GameDataError("Bob %s has no idle animation", table.get_string("name").c_str());
	}
}

/**
 * Only tribe bobs (workers, ships) have a vision range, since it would be irrelevant
 * for world bobs (critters).
 *
 * \returns radius (in fields) of area that the bob can see
 */
uint32_t BobDescr::vision_range() const {
	return vision_range_;
}

/**
 * Create a bob of this type
 */
Bob& BobDescr::create(EditorGameBase& egbase, Player* const owner, const Coords& coords) const {
	Bob& bob = create_object();
	bob.set_owner(owner);
	bob.set_position(egbase, coords);
	bob.init(egbase);

	return bob;
}

Bob::Bob(const BobDescr& init_descr)
   : MapObject(&init_descr),
     position_(FCoords(Coords(0, 0), nullptr)),  // not linked anywhere
     linknext_(nullptr),
     linkpprev_(nullptr),
     anim_(0),
     animstart_(0),
     walking_(IDLE),
     walkstart_(0),
     walkend_(0),
     actid_(0),
     actscheduled_(false),
     in_act_(false) {
}

/**
 * Cleanup an object. Removes map links
 */
Bob::~Bob() {
	if (position_.field) {
		molog(owner().egbase().get_gametime(), "Bob::~Bob: pos_.field != 0, cleanup() not called!\n");
		abort();
	}
}

/**
 * Initialize the object
 *
 * \note Make sure you call this from derived classes!
 */
bool Bob::init(EditorGameBase& egbase) {
	MapObject::init(egbase);

	if (upcast(Game, game, &egbase)) {
		schedule_act(*game, Duration(1));
	} else {
		// In editor: play idle task forever
		set_animation(egbase, descr().get_animation("idle", this));
	}
	return true;
}

/**
 * Perform independent cleanup as necessary.
 */
void Bob::cleanup(EditorGameBase& egbase) {
	while (!stack_.empty()) {  //  bobs in the editor do not have tasks
		do_pop_task(dynamic_cast<Game&>(egbase));
	}

	set_owner(nullptr);  // implicitly remove ourselves from owner's map

	if (position_.field) {
		position_.field = nullptr;
		*linkpprev_ = linknext_;
		if (linknext_) {
			linknext_->linkpprev_ = linkpprev_;
		}
	}

	MapObject::cleanup(egbase);
}

/**
 * Called by Cmd_Queue when a CMD_ACT event is triggered.
 * Hand the acting over to the task.
 *
 * Change to the next task if necessary.
 */
void Bob::act(Game& game, uint32_t const data) {
	// Eliminate spurious calls of act().
	// These calls are to be expected and perfectly normal, e.g. when a carrier's
	// idle task is interrupted by the request to pick up a ware from a flag.
	if (data != actid_) {
		return;
	}

	++actid_;
	actscheduled_ = false;

	if (stack_.empty()) {
		signal_ = "";
		init_auto_task(game);

		if (stack_.empty()) {
			throw wexception("MO(%u): init_auto_task() failed to set a task", serial());
		}
		if (!actscheduled_) {
			throw wexception("MO(%u): init_auto_task() failed to schedule something", serial());
		}

		return;
	}

	do_act(game);
}

/**
 * Perform the actual call to update() as appropriate.
 */
void Bob::do_act(Game& game) {
	MutexLock m(MutexLock::ID::kObjects);
	assert(!in_act_);
	assert(!stack_.empty());

	in_act_ = true;

	const Task& task = *top_state().task;

	(this->*task.update)(game, top_state());

	if (!actscheduled_) {
		throw wexception("MO(%u): update[%s] failed to act", serial(), task.name);
	}

	in_act_ = false;
}

/**
 * Kill self ASAP.
 */
void Bob::schedule_destroy(Game& game) {
	MapObject::schedule_destroy(game);
	++actid_;  // to skip over any updates that may be scheduled
	actscheduled_ = true;
}

/**
 * Schedule a new act for the current task. All other pending acts are
 * cancelled.
 */
void Bob::schedule_act(Game& game, const Duration& tdelta) {
	MapObject::schedule_act(game, tdelta, actid_);
	actscheduled_ = true;
}

/**
 * Explicitly state that we don't want to act.
 */
void Bob::skip_act() {
	assert(in_act_);

	actscheduled_ = true;
}

/**
 * Push a new task onto the stack.
 *
 * push_task() itself does not call any functions of the task, so the caller
 * can fill the state information with parameters for the task.
 */
void Bob::push_task(Game& game, const Task& task, const Duration& tdelta) {
	assert(!task.unique || !get_state(task));
	assert(in_act_ || stack_.empty());

	stack_.push_back(State(&task));
	schedule_act(game, tdelta);
}

/**
 * Actually pop the top-most task, but don't schedule anything.
 */
void Bob::do_pop_task(Game& game) {
	State& state = top_state();

	if (state.task->pop) {
		(this->*state.task->pop)(game, state);
	}

	delete state.path;
	delete state.route;

	stack_.pop_back();
}

/**
 * Remove the current task from the stack.
 *
 * pop_task() itself does not call any parent task functions, but it ensures
 * that it will happen.
 */
void Bob::pop_task(Game& game) {
	assert(in_act_);

	do_pop_task(game);

	schedule_act(game, Duration(10));
}

/**
 * Get the bottom-most (usually the only) state of this task from the stack.
 * \return 0 if this task is not running at all.
 */
Bob::State* Bob::get_state(const Task& task) {
	std::vector<State>::iterator it = stack_.end();

	while (it != stack_.begin()) {
		--it;

		if (it->task == &task) {
			return &*it;
		}
	}

	return nullptr;
}

Bob::State const* Bob::get_state(const Task& task) const {
	std::vector<State>::const_iterator it = stack_.end();

	while (it != stack_.begin()) {
		--it;

		if (it->task == &task) {
			return &*it;
		}
	}

	return nullptr;
}

/**
 * Mark the current signal as handled.
 */
void Bob::signal_handled() {
	assert(in_act_);

	signal_.clear();
}

/**
 * Send the given signal to this bob.
 *
 * Signal delivery is asynchronous, i.e. this functions guarantees that
 * the top-most task's update() function is called, but only with a certain
 * delay.
 *
 * This function also calls all tasks' signal_immediate() function immediately.
 *
 * \param g the \ref Game object
 * \param sig the signal string
 */
void Bob::send_signal(Game& game, char const* const sig) {
	assert(*sig);  //  use set_signal() for signal removal

	for (State& state : stack_) {
		if (state.task->signal_immediate) {
			(this->*state.task->signal_immediate)(game, state, sig);
		}
	}

	signal_ = sig;
	schedule_act(game, Duration(10));
}

/**
 * Force a complete reset of the state stack.
 *
 * The state stack is emptied completely, and an init auto task is scheduled
 * as if the Bob has just been created and initialized.
 */
void Bob::reset_tasks(Game& game) {
	while (!stack_.empty()) {
		do_pop_task(game);
	}

	signal_.clear();

	++actid_;
	schedule_act(game, Duration(10));
}

/**
 * Wait a time or indefinitely.
 *
 * Every signal can interrupt this task.  No signals are caught.
 */

Bob::Task const Bob::taskIdle = {"idle", &Bob::idle_update,
                                 nullptr,  // signal_immediate
                                 nullptr, true};

/**
 * Start an idle phase, using the given animation
 *
 * If the timeout is a positive value, the idle phase stops after the given
 * time.
 *
 * This task always succeeds unless interrupted.
 */
void Bob::start_task_idle(Game& game, uint32_t const anim, int32_t const timeout, Vector2i offset) {
	assert(timeout < 0 || timeout > 0);

	set_animation(game, anim);

	push_task(game, taskIdle);

	top_state().ivar1 = timeout;
	top_state().ivar2 = offset.x;
	top_state().ivar3 = offset.y;
}

void Bob::idle_update(Game& game, State& state) {
	if (!state.ivar1 || !get_signal().empty()) {
		return pop_task(game);
	}

	if (state.ivar1 > 0) {
		schedule_act(game, Duration(state.ivar1));
	} else {
		skip_act();
	}

	state.ivar1 = 0;
}

bool Bob::is_idle() const {
	return get_state(taskIdle);
}

/**
 * Move along a predefined path.
 * \par ivar1 the step number.
 * \par ivar2 non-zero if we should force moving onto the final field.
 * \par ivar3 number of steps to take maximally or -1
 *
 * Sets the following signal(s):
 * "fail" - cannot move along the given path
 */
Bob::Task const Bob::taskMovepath = {"movepath", &Bob::movepath_update,
                                     nullptr,  // signal_immediate
                                     nullptr, true};

struct BlockedTracker {
	struct CoordData {
		Coords coord;
		int dist;
	};
	// Distance-based ordering as a heuristic for unblock()
	struct CoordOrdering {
		bool operator()(const CoordData& a, const CoordData& b) const {
			if (a.dist != b.dist) {
				return a.dist < b.dist;
			}
			return std::forward_as_tuple(a.coord.y, a.coord.x) <
			       std::forward_as_tuple(b.coord.y, b.coord.x);
		}
	};
	using Cache = std::map<CoordData, bool, CoordOrdering>;

	BlockedTracker(Game& game, Bob& bob, const Coords& finaldest)
	   : game_(game), bob_(bob), map_(game.map()), finaldest_(finaldest) {
		nrblocked_ = 0;
		disabled_ = false;
	}

	// This heuristic tries to unblock fields that are close to the destination,
	// in the hope that subsequent pathfinding will find a way to bring us
	// closer, if not complete to, the destination
	void unblock() {
		uint32_t origblocked = nrblocked_;
		int unblockprob = nrblocked_;

		for (Cache::iterator it = nodes_.begin(); it != nodes_.end() && unblockprob > 0; ++it) {
			if (it->second) {
				if (static_cast<int32_t>(game_.logic_rand() % origblocked) < unblockprob) {
					it->second = false;
					--nrblocked_;
					unblockprob -= 2;
				}
			}
		}
	}

	bool is_blocked(const FCoords& field) {
		if (disabled_) {
			return false;
		}

		CoordData cd;
		cd.coord = field;
		cd.dist = map_.calc_distance(field, finaldest_);

		Cache::iterator it = nodes_.find(cd);
		if (it != nodes_.end()) {
			return it->second;
		}

		bool const blocked = bob_.check_node_blocked(game_, field, false);
		nodes_.insert(std::make_pair(cd, blocked));
		if (blocked) {
			++nrblocked_;
		}
		return blocked;
	}

	Game& game_;
	Bob& bob_;
	const Map& map_;
	Coords finaldest_;
	Cache nodes_;
	int nrblocked_;
	bool disabled_;
};

struct CheckStepBlocked {
	explicit CheckStepBlocked(BlockedTracker& tracker) : tracker_(tracker) {
	}

	bool allowed(const Map&, FCoords, FCoords end, int32_t, CheckStep::StepId) const {
		if (end == tracker_.finaldest_) {
			return true;
		}
		return !tracker_.is_blocked(end);
	}
	bool reachable_dest(const Map&, FCoords) const {
		return true;
	}

	BlockedTracker& tracker_;
};

/**
 * Start moving to the given destination. persist is the same parameter as
 * for Map::findpath().
 *
 * \return false if no path could be found.
 *
 * \note The task finishes once the goal has been reached. It may fail.
 *
 * \param only_step defines how many steps should be taken, before this
 * returns as a success
 */
bool Bob::start_task_movepath(Game& game,
                              const Coords& dest,
                              int32_t const persist,
                              const DirAnimations& anims,
                              bool const forceonlast,
                              int32_t const only_step,
                              bool const forceall) {
	Path path;
	BlockedTracker tracker(game, *this, dest);
	CheckStepAnd cstep;

	if (forceonlast) {
		cstep.add(CheckStepWalkOn(descr().movecaps(), true));
	} else {
		cstep.add(CheckStepDefault(descr().movecaps()));
	}
	cstep.add(CheckStepBlocked(tracker));

	if (forceall) {
		tracker.disabled_ = true;
	}

	const Map& map = game.map();
	if (map.findpath(position_, dest, persist, path, cstep) < 0) {
		if (!tracker.nrblocked_) {
			return false;
		}

		tracker.unblock();
		if (map.findpath(position_, dest, persist, path, cstep) < 0) {
			if (!tracker.nrblocked_) {
				return false;
			}

			tracker.disabled_ = true;
			if (map.findpath(position_, dest, persist, path, cstep) < 0) {
				return false;
			}
		}
	}

	push_task(game, taskMovepath);
	State& state = top_state();
	state.path = new Path(path);
	state.ivar1 = 0;  // step #
	state.ivar2 = forceonlast ? 1 : (forceall ? 2 : 0);
	state.ivar3 = only_step;
	state.diranims = anims;
	return true;
}

/**
 * Start moving along the given, precalculated path.
 */
void Bob::start_task_movepath(Game& game,
                              const Path& path,
                              const DirAnimations& anims,
                              bool const forceonlast,
                              int32_t const only_step) {
	assert(path.get_start() == get_position());

	push_task(game, taskMovepath);
	State& state = top_state();
	state.path = new Path(path);
	state.ivar1 = 0;
	state.ivar2 = forceonlast ? 1 : 0;
	state.ivar3 = only_step;
	state.diranims = anims;
}

/**
 * Move to the given index on the given path. The current position must be on
 * the given path.
 *
 * \return true if a task has been started, or false if we already are on
 * the given path index.
 */
bool Bob::start_task_movepath(Game& game,
                              const Path& origpath,
                              int32_t const index,
                              const DirAnimations& anims,
                              bool const forceonlast,
                              int32_t const only_step) {
	CoordPath path(game.map(), origpath);
	int32_t const curidx = path.get_index(get_position());

	if (curidx == -1) {
		molog(game.get_gametime(), "ERROR: (%i, %i) is not on the given path:\n", get_position().x,
		      get_position().y);
		for (const Coords& coords : path.get_coords()) {
			molog(game.get_gametime(), "* (%i, %i)\n", coords.x, coords.y);
		}
		log_general_info(game);
		log_err_time(game.get_gametime(), "%s", get_backtrace().c_str());
		throw wexception("MO(%u): start_task_movepath(index): not on path", serial());
	}

	if (curidx != index) {
		if (curidx < index) {
			path.truncate(index);
			path.trim_start(curidx);
		} else {
			path.truncate(curidx);
			path.trim_start(index);
			path.reverse();
		}
		start_task_movepath(game, path, anims, forceonlast, only_step);
		return true;
	}

	return false;
}

void Bob::movepath_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		return pop_task(game);
	}

	assert(state.ivar1 >= 0);
	Path const* const path = state.path;

	if (!path) {
		// probably success; this can happen when loading a game
		// that contains a zero-length path.
		return pop_task(game);
	}

	if (static_cast<Path::StepVector::size_type>(state.ivar1) >= path->get_nsteps()) {
		assert(position_ == path->get_end());
		return pop_task(game);  //  success
	}
	if (state.ivar1 == state.ivar3) {
		// We have stepped all steps that we were asked for.
		// This is some kind of success, though we do not are were we wanted
		// to go
		return pop_task(game);
	}

	Direction const dir = (*path)[state.ivar1];

	// Slowing down a ship if two or more on same spot
	// Using probability of 1/8 and pausing it for 5, 10 or 15 seconds
	if (game.logic_rand() % 8 == 0) {
		if (descr().type() == MapObjectType::SHIP) {
			const uint32_t ships_count = game.map().find_bobs(
			   game, Widelands::Area<Widelands::FCoords>(get_position(), 0), nullptr, FindBobShip());
			assert(ships_count > 0);
			if (ships_count > 1) {
				molog(game.get_gametime(), "Pausing the ship because %d ships on the same spot\n",
				      ships_count);
				return start_task_idle(
				   game, state.diranims.get_animation(dir), ((game.logic_rand() % 3) + 1) * 5000);
			}
		}
	}

	bool forcemove = (state.ivar2 && static_cast<Path::StepVector::size_type>(state.ivar1) + 1 ==
	                                    path->get_nsteps());

	++state.ivar1;
	return start_task_move(game, dir, state.diranims, state.ivar2 == 2 ? true : forcemove);
	// Note: state pointer is invalid beyond this point
}

/**
 * Move into one direction for one step.
 * \li ivar1: direction
 * \li ivar2: non-zero if the move should be forced
 */
Bob::Task const Bob::taskMove = {"move", &Bob::move_update, nullptr, nullptr, true};

/**
 * Move into the given direction, without passability checks.
 */
void Bob::start_task_move(Game& game,
                          int32_t const dir,
                          const DirAnimations& anims,
                          bool const forcemove) {
	int32_t const tdelta =
	   start_walk(game, static_cast<WalkingDir>(dir), anims.get_animation(dir), forcemove);
	if (tdelta < 0) {
		return send_signal(game, tdelta == -2 ? "blocked" : "fail");
	}
	push_task(game, taskMove, Duration(tdelta));
}

void Bob::move_update(Game& game, State&) {
	if (walkend_ <= game.get_gametime()) {
		end_walk();
		return pop_task(game);
	}
	//  Only end the task once we've actually completed the step
	// Ignore signals until then
	return schedule_act(game, walkend_ - game.get_gametime());
}

// Calculates the actual position to draw on from the base node position. This
// function takes walking etc. into account.
//
// pos is the location, in pixels, of the node position_ on screen with scale
// and height taken into account.
Vector2f Bob::calc_drawpos(const EditorGameBase& game,
                           const Vector2f& field_on_dst,
                           const float scale) const {
	const Map& map = game.map();
	const FCoords end = position_;
	FCoords start;
	Vector2f spos = field_on_dst;
	Vector2f epos = field_on_dst;

	const float triangle_w = kTriangleWidth * scale;
	const float triangle_h = kTriangleHeight * scale;

	bool bridge = false;
	switch (walking_) {
	case WALK_NW:
		map.get_brn(end, &start);
		spos.x += triangle_w / 2.f;
		spos.y += triangle_h;
		bridge = is_bridge_segment(end.field->road_southeast);
		break;
	case WALK_NE:
		map.get_bln(end, &start);
		spos.x -= triangle_w / 2.f;
		spos.y += triangle_h;
		bridge = is_bridge_segment(end.field->road_southwest);
		break;
	case WALK_W:
		map.get_rn(end, &start);
		spos.x += triangle_w;
		bridge = is_bridge_segment(end.field->road_east);
		break;
	case WALK_E:
		map.get_ln(end, &start);
		spos.x -= triangle_w;
		bridge = is_bridge_segment(start.field->road_east);
		break;
	case WALK_SW:
		map.get_trn(end, &start);
		spos.x += triangle_w / 2.f;
		spos.y -= triangle_h;
		bridge = is_bridge_segment(start.field->road_southwest);
		break;
	case WALK_SE:
		map.get_tln(end, &start);
		spos.x -= triangle_w / 2.f;
		spos.y -= triangle_h;
		bridge = is_bridge_segment(start.field->road_southeast);
		break;

	case IDLE:
		start.field = nullptr;
		break;
	}

	if (start.field) {
		spos.y += end.field->get_height() * kHeightFactor * scale;
		spos.y -= start.field->get_height() * kHeightFactor * scale;

		assert(walkstart_ <= game.get_gametime());
		assert(walkstart_ < walkend_);
		const float f = math::clamp(static_cast<float>(game.get_gametime().get() - walkstart_.get()) /
		                               (walkend_.get() - walkstart_.get()),
		                            0.f, 1.f);
		epos.x = f * epos.x + (1.f - f) * spos.x;
		epos.y = f * epos.y + (1.f - f) * spos.y;
		if (bridge) {
			epos.y -= game.player(end.field->get_owned_by()).tribe().bridge_height() * scale *
			          (1 - 4 * (f - 0.5f) * (f - 0.5f));
		}
	}
	return epos;
}

/// It LERPs between start and end position when we are walking.
/// Note that the current node is actually the node that we are walking to, not
/// the the one that we start from.
void Bob::draw(const EditorGameBase& egbase,
               const InfoToDraw&,
               const Vector2f& field_on_dst,
               const Widelands::Coords& coords,
               const float scale,
               RenderTarget* dst) const {
	if (!anim_) {
		return;
	}
	auto* const bob_owner = get_owner();
	Vector2f adjust_field_on_dst = field_on_dst;
	if (const State* s = get_state(taskIdle)) {
		adjust_field_on_dst.x += s->ivar2;
		adjust_field_on_dst.y += s->ivar3;
	}
	dst->blit_animation(calc_drawpos(egbase, adjust_field_on_dst, scale), coords, scale, anim_,
	                    Time(egbase.get_gametime().get() - animstart_.get()),
	                    (bob_owner == nullptr) ? nullptr : &bob_owner->get_playercolor());
}

/**
 * Set a looping animation, starting now.
 */
void Bob::set_animation(const EditorGameBase& egbase, uint32_t const anim) {
	anim_ = anim;
	animstart_ = egbase.get_gametime();
}

/**
 * Cause the object to walk, honoring passable/unwalkable parts of the map
 * using movecaps. If force is true, the passability check is skipped.
 *
 * \return the number of milliseconds after which the walk has ended. You must
 * call end_walk() after this time, so schedule a task_act(). Returns -1
 * if the step is forbidden, and -2 if it is currently blocked.
 */
int32_t Bob::start_walk(Game& game, WalkingDir const dir, uint32_t const a, bool const force) {
	FCoords newnode;

	const Map& map = game.map();
	map.get_neighbour(position_, dir, &newnode);

	// Move capability check
	if (!force) {
		CheckStepDefault cstep(descr().movecaps());

		if (!cstep.allowed(map, position_, newnode, dir, CheckStep::stepNormal)) {
			return -1;
		}
	}

	//  Always call check_node_blocked, because it might communicate with other
	//  bobs (as is the case for soldiers on the battlefield).
	if (check_node_blocked(game, newnode, true) && !force) {
		return -2;
	}

	// Move is go
	int32_t const tdelta = map.calc_cost(position_, dir);
	assert(tdelta > 0);

	walking_ = dir;
	walkstart_ = game.get_gametime();
	walkend_ = walkstart_ + Duration(tdelta);

	set_position(game, newnode);
	set_animation(game, a);

	return tdelta;  // yep, we were successful
}

bool Bob::check_node_blocked(Game& game, const FCoords& field, bool) {
	// Battles always block movement!
	std::vector<Bob*> soldiers;
	game.map().find_bobs(game, Area<FCoords>(field, 0), &soldiers, FindBobEnemySoldier(get_owner()));

	if (!soldiers.empty()) {
		for (Bob* temp_bob : soldiers) {
			upcast(Soldier, soldier, temp_bob);
			if (soldier->get_battle()) {
				return true;
			}
		}
	}

	return false;
}

/**
 * Give the bob a new owner.
 *
 * This will update the owner's viewing area.
 */
void Bob::set_owner(Player* const player) {
	if (owner_ && position_.field) {
		owner_.load()->unsee_area(Area<FCoords>(get_position(), descr().vision_range()));
	}
	owner_ = player;
	if (owner_ && position_.field) {
		owner_.load()->see_area(Area<FCoords>(get_position(), descr().vision_range()));
	}
}

/**
 * Move the bob to a new position.
 *
 * Performs the necessary (un)linking in the \ref Field structures and
 * updates the owner's viewing area, if the bob has an owner.
 */
void Bob::set_position(EditorGameBase& egbase, const Coords& coords) {
	FCoords oldposition = position_;

	if (position_.field) {
		*linkpprev_ = linknext_;
		if (linknext_) {
			linknext_->linkpprev_ = linkpprev_;
		}
	}

	position_ = egbase.map().get_fcoords(coords);

	linknext_ = position_.field->bobs;
	linkpprev_ = &position_.field->bobs;
	if (linknext_) {
		linknext_->linkpprev_ = &linknext_;
	}
	*linkpprev_ = this;

	if (owner_) {
		owner_.load()->see_area(Area<FCoords>(get_position(), descr().vision_range()));
		if (oldposition.field) {
			owner_.load()->unsee_area(Area<FCoords>(oldposition, descr().vision_range()));
		}
	}

	// Since pretty much everything in Widelands eventually results in the
	// movement of a worker (e.g. transporting wares etc.), this should
	// help us to find desyncs pretty rapidly.
	// In particular, I wanted to add something to set_position because
	// it involves coordinates and will thus additionally highlight desyncs
	// in pathfinding even when two paths have the same length, and in
	// randomly generated movements.
	if (upcast(Game, game, &egbase)) {
		StreamWrite& ss = game->syncstream();
		ss.unsigned_8(SyncEntry::kBobSetPosition);
		ss.unsigned_32(serial());
		ss.signed_16(coords.x);
		ss.signed_16(coords.y);
	}
}

/// Give debug information.
void Bob::log_general_info(const EditorGameBase& egbase) const {
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "Owner: %p\n", owner_.load());
	FORMAT_WARNINGS_ON
	molog(egbase.get_gametime(), "Postition: (%i, %i)\n", position_.x, position_.y);
	molog(egbase.get_gametime(), "ActID: %i\n", actid_);
	molog(egbase.get_gametime(), "ActScheduled: %s\n", actscheduled_ ? "true" : "false");
	molog(egbase.get_gametime(), "Animation: %s\n",
	      anim_ ? descr().get_animation_name(anim_).c_str() : "\\<none\\>");

	molog(egbase.get_gametime(), "AnimStart: %i\n", animstart_.get());
	molog(egbase.get_gametime(), "WalkingDir: %i\n", walking_);
	molog(egbase.get_gametime(), "WalkingStart: %i\n", walkstart_.get());
	molog(egbase.get_gametime(), "WalkEnd: %i\n", walkend_.get());

	molog(egbase.get_gametime(), "Signal: %s\n", signal_.c_str());

	molog(egbase.get_gametime(), "Stack size: %" PRIuS "\n", stack_.size());

	for (size_t i = 0; i < stack_.size(); ++i) {
		molog(egbase.get_gametime(), "Stack dump %" PRIuS "/%" PRIuS "\n", i + 1, stack_.size());

		molog(egbase.get_gametime(), "* task->name: %s\n", stack_[i].task->name);

		molog(egbase.get_gametime(), "* ivar1: %i\n", stack_[i].ivar1);
		molog(egbase.get_gametime(), "* ivar2: %i\n", stack_[i].ivar2);
		molog(egbase.get_gametime(), "* ivar3: %i\n", stack_[i].ivar3);

		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* object pointer: %p\n", stack_[i].objvar1.get(egbase));
		FORMAT_WARNINGS_ON
		molog(egbase.get_gametime(), "* svar1: %s\n", stack_[i].svar1.c_str());

		molog(egbase.get_gametime(), "* coords: (%i, %i)\n", stack_[i].coords.x, stack_[i].coords.y);
		molog(egbase.get_gametime(), "* diranims:");
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			molog(egbase.get_gametime(), " %d", stack_[i].diranims.get_animation(dir));
		}
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "\n* path: %p\n", stack_[i].path);
		FORMAT_WARNINGS_ON
		if (stack_[i].path) {
			const Path& path = *stack_[i].path;
			molog(egbase.get_gametime(), "** Path length: %" PRIuS "\n", path.get_nsteps());
			molog(
			   egbase.get_gametime(), "** Start: (%i, %i)\n", path.get_start().x, path.get_start().y);
			molog(egbase.get_gametime(), "** End: (%i, %i)\n", path.get_end().x, path.get_end().y);
			// Printing all coordinates of the path
			CoordPath coordpath(egbase.map(), path);
			for (const Coords& coords : coordpath.get_coords()) {
				molog(egbase.get_gametime(), "*  (%i, %i)\n", coords.x, coords.y);
			}
		}
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* route: %p\n", stack_[i].route);
		molog(egbase.get_gametime(), "* program: %p\n", stack_[i].route);
		FORMAT_WARNINGS_ON
	}
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 1;

void Bob::Loader::load(FileRead& fr) {
	MapObject::Loader::load(fr);

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			Bob& bob = get<Bob>();

			if (PlayerNumber owner_number = fr.unsigned_8()) {
				if (owner_number > egbase().map().get_nrplayers()) {
					throw GameDataError("owner number is %u but there are only %u players", owner_number,
					                    egbase().map().get_nrplayers());
				}

				Player* owner = egbase().get_player(owner_number);
				if (!owner) {
					throw GameDataError("owning player %u does not exist", owner_number);
				}

				bob.set_owner(owner);
			}

			bob.set_position(egbase(), read_coords_32(&fr));

			// Animation. If the animation is no longer known, pick the main animation instead.
			std::string animname = fr.c_string();
			if (animname.empty()) {
				bob.anim_ = 0;
			} else if (bob.descr().is_animation_known(animname)) {
				bob.anim_ = bob.descr().get_animation(animname, &bob);
			} else {
				bob.anim_ = bob.descr().main_animation();
				log_warn("Unknown animation '%s' for bob '%s', using main animation instead.\n",
				         animname.c_str(), bob.descr().name().c_str());
			}

			bob.animstart_ = Time(fr);
			bob.walking_ = static_cast<WalkingDir>(read_direction_8_allow_null(&fr));
			if (bob.walking_) {
				bob.walkstart_ = Time(fr);
				bob.walkend_ = Time(fr);
			}

			bob.actid_ = fr.unsigned_32();
			bob.signal_ = fr.c_string();

			uint32_t stacksize = fr.unsigned_32();
			bob.stack_.resize(stacksize);
			states.resize(stacksize);
			for (uint32_t i = 0; i < stacksize; ++i) {
				State& state = bob.stack_[i];
				LoadState& loadstate = states[i];

				state.task = get_task(fr.c_string());
				state.ivar1 = fr.signed_32();
				state.ivar2 = fr.signed_32();
				state.ivar3 = fr.signed_32();
				loadstate.objvar1 = fr.unsigned_32();
				state.svar1 = fr.c_string();
				state.coords = read_coords_32_allow_null(&fr, egbase().map().extent());

				if (fr.unsigned_8()) {
					uint32_t anims[6];
					for (uint32_t& anim : anims) {
						std::string dir_animname = fr.c_string();
						if (bob.descr().is_animation_known(dir_animname)) {
							anim = bob.descr().get_animation(dir_animname, &bob);
						} else {
							anim = bob.descr().main_animation();
							log_warn(
							   "Unknown directional animation '%s' for bob '%s', using main animation "
							   "instead.\n",
							   dir_animname.c_str(), bob.descr().name().c_str());
						}
					}
					state.diranims =
					   DirAnimations(anims[0], anims[1], anims[2], anims[3], anims[4], anims[5]);
				}

				if (fr.unsigned_8()) {
					state.path = new Path;
					state.path->load(fr, egbase().map());
				}

				if (fr.unsigned_8()) {
					state.route = new Route;
					state.route->load(loadstate.route, fr);
				}

				std::string programname = fr.c_string();
				if (!programname.empty()) {
					state.program = get_program(programname);
				}
			}
		} else {
			throw UnhandledVersionError("Bob", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("loading bob: %s", e.what());
	}
}

void Bob::Loader::load_pointers() {
	MapObject::Loader::load_pointers();

	Bob& bob = get<Bob>();
	for (uint32_t i = 0; i < bob.stack_.size(); ++i) {
		State& state = bob.stack_[i];
		LoadState& loadstate = states[i];

		if (loadstate.objvar1) {
			state.objvar1 = &mol().get<MapObject>(loadstate.objvar1);
		}
		if (state.route) {
			state.route->load_pointers(loadstate.route, mol());
		}
	}
}

void Bob::Loader::load_finish() {
	MapObject::Loader::load_finish();

	// Care about new mapobject saving / loading - map objects don't get a task,
	// if created in the editor, so we give them one here.
	//  See bug #537392 for more information:
	//   https://bugs.launchpad.net/widelands/+bug/537392
	Bob& bob = get<Bob>();
	if (bob.stack_.empty() && egbase().get_gametime().get() == 0) {
		if (upcast(Game, game, &egbase())) {
			bob.init_auto_task(*game);
		}
	}
}

const Bob::Task* Bob::Loader::get_task(const std::string& name) {
	if (name == "move") {
		return &taskMove;
	}
	if (name == "movepath") {
		return &taskMovepath;
	}
	if (name == "idle") {
		return &taskIdle;
	}

	throw GameDataError("unknown bob task '%s'", name.c_str());
}

const MapObjectProgram* Bob::Loader::get_program(const std::string& name) {
	throw GameDataError("unknown map object program '%s'", name.c_str());
}

void Bob::save(EditorGameBase& eg, MapObjectSaver& mos, FileWrite& fw) {
	MapObject::save(eg, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);

	const Widelands::Player* const owner = owner_.load();
	fw.unsigned_8(owner ? owner->player_number() : 0);
	write_coords_32(&fw, position_);

	// linkprev_ and linknext_ are recreated automatically

	fw.c_string(anim_ ? descr().get_animation_name(anim_) : "");
	animstart_.save(fw);
	write_direction_8_allow_null(&fw, walking_);
	if (walking_) {
		walkstart_.save(fw);
		walkend_.save(fw);
	}

	fw.unsigned_32(actid_);
	fw.c_string(signal_);

	fw.unsigned_32(stack_.size());
	for (const State& state : stack_) {
		fw.c_string(state.task->name);
		fw.signed_32(state.ivar1);
		fw.signed_32(state.ivar2);
		fw.signed_32(state.ivar3);
		if (const MapObject* obj = state.objvar1.get(eg)) {
			fw.unsigned_32(mos.get_object_file_index(*obj));
		} else {
			fw.unsigned_32(0);
		}
		fw.c_string(state.svar1);

		write_coords_32(&fw, state.coords);

		if (state.diranims) {
			fw.unsigned_8(1);
			for (int dir = 1; dir <= 6; ++dir) {
				fw.c_string(descr().get_animation_name(state.diranims.get_animation(dir)).c_str());
			}
		} else {
			fw.unsigned_8(0);
		}

		if (state.path) {
			fw.unsigned_8(1);
			state.path->save(fw);
		} else {
			fw.unsigned_8(0);
		}

		if (state.route) {
			fw.unsigned_8(1);
			state.route->save(fw, eg, mos);
		} else {
			fw.unsigned_8(0);
		}

		fw.c_string(state.program ? state.program->name() : "");
	}
}
}  // namespace Widelands
