/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef WLAPPLICATION_H
#define WLAPPLICATION_H

//Workaround for bug http://sourceforge.net/p/mingw/bugs/2152/
#ifdef __MINGW32__
#define _USE_32BIT_TIME_T 1
#endif

#include <cstring>
#include <map>
#include <stdexcept>
#include <string>

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_types.h>

#include "point.h"


namespace Widelands {class Game;}

///Thrown if a commandline parameter is faulty
struct Parameter_error : public std::runtime_error {
	explicit Parameter_error() : std::runtime_error("") {}
	explicit Parameter_error(std::string text)
		: std::runtime_error(text)
	{}
	virtual ~Parameter_error() throw () {}
};

// input
struct InputCallback {
	void (*mouse_press)
	(const Uint8 button, // Button number as #defined in SDL_mouse.h.
	 int32_t x, int32_t y);      // The coordinates of the mouse at press time.
	void (*mouse_release)
	(const Uint8 button, // Button number as #defined in SDL_mouse.h.
	 int32_t x, int32_t y);      // The coordinates of the mouse at release time.
	void (*mouse_move)
	(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	void (*key)        (bool down, SDL_keysym code);
};

/// You know main functions, of course. This is the main struct.
///
/// The oversimplified version: everything else is either game logic or GUI.
///
/// WLAppplication bundles all initialization and shutdown code in one neat
/// package. It also includes all (well, most) system abstractions, notably
/// i18n, input handling, timing, low level networking and graphics setup (the
/// actual graphics work is done by Graphic).
///
/// \todo Is the above part about i18n still true? \#bedouin
///
/// Equally important, the main event loop is chugging along in this class.
/// [not yet but some time in the future \#bedouin8sep2007]
///
/// \par WLApplication is a singleton
///
/// Because of it's special purpose, having more than one WLApplication is
/// useless. So we implement singleton semantics:
/// \li A private(!) static class variable (--> unique for the whole program,
///     although nobody can get at it) the_singleton holds a pointer to the
///     only instance of WLApplication. It is private because it would not be a
///     struct variable otherwise.
/// \li There is no public constructor. If there was, you would be able to
///     create more WLApplications. So constructor access must be encapsulated
///     too.
/// \li The only way to get at the WLApplication object is to call
///     WLApplication::get(), which is static as well. Because of this, get()
///     can access the_singleton even if no WLApplication object has been
///     instantiated yet. get() will \e always give you a valid WLApplication.
///     If one does not exist yet, it will be created.
/// \li A destructor does not make sense. Just make sure you call
///     shutdown_settings() and shutdown_hardware() when you are done - in a
///     sense, it is a destructor without the destruction part ;-)
///
/// These measures \e guarantee that there are no stray WLApplication objects
/// floating around by accident.
///
/// For testing purposes, we can spawn a second process with widelands running
/// in it (see init_double_game()). The fact that WLApplication is a singleton
/// is not touched by this: the processes start out as a byte exact memory
/// copy, so the two instances ca not know (except for fork()'s return value)
/// that they are (or are not) a primary thread. Each WLApplication singleton
/// really *is* a singleton - inside it's own process.
///
/// Forking does not work on windows, but nobody cares enough to investigate.
/// It is only a debugging convenience anyway.
///
///
/// \par The mouse cursor
///
/// SDL can handle a mouse cursor on its own, but only in black and white. That
/// is not sufficient.
///
/// So Widelands must paint its own cursor and hide the system cursor.
///
/// Ordinarily, relative coordinates break down when the cursor leaves the
/// window. This means we have to grab the mouse, then relative coords are
/// always available.
/// \todo Actually do grab the mouse when it is locked
///
/// \todo Graphics are currently not handled by WLApplication, and it is
/// non essential for playback anyway. Additionally, we will want several
/// rendering backends (software and OpenGL). Maybe the graphics backend loader
/// code should be in System, while the actual graphics work is done elsewhere.
/// \todo Refactor the mainloop
/// \todo Sensible use of exceptions (goes for whole game)
struct WLApplication {
	static WLApplication * get(int const argc = 0, char const * * argv = nullptr);
	~WLApplication();

	enum GameType {NONE, EDITOR, REPLAY, SCENARIO, LOADGAME, NETWORK, INTERNET};

	void run();

	/// \warning true if an external entity wants us to quit
	bool should_die() const {return m_should_die;}

	int32_t get_time();

	/// Get the state of the current KeyBoard Button
	/// \warning This function doesn't check for dumbness
	bool get_key_state(SDLKey const key) const {return SDL_GetKeyState(nullptr)[key];}

	//@{
	void warp_mouse(Point);
	void set_input_grab(bool grab);

	/// The mouse's current coordinates
	Point get_mouse_position() const {return m_mouse_position;}
	//
	/// Find out whether the mouse is currently pressed
	bool is_mouse_pressed() const {return SDL_GetMouseState(nullptr, nullptr); }

	/// Swap left and right mouse key?
	void set_mouse_swap(const bool swap) {m_mouse_swapped = swap;}

	/// Lock the mouse cursor into place (e.g., for scrolling the map)
	void set_mouse_lock(const bool locked) {m_mouse_locked = locked;}
	//@}

	void init_graphics(int32_t w, int32_t h, bool fullscreen, bool opengl);

	/**
	 * Refresh the graphics from the latest options.
	 *
	 * \note See caveats for \ref init_graphics()
	 */
	void refresh_graphics();

	void handle_input(InputCallback const *);

	void mainmenu();
	void mainmenu_singleplayer();
	void mainmenu_multiplayer();
	void mainmenu_editor();

	bool new_game();
	bool load_game();
	bool campaign_game();
	void replay();

#ifndef NDEBUG
#ifndef _WIN32
	//not all of these need to be public, but I consider signal handling
	//a public interface
	//@{
	void init_double_game();
	static void signal_handler (int32_t sig);
	static void quit_handler();
	static void yield_double_game();
	//@}

	// Used for --double
	//@{
	static int32_t pid_me;
	static int32_t pid_peer;
	///\todo Explain me
	static volatile int32_t may_run;
	//@}
#endif
#endif

	static void show_usage();

	static void emergency_save(Widelands::Game &);

protected:
	WLApplication(int argc, char const * const * argv);

	bool poll_event(SDL_Event &);

	bool init_settings();
	void init_language();
	std::string find_relative_locale_path(std::string localedir);
	std::string get_executable_path();
	void shutdown_settings();

	bool init_hardware();
	void shutdown_hardware();

	void parse_commandline(int argc, char const * const * argv);
	void handle_commandline_parameters();

	void setup_searchpaths(std::string argv0);
	void setup_homedir();

	void cleanup_replays();

	bool redirect_output(std::string path = "");

	/**
	 * The commandline, conveniently repackaged
	 * This is usually not empty, it contains at least the tuple
	 * {"EXENAME", argv0}
	 */
	std::map<std::string, std::string> m_commandline;

	std::string m_filename;

	/// Script to be run after the game was started with --editor,
	/// --scenario or --loadgame.
	std::string m_script_to_run;

	//Log all output to this file if set, otherwise use cout
	std::string m_logfile;

	GameType m_game_type;

	///True if left and right mouse button should be swapped
	bool  m_mouse_swapped;

	/// When apple is involved, the middle mouse button is sometimes send, even
	/// if it wasn't pressed. We try to revert this and this helps.
	bool  m_faking_middle_mouse_button;

	///The current position of the mouse pointer
	Point m_mouse_position;

	///If true, the mouse cursor will \e not move with a mousemotion event:
	///instead, the map will be scrolled
	bool  m_mouse_locked;

	///If the mouse needs to be moved in warp_mouse(), this Point is
	///used to cancel the resulting SDL_MouseMotionEvent.
	Point m_mouse_compensate_warp;

	///true if an external entity wants us to quit
	bool   m_should_die;

	//do we want to search the default places for widelands installs
	bool   m_default_datadirs;
	std::string m_homedir;

	/// flag indicating if stdout and stderr have been redirected
	bool m_redirected_stdio;
private:
	///Holds this process' one and only instance of WLApplication, if it was
	///created already. nullptr otherwise.
	///\note This is private on purpose. Read the class documentation.
	static WLApplication * the_singleton;

	void _handle_mousebutton(SDL_Event &, InputCallback const *);

};

#endif
