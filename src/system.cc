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

#include <SDL.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <vector>
#include "error.h"
#include "filesystem.h"
#include "machdep.h"
#include "profile.h"
#include "system.h"
#include "wexception.h"
#include "font_handler.h"
#include "sound_handler.h"

#include "constants.h"
#include "network_ggz.h"

#include "wlapplication.h"

static std::vector<std::string> l_textdomains;

Graphic *g_gr;

extern int get_playback_offset();
extern void write_record_char(char v);
extern char read_record_char();
extern void write_record_int(int v);
extern int read_record_int();
extern void write_record_code(uchar code);
extern void read_record_code(uchar code);

struct SYS sys;

/*
 * Localisation functions
 */

/*
 * Grab a given TextDomain. We keep a stack
 * if a new one is grabbed, it is pushed on the stack
 * on releasing it is dropped and the previous
 * one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs it's textdomain
 * loads all data and releases it -> we're back in
 * widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void Sys_GrabTextdomain( const char* domain) {
	bind_textdomain_codeset (domain, "UTF-8");
	bindtextdomain( domain, LOCALE_PATH );
	textdomain(domain);

	l_textdomains.push_back( domain );
}

void Sys_ReleaseTextdomain( void ) {
	l_textdomains.pop_back();

	if (l_textdomains.size()>0) { //don't try to get the previous TD when the very first one ('widelands') just got dropped
		const char* domain = l_textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain( domain, LOCALE_PATH );
		textdomain(domain);
	}
}

/*
 * Set The locale to the given string
 */
void Sys_SetLocale( const char* str ) {
	if( !str )
		str = "";

	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG", str, 1);
	setenv ("LC_ALL", str, 1);
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", str, 1);
	setenv ("LC_ALL", str, 1);
#endif

#ifdef _WIN32
	const std::string env = std::string("LANG=") + str;
	putenv(env.c_str());
#endif

	setlocale(LC_ALL, str);
	sys.locale=str;

	if( l_textdomains.size() ) {
		const char* domain = l_textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain( domain, LOCALE_PATH );
		textdomain(domain);
	}
}

/*
 * Get the current locale
 */
std::string Sys_GetLocale()
{
	return sys.locale;
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

	if (g_app->get_playback()) {
		read_record_code(RFC_GETTIME);
		time = read_record_int();
	} else
		time = SDL_GetTicks();

	if (g_app->get_record()) {
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

	if (g_app->get_playback()) // don't warp anything during playback
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
	if (g_app->get_playback())
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
			case SDL_USEREVENT:
				if (ev->user.code==Sound_Handler::SOUND_HANDLER_CHANGE_MUSIC)
					g_sound_handler.change_music();

				break;
			}
		}
	}

	if (g_app->get_record())
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
			if (throttle && !g_app->get_playback())
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

	NetGGZ::ref()->data();
	NetGGZ::ref()->datacore();

	// We need to empty the SDL message queue always, even in playback mode
	// In playback mode, only F10 for premature exiting works
	if (g_app->get_playback()) {
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
Get the state of the current KeyBoard Button

warning: this function doesn't check for dumbness
===============
*/
bool Sys_GetKeyState(uint key) {
	return SDL_GetKeyState(0)[key];
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
	if (g_app->get_playback())
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
Graphic* SW16_CreateGraphics(int w, int h, int bpp, bool fullscreen);
void Sys_InitGraphics(int w, int h, int bpp, bool fullscreen)
{
	if (w == sys.gfx_w && h == sys.gfx_h && fullscreen == sys.gfx_fullscreen)
		return;

	if (g_gr)
	{
		delete g_gr;
		g_gr = 0;
	}

	sys.gfx_w = w;
	sys.gfx_h = h;
	sys.gfx_fullscreen = fullscreen;

	// If we are not to be shut down
	if( w && h ) {
		g_gr = SW16_CreateGraphics(w, h, bpp, fullscreen);
		Sys_SetMaxMouseCoords(w, h);
	}
}


#ifdef DEBUG
#ifndef __WIN32__
#include <signal.h>

static int pid_me=0, pid_peer=0;

static volatile int may_run=0;

static void signal_handler (int sig)
{
	may_run++;
}

static void quit_handler ()
{
	kill (pid_peer, SIGTERM);
	sleep (2);
	kill (pid_peer, SIGKILL);
}

void yield_double_game ()
{
	if (pid_me==0)
		return;

	if (may_run>0) {
		may_run--;
		kill (pid_peer, SIGUSR1);
	}

	if (may_run==0)
		usleep (500000);

	// using sleep instead of pause avoids a race condition
	// and a deadlock during connect
}

void init_double_game ()
{
	if (pid_me!=0)
		return;

	pid_me=getpid();
	pid_peer=fork();

	assert (pid_peer>=0);

	if (pid_peer==0) {
		pid_peer=pid_me;
		pid_me=getpid();

		may_run=1;
	}

	signal (SIGUSR1, signal_handler);

	atexit (quit_handler);
}
#endif // WIN32
#endif // DEBUG

