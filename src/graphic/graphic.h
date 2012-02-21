/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "animation_gfx.h"
#include "picture_id.h"
#include "surfaceptr.h"
#include "rect.h"

#include <png.h>

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#define MAX_RECTS 20

namespace UI {struct ProgressWindow;}

struct IPixelAccess;
struct RenderTarget;
struct Graphic;
struct Road_Textures;
struct StreamWrite;
struct Texture;
struct SDL_Surface;
struct SDL_Rect;

//@{
/// This table is used by create_grayed_out_pic()to map colors to grayscle. It
/// is initialized in Graphic::Graphic().
extern uint32_t luminance_table_r[0x100];
extern uint32_t luminance_table_g[0x100];
extern uint32_t luminance_table_b[0x100];
//@}

/// Stores the capabilities of opengl
struct GLCaps
{
	/// The OpenGL major version
	int major_version;
	/// The OpenGL minor version
	int minor_version;
	/// The maximum texture size
	int tex_max_size;
	/// If true sizes of texture must be a power of two
	bool tex_power_of_two;
	/// How many bits the stencil buffer support
	int stencil_buffer_bits;
	/// How many Aux Buffers the opengl context support
	int aux_buffers;
	/// Whether the BlendEquation support is available
	bool blendequation;
	/// Maximum number of textures that can be combined
	int max_tex_combined;
	/// Whether multitexturing is supported
	bool multitexture;
};

/**
 * A structure to store the capabilities of the current rendere. This is set
 * during init() and can be retrieved by g_gr->get_caps()
 */
struct GraphicCaps
{
	/// The renderer allows rendering (blit, draw_line) to offscreen surfaces
	bool offscreen_rendering;
	/// The capabilities of the opengl hardware and drive
	GLCaps gl;
};

/**
 * Picture caches (modules).
 *
 * \ref Graphic maintains a cache of \ref PictureID s to avoid continuous re-loading of
 * pictures that may not be referenced all the time (e.g. UI elements).
 *
 * This cache is separated into different modules, and can be flushed per-module.
 */
enum PicMod {
	PicMod_UI = 0,
	PicMod_Menu,
	PicMod_Game,

	// Must be last
	PicMod_Last
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
	PictureID load_image(std::string const &, bool alpha = false);
	const PictureID & get_picture(PicMod, std::string const &, bool alpha = true)
		__attribute__ ((pure));
	void add_picture_to_cache(PicMod, const std::string &, PictureID);
	const PictureID & get_no_picture() const;

	void get_picture_size
		(const PictureID & pic, uint32_t & w, uint32_t & h) const;
	PictureID get_offscreen_picture(OffscreenSurfacePtr surface) const;

	void save_png(const PictureID &, StreamWrite *) const;
	void save_png(SurfacePtr surf, StreamWrite *) const;
	void save_png(IPixelAccess & pix, StreamWrite *) const;

	PictureID convert_sdl_surface_to_picture(SDL_Surface *, bool alpha = false);

	OffscreenSurfacePtr create_offscreen_surface(int32_t w, int32_t h);
	PictureID create_picture(int32_t w, int32_t h, bool alpha = false);

	PictureID create_grayed_out_pic(const PictureID & picid);
	PictureID create_changed_luminosity_pic
		(const PictureID & picid, const float factor, const bool halve_alpha = false);

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

	uint32_t get_maptexture(char const & fnametempl, uint32_t frametime);
	void animate_maptextures(uint32_t time);
	void reset_texture_animation_reminder();

	void load_animations(UI::ProgressWindow & loader_ui);
	AnimationGfx::Index nr_frames(uint32_t const anim = 0) const;
	uint32_t get_animation_frametime(uint32_t anim) const;
	void get_animation_size
		(const uint32_t anim,
		 const uint32_t time,
		 uint32_t & w,
		 uint32_t & h)
		const;

	void screenshot(const char & fname) const;
	Texture * get_maptexture_data(uint32_t id);
	AnimationGfx * get_animation(uint32_t) const;

	void set_world(std::string);
	PictureID get_road_texture(int32_t roadtex);
	PictureID get_edge_texture();

	GraphicCaps const & caps() const throw () {return m_caps;}

private:
	SDL_Surface * extract_sdl_surface(IPixelAccess & pix, Rect srcrect);

protected:
	// Static helper function for png writing
	static void m_png_write_function
		(png_structp png_ptr,
		 png_bytep data,
		 png_size_t length);
	static void m_png_flush_function (png_structp png_ptr);

	/// This is the main screen Surface.
	/// A RenderTarget for this can be retrieved with get_render_target()
	SurfacePtr m_screen;
	/// This saves a copy of the screen SDL_Surface. This is needed for
	/// opengl rendering as the SurfaceOpenGL does not use it. It allows
	/// manipulation the screen context.
	SDL_Surface * m_sdl_screen;
	/// A RenderTarget for m_screen. This is initialized during init()
	RenderTarget * m_rendertarget;
	/// keeps track which screen regions needs to be redrawn during the next
	/// update(). Only used for SDL rendering.
	SDL_Rect m_update_rects[MAX_RECTS];
	/// saves how many screen regions need updating. @see m_update_rects
	int32_t m_nr_update_rects;
	/// This marks the komplete screen for updating.
	bool m_update_fullscreen;
	/// stores which features the current renderer has
	GraphicCaps m_caps;

	struct PictureRec {
		PictureID picture;

		/// bit-mask of modules that this picture exists in
		uint32_t modules;
	};

	typedef std::map<std::string, PictureRec> Picturemap;
	typedef Picturemap::iterator pmit;

	/// hash of cached filename/picture pairs
	Picturemap m_picturemap;

	Road_Textures * m_roadtextures;
	PictureID m_edgetexture;
	std::vector<Texture *> m_maptextures;
	std::vector<AnimationGfx *> m_animations;
};

extern Graphic * g_gr;
extern bool g_opengl;

#endif
