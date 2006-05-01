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

#include "game.h"
#include "graphic.h"
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// input
struct InputCallback {
	void (*mouse_click)(bool down, int btn, uint btns, int x, int y);
	void (*mouse_move)(uint btns, int x, int y, int xdiff, int ydiff);
	void (*key)(bool down, int code, char c);
};

///the Graphic "singleton". \todo make into a real singleton
extern Graphic *g_gr;

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
 * Equally important, the main event loop is chugging along in this class. [not
 * yet but soon #fweber4mar2006]
 *
 * \par WLApplication is a singleton
 *
 * Because of it's special purpose, having more than one WLApplication is
 * useless. So we implement singleton semantics:
 * \li A private(!) static class variable (--> unique for the whole program,
 *     although nobody can get at it) \ref the_singleton holds a pointer to the
 *     only instance of WLApplication. It's private because it wouldn't be a
 *     class variable otherwise.
 * \li There is no public constructor. If there was, you'd be able to create
 *     more WLApplications. So constructor access must be encapsulated too.
 * \li The only way to get at the WLApplication object is to call
 *     \ref WLApplication::get(), which is static as well. Because of this,
 *     \ref get() can access \ref the_singleton even if no WLApplication object
 *     has been instantiated yet.
 *     \ref get() will \e always give you a valid WLApplication. If one doesn't
 *     exist yet, it will be created.
 * \li A destructor does not make sense. Just make sure you call \ref shutdown()
 *     when you're done - in a sense, it's a destructor without the destruction
 *     part ;-)
 *
 * These measures \e guarantee that there are no stray WLApplication objects
 * floating around by accident.
 *
 * For testing purposes, we can spawn a second thread with widelands running in
 * it (see \ref init_double_game() ). The fact that WLApplication is a singleton
 * is not touched by this: threads start out as a byte exact memory copy, so
 * the two instances can't know (except for fork()'s return value) that they are
 * (or are not) a primary thread. Each WLApplication singleton really *is* a
 * singleton - inside it's own process.
 *
 * \par Session recording and playback
 *
 * For debugging, e.g. profiling a real game without incurring the speed dis-
 * advantage while playing, the WLApplication can record (and of course play
 * back) a complete game session. To do so with guaranteed repeatability, every
 * single event - including low level stuff like mouse movement - gets recorded
 * or played back.
 *
 * During playback, external events are ignored to avoid interference with the
 * playback (exception: F10 will cancel the playback immediately)
 *
 * Recording/Playback does not work with --double. It could be made possible
 * but that feature wouldn't be very useful.
 *
 * \par The mouse cursor
 *
 * SDL can handle a mouse cursor on it's own, but only in black'n'white. That's
 * not sufficient for a game.
 *
 * So Widelands must paint it's own cursor (even more so, as we have \e two
 * cursors: the freefloating hand and the cross that moves on nodes) and hide
 * the system cursor. Ordinarily, relative coordinates break down when the
 * cursor leaves the window. This means we have to grab the mouse, then realtive
 * coords are always available.
 *
 * Mouse: Some mouse functions deal with button mask bits. Bits are simply
 * obtained as (1 << btnnr), so bitmask 5 = (1<<0)|(1<<2) means: "left and right
 * are pressed"
 *
 * We also implement different mouse speed settings. To make this work, internal
 * mouse position is kept at subpixel resolution.
 *
 * \todo What happens if a playback is canceled? Does the game continue or quit?
 * \todo Can recording be canceled?
 * \todo Should we allow to trigger recording ingame, starting with a snapshot
 * savegame? Preferrably, the log would be stored inside the savegame. A new
 * user interface for starting / stopping playback may bue useful with this.
 * \todo How about a "pause" button during playback to examine the current game
 * state?
 * \todo Graphics are currently not handled by WLApplication, and it is non-
 * essential for playback anyway. Additionally, we'll want several rendering
 * backends (software and OpenGL). Maybe the graphics backend loader code should
 * be in System, while the actual graphics work is done elsewhere.
 * \todo Mouse handling is not documented yet
 * \todo Refactor the mainloop
 * \todo Sensible use of exceptions (goes for whole game)
 */

class WLApplication {
public:
	static WLApplication * const get(const int argc=0, const char **argv=0);
	~WLApplication();

	void run();

	//@{
	///\return true if the game is being recorded
const bool get_record() {return m_record;}

	///\return true if the currently running game is a playback
	const bool get_playback() {return m_playback;}

	///\return byte offset into the playback file, used with file reading
	const long int get_playback_offset();

	///\return a handle to the recording file
	FILE *get_rec_file() {return m_frecord;}

	///\return a handle to the playback file
	FILE *get_play_file() {return m_fplayback;}
	//@}

	///\return true if an external entity wants us to quit
	const bool should_die() {return m_should_die;}

	const int get_time();

	/// Get the state of the current KeyBoard Button
	/// \warning This function doesn't check for dumbness
	const bool get_key_state(const int key) {return SDL_GetKeyState(0)[key];}

	//@{
	void set_mouse_pos(const int x, const int y);

	/// Which mouse buttons are or are not pressed?
	const uint get_mouse_buttons() {return m_mouse_buttons;}

	/// The mouse's current X coordinate
	const int get_mouse_x() {return m_mouse_x;}

	/// The mouse's current Y coordinate
	const int get_mouse_y() {return m_mouse_y;}

	void set_input_grab(const bool grab);

	/// Swap left and right mouse key?
	void set_mouse_swap(const bool swap) {m_mouse_swapped = swap;}

	void set_mouse_speed(const float speed);

	/// Lock the mouse cursor into place (e.g., for scrolling the map)
	void set_mouse_lock(const bool locked) {m_mouse_locked = locked;}

	void set_max_mouse_coords(const int x, const int y);

	void do_warp_mouse(const int x, const int y);
	//@}

	void init_graphics(const int w, const int h, const int bpp,
	                   const bool fullscreen);

	void handle_input(const InputCallback *cb);

	//@{
	void mainmenu();
	void mainmenu_singleplayer();
	void mainmenu_multiplayer();
	//@}

	//not all of these need to be public, but I consider signal handling
	//a public interface
	//@{
	void init_double_game();
	static void signal_handler (int sig);
	static void quit_handler();
	static void yield_double_game();
	//@}

	// Used for --double
	//@{
	static int pid_me;
	static int pid_peer;
	///\todo Explain me
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

	///The commandline, conveniently repackaged
	std::map<std::string, std::string> m_commandline;

	//@{
	///Whether we are recording
	bool m_playback;

	///Whether we are playing back
	bool m_record;

	///The recording file's name.
	///\note This does \e not go through the layered filesystem!
	char m_recordname[256];

	///The playback file's name.
	///\note This does \e not go through the layered filesystem!
	char m_playbackname[256];

	///The recording file (or NULL)
	FILE *m_frecord;

	///The playback file (or NULL)
	FILE *m_fplayback;
	//@}

	/// Mouse handling
	//@{
	bool		m_input_grab;		// config options

	///True if left and right mouse button should be swapped
	bool		m_mouse_swapped;

	///The (internal) mouse speed
	float		m_mouse_speed;

	///Current state of the mouse buttons
	///\todo Replace by direct calls to SDL functions???
	uint		m_mouse_buttons;
	int             m_mouse_x;              // mouse position seen by the outside
	int		m_mouse_y;

	///Boundary for the internal mouse's movement - identical to m_gfx_w
	///\todo Remove this in favour of m_gfx_w?
	int		m_mouse_maxx;

	///Boundary for the internal mouse's movement - identical to m_gfx_y
	///\todo Remove this in favour of m_gfx_y?
	int		m_mouse_maxy;
	bool		m_mouse_locked;

	///the internal mouse position (sub-pixel accuracy for use with mouse
	///speed)
	float           m_mouse_internal_x;

	///the internal mouse position (sub-pixel accuracy for use with mouse
	///speed)
	float		m_mouse_internal_y;

	int		m_mouse_internal_compx;

	int		m_mouse_internal_compy;
	//@}

	///Whether the SDL has been activated successfully
	///\todo Replace with \ref SDL_WasInit()
	bool		m_sdl_active;

	///true if an external entity wants us to quit
	bool		m_should_die;

	///The Widelands window's width in pixels
	int		m_gfx_w;

	///The Widelands window's height in pixels
	int		m_gfx_h;

	///If true Widelands is (should be, we never know ;-) running
	///in a fullscreen window
	bool		m_gfx_fullscreen;

	///The game that is currently being played (or NULL, if in main menu)
	Game *m_game;

private:
	///Holds this process' one and only instance of WLApplication, if it was
	///created already. NULL otherwise.
	///\note This is private on purpose. Read the class documentation.
	static WLApplication *the_singleton;
};

#endif
