/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include <SDL.h>

#include "options.h"
#include "graphic.h"


Graphic *g_gr = 0;


/*
Notes on the implementation
---------------------------

Mouse:
When in GrabInput mode (default), the system and SDL mouse cursor is not
connected to the internal mouse position. We rely on SDL to provide the correct
relative movement information even when the mouse cursor is close to the window
border. We don't not use the absolute mouse position provided by SDL at all.
The internal mouse position is kept with sub-pixel accuracy to make mouse speed
work.

When GrabInput mode is off
*/

static struct {
	bool		active;
	bool		should_die;

	FILE		*frecord;
	FILE		*fplayback;

	// Input
	bool		input_grab;		// config options
	bool		mouse_swapped;
	float		mouse_speed;

	uint		mouse_buttons;
	int		mouse_x;			// mouse position seen by the outside
	int		mouse_y;
	int		mouse_maxx;
	int		mouse_maxy;
	bool		mouse_locked;

	float		mouse_internal_x;		// internal state (only used in non-playback)
	float		mouse_internal_y;
	int		mouse_internal_compx;
	int		mouse_internal_compy;

	// Graphics
	int		gfx_system;
	int		gfx_w;
	int		gfx_h;
	bool		gfx_fullscreen;
} sys;

static char sys_recordname[256] = "";
static char sys_playbackname[256] = "";

/*
Record file codes

It should be possible to use record files across different platforms.
However, 64 bit platforms are currently not supported.
*/
#define RFC_MAGIC		0x0ACAD100 // change this and I will ensure your death will be a most unpleasant one

enum {
	RFC_GETTIME = 0x01,
	RFC_EVENT = 0x02,
	RFC_ENDEVENTS = 0x03,
};

enum {
	RFC_KEYDOWN = 0x10,
	RFC_KEYUP = 0x11,
	RFC_MOUSEBUTTONDOWN = 0x12,
	RFC_MOUSEBUTTONUP = 0x13,
	RFC_MOUSEMOTION = 0x14,
	RFC_QUIT = 0x15
};


/*
===============
get_playback_offset

Returns the position in the playback file
===============
*/
static int get_playback_offset()
{
	assert(sys.fplayback);

	return ftell(sys.fplayback);
}


/*
===============
write_record_char
read_record_char
write_record_int
read_record_int
write_record_code
read_record_code

Simple wrapper functions to make stdio file access less painful
===============
*/
static void write_record_char(char v)
{
	assert(sys.frecord);

	if (fwrite(&v, sizeof(v), 1, sys.frecord) != 1)
		throw wexception("Write of 1 byte to record failed.");
	fflush(sys.frecord);
}

static char read_record_char()
{
	char v;

	assert(sys.fplayback);

	if (fread(&v, sizeof(v), 1, sys.fplayback) != 1)
		throw wexception("Read of 1 byte from record failed.");

	return v;
}

static void write_record_int(int v)
{
	assert(sys.frecord);

	v = Little32(v);
	if (fwrite(&v, sizeof(v), 1, sys.frecord) != 1)
		throw wexception("Write of 4 bytes to record failed.");
	fflush(sys.frecord);
}

static int read_record_int()
{
	int v;

	assert(sys.fplayback);

	if (fread(&v, sizeof(v), 1, sys.fplayback) != 1)
		throw wexception("Read of 4 bytes from record failed.");

	return Little32(v);
}

static void write_record_code(uchar code)
{
	write_record_char(code);
}

static void read_record_code(uchar code)
{
	uchar filecode;

	filecode = read_record_char();

	if (filecode != code)
		throw wexception("%08X: Bad code %02X during playback (%02X expected). Mismatching executable versions?",
						get_playback_offset()-1, filecode, code);
}


/*
===============
Sys_Init

Initialize lower level libraries (i.e. SDL)
===============
*/
void Sys_Init()
{
	sys.should_die = false;
	sys.frecord = 0;
	sys.fplayback = 0;

	try
	{
		// Open record file if necessary
		if (sys_recordname[0]) {
			sys.frecord = fopen(sys_recordname, "wb");
			if (!sys.frecord)
				throw wexception("Failed to open record file %s", sys_recordname);

			write_record_int(RFC_MAGIC);
		}

		if (sys_playbackname[0]) {
			sys.fplayback = fopen(sys_playbackname, "rb");
			if (!sys.fplayback)
				throw wexception("Failed to open playback file %s", sys_playbackname);

			if (read_record_int() != RFC_MAGIC)
				throw wexception("Playback file has wrong magic number");
		}

		// Input
		sys.input_grab = false;
		sys.mouse_swapped = false;
		sys.mouse_locked = false;
		sys.mouse_speed = 1.0;
		sys.mouse_buttons = 0;
		sys.mouse_x = sys.mouse_y = 0;
		sys.mouse_maxx = sys.mouse_maxy = 0;
		sys.mouse_internal_x = sys.mouse_internal_y = 0;
		sys.mouse_internal_compx = sys.mouse_internal_compy = 0;

		Section *s = g_options.pull_section("global");

		Sys_SetMouseSwap(s->get_bool("swapmouse", false));
		Sys_SetMouseSpeed(s->get_float("mousespeed", 1.0));

      if(s->get_bool("coredump", false)) {
         if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0)
            throw wexception("Failed to initialize SDL: %s", SDL_GetError());
      } else {
         if (SDL_Init(SDL_INIT_VIDEO) < 0)
            throw wexception("Failed to initialize SDL: %s", SDL_GetError());
      }
      sys.active = true;

		SDL_ShowCursor(SDL_DISABLE);
		Sys_SetInputGrab(s->get_bool("inputgrab", true));
		SDL_EnableUNICODE(1); // useful for e.g. chat messages
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

		// Graphics
		sys.gfx_system = GFXSYS_NONE;
	}
	catch(...) {
		if (sys.active)
			SDL_Quit();
		if (sys.frecord)
			fclose(sys.frecord);
		if (sys.fplayback)
			fclose(sys.fplayback);
		sys.active = false;
	}
}

/*
===============
Sys_Shutdown

Shutdown the system
===============
*/
void Sys_Shutdown()
{
	if (g_gr)
		{
		log("Sys_Shutdown: graphics system not shut down\n");
		Sys_InitGraphics(GFXSYS_NONE, 0, 0, false);
		}

	SDL_Quit();
	sys.active = false;

	if (sys.frecord) {
		fclose(sys.frecord);
		sys.frecord = 0;
	}
	if (sys.fplayback) {
		fclose(sys.fplayback);
		sys.fplayback = 0;
	}
}

/*
===============
Sys_SetRecordFile
Sys_SetPlaybackFile

Set the file to record to or playback from. Must be called before Sys_Init().
The files do not go through FileSystem but are accessed through stdio functions.
In theory it should be possible to record and playback at the same time, though
I don't see why you would want to do that.
===============
*/
void Sys_SetRecordFile(const char *filename)
{
	snprintf(sys_recordname, sizeof(sys_recordname), "%s", filename);
}

void Sys_SetPlaybackFile(const char *filename)
{
	snprintf(sys_playbackname, sizeof(sys_playbackname), "%s", filename);
}

/*
===============
Sys_ShouldDie

Returns true if an external entity wants us to quit
===============
*/
bool Sys_ShouldDie()
{
	return sys.should_die;
}

/*
===============
Sys_GetTime

Return the current time, in milliseconds
===============
*/
int Sys_GetTime()
{
	int time;

	if (sys.fplayback) {
		read_record_code(RFC_GETTIME);
		time = read_record_int();
	} else
		time = SDL_GetTicks();

	if (sys.frecord) {
		write_record_code(RFC_GETTIME);
		write_record_int(time);
	}

	return time;
}


/*
===============
Sys_DoWarpMouse

Warp the SDL mouse cursor to the given position.
Store the delta in sys.mouse_internal_compx/y, so that the resulting motion
event can be eliminated.
===============
*/
static void Sys_DoWarpMouse(int x, int y)
{
	int curx, cury;

	if (sys.fplayback) // don't warp anything during playback
		return;

	SDL_GetMouseState(&curx, &cury);

	if (curx == x && cury == y)
		return;

	sys.mouse_internal_compx += curx - x;
	sys.mouse_internal_compy += cury - y;

	SDL_WarpMouse(x, y);
}


/*
===============
Sys_PollEvent

Get an event from the SDL queue, just like SDL_PollEvent.
Perform the meat of playback/record stuff when needed.

throttle is a hack to stop record files from getting extremely huge.
If it is set to true, we will idle loop if we can't get an SDL_Event
returned immediately if we're recording. If there is no user input,
the actual mainloop will be throttled to 100fps.
===============
*/
bool Sys_PollEvent(SDL_Event *ev, bool throttle)
{
	bool haveevent;

restart:
	if (sys.fplayback)
	{
		uchar code = read_record_char();

		if (code == RFC_EVENT)
		{
			code = read_record_char();

			switch(code) {
			case RFC_KEYDOWN:
			case RFC_KEYUP:
				ev->type = (code == RFC_KEYUP) ? SDL_KEYUP : SDL_KEYDOWN;
				ev->key.keysym.sym = (SDLKey)read_record_int();
				ev->key.keysym.unicode = read_record_int();
				break;

			case RFC_MOUSEBUTTONDOWN:
			case RFC_MOUSEBUTTONUP:
				ev->type = (code == RFC_MOUSEBUTTONUP) ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
				ev->button.button = read_record_char();
				break;

			case RFC_MOUSEMOTION:
				ev->type = SDL_MOUSEMOTION;
				ev->motion.x = read_record_int();
				ev->motion.y = read_record_int();
				ev->motion.xrel = read_record_int();
				ev->motion.yrel = read_record_int();
				break;

			case RFC_QUIT:
				ev->type = SDL_QUIT;
				break;

			default:
				throw wexception("%08X: Unknown event type %02X in playback.", get_playback_offset()-1, code);
			}

			haveevent = true;
		}
		else if (code == RFC_ENDEVENTS)
		{
			haveevent = false;
		}
		else
			throw wexception("%08X: Bad code %02X in event playback.", get_playback_offset()-1, code);
	}
	else
	{
		haveevent = SDL_PollEvent(ev);

		if (haveevent)
		{
			// We edit mouse motion events in here, so that differences caused by
			// GrabInput or mouse speed settings are invisible to the rest of the code
			switch(ev->type) {
			case SDL_MOUSEMOTION:
				ev->motion.xrel += sys.mouse_internal_compx;
				ev->motion.yrel += sys.mouse_internal_compy;
				sys.mouse_internal_compx = sys.mouse_internal_compy = 0;

				if (sys.input_grab)
				{
					float xlast = sys.mouse_internal_x;
					float ylast = sys.mouse_internal_y;

					sys.mouse_internal_x += ev->motion.xrel * sys.mouse_speed;
					sys.mouse_internal_y += ev->motion.yrel * sys.mouse_speed;

					ev->motion.xrel = (int)sys.mouse_internal_x - (int)xlast;
					ev->motion.yrel = (int)sys.mouse_internal_y - (int)ylast;

					if (sys.mouse_locked)
					{
						// mouse is locked; so don't move the cursor
						sys.mouse_internal_x = xlast;
						sys.mouse_internal_y = ylast;
					}
					else
					{
						if (sys.mouse_internal_x < 0)
							sys.mouse_internal_x = 0;
						else if (sys.mouse_internal_x >= sys.mouse_maxx-1)
							sys.mouse_internal_x = sys.mouse_maxx-1;
						if (sys.mouse_internal_y < 0)
							sys.mouse_internal_y = 0;
						else if (sys.mouse_internal_y >= sys.mouse_maxy-1)
							sys.mouse_internal_y = sys.mouse_maxy-1;
					}

					ev->motion.x = (int)sys.mouse_internal_x;
					ev->motion.y = (int)sys.mouse_internal_y;
				}
				else
				{
					int xlast = sys.mouse_x;
					int ylast = sys.mouse_y;

					if (sys.mouse_locked)
					{
						Sys_SetMousePos(xlast, ylast);

						ev->motion.x = xlast;
						ev->motion.y = ylast;
					}
				}

				break;
			}
		}
	}

	if (sys.frecord)
	{
		if (haveevent)
		{
			switch(ev->type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				write_record_char(RFC_EVENT);
				write_record_char((ev->type == SDL_KEYUP) ? RFC_KEYUP : RFC_KEYDOWN);
				write_record_int(ev->key.keysym.sym);
				write_record_int(ev->key.keysym.unicode);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				write_record_char(RFC_EVENT);
				write_record_char((ev->type == SDL_MOUSEBUTTONUP) ? RFC_MOUSEBUTTONUP : RFC_MOUSEBUTTONDOWN);
				write_record_char(ev->button.button);
				break;

			case SDL_MOUSEMOTION:
				write_record_char(RFC_EVENT);
				write_record_char(RFC_MOUSEMOTION);
				write_record_int(ev->motion.x);
				write_record_int(ev->motion.y);
				write_record_int(ev->motion.xrel);
				write_record_int(ev->motion.yrel);
				break;

			case SDL_QUIT:
				write_record_char(RFC_EVENT);
				write_record_char(RFC_QUIT);
				break;

			default:
				goto restart; // can't really do anything useful with this command
			}
		}
		else
		{
			// Implement the throttle to avoid very quick inner mainloops when
			// recoding a session
			if (throttle && !sys.fplayback)
			{
				static int lastthrottle = 0;
				int time = SDL_GetTicks();

				if (time - lastthrottle < 10)
					goto restart;

				lastthrottle = time;
			}

			write_record_char(RFC_ENDEVENTS);
		}
	}
	else
	{
		if (haveevent)
		{
			// Eliminate any unhandled events to make sure record and playback are
			// _really_ the same.
			// Yes I know, it's overly paranoid but hey...
			switch(ev->type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_QUIT:
				break;

			default:
				goto restart;
			}
		}
	}

	return haveevent;
}


/*
===============
Sys_HandleInput

Run the event queue, get packets from the network, etc...
===============
*/
void Sys_HandleInput(InputCallback *cb)
{
	bool gotevents = false;
	SDL_Event ev;

	// We need to empty the SDL message queue always, even in playback mode
	// In playback mode, only F10 for premature exiting works
	if (sys.fplayback) {
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_F10) // TEMP - get out of here quick
					sys.should_die = true;
				break;

			case SDL_QUIT:
				sys.should_die = true;
				break;
			}
		}
	}

	// Usual event queue
	while(Sys_PollEvent(&ev, !gotevents)) {
		int button;

		gotevents = true;

		// CAREFUL: Record files do not save the entire SDL_Event structure.
		// Therefore, playbacks are incomplete. When you change the following
		// code so that it uses previously unused fields in SDL_Event,
		// please also take a look at Sys_PollEvent()

		switch(ev.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (ev.key.keysym.sym == SDLK_F10) // TEMP - get out of here quick
			{
				if (ev.type == SDL_KEYDOWN)
					sys.should_die = true;
				break;
			}
			if (ev.key.keysym.sym == SDLK_F11) // take screenshot
			{
				if (ev.type == SDL_KEYDOWN) {
					char buf[256];
					int nr;

					for(nr = 0; nr < 10000; nr++) {
						snprintf(buf, sizeof(buf), "shot%04i.bmp", nr);
						if (g_fs->FileExists(buf))
							continue;
						g_gr->screenshot(buf);
						break;
					}
				}
				break;
			}

			if (cb && cb->key) {
				int c;

				c = ev.key.keysym.unicode;
				if (c < 32 || c >= 128)
					c = 0;

				cb->key(ev.type == SDL_KEYDOWN, ev.key.keysym.sym, (char)c);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			button = ev.button.button-1;
			if (sys.mouse_swapped) {
				if (button == MOUSE_LEFT) button = MOUSE_RIGHT;
				else if (button == MOUSE_RIGHT) button = MOUSE_LEFT;
			}

			if (ev.type == SDL_MOUSEBUTTONDOWN)
				sys.mouse_buttons |= 1 << button;
			else
				sys.mouse_buttons &= ~(1 << button);

			if (cb && cb->mouse_click)
				cb->mouse_click(ev.type == SDL_MOUSEBUTTONDOWN, button, sys.mouse_buttons,
				                (int)sys.mouse_x, (int)sys.mouse_y);
			break;

		case SDL_MOUSEMOTION: {
			// All the interesting stuff is now in Sys_PollEvent()
			int xdiff = ev.motion.xrel;
			int ydiff = ev.motion.yrel;

			sys.mouse_x = ev.motion.x;
			sys.mouse_y = ev.motion.y;

			if (!xdiff && !ydiff)
				break;

			if (cb && cb->mouse_move)
				cb->mouse_move(sys.mouse_buttons, sys.mouse_x, sys.mouse_y, xdiff, ydiff);

			break;
		}

		case SDL_QUIT:
			sys.should_die = true;
			break;

		default:
			break;
		}
	}
}

/*
===============
Sys_GetMouseButtons
Sys_GetMouseX
Sys_GetMouseY
===============
*/
uint Sys_GetMouseButtons()
{
	return sys.mouse_buttons;
}

int Sys_GetMouseX()
{
	return (int)sys.mouse_x;
}

int Sys_GetMouseY()
{
	return (int)sys.mouse_y;
}


/*
===============
Sys_SetMousePos

Move the mouse cursor.
No mouse moved event will be issued.
===============
*/
void Sys_SetMousePos(int x, int y)
{
	sys.mouse_x = x;
	sys.mouse_y = y;
	sys.mouse_internal_x = x;
	sys.mouse_internal_y = y;

	if (!sys.input_grab)
		Sys_DoWarpMouse(x, y); // sync mouse positions
}


/*
===============
Sys_SetInputGrab

Changes input grab mode.
===============
*/
void Sys_SetInputGrab(bool grab)
{
	if (sys.fplayback)
		return; // ignore in playback mode

	sys.input_grab = grab;

	if (grab)
	{
		SDL_WM_GrabInput(SDL_GRAB_ON);

		sys.mouse_internal_x = sys.mouse_x;
		sys.mouse_internal_y = sys.mouse_y;
	}
	else
	{
		SDL_WM_GrabInput(SDL_GRAB_OFF);

		Sys_DoWarpMouse(sys.mouse_x, sys.mouse_y);
	}
}


/*
===============
Sys_SetMouseSwap
===============
*/
void Sys_SetMouseSwap(bool swap)
{
	sys.mouse_swapped = swap;
}


/*
===============
Sys_SetMouseSpeed
===============
*/
void Sys_SetMouseSpeed(float speed)
{
	if (speed <= 0.1 || speed >= 10.0)
		speed = 1.0;
	sys.mouse_speed = speed;
}

/*
===============
Sys_SetMaxMouseCoords [kludge]

Set the mouse boundary after a change of resolution
This is manually imported by graphic.cc
===============
*/
void Sys_SetMaxMouseCoords(int x, int y)
{
	sys.mouse_maxx = x;
	sys.mouse_maxy = y;
}


/*
===============
Sys_MouseLock

Lock the mouse cursor into place (e.g., for scrolling the map)
===============
*/
void Sys_MouseLock(bool locked)
{
	sys.mouse_locked = locked;
}


/*
===============
Sys_InitGraphics

Initialize the graphics subsystem (or shutdown, if system == GFXSYS_NONE) with
the given resolution.
Throws an exception on failure.

Note: Because of the way pictures are handled now, this function must not be
      called while UI elements are active.
===============
*/
Graphic* SW16_CreateGraphics(int w, int h, bool fullscreen);
void Font_ReloadAll();

void Sys_InitGraphics(int system, int w, int h, bool fullscreen)
{
	if (system == sys.gfx_system && w == sys.gfx_w && h == sys.gfx_h && fullscreen == sys.gfx_fullscreen)
		return;

	if (g_gr)
		{
		delete g_gr;
		g_gr = 0;
		}
	
	sys.gfx_system = system;
	sys.gfx_w = w;
	sys.gfx_h = h;
	sys.gfx_fullscreen = fullscreen;

	switch(system)
		{
		case GFXSYS_SW16:
			g_gr = SW16_CreateGraphics(w, h, fullscreen);
			break;
		}

	Sys_SetMaxMouseCoords(w, h);

	// Fonts need to reload images...
	if (g_gr)
		Font_ReloadAll();
}
