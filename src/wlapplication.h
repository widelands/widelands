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

/// A macro to make i18n more readable
#define _( str ) WLApplication::translate( str )

// input
struct InputCallback {
	void (*mouse_click)(bool down, int btn, uint btns, int x, int y);
	void (*mouse_move)(uint btns, int x, int y, int xdiff, int ydiff);
	void (*key)(bool down, int code, char c);
};

//TODO: rework this. bitshifting is not beautiful in cross platform systems
//TODO: and it does not belong in this header either!
enum { // use 1<<MOUSE_xxx for bitmasks
	MOUSE_LEFT = 0,
	MOUSE_MIDDLE,
	MOUSE_RIGHT
};

/*
All interactions with the OS except for file access and graphics are handled
by the Sys_xxx type functions. Most notably:
 - timing
 - input
 - low-level networking

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
 * Note:There is no need for a destructor, just make sure you call
 * \ref shutdown().
 *
 * \todo Document that record/playback files are journals
 */
class WLApplication {
public:
	static WLApplication *get(int argc=0, char **argv=0);

	void run();
	bool init();
	void shutdown();

	//Locale handling
	static const char* translate( const char* str ) {return gettext( str );}
	void grab_textdomain( const char* );
	void release_textdomain();
	void set_locale( const char* = 0);
	std::string get_locale() {return m_locale;}

	///Record/Playback file handling
	//@{
	bool get_playback() {return m_playback;}
	int get_playback_offset();
	bool get_record() {return m_record;}
	FILE *get_rec_file() {return m_frecord;}
	FILE *get_play_file() {return m_fplayback;}
	//@}

	/**
	 * Returns true if an external entity wants us to quit
	 */
	bool should_die() {return m_should_die;}
	int get_time();

	/// Mouse related settings
	//@{
	uint get_mouse_buttons() {return m_mouse_buttons;}
	int get_mouse_x() {return (int)m_mouse_x;}
	int get_mouse_y() {return (int)m_mouse_y;}

	/**
	 * Get the state of the current KeyBoard Button
	 * \warning This function doesn't check for dumbness
	 */
	bool get_key_state(uint key) {return SDL_GetKeyState(0)[key];}
	void set_mouse_pos(int x, int y);

	void set_input_grab(bool grab);
	void set_mouse_swap(bool swap) {m_mouse_swapped = swap;}
	void set_mouse_speed(float speed);

	/**
	 * Lock the mouse cursor into place (e.g., for scrolling the map)
	 */
	void set_mouse_lock(bool locked) {m_mouse_locked = locked;}

	void set_max_mouse_coords(int x, int y);
	void do_warp_mouse(int x, int y);
	//@}

	void init_graphics(int w, int h, int bpp, bool fullscreen);

	void handle_input(InputCallback *cb);

protected:
	WLApplication(int argc, char **argv);

	bool poll_event(SDL_Event *ev, bool throttle);

	bool init_settings();
	void shutdown_settings();

	bool init_hardware();
	void shutdown_hardware();

	bool init_recordplaybackfile();
	void shutdown_recordplaybackfile();

	bool parse_command_line();
	void show_usage();

	/// The commandline
	///\todo make this a vector<string>
	//@{
	int m_argc;
	char **m_argv;
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
