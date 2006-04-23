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

#ifndef SYSTEM_H
#define SYSTEM_H

#include "graphic.h"
#include <libintl.h>
#include <locale.h>
#include <string>
#include "types.h"

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
==============================================================================

SYSTEM ABSTRACTION

==============================================================================
*/

extern Graphic *g_gr;

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

void init_double_game ();

// basic initialization etc..
void Sys_Init();
void Sys_Shutdown();

void Sys_SetRecordFile(const char *filename);
void Sys_SetPlaybackFile(const char *filename);

bool Sys_ShouldDie();

// timing
int Sys_GetTime();

// input
struct InputCallback {
	void (*mouse_click)(bool down, int btn, uint btns, int x, int y);
	void (*mouse_move)(uint btns, int x, int y, int xdiff, int ydiff);
	void (*key)(bool down, int code, char c);
};

// locale
#define _( str ) Sys_Translate( str )
void Sys_GrabTextdomain( const char* );
void Sys_ReleaseTextdomain( void );
void Sys_SetLocale( const char* = 0);
std::string Sys_GetLocale();
inline const char* Sys_Translate( const char* str ) {
	return gettext( str );
}

void Sys_HandleInput(InputCallback *cb);
uint Sys_GetMouseButtons();
int Sys_GetMouseX();
int Sys_GetMouseY();
void Sys_SetMousePos(int x, int y);
bool Sys_GetKeyState(uint key);

void Sys_SetInputGrab(bool grab);
void Sys_SetMouseSwap(bool swap);
void Sys_SetMouseSpeed(float speed);

void Sys_MouseLock(bool locked);

// graphics
void Sys_InitGraphics(int w, int h, int bpp, bool fullscreen);

struct SYS {
	bool		sdl_active;
	bool		should_die;

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
	int		gfx_w;
	int		gfx_h;
	bool		gfx_fullscreen;

	std::string		locale;
};

extern struct SYS sys;

#endif
