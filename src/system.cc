/*
 * Copyright (C) 2002 by the Widelands Development Team
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


System *g_system = 0;

/*
Notes on the implementation
---------------------------

Mouse:
The mouse that is seen by the OS and SDL is always in the center of the screen.
Our own virtual mouse is moved independently.
Obviously, we draw the mouse ourselves.

Internally, mouse position is kept with sub-pixel accuracy to make mouse speed
work.
*/

/*
===============
System::System

Initialize lower level libraries (i.e. SDL)
===============
*/
System::System()
{
	m_should_die = false;
	
	// Input
	m_mouse_swapped = false;
	m_mouse_speed = 1.0;
	m_mouse_buttons = 0;
	m_mouse_x = m_mouse_y = 0;
	m_mouse_maxx = m_mouse_maxy = 0;

	Section *s = g_options.get_safe_section("global");

	set_mouse_swap(s->get_bool("swapmouse", false));
	set_mouse_speed(s->get_float("mousespeed", 1.0));

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		throw wexception("Failed to initialize SDL: %s", SDL_GetError());

	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_EnableUNICODE(1); // useful for e.g. chat messages
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

/*
===============
System::~System

Shutdown the system	
===============
*/
System::~System()
{
	SDL_Quit();
}

/*
===============
System::should_die

Returns true if an external entity wants us to quit
===============
*/	
bool System::should_die()
{
	return m_should_die;
}

/*
===============
System::get_time

Return the current time, in milliseconds
===============
*/
int System::get_time()
{
	return SDL_GetTicks();
}
	
/*
===============
System::handle_input

Run the event queue, get packets from the network, etc...
===============
*/
void System::handle_input(InputCallback *cb)
{
	SDL_Event ev;
	
	while(SDL_PollEvent(&ev)) {
		int button;
	
		switch(ev.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (ev.key.keysym.sym == SDLK_F10) // TEMP - get out of here quick
			{
				if (ev.type == SDL_KEYDOWN)
					m_should_die = true;
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
						g_gr.screenshot(buf);
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
			if (m_mouse_swapped) {
				if (button == MOUSE_LEFT) button = MOUSE_RIGHT;
				else if (button == MOUSE_RIGHT) button = MOUSE_LEFT;
			}
			
			if (ev.type == SDL_MOUSEBUTTONDOWN)
				m_mouse_buttons |= 1 << button;
			else
				m_mouse_buttons &= ~(1 << button);
			
			if (cb && cb->mouse_click)
				cb->mouse_click(ev.type == SDL_MOUSEBUTTONDOWN, button, m_mouse_buttons,
				                (int)m_mouse_x, (int)m_mouse_y);
			break;

		case SDL_MOUSEMOTION:
		{
			int centerx = m_mouse_maxx / 2;
			int centery = m_mouse_maxy / 2;
			
			if (ev.motion.x == centerx && ev.motion.y == centery)
				break;
				
			int mxd = ev.motion.x - centerx;
			int myd = ev.motion.y - centery;
			
			SDL_WarpMouse(centerx, centery);
			
			float xlast = m_mouse_x;
			float ylast = m_mouse_y;
			
			m_mouse_x += mxd * m_mouse_speed;
			m_mouse_y += myd * m_mouse_speed;
			
			if (m_mouse_x < 0)
				m_mouse_x = 0;
			else if (m_mouse_x >= m_mouse_maxx-1)
				m_mouse_x = m_mouse_maxx-1;
			if (m_mouse_y < 0)
				m_mouse_y = 0;
			else if (m_mouse_y >= m_mouse_maxy-1)
				m_mouse_y = m_mouse_maxy-1;
			
			int xdiff = (int)m_mouse_x - (int)xlast;
			int ydiff = (int)m_mouse_y - (int)ylast;
			
			if (!xdiff && !ydiff)
				break;
			
			if (cb && cb->mouse_move)
				cb->mouse_move(m_mouse_buttons, (int)m_mouse_x, (int)m_mouse_y, xdiff, ydiff);
		} break;

		case SDL_QUIT:
			m_should_die = true;
			break;

		default:
			break;
		}
	}
}	

/*
===============
System::get_mouse_buttons
System::get_mouse_x
System::get_mouse_y
===============
*/
uint System::get_mouse_buttons()
{
	return m_mouse_buttons;
}

int System::get_mouse_x()
{
	return (int)m_mouse_x;
}

int System::get_mouse_y()
{
	return (int)m_mouse_y;
}

/*
===============
System::set_mouse_pos

Move the mouse cursor.
No mouse moved event will be issued.
===============
*/
void System::set_mouse_pos(int x, int y)
{
	m_mouse_x = x;
	m_mouse_y = y;
}

/*
===============
System::set_mouse_swap
===============
*/
void System::set_mouse_swap(bool swap)
{
	m_mouse_swapped = swap;
}

/*
===============
System::set_mouse_speed
===============
*/
void System::set_mouse_speed(float speed)
{
	if (speed <= 0.1 || speed >= 10.0)
		speed = 1.0;
	m_mouse_speed = speed;
}
	
/*
===============
System::set_max_mouse_coords [kludge]

Set the mouse boundary after a change of resolution
===============
*/
void System::set_max_mouse_coords(int x, int y)
{
	m_mouse_maxx = x;
	m_mouse_maxy = y;
}

