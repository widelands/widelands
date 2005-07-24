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

#include <string>
#include "types.h"

/*
==============================================================================

SYSTEM ABSTRACTION

==============================================================================
*/

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

#endif
