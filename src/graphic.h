/*
 * Copyright (C) 2002-2004 by the Wide Lands Development Team
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

#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "rgbcolor.h"
#include "types.h"

class RenderTarget;

enum { // picture module flags
	PicMod_UI = 1,
	PicMod_Menu = 2,
	PicMod_Game = 4
};

/*
class Graphic

This interface represents the framebuffer / screen.

Picture IDs can be allocated using get_picture() and used in RenderTarget::blit().
Pictures are only loaded from disk once and thrown out of memory when the
graphics system is unloaded, or when flush() is called with the appropriate
module flag.
*/
class Graphic {
public:
	virtual ~Graphic() { }

	virtual int get_xres() = 0;
	virtual int get_yres() = 0;
	virtual RenderTarget* get_render_target() = 0;
	virtual void update_fullscreen() = 0;
	virtual void update_rectangle(int x, int y, int w, int h) = 0;
	virtual bool need_update() = 0;
	virtual void refresh() = 0;

	virtual void flush(int mod) = 0;
	virtual uint get_picture(int mod, const char* fname) = 0;
	virtual uint get_picture(int mod, const char* fname, RGBColor clrkey) = 0;
	virtual void get_picture_size(uint pic, int* pw, int* ph) = 0;
	virtual uint create_surface(int w, int h) = 0;
	virtual uint create_surface(int w, int h, RGBColor clrkey) = 0;
	virtual void free_surface(uint pic) = 0;
	virtual RenderTarget* get_surface_renderer(uint pic) = 0;

	virtual uint get_maptexture(const char* fnametempl, uint frametime) = 0;
	virtual void animate_maptextures(uint time) = 0;

	virtual void load_animations() = 0;
	virtual void get_animation_size(uint anim, uint time, int* w, int* h) = 0;

	virtual void screenshot(const char* fname) = 0;
	virtual const char* get_maptexture_picture (uint id) = 0;
	// HACK: needed to load the old font
	virtual uint get_picture(int mod, int w, int h, const ushort* data, RGBColor clrkey) = 0;
};

extern Graphic* g_gr;


#endif /* GRAPHIC_H */
