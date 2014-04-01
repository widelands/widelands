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

#include "logic/bob.h"

#include <cstdlib>

#include "backtrace.h"
#include "economy/route.h"
#include "economy/transfer.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/checkstep.h"
#include "logic/critter_bob.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "profile/profile.h"
#include "upcast.h"
#include "wexception.h"
#include "wui/mapviewpixelconstants.h"


namespace Widelands {

/**
 * Only tribe bobs have a vision range, since it would be irrelevant
 * for world bobs.
 *
 * Currently, all bobs use the tribe's default vision range.
 *
 * \returns radius (in fields) of area that the bob can see
 */
uint32_t BobDescr::vision_range() const
{
	if (m_owner_tribe) {
		if (upcast(const Ship_Descr, ship, this))
			return ship->vision_range();
		return m_owner_tribe->get_bob_vision_range();
	}

	return 0;
}


BobDescr::BobDescr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const * const tribe)
	:
	Map_Object_Descr(_name, _descname),
	m_owner_tribe(tribe)
{
	{ //  global options
		Section & idle_s = prof.get_safe_section("idle");

		add_animation("idle", g_gr->animations().load(directory, idle_s));
	}

	// Parse attributes
	while (Section::Value const * val = global_s.get_next_val("attrib")) {
		uint32_t const attrib = get_attribute_id(val->get_string());

		if (attrib < Map_Object::HIGHEST_FIXED_ATTRIBUTE)
			throw game_data_error("bad attribute \"%s\"", val->get_string());

		add_attribute(attrib);
	}
}


/**
 * Create a bob of this type
 */
Bob & BobDescr::create
	(Editor_Game_Base & egbase,
	 Player * const owner,
	 const Coords & coords)
	const
{
	Bob & bob = create_object();
	bob.set_owner(owner);
	bob.set_position(egbase, coords);
	bob.init(egbase);

	return bob;
}


Bob::Bob(const BobDescr & _descr) :
Map_Object       (&_descr),
m_owner          (nullptr),
m_position       (FCoords(Coords(0, 0), nullptr)), // not linked anywhere
m_linknext       (nullptr),
m_linkpprev      (nullptr),
m_anim           (0),
m_animstart      (0),
m_walking        (IDLE),
m_walkstart      (0),
m_walkend        (0),
m_actid          (0),
m_actscheduled   (false),
m_in_act         (false)
{}


/**
 * Cleanup an object. Removes map links
 */
Bob::~Bob()
{
	if (m_position.field) {
		molog
			("Map_Object::~Map_Object: m_pos.field != 0, cleanup() not "
			 "called!\n");
		abort();
	}
}


/**
 * Initialize the object
 *
 * \note Make sure you call this from derived classes!
 */
void Bob::init(Editor_Game_Base & egbase)
{
	Map_Object::init(egbase);

	if (upcast(Game, game, &egbase))
		schedule_act(*game, 1);
	else
		// In editor: play idle task forever
		set_animation(egbase, descr().get_animation("idle"));
}


/**
 * Perform independent cleanup as necessary.
 */
void Bob::cleanup(Editor_Game_Base & egbase)
{
	while (!m_stack.empty()) //  bobs in the editor do not have tasks
		do_pop_task(ref_cast<Game, Editor_Game_Base>(egbase));

	set_owner(nullptr); // implicitly remove ourselves from owner's map

	if (m_position.field) {
		m_position.field = nullptr;
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	Map_Object::cleanup(egbase);
}


/**
 * Called by Cmd_Queue when a CMD_ACT event is triggered.
 * Hand the acting over to the task.
 *
 * Change to the next task if necessary.
 */
void Bob::act(Game & game, uint32_t const data)
{
	// Eliminate spurious calls of act().
	// These calls are to be expected and perfectly normal, e.g. when a carrier's
	// idle task is interrupted by the request to pick up a ware from a flag.
	if (data != m_actid)
		return;

	++m_actid;
	m_actscheduled = false;

	if (m_stack.empty()) {
		m_signal = "";
		init_auto_task(game);

		if (m_stack.empty())
			throw wexception
				("MO(%u): init_auto_task() failed to set a task", serial());
		if (!m_actscheduled)
			throw wexception
				("MO(%u): init_auto_task() failed to schedule something", serial());

		return;
	}

	do_act(game);
}


/**
 * Perform the actual call to update() as appropriate.
 */
void Bob::do_act(Game & game)
{
	assert(!m_in_act);
	assert(m_stack.size());

	m_in_act = true;

	const Task & task = *top_state().task;

	(this->*task.update)(game, top_state());

	if (!m_actscheduled)
		throw wexception
			("MO(%u): update[%s] failed to act", serial(), task.name);

	m_in_act = false;
}


/**
 * Kill self ASAP.
 */
void Bob::schedule_destroy(Game & game)
{
	Map_Object::schedule_destroy(game);
	++m_actid; // to skip over any updates that may be scheduled
	m_actscheduled = true;
}


/**
 * Schedule a new act for the current task. All other pending acts are
 * cancelled.
 */
void Bob::schedule_act(Game & game, uint32_t tdelta)
{
	Map_Object::schedule_act(game, tdelta, m_actid);
	m_actscheduled = true;
}


/**
 * Explicitly state that we don't want to act.
 */
void Bob::skip_act()
{
	assert(m_in_act);

	m_actscheduled = true;
}


/**
 * Push a new task onto the stack.
 *
 * push_task() itself does not call any functions of the task, so the caller
 * can fill the state information with parameters for the task.
 */
void Bob::push_task(Game & game, const Task & task, uint32_t const tdelta)
{
	assert(not task.unique or not get_state(task));
	assert(m_in_act || m_stack.empty());

	m_stack.push_back(State(&task));
	schedule_act(game, tdelta);
}


/**
 * Actually pop the top-most task, but don't schedule anything.
 */
void Bob::do_pop_task(Game & game)
{
	State & state = top_state();

	if (state.task->pop)
		(this->*state.task->pop)(game, state);

	delete state.path;
	delete state.route;

	m_stack.pop_back();
}


/**
 * Remove the current task from the stack.
 *
 * pop_task() itself does not call any parent task functions, but it ensures
 * that it will happen.
 */
void Bob::pop_task(Game & game)
{
	assert(m_in_act);

	do_pop_task(game);

	schedule_act(game, 10);
}


/**
 * Get the bottom-most (usually the only) state of this task from the stack.
 * \return 0 if this task is not running at all.
 */
Bob::State * Bob::get_state(const Task & task)
{
	std::vector<State>::iterator it = m_stack.end();

	while (it != m_stack.begin()) {
		--it;

		if (it->task == &task)
			return &*it;
	}

	return nullptr;
}

Bob::State const * Bob::get_state(const Task & task) const
{
	std::vector<State>::const_iterator it = m_stack.end();

	while (it != m_stack.begin()) {
		--it;

		if (it->task == &task)
			return &*it;
	}

	return nullptr;
}


/**
 * Mark the current signal as handled.
 */
void Bob::signal_handled()
{
	assert(m_in_act);

	m_signal.clear();
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
void Bob::send_signal(Game & game, char const * const sig)
{
	assert(*sig); //  use set_signal() for signal removal

	for (uint32_t i = 0; i < m_stack.size(); ++i) {
		State & state = m_stack[i];

		if (state.task->signal_immediate)
			(this->*state.task->signal_immediate)(game, state, sig);
	}

	m_signal = sig;
	schedule_act(game, 10);
}


/**
 * Force a complete reset of the state stack.
 *
 * The state stack is emptied completely, and an init auto task is scheduled
 * as if the Bob has just been created and initialized.
 */
void Bob::reset_tasks(Game & game)
{
	while (!m_stack.empty())
		do_pop_task(game);

	m_signal.clear();

	++m_actid;
	schedule_act(game, 10);
}



/**
 * Wait a time or indefinitely.
 *
 * Every signal can interrupt this task.  No signals are caught.
 */

Bob::Task const Bob::taskIdle = {
	"idle",
	&Bob::idle_update,
	nullptr, // signal_immediate
	nullptr,
	true
};


/**
 * Start an idle phase, using the given animation
 *
 * If the timeout is a positive value, the idle phase stops after the given
 * time.
 *
 * This task always succeeds unless interrupted.
 */
void Bob::start_task_idle
	(Game & game, uint32_t const anim, int32_t const timeout)
{
	assert(timeout < 0 || timeout > 0);

	set_animation(game, anim);

	push_task(game, taskIdle);

	top_state().ivar1 = timeout;
}


void Bob::idle_update(Game & game, State & state)
{
	if (!state.ivar1 || get_signal().size())
		return pop_task(game);

	if (state.ivar1 > 0)
		schedule_act(game, state.ivar1);
	else
		skip_act();

	state.ivar1 = 0;
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
Bob::Task const Bob::taskMovepath = {
	"movepath",
	&Bob::movepath_update,
	nullptr, // signal_immediate
	nullptr,
	true
};

struct BlockedTracker {
	struct CoordData {
		Coords coord;
		int dist;
	};
	// Distance-based ordering as a heuristic for unblock()
	struct CoordOrdering {
		bool operator()(const CoordData & a, const CoordData & b) const {
			if (a.dist != b.dist)
				return a.dist < b.dist;
			return a.coord.all < b.coord.all;
		}
	};
	typedef std::map<CoordData, bool, CoordOrdering> Cache;

	BlockedTracker(Game & game, Bob & bob, const Coords & finaldest)
		: m_game(game), m_bob(bob), m_map(game.map()), m_finaldest(finaldest)
	{
		nrblocked = 0;
		disabled = false;
	}

	// This heuristic tries to unblock fields that are close to the destination,
	// in the hope that subsequent pathfinding will find a way to bring us
	// closer, if not complete to, the destination
	void unblock() {
		uint32_t origblocked = nrblocked;
		int unblockprob = nrblocked;

		for
			(Cache::iterator it = nodes.begin();
			 it != nodes.end() && unblockprob > 0;
			 ++it)
		{
			if (it->second) {
				if
					(static_cast<int32_t>(m_game.logic_rand() % origblocked)
					 <
					 unblockprob)
				{
					it->second = false;
					--nrblocked;
					unblockprob -= 2;
				}
			}
		}
	}

	bool isBlocked(const FCoords & field) {
		if (disabled)
			return false;

		CoordData cd;
		cd.coord = field;
		cd.dist = m_map.calc_distance(field, m_finaldest);

		Cache::iterator it = nodes.find(cd);
		if (it != nodes.end())
			return it->second;

		bool const blocked = m_bob.checkNodeBlocked(m_game, field, false);
		nodes.insert(std::make_pair(cd, blocked));
		if (blocked)
			++nrblocked;
		return blocked;
	}

	Game & m_game;
	Bob  & m_bob;
	Map  & m_map;
	Coords m_finaldest;
	Cache nodes;
	int nrblocked;
	bool disabled;
};

struct CheckStepBlocked {
	CheckStepBlocked(BlockedTracker & tracker) : m_tracker(tracker) {}

	bool allowed(Map &, FCoords, FCoords end, int32_t, CheckStep::StepId) const
	{
		if (end == m_tracker.m_finaldest)
			return true;

		return !m_tracker.isBlocked(end);
	}
	bool reachabledest(Map &, FCoords) const {return true;}

	BlockedTracker & m_tracker;
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
bool Bob::start_task_movepath
	(Game                & game,
	 const Coords        & dest,
	 int32_t         const persist,
	 const DirAnimations & anims,
	 bool            const forceonlast,
	 int32_t         const only_step)
{
	Path path;
	BlockedTracker tracker(game, *this, dest);
	CheckStepAnd cstep;

	if (forceonlast)
		cstep.add(CheckStepWalkOn(descr().movecaps(), true));
	else
		cstep.add(CheckStepDefault(descr().movecaps()));
	cstep.add(CheckStepBlocked(tracker));

	Map & map = game.map();
	if (map.findpath(m_position, dest, persist, path, cstep) < 0) {
		if (!tracker.nrblocked)
			return false;

		tracker.unblock();
		if (map.findpath(m_position, dest, persist, path, cstep) < 0) {
			if (!tracker.nrblocked)
				return false;

			tracker.disabled = true;
			if (map.findpath(m_position, dest, persist, path, cstep) < 0)
				return false;
		}
	}

	push_task(game, taskMovepath);
	State & state  = top_state();
	state.path     = new Path(path);
	state.ivar1    = 0; // step #
	state.ivar2    = forceonlast ? 1 : 0;
	state.ivar3    = only_step;
	state.diranims = anims;
	return true;
}


/**
 * Start moving along the given, precalculated path.
 */
void Bob::start_task_movepath
	(Game                & game,
	 const Path          & path,
	 const DirAnimations & anims,
	 bool            const forceonlast,
	 int32_t         const only_step)
{
	assert(path.get_start() == get_position());

	push_task(game, taskMovepath);
	State & state  = top_state();
	state.path     = new Path(path);
	state.ivar1    = 0;
	state.ivar2    = forceonlast ? 1 : 0;
	state.ivar3    = only_step;
	state.diranims = anims;
}


/**
 * Move to the given index on the given path. The current position must be on
 * the given path.
 *
 * \return true if a task has been started, or false if we already are on
 * the given path index.
 */
bool Bob::start_task_movepath
	(Game                & game,
	 const Path          & origpath,
	 int32_t         const index,
	 const DirAnimations & anims,
	 bool            const forceonlast,
	 int32_t         const only_step)
{
	CoordPath path(game.map(), origpath);
	int32_t const curidx = path.get_index(get_position());

	if (curidx == -1) {
		molog
			("ERROR: (%i, %i) is not on the given path:\n",
			 get_position().x, get_position().y);
		container_iterate_const(std::vector<Coords>, path.get_coords(), i)
			molog("* (%i, %i)\n", i.current->x, i.current->y);
		log_general_info(game);
		log("%s", get_backtrace().c_str());
		throw wexception
			("MO(%u): start_task_movepath(index): not on path", serial());
	}

	if (curidx != index) {
		if (curidx < index) {
			path.truncate(index);
			path.starttrim(curidx);
		} else {
			path.truncate(curidx);
			path.starttrim(index);
			path.reverse();
		}
		start_task_movepath(game, path, anims, forceonlast, only_step);
		return true;
	}

	return false;
}


void Bob::movepath_update(Game & game, State & state)
{
	if (get_signal().size()) {
		return pop_task(game);
	}

	assert(state.ivar1 >= 0);
	Path const * const path = state.path;

	if (!path)
		// probably success; this can happen when loading a game
		// that contains a zero-length path.
		return pop_task(game);

	if
		(static_cast<Path::Step_Vector::size_type>(state.ivar1)
		 >=
		 path->get_nsteps())
	{
		assert(m_position == path->get_end());
		return pop_task(game); //  success
	} else if (state.ivar1 == state.ivar3)
		// We have stepped all steps that we were asked for.
		// This is some kind of success, though we do not are were we wanted
		// to go
		return pop_task(game);

	Direction const dir = (*path)[state.ivar1];
	bool forcemove = false;

	if
		(state.ivar2
		 and
		 static_cast<Path::Step_Vector::size_type>(state.ivar1) + 1
		 ==
		 path->get_nsteps())
	{
		forcemove = true;
	}

	++state.ivar1;
	return start_task_move(game, dir, state.diranims, forcemove);
	// Note: state pointer is invalid beyond this point
}


/**
 * Move into one direction for one step.
 * \li ivar1: direction
 * \li ivar2: non-zero if the move should be forced
 */
Bob::Task const Bob::taskMove = {
	"move",
	&Bob::move_update,
	nullptr,
	nullptr,
	true
};


/**
 * Move into the given direction, without passability checks.
 */
void Bob::start_task_move
	(Game                & game,
	 int32_t         const dir,
	 const DirAnimations & anims,
	 bool            const forcemove)
{
	int32_t const tdelta =
		start_walk
			(game,
			 static_cast<WalkingDir>(dir),
			 anims.get_animation(dir),
			 forcemove);
	if (tdelta < 0)
		return send_signal(game, tdelta == -2 ? "blocked" : "fail");
	push_task(game, taskMove, tdelta);
}


void Bob::move_update(Game & game, State &)
{
	if (m_walkend <= game.get_gametime()) {
		end_walk();
		return pop_task(game);
	} else
		//  Only end the task once we've actually completed the step
		// Ignore signals until then
		return schedule_act(game, m_walkend - game.get_gametime());
}


/// Calculates the actual position to draw on from the base node position.
/// This function takes walking etc. into account.
///
/// pos is the location, in pixels, of the node m_position (height is already
/// taken into account).
Point Bob::calc_drawpos(const Editor_Game_Base & game, const Point pos) const
{
	const Map & map = game.get_map();
	const FCoords end = m_position;
	FCoords start;
	Point spos = pos, epos = pos;

	switch (m_walking) {
	case WALK_NW:
		map.get_brn(end, &start);
		spos.x += TRIANGLE_WIDTH / 2;
		spos.y += TRIANGLE_HEIGHT;
		break;
	case WALK_NE:
		map.get_bln(end, &start);
		spos.x -= TRIANGLE_WIDTH / 2;
		spos.y += TRIANGLE_HEIGHT;
		break;
	case WALK_W:
		map.get_rn(end, &start);
		spos.x += TRIANGLE_WIDTH;
		break;
	case WALK_E:
		map.get_ln(end, &start);
		spos.x -= TRIANGLE_WIDTH;
		break;
	case WALK_SW:
		map.get_trn(end, &start);
		spos.x += TRIANGLE_WIDTH / 2;
		spos.y -= TRIANGLE_HEIGHT;
		break;
	case WALK_SE:
		map.get_tln(end, &start);
		spos.x -= TRIANGLE_WIDTH / 2;
		spos.y -= TRIANGLE_HEIGHT;
		break;

	case IDLE: start.field = nullptr; break;
	default:
		assert(false);
		break;
	}

	if (start.field) {
		spos.y += end.field->get_height() * HEIGHT_FACTOR;
		spos.y -= start.field->get_height() * HEIGHT_FACTOR;

		assert(m_walkstart <= game.get_gametime());
		assert(m_walkstart < m_walkend);
		float f =
			static_cast<float>(game.get_gametime() - m_walkstart)
			/
			(m_walkend - m_walkstart);

		if (f < 0)
			f = 0;
		else if (f > 1)
			f = 1;

		epos.x = static_cast<int32_t>(f * epos.x + (1 - f) * spos.x);
		epos.y = static_cast<int32_t>(f * epos.y + (1 - f) * spos.y);
	}

	return epos;
}


/// It LERPs between start and end position when we are walking.
/// Note that the current node is actually the node that we are walking to, not
/// the the one that we start from.
void Bob::draw
	(const Editor_Game_Base & egbase, RenderTarget & dst, const Point& pos) const
{
	if (m_anim)
		dst.drawanim
			(calc_drawpos(egbase, pos),
			 m_anim,
			 egbase.get_gametime() - m_animstart,
			 get_owner());
}


/**
 * Set a looping animation, starting now.
 */
void Bob::set_animation(Editor_Game_Base & egbase, uint32_t const anim)
{
	m_anim = anim;
	m_animstart = egbase.get_gametime();
}


/**
 * Cause the object to walk, honoring passable/impassable parts of the map
 * using movecaps. If force is true, the passability check is skipped.
 *
 * \return the number of milliseconds after which the walk has ended. You must
 * call end_walk() after this time, so schedule a task_act(). Returns -1
 * if the step is forbidden, and -2 if it is currently blocked.
 */
int32_t Bob::start_walk
	(Game & game, WalkingDir const dir, uint32_t const a, bool const force)
{
	FCoords newnode;

	Map & map = game.map();
	map.get_neighbour(m_position, dir, &newnode);

	// Move capability check
	if (!force) {
		CheckStepDefault cstep(descr().movecaps());

		if (!cstep.allowed(map, m_position, newnode, dir, CheckStep::stepNormal))
			return -1;
	}

	//  Always call checkNodeBlocked, because it might communicate with other
	//  bobs (as is the case for soldiers on the battlefield).
	if (checkNodeBlocked(game, newnode, true) and !force)
		return -2;

	// Move is go
	int32_t const tdelta = map.calc_cost(m_position, dir);
	assert(tdelta);

	m_walking = dir;
	m_walkstart = game.get_gametime();
	m_walkend = m_walkstart + tdelta;

	set_position(game, newnode);
	set_animation(game, a);

	return tdelta; // yep, we were successful
}


bool Bob::checkNodeBlocked(Game & game, const FCoords & field, bool)
{
	// Battles always block movement!
	std::vector<Bob *> soldiers;
	game.map().find_bobs
		(Area<FCoords>(field, 0), &soldiers, FindBobEnemySoldier(get_owner()));

	if (!soldiers.empty()) {
		container_iterate(std::vector<Bob *>, soldiers, i) {
			Soldier & soldier = ref_cast<Soldier, Bob>(**i.current);
			if (soldier.getBattle())
				return true;
		}
	}

	return false;
}


/**
 * Give the bob a new owner.
 *
 * This will update the owner's viewing area.
 */
void Bob::set_owner(Player * const player)
{
	if (m_owner && m_position.field)
		m_owner->unsee_area(Area<FCoords>(get_position(), vision_range()));

	m_owner = player;

	if (m_owner != nullptr && m_position.field)
		m_owner->see_area(Area<FCoords>(get_position(), vision_range()));
}


/**
 * Move the bob to a new position.
 *
 * Performs the necessary (un)linking in the \ref Field structures and
 * updates the owner's viewing area, if the bob has an owner.
 */
void Bob::set_position(Editor_Game_Base & egbase, const Coords & coords)
{
	FCoords oldposition = m_position;

	if (m_position.field) {
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	m_position = egbase.map().get_fcoords(coords);

	m_linknext = m_position.field->bobs;
	m_linkpprev = &m_position.field->bobs;
	if (m_linknext)
		m_linknext->m_linkpprev = &m_linknext;
	*m_linkpprev = this;

	if (m_owner != nullptr) {
		m_owner->see_area(Area<FCoords>(get_position(), vision_range()));

		if (oldposition.field)
			m_owner->unsee_area(Area<FCoords>(oldposition, vision_range()));
	}

	// Since pretty much everything in Widelands eventually results in the
	// movement of a worker (e.g. transporting wares etc.), this should
	// help us to find desyncs pretty rapidly.
	// In particular, I wanted to add something to set_position because
	// it involves coordinates and will thus additionally highlight desyncs
	// in pathfinding even when two paths have the same length, and in
	// randomly generated movements.
	if (upcast(Game, game, &egbase)) {
		StreamWrite & ss = game->syncstream();
		ss.Unsigned32(serial());
		ss.Signed16(coords.x);
		ss.Signed16(coords.y);
	}
}

/// Give debug information.
void Bob::log_general_info(const Editor_Game_Base & egbase)
{
	molog("Owner: %p\n", m_owner);
	molog("Postition: (%i, %i)\n", m_position.x, m_position.y);
	molog("ActID: %i\n", m_actid);
	molog("ActScheduled: %s\n", m_actscheduled ? "true" : "false");
	molog
		("Animation: %s\n",
		 m_anim ? descr().get_animation_name(m_anim).c_str() : "\\<none\\>");

	molog("AnimStart: %i\n", m_animstart);
	molog("WalkingDir: %i\n", m_walking);
	molog("WalkingStart: %i\n", m_walkstart);
	molog("WalkEnd: %i\n", m_walkend);

	molog("Signal: %s\n", m_signal.c_str());

	molog("Stack size: %lu\n", static_cast<long unsigned int>(m_stack.size()));

	for (size_t i = 0; i < m_stack.size(); ++i) {
		molog
			("Stack dump %lu/%lu\n",
			 static_cast<long unsigned int>(i + 1),
			 static_cast<long unsigned int>(m_stack.size()));

		molog("* task->name: %s\n", m_stack[i].task->name);

		molog("* ivar1: %i\n", m_stack[i].ivar1);
		molog("* ivar2: %i\n", m_stack[i].ivar2);
		molog("* ivar3: %i\n", m_stack[i].ivar3);

		molog("* object pointer: %p\n", m_stack[i].objvar1.get(egbase));
		molog("* svar1: %s\n", m_stack[i].svar1.c_str());

		molog("* coords: (%i, %i)\n", m_stack[i].coords.x, m_stack[i].coords.y);
		molog("* diranims:");
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			molog(" %d", m_stack[i].diranims.get_animation(dir));
		}
		molog("\n* path: %p\n",  m_stack[i].path);
		if (m_stack[i].path) {
			const Path & path = *m_stack[i].path;
			Path::Step_Vector::size_type nr_steps = path.get_nsteps();
			molog
				("** Path length: %lu\n",
				 static_cast<long unsigned int>(nr_steps));
			molog("** Start: (%i, %i)\n", path.get_start().x, path.get_start().y);
			molog("** End: (%i, %i)\n", path.get_end().x, path.get_end().y);
			for (Path::Step_Vector::size_type j = 0; j < nr_steps; ++j)
				molog
					("** Step %lu/%lu: %i\n",
					 static_cast<long unsigned int>(j + 1),
					 static_cast<long unsigned int>(nr_steps), path[j]);
		}
		molog("* route: %p\n",  m_stack[i].route);

		molog("* program: %p\n",  m_stack[i].route);
	}
}


/*
==============================

Load/save support

==============================
*/

#define BOB_SAVEGAME_VERSION 1

Bob::Loader::Loader()
{
}

void Bob::Loader::load(FileRead & fr)
{
	Map_Object::Loader::load(fr);

	uint8_t version = fr.Unsigned8();
	if (version != BOB_SAVEGAME_VERSION)
		throw game_data_error("unknown/unhandled version: %u", version);

	Bob & bob = get<Bob>();

	if (Player_Number owner_number = fr.Unsigned8()) {
		if (owner_number > egbase().map().get_nrplayers())
			throw game_data_error
				("owner number is %u but there are only %u players",
				 owner_number, egbase().map().get_nrplayers());

		Player * owner = egbase().get_player(owner_number);
		if (!owner)
			throw game_data_error("owning player %u does not exist", owner_number);

		bob.set_owner(owner);
	}

	bob.set_position(egbase(), fr.Coords32());

	std::string animname = fr.CString();
	bob.m_anim = animname.size() ? bob.descr().get_animation(animname) : 0;
	bob.m_animstart = fr.Signed32();
	bob.m_walking = static_cast<WalkingDir>(fr.Direction8_allow_null());
	if (bob.m_walking) {
		bob.m_walkstart = fr.Signed32();
		bob.m_walkend = fr.Signed32();
	}

	bob.m_actid = fr.Unsigned32();
	bob.m_signal = fr.CString();

	uint32_t stacksize = fr.Unsigned32();
	bob.m_stack.resize(stacksize);
	states.resize(stacksize);
	for (uint32_t i = 0; i < stacksize; ++i) {
		State & state = bob.m_stack[i];
		LoadState & loadstate = states[i];

		state.task = get_task(fr.CString());
		state.ivar1 = fr.Signed32();
		state.ivar2 = fr.Signed32();
		state.ivar3 = fr.Signed32();
		loadstate.objvar1 = fr.Unsigned32();
		state.svar1 = fr.CString();
		state.coords = fr.Coords32_allow_null(egbase().map().extent());

		if (fr.Unsigned8()) {
			uint32_t anims[6];
			for (int j = 0; j < 6; ++j)
				anims[j] = bob.descr().get_animation(fr.CString());
			state.diranims = DirAnimations(anims[0], anims[1], anims[2], anims[3], anims[4], anims[5]);
		}

		if (fr.Unsigned8()) {
			state.path = new Path;
			state.path->load(fr, egbase().map());
		}

		if (fr.Unsigned8()) {
			state.route = new Route;
			state.route->load(loadstate.route, fr);
		}

		std::string programname = fr.CString();
		if (programname.size())
			state.program = get_program(programname);
	}
}

void Bob::Loader::load_pointers()
{
	Map_Object::Loader::load_pointers();

	Bob & bob = get<Bob>();
	for (uint32_t i = 0; i < bob.m_stack.size(); ++i) {
		State & state = bob.m_stack[i];
		LoadState & loadstate = states[i];

		if (loadstate.objvar1)
			state.objvar1 = &mol().get<Map_Object>(loadstate.objvar1);

		if (state.route)
			state.route->load_pointers(loadstate.route, mol());
	}
}

void Bob::Loader::load_finish()
{
	Map_Object::Loader::load_finish();

	// Care about new mapobject saving / loading - map objects don't get a task,
	// if created in the editor, so we give them one here.
	//  See bug #537392 for more information:
	//   https://bugs.launchpad.net/widelands/+bug/537392
	Bob & bob = get<Bob>();
	if (bob.m_stack.empty() && !egbase().get_gametime())
		if (upcast(Game, game, &egbase())) {
			bob.init_auto_task(*game);
		}

}

const Bob::Task * Bob::Loader::get_task(const std::string & name)
{
	if (name == "move") return &taskMove;
	if (name == "movepath") return &taskMovepath;
	if (name == "idle") return &taskIdle;

	throw game_data_error("unknown bob task '%s'", name.c_str());
}

const BobProgramBase * Bob::Loader::get_program(const std::string & name)
{
	throw game_data_error("unknown bob program '%s'", name.c_str());
}

void Bob::save
	(Editor_Game_Base & eg, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	Map_Object::save(eg, mos, fw);

	fw.Unsigned8(BOB_SAVEGAME_VERSION);

	fw.Unsigned8(m_owner ? m_owner->player_number() : 0);
	fw.Coords32(m_position);

	// m_linkpprev and m_linknext are recreated automatically

	fw.CString(m_anim ? descr().get_animation_name(m_anim) : "");
	fw.Signed32(m_animstart);
	fw.Direction8_allow_null(m_walking);
	if (m_walking) {
		fw.Signed32(m_walkstart);
		fw.Signed32(m_walkend);
	}

	fw.Unsigned32(m_actid);
	fw.CString(m_signal);

	fw.Unsigned32(m_stack.size());
	for (unsigned int i = 0; i < m_stack.size(); ++i) {
		const State & state = m_stack[i];

		fw.CString(state.task->name);
		fw.Signed32(state.ivar1);
		fw.Signed32(state.ivar2);
		fw.Signed32(state.ivar3);
		if (const Map_Object * obj = state.objvar1.get(eg)) {
			fw.Unsigned32(mos.get_object_file_index(*obj));
		} else {
			fw.Unsigned32(0);
		}
		fw.CString(state.svar1);

		fw.Coords32(state.coords);

		if (state.diranims) {
			fw.Unsigned8(1);
			for (int dir = 1; dir <= 6; ++dir)
				fw.CString
					(descr().get_animation_name
						(state.diranims.get_animation(dir)).c_str());
		} else {
			fw.Unsigned8(0);
		}

		if (state.path) {
			fw.Unsigned8(1);
			state.path->save(fw);
		} else {
			fw.Unsigned8(0);
		}

		if (state.route) {
			fw.Unsigned8(1);
			state.route->save(fw, eg, mos);
		} else {
			fw.Unsigned8(0);
		}

		fw.CString(state.program ? state.program->get_name() : "");
	}
}

}
