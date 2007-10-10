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

#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "animation_gfx.h"
#include "picture.h"

#include <vector>

#include <png.h>
#include <stdint.h>

/**
 * Names of road terrains
 */
#define ROAD_NORMAL_PIC "pics/roadt_normal.png"
#define ROAD_BUSY_PIC   "pics/roadt_busy.png"

#define MAX_RECTS 20

class RenderTarget;
class FileWrite;
class Surface;
class Graphic;

///\todo Get rid of this global function
SDL_Surface* LoadImage(const char * const filename);

/**
 * A renderer to get pixels to a 16bit framebuffer.
 *
 * Picture IDs can be allocated using \ref get_picture() and used in
 * \ref RenderTarget::blit().
 *
 * Pictures are only loaded from disk once and thrown out of memory when the
 * graphics system is unloaded, or when \ref flush() is called with the
 * appropriate module flag; the user can request to flush one single picture
 * alone, but this is only used (and usefull) in the editor.
*/

class Graphic
{
public:
	Graphic(int32_t w, int32_t h, int32_t bpp, bool fullscreen);
	~Graphic();

	int32_t get_xres();
	int32_t get_yres();
	RenderTarget* get_render_target();
	void toggle_fullscreen();
	void update_fullscreen();
	void update_rectangle(int32_t x, int32_t y, int32_t w, int32_t h);
	void update_rectangle(const Rect & rect)
	{update_rectangle (rect.x, rect.y, rect.w, rect.h);}
	bool need_update();
	void refresh(bool force = true);

	void flush(int32_t mod);
	uint32_t get_picture(int32_t mod, const char* fname);
	void get_picture_size(const uint32_t pic, uint32_t & w, uint32_t & h);
	uint32_t get_picture(const int32_t mod, Surface &, const char * const name = 0);
	Surface* get_picture_surface(uint32_t id);
	void save_png(uint32_t, FileWrite*);
	uint32_t create_surface(int32_t w, int32_t h);
	void free_surface(uint32_t pic);
	RenderTarget* get_surface_renderer(uint32_t pic);

	enum  ResizeMode {
		// do not worry about proportions, just sketch to requested size
		ResizeMode_Loose,
		// keep proportions, clip wider edge
		ResizeMode_Clip,
		// keep proportions, leave empty border if needed
		ResizeMode_LeaveBorder,
		// keep proportions, balance clipping and borders
		ResizeMode_Average,
	};

	uint32_t get_resized_picture(const uint32_t, const uint32_t w, const uint32_t h, ResizeMode);
	SDL_Surface* resize(const uint32_t index, const uint32_t w, const uint32_t h);

	uint32_t get_maptexture(const char & fnametempl, const uint32_t frametime);
	void animate_maptextures(uint32_t time);
	void reset_texture_animation_reminder();

	void load_animations(UI::ProgressWindow & loader_ui);
	AnimationGfx::Index nr_frames(const uint32_t anim=0) const;
	void get_animation_size(const uint32_t anim, const uint32_t time, uint32_t & w, uint32_t & h);

	void screenshot(const char & fname) const;
	const char* get_maptexture_picture (uint32_t id);
	Texture* get_maptexture_data(uint32_t id);
	AnimationGfx* get_animation(const uint32_t anim) const;

	Surface* get_road_texture(int32_t roadtex);

protected:
	/// Static function for png writing
	static void m_png_write_function
		(png_structp png_ptr, png_bytep data, png_size_t length);

	std::vector<Picture>::size_type find_free_picture();

	typedef std::map<std::string, std::vector<Picture>::size_type> picmap_t;

	Surface m_screen;
	RenderTarget * m_rendertarget;
	SDL_Rect m_update_rects[MAX_RECTS];
	int32_t m_nr_update_rects;
	bool m_update_fullscreen;

	std::vector<Picture> m_pictures;
	/// hash of filename/picture ID pairs
	picmap_t m_picturemap;

	Road_Textures* m_roadtextures;
	std::vector<Texture *> m_maptextures;
	std::vector<AnimationGfx *> m_animations;
};

extern Graphic* g_gr;

#endif

