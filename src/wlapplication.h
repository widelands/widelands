/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#ifndef WLAPPLICATION_H
#define WLAPPLICATION_H

#include "graphic.h"
#include <string>
#include <vector>

/// A macro to make i18n more readable
#define _( str ) WLApplication::translate( str )

// input
struct InputCallback {
	void (*mouse_click)(bool down, int btn, uint btns, int x, int y);
	void (*mouse_move)(uint btns, int x, int y, int xdiff, int ydiff);
	void (*key)(bool down, int code, char c);
};

//TODO: rework this. bitshifting is not beautiful in cross platform systems
enum { // use 1<<MOUSE_xxx for bitmasks
   MOUSE_LEFT = 0,
   MOUSE_MIDDLE,
   MOUSE_RIGHT
};

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

/*
System will be the one place for complete session playback technology.
Using a command line option, all input etc.. that passes through System can be
saved in a file and played back later for intensive and slow profiling and
testing (valgrind comes to mind).
(This is completely independent from recorded games; recorded games consist of
saved player commands and can be recorded and played back from the GUI)

Note/TODO: Graphics are currently not handled by System, and it is non-essential
for playback anyway. Additionally, we'll want several rendering backends
(software and OpenGL).
Maybe the graphics backend loader code should be in System, while the actual
graphics work is done elsewhere.

Mouse: Some mouse functions deal with button mask bits. Bits are simply obtained
as (1 << btnnr), so bitmask 5 = (1<<0)|(1<<2) means: "left and right are pressed"
*/

/**
 * You know main functions, of course. This is the main class.
 *
 * The oversimplified version: everything else is either game logic or GUI
 *
 * WLAppplication bundles all initialization and shutdown code in one neat
 * package. It also includes all (well, most) system abstractions, notably
 * i18n, input handling, timing, low level networking and graphics setup (the
 * actual graphics work is done by \ref class Graphic).
 *
 * Equally important, the main event loop is chugging along in this class.
 *
 * \par WLApplication is a singleton
 *
 * Because of it's special purpose, having more than one WLApplication is
 * useless. So we implement singleton semantics:
 * \li A private(!) class variable (--> unique for the whole program)
 *     \ref the_singleton holds the only instance of WLApplication. It's private
 *     to make sure that \e nobody messes around with this very important thing.
 * \li There is no public constructor. If there was, you'd be able to create
 *     more WLApplications.
 * \li The only way to get at the WLApplication object is to call
 *     \ref WLApplication::get(), which is static as well. Because of this,
 *     \ref get() can access \ref the_singleton even if no WLApplication object
 *     has been instantiated yet.
 *     \ref get() will \e always give you a valid WLApplication. If it doesn't
 *     exist, it will create one.
 * \li There is no need for a destructor. Just make sure you call
 *     \ref shutdown() when you're done.
 *
 * For testing purposes, we can spawn a second thread with widelands running in
 * it (see \ref init_double_game() ). The fact that WLApplication is a singleton
 * is not touched by this: threads start out as a byte exact memory copy, so
 * the two instances can't know (except for fork()'s return value) that they are
 * (or are not) a primary thread. Each WLApplication singleton really *is* a
 * singleton - inside it's process.
 *
 * \par Session recording and playback
 *
 * For debugging purposes, the WLApplication can record (and of course play
 * back) a complete game session. Recording/Playback does \e not work with
 * --double !
 */
class WLApplication {
public:
	static WLApplication * const get(const int argc=0, const char **argv=0);

	void run();
	const bool init();
	void shutdown();

	// i18n
	//@{
static const char* translate( const char* str ){return gettext( str );}
	void grab_textdomain( const char* );
	void release_textdomain();
	void set_locale( const char* = 0);
	const std::string get_locale() {return m_locale;}
	//@}

	// Record/Playback file handling
	//@{
	const bool get_playback() {return m_playback;}
	const long int get_playback_offset();
	const bool get_record() {return m_record;}
	FILE *get_rec_file() {return m_frecord;}
	FILE *get_play_file() {return m_fplayback;}
	//@}

	/**
	 * Returns true if an external entity wants us to quit
	 */
	const bool should_die() {return m_should_die;}
	const int get_time();

	/**
	 * Get the state of the current KeyBoard Button
	 * \warning This function doesn't check for dumbness
	 */
	const bool get_key_state(const int key) {return SDL_GetKeyState(0)[key];}

	// Mouse related properties
	//@{
	void set_mouse_pos(const int x, const int y);
	/// Which mouse buttons are or are not pressed?
	const uint get_mouse_buttons() {return m_mouse_buttons;}
	/// The current X coordinate
	const int get_mouse_x() {return m_mouse_x;}
	/// The current Y coordinate
	const int get_mouse_y() {return m_mouse_y;}

	void set_input_grab(const bool grab);
	/// Swap left and right mouse key?
	void set_mouse_swap(const bool swap) {m_mouse_swapped = swap;}
	void set_mouse_speed(const float speed);

	/**
	 * Lock the mouse cursor into place (e.g., for scrolling the map)
	 */
	void set_mouse_lock(const bool locked) {m_mouse_locked = locked;}
	void set_max_mouse_coords(const int x, const int y);
	void do_warp_mouse(const int x, const int y);
	//@}

	void init_graphics(const int w, const int h, const int bpp,
	                   const bool fullscreen);

	void handle_input(const InputCallback *cb);

	/**
	 * Menus
	 */
	void mainmenu();
	void mainmenu_singleplayer();
	void mainmenu_multiplayer();

	//to be removed
	//@{
	void WLApplication::init_double_game();
	static void WLApplication::signal_handler (int sig);
	static void WLApplication::quit_handler();
	static void WLApplication::yield_double_game();
	//@}

	//@{
	static int pid_me;
	static int pid_peer;
	static volatile int may_run;
	//@}

protected:
	WLApplication(const int argc, const char **argv);

	const bool poll_event(SDL_Event *ev, const bool throttle);

	const bool init_settings();
	void shutdown_settings();

	const bool init_hardware();
	void shutdown_hardware();

	const bool init_recordplaybackfile();
	void shutdown_recordplaybackfile();

	const bool parse_command_line();
	void show_usage();

	/// The commandline, conveniently repackaged
	///\todo make this a vector<string>
	//@{
	const int m_argc;
	const char **m_argv;
	//@}

	/// i18n
	//@{
	std::string m_locale;
	std::vector<std::string> m_textdomains;
	//@}

	/// Journal files
	//@{
	bool m_playback, m_record;
	char m_recordname[256], m_playbackname[256];
	FILE *m_frecord, *m_fplayback;
	//@}

	/// Mouse handling
	//@{
	bool		m_input_grab;		// config options
	bool		m_mouse_swapped;
	float		m_mouse_speed;

	uint		m_mouse_buttons;
	int		m_mouse_x;			// mouse position seen by the outside
	int		m_mouse_y;
	int		m_mouse_maxx;
	int		m_mouse_maxy;
	bool		m_mouse_locked;

	float		m_mouse_internal_x;		// internal state (only used in non-playback)
	float		m_mouse_internal_y;
	int		m_mouse_internal_compx;
	int		m_mouse_internal_compy;
	//@}

	bool		m_sdl_active;
	bool		m_should_die;

	// Graphics
	int		m_gfx_w;
	int		m_gfx_h;
	bool		m_gfx_fullscreen;

private:
	static WLApplication *the_singleton;
};

#endif
