/*
 * Copyright (C) 2002 by The Widelands Development Team
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
#include "world.h"
#include "game.h"
#include "cmd_queue.h"
#include "player.h"
#include "bob.h"
#include "map.h"
#include "profile.h"


/*
==============================================================================

Bob IMPLEMENTATION		

==============================================================================
*/

/*
===============
Bob_Descr::Bob_Descr
Bob_Descr::~Bob_Descr
===============
*/
Bob_Descr::Bob_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
}

Bob_Descr::~Bob_Descr(void)
{
}

/*
===============
Bob_Descr::read

Parse additional information from the config file
===============
*/
void Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
	m_idle_anim.parse(directory, prof->get_safe_section("idle"), picname, encdata);
}

/*
===============
Bob_Descr::create

Create a bob of this type
===============
*/
Bob *Bob_Descr::create(Game *g, Player *owner, Coords coords)
{
	Bob *bob = create_object();
	bob->set_owner(owner);
	bob->set_position(g, coords);
	bob->init(g);
	return bob;
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Bob::Bob

Zero-initialize a map object
===============
*/
Bob::Bob(Bob_Descr* descr)
	: Map_Object(descr)
{
	m_owner = 0;
	m_position.x = m_position.y = 0; // not linked anywhere
	m_position.field = 0;
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

/*
===============
Bob::~Bob()

Cleanup an object. Removes map links
===============
*/
Bob::~Bob()
{
	if (m_position.field) {
		log("Map_Object::~Map_Object: m_pos.field != 0, cleanup() not called!\n");
		*(int *)0 = 0;
	}
}

/*
===============
Bob::get_type
===============
*/
int Bob::get_type()
{
	return BOB;
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
Bob::init

Make sure you call this from derived classes!

Initialize the object by setting the initial task.
===============
*/
void Bob::init(Game* g)
{
	Map_Object::init(g);

	// Initialize task system
	m_lasttask = 0;
	m_lasttask_success = true;
	m_nexttask = 0;
	
	do_next_task(g);
}


/*
===============
Bob::cleanup

Perform Game-related cleanup as necessary.
===============
*/
void Bob::cleanup(Game *g)
{
	if (get_current_task())
		task_end(g); // subtle...
	
	if (m_position.field) {
		m_position.field = 0;
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}
	
	Map_Object::cleanup(g);
}


/*
===============
Bob::act

Hand the acting over to the task
 
Change to the next task if necessary.
===============
*/
void Bob::act(Game* g)
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

/*
===============
Bob::do_next_task [private]

Try to get the next task running.
===============
*/
void Bob::do_next_task(Game* g)
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

/*
===============
Bob::start_task

Start the given task.

Only allowed when m_task_switching, i.e. from init() and act().
Consequently, derived classes can only call this from task_start_best().
===============
*/
void Bob::start_task(Game* g, uint task)
{
	assert(m_task_switching);
	assert(!m_task);
	
	m_task = task;
}

/*
===============
Bob::do_start_task [private]

Actually start the task (m_task is set already)
===============
*/
void Bob::do_start_task(Game* g)
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

/*
===============
Bob::end_task

Let the task end itself, indicating success or failure.
nexttask will be passed to task_start_best() to help the decision.

Only allowed when m_task_acting, i.e. from act() or start_task()
and thus only from task_begin() and task_act()

Be aware that end_task() calls task_end() which may cleanup some
structures belonging to your task.
===============
*/
void Bob::end_task(Game* g, bool success, uint nexttask)
{
	assert(m_task_acting);
	assert(m_task);

	task_end(g);
		
	m_lasttask = m_task;
	m_lasttask_success = success;
	m_nexttask = nexttask;
	
	m_task = 0;
}

/*
===============
Bob::start_task_idle

Start an idle phase, using the given animation
If the timeout is a positive value, the idle phase stops after the given
time.

This task always succeeds.
===============
*/
void Bob::start_task_idle(Game* g, Animation* anim, int timeout)
{
	// timeout == 0 will wait indefinitely - probably NOT what you want (use -1 for infinite)
	assert(timeout < 0 || timeout > 0);

	set_animation(g, anim);
	task.idle.timeout = timeout;
	start_task(g, TASK_IDLE);
}

/*
===============
Bob::start_task_movepath

Start moving to the given destination. persist is the same parameter as
for Map::findpath().

Returns false if no path could be found.

The task finishes once the goal has been reached. It may fail.
===============
*/
bool Bob::start_task_movepath(Game* g, Coords dest, int persist, DirAnimations *anims)
{
	task.movepath.path = new Path;

	if (g->get_map()->findpath(m_position, dest, get_movecaps(), persist, task.movepath.path) < 0) {
		delete task.movepath.path;
		return false;
	}
	
	task.movepath.step = 0;
	task.movepath.anims = anims;
	
	start_task(g, TASK_MOVEPATH);
	return true;
}
		
/*
===============
Bob::task_begin [virtual]

This function is called to start a task.

In this function, you may:
 - call end_task()
 - call set_animation(), start_walk(), set_position() and similar functions
 - call task_act() for "array-based" tasks
 
You can schedule a call to task_act() by returning the time, in milliseconds,
until task_act() should be could. NOTE: this time is relative to the current
time!
If you return a value <= 0, task_act() will _never_ be called. This means that
the task can never end - it will continue till infinity (note that this may
be changed at a later point, introducing something like interrupt_task).
===============
*/
int Bob::task_begin(Game* g)
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

/*
===============
Bob::task_act [virtual]

Calls to this function are scheduled by this function and task_begin().

In this function you may call all the functions available in task_begin().

As with task_begin(), you can also schedule another call to task_act() by
returning a value > 0
===============
*/
int Bob::task_act(Game* g)
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
			assert(m_position == task.movepath.path->get_end());
			end_task(g, true, 0); // success
			return 0;
		}

		char dir = task.movepath.path->get_step(task.movepath.step);
	
		int tdelta = start_walk(g, (WalkingDir)dir, task.movepath.anims->get_animation(dir));
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

/*
===============
Bob::task_end [virtual]

Called by end_task(). Use it to clean up any structures allocated in
task_begin() or a start_task_*() type function.
===============
*/
void Bob::task_end(Game*)
{
	switch(get_current_task()) {
	case TASK_MOVEPATH:
		if (task.movepath.path)
			delete task.movepath.path;
		break;
	}
}

/*
===============
Bob::draw

Draw the map object. 
posx/posy is the on-bitmap position of the field we're currently on,
WITHOUT height taken into account.

It LERPs between start and end position when we're walking.
Note that the current field is actually the field we're walking to, not
the one we start from.
===============
*/
void Bob::draw(Game *game, Bitmap* dst, int posx, int posy)
{
	if (!m_anim)
		return;

	Map *map = game->get_map();
	FCoords end;
	FCoords start;
	int sx, sy;
	int ex, ey;
	const uchar *playercolors = 0;
	
	if (get_owner())
		playercolors = get_owner()->get_playercolor_rgb();

	end = m_position;
	ex = posx;
	ey = posy;

	sx = ex;
	sy = ey;
	
	switch(m_walking) {
	case WALK_NW: map->get_brn(end, &start); sx += FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_NE: map->get_bln(end, &start); sx -= FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_W: map->get_rn(end, &start); sx += FIELD_WIDTH; break;
	case WALK_E: map->get_ln(end, &start); sx -= FIELD_WIDTH; break;
	case WALK_SW: map->get_trn(end, &start); sx += FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	case WALK_SE: map->get_tln(end, &start); sx -= FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	
	case IDLE: start.field = 0; break;
	}

	if (start.field) {
		sy += end.field->get_height()*HEIGHT_FACTOR;
		sy -= start.field->get_height()*HEIGHT_FACTOR;

		float f = (float)(game->get_gametime() - m_walkstart) / (m_walkend - m_walkstart);
		if (f < 0) f = 0;
		else if (f > 1) f = 1;
		
		ex = (int)(f*ex + (1-f)*sx);
		ey = (int)(f*ey + (1-f)*sy);
	}

	copy_animation_pic(dst, m_anim, game->get_gametime() - m_animstart, ex, ey, playercolors);
}


/*
===============
Bob::set_animation

Set a looping animation, starting now.
===============
*/
void Bob::set_animation(Game* g, Animation* anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/*
===============
Bob::is_walking

Return true if we're currently walking
===============
*/
bool Bob::is_walking()
{
	return m_walking != IDLE;
}

/*
===============
Bob::end_walk

Call this from your task_act() function that was scheduled after start_walk().
===============
*/
void Bob::end_walk(Game* g)
{
	m_walking = IDLE;
}


/*
===============
Bob::start_walk

Cause the object to walk, honoring passable/impassable parts of the map using movecaps.

Returns the number of milliseconds after which the walk has ended. You must 
call end_walk() after this time, so schedule a task_act().

Returns a negative value when we can't walk into the requested direction.
===============
*/
int Bob::start_walk(Game *g, WalkingDir dir, Animation *a)
{
	FCoords newf;
	
	g->get_map()->get_neighbour(m_position, dir, &newf);

	// Move capability check by ANDing with the field caps
	//
	// The somewhat crazy check involving MOVECAPS_SWIM should allow swimming objects to
	// temporarily land.
	uint movecaps = get_movecaps();

	if (!(m_position.field->get_caps() & movecaps & MOVECAPS_SWIM && newf.field->get_caps() & MOVECAPS_WALK) &&
	    !(newf.field->get_caps() & movecaps))
		return -1;

	// Move is go
	int tdelta = 2000; // :TODO: height-based speed changes
	
	m_walking = dir;
	m_walkstart = g->get_gametime();
	m_walkend = m_walkstart + tdelta;
	
	set_position(g, newf);
	set_animation(g, a);
	
	return tdelta; // yep, we were successful
}

/*
===============
Bob::set_position

Moves the Map_Object to the given position.
===============
*/
void Bob::set_position(Game* g, Coords coords)
{
	if (m_position.field) {
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	m_position = FCoords(coords, g->get_map()->get_field(coords));
	
	m_linknext = m_position.field->bobs;
	m_linkpprev = &m_position.field->bobs;
	if (m_linknext)
		m_linknext->m_linkpprev = &m_linknext;
	m_position.field->bobs = this;
}


/*
==============================================================================   

class Critter_Bob

==============================================================================   
*/

// 
// Description
// 
class Critter_Bob_Descr : public Bob_Descr {
   public:
      Critter_Bob_Descr(const char *name);
      virtual ~Critter_Bob_Descr(void) { } 

      virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
      Bob *create_object();

      inline bool is_swimming(void) { return m_swimming; }
      inline DirAnimations* get_walk_anims(void) { return &m_walk_anims; }

   private:
		DirAnimations	m_walk_anims;
      bool				m_swimming;
};

Critter_Bob_Descr::Critter_Bob_Descr(const char *name)
	: Bob_Descr(name)
{
	m_swimming = 0;
}

void Critter_Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Bob_Descr::parse(directory, prof, encdata);

	Section *s = prof->get_safe_section("global");
	
	s->get_int("stock", 0);
	m_swimming = s->get_bool("swimming", false);
	
   // Read all walking animations.
	// Default settings are in [walk]
	char sectname[256];
	
	snprintf(sectname, sizeof(sectname), "%s_walk_??", m_name);
	m_walk_anims.parse(directory, prof, sectname, prof->get_section("walk"), encdata);
}


//
// Implementation
//
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

class Critter_Bob : public Bob {
	MO_DESCR(Critter_Bob_Descr);

public:
	Critter_Bob(Critter_Bob_Descr *d);
	virtual ~Critter_Bob(void);

	uint get_movecaps();

	virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d)
	: Bob(d)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint Critter_Bob::get_movecaps() { return get_descr()->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }

void Critter_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	if (prev == TASK_IDLE)
	{
		// Pick a target at random
		Coords dst;
		
		dst.x = m_position.x + (g->logic_rand()%5) - 2;
		dst.y = m_position.y + (g->logic_rand()%5) - 2;
		
		if (start_task_movepath(g, dst, 3, get_descr()->get_walk_anims()))
			return;
	
		start_task_idle(g, get_descr()->get_idle_anim(), 1 + g->logic_rand()%1000);
		return;
	}
	
	// idle for a longer period
	start_task_idle(g, get_descr()->get_idle_anim(), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
}

Bob *Critter_Bob_Descr::create_object()
{
	return new Critter_Bob(this);
}


/*
==============================================================================

Bob_Descr factory

==============================================================================
*/

/*
===============
Bob_Descr::create_from_dir(const char *directory) [static]
 
Master factory to read a bob from the given directory and create the
appropriate description class.
===============
*/
Bob_Descr *Bob_Descr::create_from_dir(const char *name, const char *directory, Profile *prof)
{
	Bob_Descr *bob = 0;

	try
	{
		Section *s = prof->get_safe_section("global");
		const char *type = s->get_safe_string("type");

		if (!strcasecmp(type, "critter")) {
			bob = new Critter_Bob_Descr(name);
		} else
			throw wexception("Unsupported bob type '%s'", type);

		bob->parse(directory, prof, 0);
	}
	catch(std::exception &e) {
		if (bob)
			delete bob;
		throw wexception("Error reading bob %s: %s", directory, e.what());
	}
	catch(...) {
		if (bob)
			delete bob;
		throw;
	}
	
	return bob;
}
