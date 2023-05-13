/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WLAPPLICATION_H
#define WL_WLAPPLICATION_H

// Workaround for bug https://sourceforge.net/p/mingw/bugs/2152/
#ifdef __MINGW32__
#ifndef _WIN64
#ifndef _USE_32BIT_TIME_T
#define _USE_32BIT_TIME_T 1
#endif
#endif
#endif

#include <atomic>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL_events.h>
#include <SDL_keyboard.h>

#include "base/vector.h"
#include "wlapplication_messages.h"

namespace UI {
class Panel;
}  // namespace UI
namespace Widelands {
class Game;
}  // namespace Widelands

/** Returns the widelands executable path. */
std::string get_executable_directory(bool logdir = true);

/// Thrown if a commandline parameter is faulty
struct ParameterError : public std::runtime_error {
	explicit ParameterError(CmdLineVerbosity level, const std::string& text = "")
	   : std::runtime_error(text), level_(level) {
	}

	CmdLineVerbosity level_;
};

// Callbacks input events to the UI. All functions return true when the event
// was handled, false otherwise.
struct InputCallback {
	bool (*mouse_press)(const uint8_t button,  // Button number as #defined in SDL_mouse.h.
	                    int32_t x,
	                    int32_t y);              // The coordinates of the mouse at press time.
	bool (*mouse_release)(const uint8_t button,  // Button number as #defined in SDL_mouse.h.
	                      int32_t x,
	                      int32_t y);  // The coordinates of the mouse at release time.
	bool (*mouse_move)(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool (*key)(bool down, SDL_Keysym code);
	bool (*textinput)(const std::string& text);
	bool (*mouse_wheel)(const int32_t x,           // The number of horizontal scroll ticks
	                    const int32_t y,           // The number of vertical scroll ticks
	                    const uint16_t modstate);  // Modifier keys pressed at the time of the event
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
// TODO(bedouin): Is the above part about i18n still true?
//
// Equally important, the main event loop is chugging along in this class.
// [not yet but some time in the future \#bedouin8sep2007]
///
/// \par WLApplication is a singleton
///
/// Because of its special purpose, having more than one WLApplication is
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
/// copy, so the two instances cannot know (except for fork()'s return value)
/// that they are (or are not) a primary thread. Each WLApplication singleton
/// really *is* a singleton - inside its own process.
///
/// Forking does not work on windows, but nobody cares enough to investigate.
/// It is only a debugging convenience anyway.
///
///
/// \par The mouse cursor
///
/// Ordinarily, relative coordinates break down when the cursor leaves the
/// window. This means we have to grab the mouse, then relative coords are
/// always available.
// TODO(unknown): Actually do grab the mouse when it is locked
// TODO(unknown): Graphics are currently not handled by WLApplication, and it is
// non essential for playback anyway. Additionally, we will want several
// rendering backends (software and OpenGL). Maybe the graphics backend loader
// code should be in System, while the actual graphics work is done elsewhere.
// TODO(unknown): Refactor the mainloop
// TODO(unknown): Sensible use of exceptions (goes for whole game)
// TODO(sirver): this class makes no sense for c++ - most of these should be
// stand alone functions.
struct WLApplication {
	static WLApplication& get(int argc = 0, char const** argv = nullptr);
	~WLApplication();

	void run();

	static void initialize_g_addons();

	/// \warning true if an external entity wants us to quit
	[[nodiscard]] bool should_die() const {
		return should_die_;
	}

	/// Get the state of the current KeyBoard Button
	/// \warning This function doesn't check for dumbness
	[[nodiscard]] bool get_key_state(SDL_Scancode const key) const {
		return SDL_GetKeyboardState(nullptr)[key] != 0U;
	}

	// @{
	void warp_mouse(Vector2i);
	void set_input_grab(bool grab);

	/// The mouse's current coordinates
	[[nodiscard]] Vector2i get_mouse_position() const {
		return mouse_position_;
	}
	//
	/// Find out whether the mouse is currently pressed
	[[nodiscard]] bool is_mouse_pressed() const {
		return SDL_GetMouseState(nullptr, nullptr) != 0U;
	}

	/// Swap left and right mouse key?
	void set_mouse_swap(const bool swap) {
		mouse_swapped_ = swap;
	}

	/// Lock the mouse cursor into place (e.g., for scrolling the map)
	void set_mouse_lock(bool locked);
	[[nodiscard]] bool is_mouse_locked() const {
		return mouse_locked_;
	}
	// @}

	[[nodiscard]] const std::string& get_datadir() const {
		return datadir_;
	}

	// Handle the given pressed key. Returns true when key was
	// handled.
	bool handle_key(bool down, const SDL_Keycode& keycode, int modifiers);
	void enable_handle_key(bool enable) {
		handle_key_enabled_ = enable;
	}

	// Pump SDL events and dispatch them.
	void handle_input(InputCallback const*);

	static void emergency_save(UI::Panel*,
	                           Widelands::Game&,
	                           const std::string& error,
	                           uint8_t player = 1,
	                           bool replace_ctrl = true,
	                           bool ask_for_bug_report = true);

private:
	WLApplication(int argc, char const* const* argv);

	bool poll_event(SDL_Event&) const;

	bool init_settings();
	void init_language();
	void shutdown_settings();

	void shutdown_hardware();

	void parse_commandline(int argc, char const* const* argv);
	void handle_commandline_parameters();

	void setup_homedir();

	void cleanup_replays();
	void cleanup_ai_files();
	void cleanup_temp_files();
	void cleanup_temp_backups(const std::string& dir);
	void cleanup_temp_backups();

	void init_and_run_game_from_template();

	bool redirect_output(std::string path = "");

	/**
	 * The commandline, conveniently repackaged.
	 */
	std::map<std::string, std::string> commandline_;

	std::string filename_;

	/// Script to be run after the game was started with --editor,
	/// --scenario or --loadgame.
	std::string script_to_run_;

	enum class GameType { kNone, kEditor, kReplay, kScenario, kLoadGame, kFromTemplate };
	GameType game_type_{GameType::kNone};

	/// True if left and right mouse button should be swapped
	bool mouse_swapped_{false};

	/// When apple is involved, the middle mouse button is sometimes send, even
	/// if it wasn't pressed. We try to revert this and this helps.
	bool faking_middle_mouse_button_{false};

	/// The current position of the mouse pointer
	Vector2i mouse_position_{Vector2i::zero()};

	/// If true, the mouse cursor will \e not move with a mousemotion event:
	/// instead, the map will be scrolled
	bool mouse_locked_{false};

	/// Makes it possible to disable the fullscreen and screenshot shortcuts
	bool handle_key_enabled_{true};

	/// true if an external entity wants us to quit
	std::atomic_bool should_die_{false};

	std::string homedir_;
#ifdef USE_XDG
	std::string userconfigdir_;
#endif

	/// flag indicating if stdout and stderr have been redirected
	bool redirected_stdio_{false};

	/// Absolute path to the data directory.
	std::string datadir_;
	std::string datadir_for_testing_;

	/// Absolute path to the locale directory.
	std::string localedir_;

	/// Prevent toggling fullscreen on and off from flickering
	uint32_t last_resolution_change_{0U};

	void handle_mousebutton(SDL_Event&, InputCallback const*);
};

#endif  // end of include guard: WL_WLAPPLICATION_H
