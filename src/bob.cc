/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "bob.h"

#include "checkstep.h"
#include "critter_bob.h"
#include "game.h"
#include "mapviewpixelconstants.h"
#include "player.h"
#include "profile.h"
#include "rendertarget.h"
#include "transport.h"
#include "tribe.h"
#include "wexception.h"

#include "upcast.h"

namespace Widelands {

/**
 * Only tribe bobs have a vision range, since it would be irrelevant
 * for world bobs.
 *
 * Currently, all bobs use the tribe's default vision range.
 *
 * \returns radius (in fields) of area that the bob can see
 */
uint32_t Bob::Descr::vision_range() const
{
	if (m_owner_tribe)
		return m_owner_tribe->get_bob_vision_range();

	return 0;
}


Bob::Descr::Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const * const tribe, EncodeData const * encdata)
	:
	Map_Object_Descr(_name, _descname),
	m_picture       (directory + global_s.get_string("picture", "menu.png")),
	m_owner_tribe(tribe)
{
	m_default_encodedata.clear();

	{ //  global options
		Section & idle_s = prof.get_safe_section("idle");

		m_default_encodedata.parse(idle_s);

		add_animation
			("idle", g_anim.get (directory, idle_s, "idle.png", encdata));
	}

	// Parse attributes
	while (Section::Value const * val = global_s.get_next_val("attrib")) {
		uint32_t const attrib = get_attribute_id(val->get_string());

		if (attrib < Map_Object::HIGHEST_FIXED_ATTRIBUTE)
			throw wexception("Bad attribute '%s'", val->get_string());

		add_attribute(attrib);
	}
}


/**
 * Create a bob of this type
 */
Bob & Bob::Descr::create
	(Editor_Game_Base & egbase, Player * const owner, Coords const coords) const
{
	Bob & bob = create_object();
	bob.set_owner(owner);
	bob.set_position(egbase, coords);
	bob.init(egbase);

	return bob;
}


/**
 * Zero-initialize a map object
 */
Bob::Bob(const Bob::Descr &_descr) :
Map_Object       (&_descr),
m_owner          (0),
m_position       (FCoords(Coords(0, 0), 0)), // not linked anywhere
m_linknext       (0),
m_linkpprev      (0),
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
		*static_cast<int32_t *>(0) = 0;
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
	while (m_stack.size()) //  bobs in the editor do not have tasks
		do_pop_task(dynamic_cast<Game &>(egbase));

	set_owner(0); // implicitly remove ourselves from owner's map

	if (m_position.field) {
		m_position.field = 0;
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
void Bob::push_task(Game & game, Task const & task)
{
	assert(m_in_act || m_stack.empty());

	m_stack.push_back(State(&task));
	schedule_act(game, 10);
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
	if (state.transfer)
		state.transfer->has_failed();

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
Bob::State * Bob::get_state(Task const & task)
{
	std::vector<State>::iterator it = m_stack.end();

	while (it != m_stack.begin()) {
		--it;

		if (it->task == &task)
			return &*it;
	}

	return 0;
}

Bob::State const * Bob::get_state(Task const & task) const
{
	std::vector<State>::const_iterator it = m_stack.end();

	while (it != m_stack.begin()) {
		--it;

		if (it->task == &task)
			return &*it;
	}

	return 0;
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
 * This function also calls all task's mask() function immediately.
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
	while (m_stack.size())
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

Bob::Task Bob::taskIdle = {
	"idle",

	&Bob::idle_update,
	0, // signal_immediate
	0
};


/**
 * Start an idle phase, using the given animation
 *
 * If the timeout is a positive value, the idle phase stops after the given
 * time.
 *
 * This task always succeeds unless interrupted.
 */
void Bob::start_task_idle(Game & game, uint32_t anim, int32_t timeout)
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
Bob::Task Bob::taskMovepath = {
	"movepath",

	&Bob::movepath_update,
	0, // signal_immediate
	0
};

struct BlockedTracker {
	struct CoordData {
		Coords coord;
		int dist;
	};
	// Distance-based ordering as a heuristic for unblock()
	struct CoordOrdering {
		bool operator()(CoordData const & a, CoordData const & b) const throw () {
			if (a.dist != b.dist)
				return a.dist < b.dist;
			return a.coord.all < b.coord.all;
		}
	};
	typedef std::map<CoordData, bool, CoordOrdering> Cache;

	BlockedTracker(Game & game, Bob & bob, Coords const finaldest)
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
				if (static_cast<int32_t>(m_game.logic_rand() % origblocked) < unblockprob) {
					it->second = false;
					--nrblocked;
					unblockprob -= 2;
				}
			}
		}
	}

	bool isBlocked(FCoords const & field) {
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
	 Coords          const dest,
	 int32_t         const persist,
	 DirAnimations const & anims,
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
	state.diranims = &anims;
	return true;
}


/**
 * Start moving along the given, precalculated path.
 */
void Bob::start_task_movepath
	(Game                &       game,
	 Path          const &       path,
	 DirAnimations const &       anims,
	 bool                  const forceonlast,
	 int32_t               const only_step)
{
	assert(path.get_start() == get_position());

	push_task(game, taskMovepath);
	State & state  = top_state();
	state.path     = new Path(path);
	state.ivar1    = 0;
	state.ivar2    = forceonlast ? 1 : 0;
	state.ivar3    = only_step;
	state.diranims = &anims;
}


/**
 * Move to the given index on the given path. The current position must be on
 * the given path.
 *
 * \return true if a task has been started, or false if we already are on
 * the given path index.
 */
bool Bob::start_task_movepath
	(Game                &       game,
	 Path          const &       origpath,
	 int32_t               const index,
	 DirAnimations const &       anims,
	 bool                  const forceonlast,
	 int32_t               const only_step)
{
	CoordPath path(game.map(), origpath);
	int32_t curidx = path.get_index(get_position());

	if (curidx == -1)
		throw wexception
			("MO(%u): start_task_movepath(index): not on path", serial());

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
	if (get_signal().size())
		return pop_task(game);

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
Bob::Task Bob::taskMove = {
	"move",

	&Bob::move_update,
	0,
	0
};


/**
 * Move into the given direction, without passability checks.
 */
void Bob::start_task_move
	(Game                &       game,
	 int32_t               const dir,
	 DirAnimations const * const anims,
	 bool                  const forcemove)
{
	push_task(game, taskMove);
	State & state  = top_state();
	state.ivar1    = dir;
	state.ivar2    = forcemove;
	state.diranims = anims;
}


void Bob::move_update(Game & game, State & state)
{
	if (state.diranims) {
		int32_t const tdelta =
			start_walk
				(game,
				 static_cast<WalkingDir>(state.ivar1),
				 state.diranims->get_animation(state.ivar1),
				 state.ivar2);
		state.diranims = 0;

		if (tdelta < 0) {
			send_signal(game, tdelta == -2 ? "blocked" : "fail");
			return pop_task(game);
		} else
			return schedule_act(game, tdelta);
	} else
		if (game.get_gametime() - m_walkend >= 0) {
			end_walk();
			return pop_task(game);
		} else
			// Only end the task once we've actually completed the step
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

	case IDLE: start.field = 0; break;
	default:
		assert(false);
	}

	if (start.field) {
		spos.y += end.field->get_height() * HEIGHT_FACTOR;
		spos.y -= start.field->get_height() * HEIGHT_FACTOR;

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
	(Editor_Game_Base const & egbase, RenderTarget & dst, Point const pos) const
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
	FCoords newf;

	Map & map = game.map();
	map.get_neighbour(m_position, dir, &newf);

	// Move capability check
	if (!force) {
		CheckStepDefault cstep(descr().movecaps());

		if (!cstep.allowed(map, m_position, newf, dir, CheckStep::stepNormal))
			return -1;
	}

	//  Always call checkNodeBlocked, because it might communicate with other
	//  bobs (as is the case for soldiers on the battlefield).
	if (checkNodeBlocked(game, newf, true) and !force)
		return -2;

	// Move is go
	int32_t const tdelta = map.calc_cost(m_position, dir);

	m_walking = dir;
	m_walkstart = game.get_gametime();
	m_walkend = m_walkstart + tdelta;

	set_position(game, newf);
	set_animation(game, a);

	return tdelta; // yep, we were successful
}


bool Bob::checkNodeBlocked(Game &, FCoords const &, bool)
{
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

	if (m_owner != 0 && m_position.field)
		m_owner->see_area(Area<FCoords>(get_position(), vision_range()));
}


/**
 * Move the bob to a new position.
 *
 * Performs the necessary (un)linking in the \ref Field structures and
 * updates the owner's viewing area, if the bob has an owner.
 */
void Bob::set_position(Editor_Game_Base & egbase, Coords const coords)
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
	if (m_linknext) m_linknext->m_linkpprev = &m_linknext;
	*m_linkpprev = this;

	if (m_owner != 0) {
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
void Bob::log_general_info(Editor_Game_Base const & egbase)
{
	molog("Owner: %p\n", m_owner);
	molog("Postition: (%i, %i)\n", m_position.x, m_position.y);
	molog("ActID: %i\n", m_actid);
	molog("ActScheduled: %s\n", m_actscheduled ? "true" : "false");
	molog
		("Animation: %s\n",
		 m_anim ? descr().get_animation_name(m_anim).c_str() : "<none>");

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
		molog("* diranims: %p\n",  m_stack[i].diranims);
		molog("* path: %p\n",  m_stack[i].path);
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
		molog("* transfer: %p\n",  m_stack[i].transfer);
		molog("* route: %p\n",  m_stack[i].route);

		molog("* program: %p\n",  m_stack[i].route);
	}
}

};
