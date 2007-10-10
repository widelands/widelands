/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef SURFACE_H
#define SURFACE_H

#include "geometry.h"
#include "rgbcolor.h"
#include <stdint.h>
#include "texture.h"

struct Editor_Game_Base;
struct Player;

/**
 * This was formerly called struct Bitmap. But now it manages an
 * SDL_Surface as it's core.
 *
 * Represents a simple bitmap without managing its memory. The rendering
 * functions do NOT perform any clipping; this is up to the caller.
*/
class Surface {
	friend class AnimationGfx;
	friend class Font_Handler; // Needs m_surface for SDL_Blitting

	SDL_Surface* m_surface;
	int32_t m_offsx;
	int32_t m_offsy;
	uint32_t m_w, m_h;

public:
	Surface() : m_surface(0), m_offsx(0), m_offsy(0) {}
	Surface(const Surface&);
	~Surface();

	/// Set surface, only call once
	void set_sdl_surface(SDL_Surface & surface);
	SDL_Surface* get_sdl_surface() {return m_surface;}

	/// Get width and height
	uint32_t get_w() const {return m_w;}
	uint32_t get_h() const {return m_h;}
	void update();

	/// Save a bitmap of this to a file
	void save_bmp(const char & fname) const;

	// For the bravest: Direct Pixel access. Use carefully
	/// Needed if you want to blit directly to the screen by memcpy
	void force_disable_alpha();
	const SDL_PixelFormat * get_format() const;
	const SDL_PixelFormat & format() const;
	uint16_t get_pitch() const {return m_surface->pitch;}
	void * get_pixels() const throw ();

	/// Lock
	void lock();
	void unlock();

	/// For the slowest: Indirect pixel access
	uint32_t get_pixel(uint32_t x, uint32_t y);
	void set_pixel(uint32_t x, uint32_t y, const Uint32 clr);

	void clear();
	void draw_rect(const Rect, const RGBColor);
	void fill_rect(const Rect, const RGBColor);
	void brighten_rect(const Rect, const int32_t factor);

	void blit(Point dst, Surface* src, Rect srcrc);
	void fast_blit(Surface* src);

	void draw_minimap
		(const Editor_Game_Base  & egbase,
		 const Player * const,
		 const Rect                rc,
		 const Point               viewpoint,
		 const uint32_t                flags);


	/// sw16_terrain.cc
	void draw_field
		(Rect &,
		 Field * const f, Field * const rf, Field * const fl, Field * const rfl,
		 const int32_t posx, const int32_t rposx, const int32_t posy,
		 const int32_t blposx, const int32_t rblposx, const int32_t blposy,
		 uint8_t roads,
		 Sint8 f_brightness,
		 Sint8 r_brightness,
		 Sint8 bl_brightness,
		 Sint8 br_brightness,
		 const Texture & tr_d_texture,
		 const Texture & l_r_texture,
		 const Texture & f_d_texture,
		 const Texture & f_r_texture,
		 bool);

private:
	void set_subwin(Rect r);
	void unset_subwin();
};

#endif
