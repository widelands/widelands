/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "instances.h"
#include "game.h"
#include "map.h"
#include "player.h"

/** Object_Manager::~Object_Manager()
 *
 * Remove all map objects
 */
Object_Manager::~Object_Manager(void)
{
	// better not throw an exception in a destructor...
	if (!m_objects.empty())
		log("Object_Manager: ouch! remaining objects\n");
}

/*
===============
Object_Manager::cleanup

Clear all objects
===============
*/
void Object_Manager::cleanup(Game *g)
{
	while(!m_objects.empty()) {
		objmap_t::iterator it = m_objects.begin();
		free_object(g, it->second);
	}
}

/** Object_Manager::create_object(Game *g, Map_Object_Descr *d, int owner)
 *
 * Create an object of type d
 */
Map_Object* Object_Manager::create_object(Game *g, Map_Object_Descr *d, int owner, int x, int y)
{
	assert(d);

	// If the object is stationary, kill any other stationary objects on the field
	if (!d->has_attribute(Map_Object::MOVABLE)) {
		std::vector<Map_Object*> objs;
		
		g->get_map()->find_objects(x, y, 0, Map_Object::MOVABLE, &objs, true);
		for(uint i = 0; i < objs.size(); i++) {
			Map_Object *obj = objs[i];
			
			assert(!obj->has_attribute(Map_Object::ROBUST));
				
			obj->die(g);
		}
	
	}
	
	// Create the object
	Map_Object* obj = d->create_object();
	
	m_lastserial++;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	
	m_objects[m_lastserial] = obj;

	obj->set_owned_by(owner);
	obj->set_position(g, x, y);
	obj->init(g);
	
	return obj;
}

/** Object_Manager::free_object(Game* g, Map_Object* obj)
 *
 * Free the given object.
 */
void Object_Manager::free_object(Game* g, Map_Object* obj)
{
	int x, y;

	obj->cleanup(g);

	m_objects.erase(obj->m_serial);
	delete obj;
}


Map_Object* Object_Ptr::get(Game* game)
{
	if (!m_serial) return 0;
	Map_Object* obj = game->get_objects()->get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}


/*
==============================================================================

Map_Object_Descr IMPLEMENTATION
		
==============================================================================
*/

/*
===============
Map_Object_Descr::has_attribute

Search for the attribute in the attribute list
===============
*/
bool Map_Object_Descr::has_attribute(uint attr)
{
	for(uint i = 0; i < m_attributes.size(); i++) {
		if (m_attributes[i] == attr)
			return true;
	}
	
	return false;
}
		

/*
===============
Map_Object_Descr::add_attribute

Add an attribute to the attribute list if it's not already there
===============
*/
void Map_Object_Descr::add_attribute(uint attr)
{
	if (!has_attribute(attr))
		m_attributes.push_back(attr);
}


/*
==============================================================================

Map_Object IMPLEMENTATION		

==============================================================================
*/

/** Map_Object::Map_Object(Map_Object_Descr* descr)
 *
 * Zero-initialize a map object
 */
Map_Object::Map_Object(Map_Object_Descr* descr)
{
	m_descr = descr;
	m_serial = 0;

	m_owned_by = 0;
	m_field = 0; // not linked anywhere
	m_pos.x = m_pos.y = 0;
	m_linknext = 0;
	m_linkpprev = 0;

	m_anim = 0;
	m_animstart = 0;
	
	m_walking = IDLE;
	m_walkstart = m_walkend = 0;
	
	m_task = 0;
	m_task_acting = false;
	m_task_switching = false;
}

/** Map_Object::~Map_Object()
 *
 * Cleanup an object. Removes map links
 */
Map_Object::~Map_Object()
{
	if (m_field) {
		log("Map_Object::~Map_Object: m_field != 0, cleanup() not called!\n");
		*(int *)0 = 0;
	}
}


/*
===============
Map_Object::die

Call this function if you want to remove the object.
It schedules the object for immediate deletion by the Cmd_Queue.

You should not delete an object directly through the Object_Manager because
it's hard to tell the backtrace you're called from.
===============
*/
void Map_Object::die(Game *g)
{
	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_REMOVE, m_serial, 0, 0);
}


/*
Objects and tasks
-----------------

Every object _always_ has a current task which it is doing.
For a boring object, this task is always an IDLE task, which will be
reduced to effectively 0 CPU overhead.

For another simple example, look at animals. They have got two states:
moving or not moving. This is actually represented as two tasks, 
IDLE and MOVE_PATH, which are both part of the default package that comes
with Map_Object.

Now there are some important considerations:
- every object must always have a task, even if it's IDLE
- be careful as to when you call task handling functions; the comments
  above each function tell you when you can call them, and which functions
  you can call from them
- a task can only end itself; it cannot be ended by someone else
- there are default, predefined tasks (TASK_IDLE, TASK_MOVEPATH); use them
- you must call start_task_*() for the default tasks. Do not start them
  directly!

To implement a new task, you need to create a new task_begin(), task_act()
and perhaps task_end(). Create a switch()-statement for the new task(s) and
call the base class' task_*() functions in the default branch.
Most likely, you'll also want a start_task_*()-type function.
*/

/*
===============
Map_Object::init

Make sure you call this from derived classes!

Initialize the object by setting the initial task.
===============
*/
void Map_Object::init(Game* g)
{
	if (has_attribute(ROBUST))
		g->get_map()->recalc_for_field(m_pos.x, m_pos.y);

	// Initialize task system
	m_lasttask = 0;
	m_lasttask_success = true;
	m_nexttask = 0;
	
	do_next_task(g);
}


/*
===============
Map_Object::cleanup

Perform Game-related cleanup as necessary.
===============
*/
void Map_Object::cleanup(Game *g)
{
	if (get_current_task())
		task_end(g); // subtle...
	
	if (m_field) {
		m_field = 0;
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
		
		if (has_attribute(Map_Object::ROBUST))
			g->get_map()->recalc_for_field(m_pos.x, m_pos.y);
	}
}


/** Map_Object::act(Game*)
 *
 * Hand the acting over to the task
 * 
 * Change to the next task if necessary.
 */
void Map_Object::act(Game* g)
{
	m_task_acting = true;
	int tdelta = task_act(g);
	// a tdelta == 0 is probably NOT what you want - make your intentions clear
	assert(!m_task || tdelta < 0 || tdelta > 0);
	m_task_acting = false;

	if (!m_task) {
		do_next_task(g);
		return;
	}
		
	if (tdelta > 0)
		g->get_cmdqueue()->queue(g->get_gametime()+tdelta, SENDER_MAPOBJECT, CMD_ACT, m_serial, 0, 0);
}

/** Map_Object::do_next_task(Game*) [private]
 *
 * Try to get the next task running.
 */
void Map_Object::do_next_task(Game* g)
{
	int task_retries = 0;
	
	assert(!m_task);
	
	while(!m_task) {
		assert(task_retries < 5); // detect infinite loops early
	
		m_task_switching = true;
		task_start_best(g, m_lasttask, m_lasttask_success, m_nexttask);
		m_task_switching = false;

		do_start_task(g);
				
		task_retries++;
	}
}

/** Map_Object::start_task(Game*, uint task)
 *
 * Start the given task.
 *
 * Only allowed when m_task_switching, i.e. from init() and act().
 * Consequently, derived classes can only call this from task_start_best().
 */
void Map_Object::start_task(Game* g, uint task)
{
	assert(m_task_switching);
	assert(!m_task);
	
	m_task = task;
}

/** Map_Object::do_start_task(Game*) [private]
 *
 * Actually start the task (m_task is set already)
 */
void Map_Object::do_start_task(Game* g)
{
	assert(m_task);

	m_task_acting = true;
	int tdelta = task_begin(g);
	// a tdelta == 0 is probably NOT what you want - make your intentions clear
	assert(!m_task || tdelta < 0 || tdelta > 0);
	m_task_acting = false;
	
	if (m_task && tdelta > 0)
		g->get_cmdqueue()->queue(g->get_gametime()+tdelta, SENDER_MAPOBJECT, CMD_ACT, m_serial, 0, 0);
}

/** Map_Object::end_task(Game*, bool success, uint nexttask)
 *
 * Let the task end itself, indicating success or failure.
 * nexttask will be passed to task_start_best() to help the decision.
 *
 * Only allowed when m_task_acting, i.e. from act() or start_task()
 * and thus only from task_begin() and task_act()
 *
 * Be aware that end_task() calls task_end() which may cleanup some
 * structures belonging to your task.
 */
void Map_Object::end_task(Game* g, bool success, uint nexttask)
{
	assert(m_task_acting);
	assert(m_task);

	task_end(g);
		
	m_lasttask = m_task;
	m_lasttask_success = success;
	m_nexttask = nexttask;
	
	m_task = 0;
}

/** Map_Object::start_task_idle(Game*, Animation* anim, int timeout)
 *
 * Start an idle phase, using the given animation
 * If the timeout is a positive value, the idle phase stops after the given
 * time.
 *
 * This task always succeeds.
 */
void Map_Object::start_task_idle(Game* g, Animation* anim, int timeout)
{
	// timeout == 0 will wait indefinitely - probably NOT what you want (use -1 for infinite)
	assert(timeout < 0 || timeout > 0);

	set_animation(g, anim);
	task.idle.timeout = timeout;
	start_task(g, TASK_IDLE);
}

/** Map_Object::start_task_movepath(Game* g, Coords dest, int persist, Animation **anims)
 *
 * Start moving to the given destination. persist is the same parameter as
 * for Map::findpath().
 * anims is an array of 6 animations, one for each direction.
 * The order is the canonical NE, E, SE, SW, W, NW (order of the enum)
 *
 * Returns false if no path could be found.
 *
 * The task finishes once the goal has been reached. It may fail.
 */
bool Map_Object::start_task_movepath(Game* g, Coords dest, int persist, Animation **anims)
{
	task.movepath.path = new Path;

	if (g->get_map()->findpath(m_pos, dest, get_movecaps(), persist, task.movepath.path) < 0) {
		delete task.movepath.path;
		return false;
	}
	
	task.movepath.step = 0;
	memcpy(task.movepath.anims, anims, sizeof(Animation*)*6);
	
	start_task(g, TASK_MOVEPATH);
	return true;
}
		
/** Map_Object::task_begin(Game*) [virtual]
 *
 * This function is called to start a task.
 *
 * In this function, you may:
 *  - call end_task()
 *  - call set_animation(), start_walk(), set_position() and similar functions
 *  - call task_act() for "array-based" tasks
 *
 * You can schedule a call to task_act() by returning the time, in milliseconds,
 * until task_act() should be could. NOTE: this time is relative to the current
 * time!
 * If you return a value <= 0, task_act() will _never_ be called. This means that
 * the task can never end - it will continue till infinity (note that this may
 * be changed at a later point, introducing something like interrupt_task).
 */
int Map_Object::task_begin(Game* g)
{
	switch(get_current_task()) {
	case TASK_IDLE:
		return task.idle.timeout;
		
	case TASK_MOVEPATH:
		return task_act(g);
	}

	cerr << "task_begin: Unhandled task " << m_task << endl;
	assert(!"task_begin: Unhandled task ");
	return -1; // shut up compiler
}

/** Map_Object::task_act(Game*) [virtual]
 *
 * Calls to this function are scheduled by this function and task_begin().
 *
 * In this function you may call all the functions available in task_begin().
 *
 * As with task_begin(), you can also schedule another call to task_act() by
 * returning a value > 0
 */
int Map_Object::task_act(Game* g)
{
	switch(get_current_task()) {
	case TASK_IDLE:
		end_task(g, true, 0); // success, no next task
		return 0; /* will be ignored */
	
	case TASK_MOVEPATH:
	{
		if (task.movepath.step)
			end_walk(g);
		
		if (task.movepath.step >= task.movepath.path->get_nsteps()) {
			assert(m_pos == task.movepath.path->get_end());
			end_task(g, true, 0); // success
			return 0;
		}

		char dir = task.movepath.path->get_step(task.movepath.step);
		Animation *a = task.movepath.anims[dir-1];
	
		int tdelta = start_walk(g, (WalkingDir)dir, a);
		if (tdelta < 0) {
			end_task(g, false, 0); // failure to reach goal
			return 0;
		}

		task.movepath.step++;		
		return tdelta;
	}
	}

	cerr << "task_act: Unhandled task " << m_task << endl;
	assert(!"task_act: Unhandled task ");
	return -1; // shut up compiler
}

/** Map_Object::task_end(Game*) [virtual]
 *
 * Called by end_task(). Use it to clean up any structures allocated in
 * task_begin() or a start_task_*() type function.
 */
void Map_Object::task_end(Game*)
{
	switch(get_current_task()) {
	case TASK_MOVEPATH:
		if (task.movepath.path)
			delete task.movepath.path;
		break;
	}
}

/** Map_Object::draw(Game *game, Bitmap* dst, int posx, int posy)
 *
 * Draw the map object. 
 * posx/posy is the on-bitmap position of the field we're currently on,
 * WITHOUT height taken into account.
 *
 * It LERPs between start and end position when we're walking.
 * Note that the current field is actually the field we're walking to, not
 * the one we start from.
 */
void Map_Object::draw(Game *game, Bitmap* dst, int posx, int posy)
{
	if (!m_anim)
		return;

	Map *map = game->get_map();
	Field *start, *end;
	int dummyx, dummyy;
	int sx, sy;
	int ex, ey;
	const uchar *playercolors = 0;
	
	if (get_owned_by()) {
		Player *player = game->get_player(get_owned_by());
		playercolors = player->get_playercolor_rgb();
	}
	
	end = m_field;
	ex = posx;
	ey = posy;

	start = 0;
	sx = ex;
	sy = ey;
	
	switch(m_walking) {
	case WALK_NW: map->get_brn(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_NE: map->get_bln(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_W: map->get_rn(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH; break;
	case WALK_E: map->get_ln(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH; break;
	case WALK_SW: map->get_trn(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	case WALK_SE: map->get_tln(m_pos.x, m_pos.y, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	
	case IDLE: break;
	}

	ey -= end->get_height()*HEIGHT_FACTOR;
	
	if (start) {
		sy -= start->get_height()*HEIGHT_FACTOR;

		float f = (float)(game->get_gametime() - m_walkstart) / (m_walkend - m_walkstart);
		if (f < 0) f = 0;
		else if (f > 1) f = 1;
		
		ex = (int)(f*ex + (1-f)*sx);
		ey = (int)(f*ey + (1-f)*sy);
	}

	copy_animation_pic(dst, m_anim, game->get_gametime() - m_animstart, ex, ey, playercolors);
}


/** Map_Object::set_animation(Game* g, Animation *anim)
 *
 * Set a looping animation, starting now.
 */
void Map_Object::set_animation(Game* g, Animation* anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/** Map_Object::is_walking()
 *
 * Return true if we're currently walking
 */
bool Map_Object::is_walking()
{
	return m_walking != IDLE;
}

/** Map_Object::end_walk(Game* g)
 *
 * Call this from your task_act() function that was scheduled after start_walk().
 */
void Map_Object::end_walk(Game* g)
{
	m_walking = IDLE;
}


/** Map_Object::start_walk()
 *
 * Cause the object to walk, honoring passable/impassable parts of the map using movecaps.
 *
 * Returns the number of milliseconds after which the walk has ended. You must 
 * call end_walk() after this time, so schedule a task_act().
 *
 * Returns a negative value when we can't walk into the requested direction.
 */
int Map_Object::start_walk(Game *g, WalkingDir dir, Animation *a)
{
	int newx, newy;
	Field *newf;
	
	switch(dir) {
	case IDLE: assert(0); break;
	case WALK_NW: g->get_map()->get_tln(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	case WALK_NE: g->get_map()->get_trn(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	case WALK_W: g->get_map()->get_ln(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	case WALK_E: g->get_map()->get_rn(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	case WALK_SW: g->get_map()->get_bln(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	case WALK_SE: g->get_map()->get_brn(m_pos.x, m_pos.y, m_field, &newx, &newy, &newf); break;
	}

	// Move capability check by ANDing with the field caps
	//
	// The somewhat crazy check involving MOVECAPS_SWIM should allow swimming objects to
	// temporarily land.
	uint movecaps = get_movecaps();

	if (!(m_field->get_caps() & movecaps & MOVECAPS_SWIM && newf->get_caps() & MOVECAPS_WALK) &&
	    !(newf->get_caps() & movecaps))
		return -1;

	// Move is go
	int tdelta = 2000; // :TODO: height-based speed changes
	
	m_walking = dir;
	m_walkstart = g->get_gametime();
	m_walkend = m_walkstart + tdelta;
	
	set_position(g, newx, newy, newf);
	set_animation(g, a);
	
	return tdelta; // yep, we were successful
}

/** Map_Object::set_position(Game* g, int x, int y, Field* f=0)
 *
 * Moves the Map_Object to the given position.
 */
void Map_Object::set_position(Game* g, int x, int y, Field* f)
{
	if (m_field) {
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	if (!f)
		f = g->get_map()->get_field(x, y);

	m_field = f;		
	m_pos.x = x;
	m_pos.y = y;
	
	m_linknext = f->objects;
	m_linkpprev = &f->objects;
	if (m_linknext)
		m_linknext->m_linkpprev = &m_linknext;
	f->objects = this;
}

