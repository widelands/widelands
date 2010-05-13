/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
#include "picture_id.h"
#include "rect.h"
//#include "surface.h"
//#include "texture.h"

#include <png.h>

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

/**
 * Names of road terrains
 */
#define ROAD_NORMAL_PIC "pics/roadt_normal.png"
#define ROAD_BUSY_PIC   "pics/roadt_busy.png"

#define MAX_RECTS 20

namespace UI {struct ProgressWindow;}

class Surface;
class Texture;
struct RenderTarget;
class Surface;
struct Graphic;
struct Road_Textures;
struct StreamWrite;
struct Texture;
struct SDL_Surface;
struct SDL_Rect;

// This table is used by create_grayed_out_pic() 
// to map colors to grayscle. It is initialized in Graphic::Graphic()
extern uint32_t luminance_table_r[0x100];
extern uint32_t luminance_table_g[0x100];
extern uint32_t luminance_table_b[0x100];

struct GLCaps
{
	int major_version;
	int minor_version;
	int tex_max_size;
	bool tex_power_of_two;
};

struct GraphicCaps
{
	bool offscreen_rendering;
	GLCaps gl;
};

/**
 * A renderer to get pixels to a 16bit framebuffer.
 *
 * Picture IDs can be allocated using \ref get_picture() and used in
 * \ref RenderTarget::blit().
 *
 * Pictures are only loaded from disk once and thrown out of memory when the
 * graphics system is unloaded, or when \ref flush() is called with the
 * appropriate module flag; the user can request to flush one single picture
 * alone, but this is only used (and useful) in the editor.
*/

struct Graphic {
	Graphic
		(int32_t w, int32_t h, int32_t bpp,
		 bool fullscreen, bool opengl);
	~Graphic();

	int32_t get_xres() const;
	int32_t get_yres() const;
	RenderTarget * get_render_target();
	void toggle_fullscreen();
	void update_fullscreen();
	void update_rectangle(int32_t x, int32_t y, int32_t w, int32_t h);
	void update_rectangle(Rect const & rect) {
		update_rectangle (rect.x, rect.y, rect.w, rect.h);
	}
	bool need_update() const;
	void refresh(bool force = true);

	void flush(PicMod module);
	void flush_animations();
	Surface & LoadImage(const std::string & fname);
	PictureID & get_picture(PicMod module, const std::string & fname)
	__attribute__ ((pure));
	PictureID get_picture
		(PicMod module, Surface &, const std::string & name = "");
	//__attribute__ ((pure));
	PictureID & get_no_picture() const;

	Surface * get_picture_surface(const PictureID & id);
	const Surface * get_picture_surface(const PictureID & id) const;

	void get_picture_size
		(const PictureID & pic, uint32_t & w, uint32_t & h) const;

	void save_png(const PictureID &, StreamWrite *) const;
	void save_png(Surface & surf, StreamWrite *) const;

	PictureID create_picture_surface(int32_t w, int32_t h);
	Surface & create_surface(SDL_Surface &);
	Surface & create_surface(Surface &);
	Surface & create_surface(int32_t w, int32_t h);
	void free_picture_surface(const PictureID & pic);

	PictureID create_grayed_out_pic(const PictureID & picid);
	RenderTarget * get_surface_renderer(const PictureID & pic);

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

	PictureID get_resized_picture
		(PictureID, uint32_t w, uint32_t h, ResizeMode);
	SDL_Surface * resize(const PictureID index, uint32_t w, uint32_t h);

	uint32_t get_maptexture(char const & fnametempl, uint32_t frametime);
	void animate_maptextures(uint32_t time);
	void reset_texture_animation_reminder();

	void load_animations(UI::ProgressWindow & loader_ui);
	AnimationGfx::Index nr_frames(uint32_t const anim = 0) const;
	void get_animation_size
		(const uint32_t anim,
		 const uint32_t time,
		 uint32_t & w,
		 uint32_t & h)
		const;

	void screenshot(const char & fname) const;
	Texture * get_maptexture_data(uint32_t id);
	AnimationGfx * get_animation(uint32_t) const;

	Surface * get_road_texture(int32_t roadtex);

	const GraphicCaps & caps() const throw()
		{ return m_caps; }

protected:
	/// Static function for png writing
	static void m_png_write_function
		(png_structp png_ptr,
		 png_bytep data,
		 png_size_t length);
	static void m_png_flush_function (png_structp png_ptr);

	Surface * m_screen;
	SDL_Surface * m_sdl_screen;
	RenderTarget * m_rendertarget;
	SDL_Rect m_update_rects[MAX_RECTS];
	int32_t m_nr_update_rects;
	bool m_update_fullscreen;
	GraphicCaps m_caps;

	/// hash of filename/picture ID pairs
	std::vector
		<std::map<std::string, boost::shared_ptr<Picture> > > m_picturemap;
	typedef std::map<std::string, boost::shared_ptr<Picture> > Picturemap;
	typedef Picturemap::iterator pmit;

	Road_Textures * m_roadtextures;
	std::vector<Texture *> m_maptextures;
	std::vector<AnimationGfx *> m_animations;
};

extern Graphic * g_gr;
extern bool g_opengl;

#endif

