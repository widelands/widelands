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
/*
Implementation of Graphic in 16-bit software mode.
*/
 
#include "widelands.h"
#include "options.h"
#include "graphic.h"
#include "pic.h"

#include <SDL.h>


/*
===============================================================================

SW16ScreenSurface -- simple wrapper around Bitmap to represent the screen

===============================================================================
*/

class SW16ScreenSurface : public Bitmap {
public:
	SW16ScreenSurface(SDL_Surface* surface);
};

/*
===============
SW16ScreenSurface::SW16Surface

Set members according to surface info.
===============
*/
SW16ScreenSurface::SW16ScreenSurface(SDL_Surface* surface)
{
	m_pixels = (ushort*)surface->pixels;
	m_w = surface->w;
	m_h = surface->h;
	m_pitch = surface->pitch / sizeof(ushort);
}


/*
===============================================================================

SW16Graphic -- 16 bit software implementation of main graphics interface

===============================================================================
*/

#define MAX_RECTS 20

class SW16Graphic : public Graphic {
public:
	SW16Graphic(int w, int h, bool fullscreen);
	virtual ~SW16Graphic();

	virtual int get_xres();
	virtual int get_yres();
	virtual RenderTarget* get_render_target();
	virtual void update_fullscreen();
	virtual void update_rectangle(int x, int y, int w, int h);
	virtual bool need_update();
	virtual void refresh();

	virtual void screenshot(const char* fname);

private:
	SDL_Surface*			m_sdlsurface;
	SW16ScreenSurface*	m_surface;
   SDL_Rect					m_update_rects[MAX_RECTS];
	int						m_nr_update_rects;
	bool						m_update_fullscreen;
};


/*
===============
SW16Graphic::SW16Graphic

Initialize the SDL video mode.
===============
*/
SW16Graphic::SW16Graphic(int w, int h, bool fullscreen)
{
	m_nr_update_rects = 0;
	m_update_fullscreen = false;
	
	// Set video mode using SDL
	int flags = SDL_SWSURFACE;
	
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	
	m_sdlsurface = SDL_SetVideoMode(w, h, 16, flags);
	if (!m_sdlsurface)
		throw wexception("Couldn't set video mode: %s", SDL_GetError());
	
	m_surface = new SW16ScreenSurface(m_sdlsurface);
}

/*
===============
SW16Graphic::~SW16Graphic

Free the surface
===============
*/
SW16Graphic::~SW16Graphic()
{
	delete m_surface;
	SDL_FreeSurface(m_sdlsurface);
}

/*
===============
SW16Graphic::get_xres
SW16Graphic::get_yres

Return the screen resolution
===============
*/
int SW16Graphic::get_xres()
{
	return m_sdlsurface->w;
}

int SW16Graphic::get_yres()
{
	return m_sdlsurface->h;
}

/*
===============
SW16Graphic::get_render_target

Return a pointer to the RenderTarget representing the screen
===============
*/
RenderTarget* SW16Graphic::get_render_target()
{
	return m_surface;
}

/*
===============
SW16Graphic::update_fullscreen

Mark the entire screen for refreshing
===============
*/
void SW16Graphic::update_fullscreen()
{
	m_update_fullscreen = true;
}

/*
===============
SW16Graphic::update_rectangle

Mark a rectangle for refreshing
===============
*/
void SW16Graphic::update_rectangle(int x, int y, int w, int h)
{
	if (m_nr_update_rects >= MAX_RECTS)
		{
		m_update_fullscreen = true;
		return;
		}

	m_update_rects[m_nr_update_rects].x = x;
	m_update_rects[m_nr_update_rects].y = y;
	m_update_rects[m_nr_update_rects].w = w;
	m_update_rects[m_nr_update_rects].h = h;
	++m_nr_update_rects;
}

/*
===============
SW16Graphic::need_update

Returns true if parts of the screen have been marked for refreshing.
===============
*/
bool SW16Graphic::need_update()
{
	return m_nr_update_rects || m_update_fullscreen;
}

/*
===============
SW16Graphic::refresh

Bring the screen uptodate.
===============
*/
void SW16Graphic::refresh()
{
//	if (m_update_fullscreen)
		SDL_UpdateRect(m_sdlsurface, 0, 0, 0, 0);
//	else
//		{
//		SDL_UpdateRects(m_sdlsurface, m_nr_update_rects, m_update_rects);
//		}
	
	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}

/*
===============
SW16Graphic::screenshot

Save a screenshot in the given file.
===============
*/
void SW16Graphic::screenshot(const char* fname)
{
	// TODO: this is incorrect; it bypasses the files code
   SDL_SaveBMP(m_sdlsurface, fname);
}


/*
===============
SW16_CreateGraphics

Factory function called by System code
===============
*/
Graphic* SW16_CreateGraphics(int w, int h, bool fullscreen)
{
	return new SW16Graphic(w, h, fullscreen);
}
