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

EncodeData IMPLEMENTAION

==============================================================================
*/

/*
===============
EncodeData::clear

Reset the EncodeData to defaults (no special colors)
===============
*/			
void EncodeData::clear()
{
	hasclrkey = false;
	hasshadow = false;
	hasplrclrs = false;
}


/*
===============
EncodeData::parse

Parse color codes from section, the following keys are currently known:

clrkey_[r,g,b]		Color key
shadowclr_[r,g,b]	color for shadow pixels
===============
*/
void EncodeData::parse(Section *s)
{
	int i;
	int r, g, b;

	// Read color key
	r = s->get_int("clrkey_r", -1);
	g = s->get_int("clrkey_g", -1);
	b = s->get_int("clrkey_b", -1);
	if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		hasclrkey = true;
		clrkey_r = r;
		clrkey_g = g;
		clrkey_b = b;
	}
	
	// Read shadow color
	r = s->get_int("shadowclr_r", -1);
	g = s->get_int("shadowclr_g", -1);
	b = s->get_int("shadowclr_b", -1);
	if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		hasshadow = true;
		shadow_r = r;
		shadow_g = g;
		shadow_b = b;
	}

	// Read player color codes	
	for(i = 0; i < 4; i++) {
		char keyname[32];
		
		snprintf(keyname, sizeof(keyname), "plrclr%i_r", i);
		r = s->get_int(keyname, -1);
		snprintf(keyname, sizeof(keyname), "plrclr%i_g", i);
		g = s->get_int(keyname, -1);
		snprintf(keyname, sizeof(keyname), "plrclr%i_b", i);
		b = s->get_int(keyname, -1);
		
		if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
			break;
		
		plrclr_r[i] = r;
		plrclr_g[i] = g;
		plrclr_b[i] = b;
	}
	
	if (i == 4)
		hasplrclrs = true;
}


/*
===============
EncodeData::add

Add another encode data. Already existing color codes are overwritten
===============
*/
void EncodeData::add(const EncodeData *other)
{
	if (other->hasclrkey) {
		hasclrkey = true;
		clrkey_r = other->clrkey_r;
		clrkey_g = other->clrkey_g;
		clrkey_b = other->clrkey_b;
	}
	
	if (other->hasshadow) {
		hasshadow = true;
		shadow_r = other->shadow_r;
		shadow_g = other->shadow_g;
		shadow_b = other->shadow_b;
	}
	
	if (other->hasplrclrs) {
		hasplrclrs = true;
		for(int i = 0; i < 4; i++) {
			plrclr_r[i] = other->plrclr_r[i];
			plrclr_g[i] = other->plrclr_g[i];
			plrclr_b[i] = other->plrclr_b[i];
		}
	}
}


/*
==============================================================================

Animation IMPLEMENTAION

==============================================================================
*/
			
Animation::Animation(void)
{
	npics = 0;
	pics = 0;
	m_frametime = FRAME_LENGTH;
	w = h = 0;
	hsx = hsy = 0;
}

Animation::~Animation(void)
{ 
	if(npics) {
		uint i; 
		for(i=0; i<npics; i++) {
			free(pics[i].data);
		}
		free(pics);
	}
}

/** Animation::add_pic(ushort size, ushort* data)
 *
 * Adds one frame of raw encoded data to the animation
 */
void Animation::add_pic(ushort size, ushort* data)
{
	if(!pics) {
		pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
		npics=1;
	} else {
		++npics;
		pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic)*npics);
	}
	pics[npics-1].data=(ushort*)malloc(size);
	pics[npics-1].parent=this;
	memcpy(pics[npics-1].data, data, size);
}

/** Animation::add_pic(Pic* pic, bool hasclrkey, ushort clrkey, bool hasshadow, ushort shadowclr)
 *
 * Adds one frame to the animation
 */
void Animation::add_pic(Pic* pic, const EncodeData *encdata)
{
	if(pic->get_w() != w && pic->get_h() != h)
		throw wexception("frame must be %ix%i pixels big", w, h);

	// Pack the EncodeData colorkey&co. to 16 bit
	ushort clrkey = 0;
	ushort shadowclr = 0;
	int hasplrclrs = 0;
	ushort plrclrs[4];
	
	if (encdata->hasclrkey)
		clrkey = pack_rgb(encdata->clrkey_r, encdata->clrkey_g, encdata->clrkey_b);
	if (encdata->hasshadow)
		shadowclr = pack_rgb(encdata->shadow_r, encdata->shadow_g, encdata->shadow_b);
	if (encdata->hasplrclrs) {
		hasplrclrs = 4;
		for(int i = 0; i < 4; i++)
			plrclrs[i] = pack_rgb(encdata->plrclr_r[i], encdata->plrclr_g[i], encdata->plrclr_b[i]);
	}
	
	// Ready to encode	
   ushort* data = 0;
	uint in, out;
	int runstart = -1; // code field for normal run
	uint npix = pic->get_w()*pic->get_h();
	ushort *pixels = pic->get_pixels();
	
	try
	{
		data = (ushort*) malloc(pic->get_w()*pic->get_h()*sizeof(ushort)*2); 
		
		in = 0;
		out = 0;
		while(in < npix)
		{
			uint count;
	
			// Deal with transparency
			if (encdata->hasclrkey && pixels[in] == clrkey) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == clrkey) {
					count++;
					in++;
				}
				
				data[out++] = (1<<14) | count;
				continue;
			}
			
			// Deal with shadow
			if (encdata->hasshadow && pixels[in] == shadowclr) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == shadowclr) {
					count++;
					in++;
				}
				
				data[out++] = (3<<14) | count;
				continue;
   		}
			
			// Check if it's a player color
			if (hasplrclrs) {
				int idx;
				
	         for(idx = 0; idx < hasplrclrs; idx++) {
            	if(pixels[in] == plrclrs[idx])
						break;
				}
				
				if (idx < hasplrclrs) {
					if (runstart >= 0) { // finish normal run
						data[runstart] = out-runstart-1;
						runstart = -1;
					}
					
					count = 1;
					in++;
					while(in < npix && pixels[in] == shadowclr) {
						count++;
						in++;
					}
					
					data[out++] = (2<<14) | count;
					data[out++] = idx;
					continue;
            }
         }
			
			// Normal run of pixels
			if (runstart < 0)
				runstart = out++;
			
			data[out++] = pixels[in++];
      }
   
		if (runstart >= 0) { // finish normal run
			data[runstart] = out-runstart-1;
			runstart = -1;
		}
		
		// Store the frame
		add_pic(out*sizeof(short), data);
	} catch(...) {
		if (data)
			free(data);
		throw;
	}
	
	free(data);
}

//
// this is a function which reads a animation from a file
//
int Animation::read(FileRead* f)
{
   ushort np;
   np = f->Unsigned16();

   uint i;
   
   ushort size;
   for(i=0; i<np; i++) {
      size = f->Unsigned16();
      
      ushort *ptr = (ushort*)f->Data(size);
      add_pic(size, ptr);
   }
  
   return RET_OK;
}

/*
===============
Animation::parse

Read an animation which sits in the given directory.
The animation is described by the given section.

This function looks for pictures in this order:
	picnametempl, if not null
	key 'pics', if present
	<sectionname>_??.bmp
===============
*/
void Animation::parse(const char *directory, Section *s, const char *picnametempl, const EncodeData *encdefaults)
{
	char templbuf[256]; // used when picnametempl == 0
	char pictempl[256];

	if (!picnametempl) {
		picnametempl = s->get_string("pics");
		if (!picnametempl) {
			snprintf(templbuf, sizeof(templbuf), "%s_??.bmp", s->get_name());
			picnametempl = templbuf;
		}
	}
	
	snprintf(pictempl, sizeof(pictempl), "%s/%s", directory, picnametempl);

	// Get colorkey, shadow color and hotspot
	EncodeData encdata;
	
	encdata.clear();
	
	if (encdefaults)
		encdata.add(encdefaults);
	
	encdata.parse(s);

	// Get animation speed
	int fps = s->get_int("fps");
	if (fps > 0)
		m_frametime = 1000 / fps;
	
	// TODO: Frames of varying size / hotspot?
	hsx = s->get_int("hot_spot_x", 0);
	hsy = s->get_int("hot_spot_y", 0);
	
	// Read frames in one by one
	for(;;) {
		char fname[256];
		int nr = npics;
		char *p;
		
		// create the file name by reverse-scanning for '?' and replacing
		strcpy(fname, pictempl);
		p = fname + strlen(fname);
		while(p > fname) {
			if (*--p != '?')
				continue;
			
			*p = '0' + (nr % 10);
			nr = nr / 10;
		}
		
		if (nr) // cycled up to maximum possible frame number
			break;
		
		// is the frame actually there?
		if (!g_fs->FileExists(fname))
			break;
	
		Pic* pic = new Pic();
		if (pic->load(fname)) {
			delete pic;
			break;
		}

		if (!npics) {
			w = pic->get_w();
			h = pic->get_h();
		}
		
		if(pic->get_w() != w && pic->get_h() != h) {
			delete pic;
			throw wexception("%s: frame must be %ix%i pixels big", fname, w, h);
		}
		
		try {
			add_pic(pic, &encdata);
		} catch(...) {
			delete pic;
			throw;
		}
		
		delete pic;
	}
	
	if (!npics)
		throw wexception("Animation %s has no frames", pictempl);
}

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
void Bob_Descr::read(const char *directory, Section *s)
{
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
   anim.parse(directory, s, picname);
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
anims is an array of 6 animations, one for each direction.
The order is the canonical NE, E, SE, SW, W, NW (order of the enum)

Returns false if no path could be found.

The task finishes once the goal has been reached. It may fail.
===============
*/
bool Bob::start_task_movepath(Game* g, Coords dest, int persist, Animation **anims)
{
	task.movepath.path = new Path;

	if (g->get_map()->findpath(m_position, dest, get_movecaps(), persist, task.movepath.path) < 0) {
		delete task.movepath.path;
		return false;
	}
	
	task.movepath.step = 0;
	memcpy(task.movepath.anims, anims, sizeof(Animation*)*6);
	
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
	int dummyx, dummyy;
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
	
	switch(dir) {
	case IDLE: assert(0); break;
	case WALK_NW: g->get_map()->get_tln(m_position, &newf); break;
	case WALK_NE: g->get_map()->get_trn(m_position, &newf); break;
	case WALK_W: g->get_map()->get_ln(m_position, &newf); break;
	case WALK_E: g->get_map()->get_rn(m_position, &newf); break;
	case WALK_SW: g->get_map()->get_bln(m_position, &newf); break;
	case WALK_SE: g->get_map()->get_brn(m_position, &newf); break;
	}

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

      virtual void read(const char *directory, Section *s);
      Bob *create_object();

      inline bool is_swimming(void) { return swimming; }
      inline Animation* get_walk_ne_anim(void) { return &walk_ne; }
      inline Animation* get_walk_nw_anim(void) { return &walk_nw; }
      inline Animation* get_walk_se_anim(void) { return &walk_se; }
      inline Animation* get_walk_sw_anim(void) { return &walk_sw; }
      inline Animation* get_walk_w_anim(void) { return &walk_w; }
      inline Animation* get_walk_e_anim(void) { return &walk_e; }

   private:
      ushort stock;
      bool swimming;
      Animation walk_ne;
      Animation walk_nw;
      Animation walk_e;
      Animation walk_w;
      Animation walk_se;
      Animation walk_sw;
};

Critter_Bob_Descr::Critter_Bob_Descr(const char *name)
	: Bob_Descr(name)
{
	stock = swimming = 0;
}

void Critter_Bob_Descr::read(const char *directory, Section *s)
{
	Bob_Descr::read(directory, s);

	stock = s->get_int("stock", 0);
   swimming = s->get_bool("swimming", false);

   // read all the other animatins
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_walk_ne_??.bmp", m_name);
	walk_ne.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_e_??.bmp", m_name);
	walk_e.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_se_??.bmp", m_name);
	walk_se.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_sw_??.bmp", m_name);
	walk_sw.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_w_??.bmp", m_name);
	walk_w.parse(directory, s, picname);

	snprintf(picname, sizeof(picname), "%s_walk_nw_??.bmp", m_name);
	walk_nw.parse(directory, s, picname);
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
		// Build the animation list - this needs a better solution (AnimationManager, anyone?)
		Animation *anims[6];
		
		anims[0] = get_descr()->get_walk_ne_anim();
		anims[1] = get_descr()->get_walk_e_anim();
		anims[2] = get_descr()->get_walk_se_anim();
		anims[3] = get_descr()->get_walk_sw_anim();
		anims[4] = get_descr()->get_walk_w_anim();
		anims[5] = get_descr()->get_walk_nw_anim();
		
		// Pick a target at random
		Coords dst;
		
		dst.x = m_position.x + (rand()%5) - 2;
		dst.y = m_position.y + (rand()%5) - 2;
		
		if (start_task_movepath(g, dst, 3, anims))
			return;
	
		start_task_idle(g, get_descr()->get_anim(), 1 + g->logic_rand()%1000);
		return;
	}
	
	// idle for a longer period
	start_task_idle(g, get_descr()->get_anim(), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
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

		bob->read(directory, s);
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
