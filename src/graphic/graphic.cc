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

#include "graphic.h"

#include "build_info.h"
#include "diranimations.h"
#include "wexception.h"
#include "i18n.h"
#include "log.h"
#include "container_iterate.h"
#include "upcast.h"

#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"

#include "font_handler.h"
#include "rendertarget.h"
#include "texture.h"

#include "render/surface_sdl.h"
#include "render/surface_opengl.h"

#include "logic/roadtype.h"
#include "logic/widelands_fileread.h"

#include "ui_basic/progresswindow.h"

#include <config.h>

#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include <cstring>
#include <iostream>

Graphic * g_gr;
bool g_opengl;

// This table is used by create_grayed_out_pic()
// to map colors to grayscle
uint32_t luminance_table_r[0x100];
uint32_t luminance_table_g[0x100];
uint32_t luminance_table_b[0x100];

/**
 * Initialize the SDL video mode.
*/
Graphic::Graphic
	(int32_t const w, int32_t const h,
	 int32_t const bpp,
	 bool    const fullscreen,
	 bool    const opengl)
	:
	m_rendertarget     (0),
	m_nr_update_rects  (0),
	m_update_fullscreen(false),
	m_roadtextures     (0)
{
	m_picturemap.resize(MaxModule);

	// Initialize the table used to create grayed pictures
	for
		(uint32_t i = 0, r = 0, g = 0, b = 0;
		 i < 0x100;
		 ++i, r += 5016388U, g += 9848226U, b += 1912603U)
	{
		luminance_table_r[i] = r;
		luminance_table_g[i] = g;
		luminance_table_b[i] = b;
	}


	//fastOpen tries to use mmap
	FileRead fr;
#ifndef WIN32
	fr.fastOpen(*g_fs, "pics/wl-ico-128.png");
#else
	fr.fastOpen(*g_fs, "pics/wl-ico-32.png");
#endif
	SDL_Surface * s = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
	SDL_WM_SetIcon(s, 0);
	SDL_FreeSurface(s);

#ifndef USE_OPENGL
	assert(not opengl);
#endif

	// Set video mode using SDL. First collect the flags

	int32_t flags = 0;
	g_opengl = false;
	SDL_Surface * sdlsurface = 0;

#ifdef USE_OPENGL
	if (opengl) {
		log("Graphics: Trying opengl\n");
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		flags |= SDL_OPENGL;
	}
#endif

	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
		log("Graphics: Trying FULLSCREEN\n");
	}

	log("Graphics: Try to set Videomode %ux%u %uBit\n", w, h, bpp);
	// Here we actually set the video mode
	sdlsurface = SDL_SetVideoMode(w, h, bpp, flags);

#ifdef USE_OPENGL
	// If we tried opengl and it was not successful try without opengl
	if (!sdlsurface and opengl)
	{
		log("Graphics: Could not set videomode: %s\n", SDL_GetError());
		log("Graphics: Trying without opengl\n");
		flags &= ~SDL_OPENGL;
		sdlsurface = SDL_SetVideoMode(w, h, bpp, flags);
	}
#endif

	if (!sdlsurface)
		throw wexception
			("Graphics: could not set video mode: %s", SDL_GetError());

	// setting the videomode was successful. Print some information now
	log("Graphics: Setting video mode was successful\n");

	if (opengl and 0 != (sdlsurface->flags & SDL_GL_DOUBLEBUFFER))
		log("Graphics: OPENGL DOUBLE BUFFERING ENABLED\n");
	if (0 != (sdlsurface->flags & SDL_FULLSCREEN))
		log("Graphics: FULLSCREEN ENABLED\n");

	// Set rendering capabilities for sdl. They are overwritten if in opengl mode
	m_caps.resize_surfaces = true;
	m_caps.offscreen_rendering = true;
	m_caps.blit_resized = false;

#ifdef USE_OPENGL
	if (0 != (sdlsurface->flags & SDL_OPENGL)) {
		//  We have successful opened an opengl screen. Print some information
		//  about opengl and set the rendering capabilities.
		log ("Graphics: OpenGL: OpenGL enabled\n");
		g_opengl = true;

		GLboolean glBool;
		GLint glInt;
		const char * str;

		glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
		log
			("Graphics: OpenGL: Double buffering %s\n",
			 (glBool == GL_TRUE)?"enabled":"disabled");

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glInt);
		log("Graphics: OpenGL: Max texture size: %u\n", glInt);
		m_caps.gl.tex_max_size = glInt;

		glGetIntegerv(GL_AUX_BUFFERS, &glInt);
		log("Graphics: OpenGL: Number of aux buffers: %u\n", glInt);
		m_caps.gl.aux_buffers = glInt;

		glGetIntegerv(GL_STENCIL_BITS, &glInt);
		log("Graphics: OpenGL: Number of stencil buffer bits: %u\n", glInt);
		m_caps.gl.stencil_buffer_bits = glInt;

		str = reinterpret_cast<const char *>(glGetString(GL_VERSION));
		m_caps.gl.major_version = atoi(str);
		m_caps.gl.minor_version = strstr(str, ".")?atoi(strstr(str, ".") + 1):0;
		log
			("Graphics: OpenGL: Version %d.%d \"%s\"\n",
			 m_caps.gl.major_version, m_caps.gl.minor_version, str);

		str = reinterpret_cast<const char *>(glGetString (GL_EXTENSIONS));
		m_caps.gl.tex_power_of_two =
			(m_caps.gl.major_version < 2) and
			(strstr(str, "GL_ARB_texture_non_power_of_two") == 0);

		log("Graphics: OpenGL: Textures ");
		log
			(m_caps.gl.tex_power_of_two?"must have a size power of two\n":
			 "may have any size\n");

		m_caps.resize_surfaces = false;
		m_caps.offscreen_rendering = false;
		m_caps.blit_resized = true;
	}
#endif

	/* Information about the video capabilities. */
	SDL_VideoInfo const * info = SDL_GetVideoInfo();
	char videodrvused[16];
	SDL_VideoDriverName(videodrvused, 16);
	log
		("**** GRAPHICS REPORT ****\n"
		 " VIDEO DRIVER %s\n"
		 " hw surface possible %d\n"
		 " window manager available %d\n"
		 " blitz_hw %d\n"
		 " blitz_hw_CC %d\n"
		 " blitz_hw_A %d\n"
		 " blitz_sw %d\n"
		 " blitz_sw_CC %d\n"
		 " blitz_sw_A %d\n"
		 " blitz_fill %d\n"
		 " video_mem %d\n"
		 " vfmt %p\n"
		 " size %d %d\n"
		 "**** END GRAPHICS REPORT ****\n",
		 videodrvused,
		 info->hw_available,
		 info->wm_available,
		 info->blit_hw,
		 info->blit_hw_CC,
		 info->blit_hw_A,
		 info->blit_sw,
		 info->blit_sw_CC,
		 info->blit_sw_A,
		 info->blit_fill,
		 info->video_mem,
		 info->vfmt,
		 info->current_w, info->current_h);

	log("Graphics: flags: %x\n", sdlsurface->flags);

	assert
		(sdlsurface->format->BytesPerPixel == 2 ||
		 sdlsurface->format->BytesPerPixel == 4);

	SDL_WM_SetCaption
		(("Widelands " + build_id() + '(' + build_type() + ')').c_str(),
		 "Widelands");

#if USE_OPENGL
	if (g_opengl) {
		glViewport(0, 0, w, h);

		// Set up OpenGL projection matrix. This transforms opengl coordiantes to
		// screen coordiantes. We set up a simple Orthogonal view which takes just
		// the x, y coordinates and ignores the z coordinate.
		// Note that the top and bottom values are interchanged. This is to invert
		// the y axis to get the same coordinates as with opengl.
		// The exact values of near and far clipping plane are not important.
		// We draw everything with z = 0. They just must not be null and have
		// different sign.
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//glOrtho(left, right, bottom, top, nearVal, farVal);
		glOrtho(0, w, h, 0, -1, 1);

		// Reset modelview matrix, disable depth testing (we do not need it)
		// And select backbuffer as default drawing target
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glDrawBuffer(GL_BACK);

		// Clear the screen before running the game.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		SDL_GL_SwapBuffers();
	}

	if (g_opengl)
		m_screen = new SurfaceOpenGL(w, h);
	else
#endif
	{
		m_screen = new SurfaceSDL(*sdlsurface);
	}
	m_screen->set_type(SURFACE_SCREEN);

	m_sdl_screen = sdlsurface;
	m_rendertarget = new RenderTarget(m_screen);
}

/**
 * Free the surface
*/
Graphic::~Graphic()
{
	for (size_t i = 1; i < m_picturemap.size(); ++i)
		flush(static_cast<PicMod>(i));

	delete m_rendertarget;
	delete m_screen;
	delete m_roadtextures;
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_xres() const
{
	return m_screen->get_w();
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_yres() const
{
	return m_screen->get_h();
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget * Graphic::get_render_target()
{
	m_rendertarget->reset();

	return m_rendertarget;
}

/**
 * Switch from fullscreen to windowed mode or vice-versa
*/
void Graphic::toggle_fullscreen()
{
	log("Try DL_WM_ToggleFullScreen...\n");
	//ToDo Make this work again
	SDL_WM_ToggleFullScreen(m_sdl_screen);
}

/**
 * Mark the entire screen for refreshing
*/
void Graphic::update_fullscreen()
{
	m_update_fullscreen = true;
}

/**
 * Mark a rectangle for refreshing
*/
void Graphic::update_rectangle(int32_t x, int32_t y, int32_t w, int32_t h)
{
	if (m_nr_update_rects >= MAX_RECTS) {
		m_update_fullscreen = true;
		return;
	}
	m_update_fullscreen = true;
	m_update_rects[m_nr_update_rects].x = x;
	m_update_rects[m_nr_update_rects].y = y;
	m_update_rects[m_nr_update_rects].w = w;
	m_update_rects[m_nr_update_rects].h = h;
	++m_nr_update_rects;
}

/**
 * Returns true if parts of the screen have been marked for refreshing.
*/
bool Graphic::need_update() const
{
	return m_nr_update_rects || m_update_fullscreen;
}

/**
 * Bring the screen uptodate.
 *
 * \param force update whole screen
*/
void Graphic::refresh(bool force)
{
#ifdef USE_OPENGL
	if (g_opengl) {
		SDL_GL_SwapBuffers();
		m_update_fullscreen = false;
		m_nr_update_rects = 0;
		return;
	}
#endif

	if (force or m_update_fullscreen)
		m_screen->update();
	else
		SDL_UpdateRects
			(m_sdl_screen, m_nr_update_rects, m_update_rects);

	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}

/**
 * Remove all resources (currently pictures) from the given modules.
 * \note flush(0) does nothing <- obviously wrong it deletes a lot!
*/
void Graphic::flush(PicMod const module) {
	// Flush pictures

	//pmit b, e = m_picturemap.end();
	for (size_t i = 0; i < m_picturemap.size(); ++i) {
		if (static_cast<PicMod>(i) == module) {
			m_picturemap[i].clear();
		}
	}

	if (!module || module & PicMod_Game) {
		container_iterate_const(std::vector<Texture *>, m_maptextures, i)
			delete *i.current;
		m_maptextures.clear();

		container_iterate_const(std::vector<AnimationGfx *>, m_animations, i)
			delete *i.current;
		m_animations.clear();

		delete m_roadtextures;
		m_roadtextures = 0;
	}

	if (not module or module & PicMod_UI) // Flush the cached Fontdatas
		UI::g_fh->flush_cache();
}


/// flushes the animations in m_animations
void Graphic::flush_animations() {
	container_iterate_const(std::vector<AnimationGfx *>, m_animations, i)
		delete *i.current;
	m_animations.clear();
}


Surface & Graphic::load_image(std::string const & fname, bool const alpha) {
	//log("Graphic::LoadImage(\"%s\")\n", fname.c_str());
	FileRead fr;
	SDL_Surface * sdlsurf;

	//fastOpen tries to use mmap
	fr.fastOpen(*g_fs, fname.c_str());

	sdlsurf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

	if (!sdlsurf)
		throw wexception("%s", IMG_GetError());

	Surface & surf = create_surface(*sdlsurf, alpha);

	return surf;
}

/**
 * Retrieves the picture ID of the picture with the given filename.
 * If the picture has already been loaded, the old ID is reused.
 * The picture is placed into the module(s) given by mod.
 *
 * \return 0 (a null-picture) if the picture cannot be loaded.
*/
PictureID & Graphic::get_picture
	(PicMod const module, const std::string & fname, bool alpha)
{
	//  Check if the picture is already loaded.
	pmit it = m_picturemap[module].find(fname);

	if (it != m_picturemap[module].end()) {
		return it->second;
	} else {
		Surface * surf;

		try {
			surf = &load_image(fname, alpha);
			//log("Graphic::get_picture(): loading picture '%s'\n", fname.c_str());
		} catch (std::exception const & e) {
			log("WARNING: Could not open %s: %s\n", fname.c_str(), e.what());
			return get_no_picture();
		}
		// Convert the surface accordingly

		// Fill in a free slot in the pictures array
		Picture & pic = * new Picture();
		PictureID id = PictureID(&pic);
		m_picturemap[module].insert(std::make_pair(fname, id));

		assert(pic.fname == 0);
		pic.fname = strdup(fname.c_str());

		//  FIXME no proper check for NULL return value!
		assert(pic.fname != 0);

		pic.surface = surf;

		it = m_picturemap[module].find(fname);
	}

	it->second->module = module;

	return it->second;
}

PictureID Graphic::get_picture
	(PicMod const module, Surface & surf, std::string const & fname)
{
	Picture & pic = * new Picture();
	PictureID id = PictureID(&pic);
	m_picturemap[module].insert(std::make_pair(fname, id));

	pic.module    = module;
	pic.surface   = &surf;

	if (fname.size() != 0) {
		assert(pic.fname == 0);
		pic.fname = strdup(fname.c_str());
	} else
		pic.fname = 0;

	//return m_picturemap.find(fname);
	return id;
}

PictureID & Graphic::get_no_picture() const {
	static PictureID invalid = PictureID(new Picture());
	return invalid;
}

/**
 * Produces a resized version of the specified picture
 *
 * This might not work with the renderer. Check g_gr->caps().resize_surfaces
 * to be sure the resizing is possible.
 * Might return same id if dimensions are the same
 */
PictureID Graphic::get_resized_picture
	(PictureID index,
	 uint32_t const w, uint32_t const h,
	 ResizeMode const mode)
{
	// Resizing is not possible with opengl surfaces
	if (g_opengl)
		g_gr->get_no_picture();

	Surface * const orig = index->surface;
	if (orig->get_w() == w and orig->get_h() == h)
		return index;

	uint32_t width = w;
	uint32_t height = h;

	if (mode != ResizeMode_Loose) {
		const double ratio_x = double(w) / orig->get_w();
		const double ratio_y = double(h) / orig->get_h();

		//  if proportions are to be kept, recalculate width and height
		if ((ratio_x - ratio_y) > 0.00001) { //  if not equal
			double ratio = 0;

			if (ResizeMode_Clip == mode)
				ratio = std::max (ratio_x, ratio_y);
			else if (ResizeMode_LeaveBorder == mode)
				ratio = std::max (ratio_x, ratio_y);
			else // average
				ratio = (ratio_x + ratio_y) / 2;

			width = uint32_t(orig->get_w() * ratio);
			height = uint32_t(orig->get_h() * ratio);
		}
	}

	PictureID pic = g_gr->create_picture_surface(w, h);

	if (mode == ResizeMode_Loose || (width == w && height == h)) {
		if (g_opengl)
		{
		} else {
			dynamic_cast<SurfaceSDL *>
				(pic->surface)->set_sdl_surface(*resize(index, w, h));
		}
	} else {
		SurfaceSDL src(*resize(index, width, height));

		// apply rectangle by adjusted size
		Rect srcrc;
		srcrc.w = std::min(w, width);
		srcrc.h = std::min(h, height);
		srcrc.x = (width - srcrc.w) / 2;
		srcrc.y = (height - srcrc.h) / 2;


		g_gr->get_surface_renderer(pic)->blitrect //  Get the rendertarget
			(Point((w - srcrc.w) / 2, (h - srcrc.h) / 2),
			 get_picture(index->module, src), srcrc);
	}
	return pic;
}

/**
 * Produces a resized version of the specified picture. This works only for
 * SDL rendering. Check g_gr->caps().resize_surfaces before calling this
 * function.
 *
 * \param index position of the source picture in the stack
 * \param w target width
 * \param h target height
 * \return resized version of picture
 */
SDL_Surface * Graphic::resize
	(const PictureID index, uint32_t const w, uint32_t const h)
{
	Surface & orig = *g_gr->get_picture_surface(index);

	if (g_opengl)
		throw wexception("Graphic::resize() not yet implemented for opengl");
	else
	{
		SurfaceSDL & origsdl = dynamic_cast<SurfaceSDL &>(orig);
		return
			zoomSurface
				(origsdl.get_sdl_surface(),
				 double(w) / orig.get_w(), double(h) / orig.get_h(),
				 1);
	}
}


/**
 * Stores the picture size in pw and ph.
 * Throws an exception if the picture doesn't exist.
*/
void Graphic::get_picture_size
	(const PictureID & pic, uint32_t & w, uint32_t & h) const
{
	assert (pic->surface);
	Surface & bmp = *pic->surface;

	w = bmp.get_w();
	h = bmp.get_h();
}

/**
 * Saves a surface to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(Surface & surf, StreamWrite * sw) const
{
	// Save a png
	png_structp png_ptr =
		png_create_write_struct
			(PNG_LIBPNG_VER_STRING, static_cast<png_voidp>(0), 0, 0);

	if (!png_ptr)
		throw wexception("Graphic::save_png: could not create png struct");

	//  Set another write function. This is potentially dangerouse because the
	//  flush function is internally called by png_write_end(), this will crash
	//  on newer libpngs. See here:
	//     https://bugs.freedesktop.org/show_bug.cgi?id=17212
	//
	//  Simple solution is to define a dummy flush function which I did here.
	png_set_write_fn
		(png_ptr,
		 sw,
		 &Graphic::m_png_write_function, &Graphic::m_png_flush_function);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(0));
		throw wexception("Graphic::save_png: could not create png info struct");
	}

	// Set jump for error
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw wexception("Graphic::save_png: could not set png setjmp");
	}

	// Fill info struct
	png_set_IHDR
		(png_ptr, info_ptr, surf.get_w(), surf.get_h(),
		 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// png_set_strip_16(png_ptr) ;

	// Start writing
	png_write_info(png_ptr, info_ptr);

	// Strip data down
	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	png_set_packing(png_ptr);

	{
		uint32_t surf_w = surf.get_w();
		uint32_t surf_h = surf.get_h();
		uint32_t row_size = 4 * surf_w;

		png_bytep rowb = 0;
		png_bytep rowp = 0;

		//Write each row
		SDL_PixelFormat * fmt;
		upcast(SurfaceSDL, sdlsurf, &surf);
#ifdef USE_OPENGL
		upcast(SurfaceOpenGL, oglsurf, &surf);
#endif
		if (sdlsurf)
		{
			fmt = const_cast<SDL_PixelFormat *>(&sdlsurf->format());
			rowb = (new png_byte[row_size]);
			if (!rowb)
				throw wexception("Out of memory.");
		}
#ifdef USE_OPENGL
		else if (oglsurf) {
			oglsurf->lock();
			fmt = 0;
		}
#endif
		else
			return;


		// Write each row
		for (uint32_t y = 0; y < surf_h; ++y) {
			rowp = rowb;
			if (sdlsurf)
				for (uint32_t x = 0; x < surf_w; rowp += 4, ++x)
					SDL_GetRGBA
						(sdlsurf->get_pixel(x, y),
						 fmt,
						 rowp + 0, rowp + 1, rowp + 2, rowp + 3);
#ifdef USE_OPENGL
			else if (oglsurf) {
				rowb = static_cast<png_bytep>
					(oglsurf->get_pixels() +
					 oglsurf->get_pitch() * (surf_h - y - 1));
			}
#endif
			else
				throw wexception("Try to save save_png with unknown surface\n");

			png_write_row(png_ptr, rowb);
		}
#ifdef USE_OPENGL
		if (oglsurf)
			oglsurf->unlock();
		else
#endif
			if (sdlsurf)
				delete rowb;
	}

	// End write
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

/**
* Saves a PictureID to a png. This can be a file or part of a stream. This
* function retrieves the Surface for the PictureID and calls
* save_png(Surface, StreamWrite)
*
* @param surf The Surface to save
* @param sw a StreamWrite where the png is written to
*/
void Graphic::save_png(const PictureID & pic_index, StreamWrite * sw) const
{
	Surface & surf = const_cast<Surface &>(*get_picture_surface(pic_index));
	save_png(surf, sw);
}

/**
 * Create a offscreen surface of specified size and return as PictureID.
 * The surface is put into a normal slot in the picture array so the surface
 * can be used in normal blit() operations. A RenderTarget for the surface can
 * be obtained using get_surface_renderer().
 * \note Surfaces do not belong to a module and must be freed explicitly.
 *
 * @param w width of the new surface
 * @param h height of the new surface
 * @param alpha if true the surface is created with alpha channel
 * @return PictureID of the new created offscreen surface
*/
PictureID Graphic::create_picture_surface(int32_t w, int32_t h, bool alpha)
{
	//log(" Graphic::create_picture_surface(%d, %d)\n", w, h);
	Surface & surf = create_surface(w, h, alpha);

	Picture & pic = *new Picture();
	PictureID id = PictureID(&pic);
	m_picturemap[PicSurface].insert(std::make_pair("", id));

	pic.module    = PicSurface; // mark as surface
	pic.surface   = &surf;
	assert(pic.rendertarget == 0);
	pic.rendertarget = new RenderTarget(pic.surface);

	return id;
}

/**
 * Create a Surface from a SDL_Surface. This creates a Surface for OpenGL or
 * Software rendering depending on the actual setting. The SDL_Surface must
 * not be used after calling this. Surface takes care of the SDL_Surface.
 *
 * @param surf a SDL_Surface from which the Surface will be created
 * @param alpha if true the surface is created with alpha channel
 * @return the new Surface created from the SDL_Surface
 */
Surface & Graphic::create_surface(SDL_Surface & surf, bool alpha)
{
	if (g_opengl)
	{
#ifdef USE_OPENGL
		return *new SurfaceOpenGL(surf);
#endif
	} else {
		SDL_Surface * surface;
		if (alpha)
			surface = SDL_DisplayFormatAlpha(&surf);
		else
			surface = SDL_DisplayFormat(&surf);
		SDL_FreeSurface(&surf);
		return *new SurfaceSDL(*surface);
	}
}

/**
* Create a Surface from an other Surface. This makes a copy of a Surface.
*
* @param surf the surface which will be copied to the new surface
* @param alpha if true the surface is created with alpha channel
* @return the new Surface
*/
Surface & Graphic::create_surface(Surface & surf, bool alpha)
{
	upcast(SurfaceSDL, sdlsurf, &surf);
	if (sdlsurf)
	{
		if (alpha)
			return
				*new SurfaceSDL
					(*SDL_DisplayFormatAlpha(sdlsurf->get_sdl_surface()));
		else
			return
				*new SurfaceSDL
					(*SDL_DisplayFormat(sdlsurf->get_sdl_surface()));
	}
	else
	{
		Surface & tsurf = create_surface(surf.get_w(), surf.get_h());

		surf.lock();
		tsurf.lock();
		for (unsigned int x = 0; x < surf.get_w(); x++)
			for (unsigned int y = 0; y < surf.get_h(); y++)
			{
				tsurf.set_pixel(x, y, surf.get_pixel(x, y));
			}
		surf.unlock();
		tsurf.unlock();

		return tsurf;
	}
}

/**
* Create a empty offscreen surface of specified size.
*
* @param w width of the new surface
* @param h height of the new surface
* @param alpha if true the surface is created with alpha channel
* @return the new created surface
*/
Surface & Graphic::create_surface(int32_t w, int32_t h, bool alpha)
{
	if (g_opengl)
	{
#ifdef USE_OPENGL
		return *new SurfaceOpenGL(w, h);
#endif
	} else {
		const SDL_PixelFormat & format = m_screen->format();
		SDL_Surface & tsurf = *SDL_CreateRGBSurface
			(SDL_SWSURFACE,
			 w, h,
			 format.BitsPerPixel,
			 format.Rmask, format.Gmask, format.Bmask, format.Amask);
		if (alpha) {
			SDL_Surface & surf = *SDL_DisplayFormatAlpha(&tsurf);
			SDL_FreeSurface(&tsurf);
			return *new SurfaceSDL(surf);
		}
		return *new SurfaceSDL(tsurf);
	}
}

/**
 * Free the given surface.
 * Unlike normal pictures, surfaces are not freed by flush().
 *
 * @param picid the PictureID ot to be freed
*/
void Graphic::free_picture_surface(const PictureID & picid) {
	if
		(picid->module != PicMod_Font &&
		 picid->module != PicSurface)
	{
		log
			("Graphic::free_surface ignoring free of %u %s\n",
			 picid->module, picid->fname);
		return;
	}
	assert(picid->module == PicMod_Font || picid->module == PicSurface);

	delete picid->surface;
	picid->surface = 0;
	delete picid->rendertarget;
	picid->rendertarget = 0;
	delete picid->fname;
	picid->fname = 0;

	container_iterate(Picturemap, m_picturemap[picid->module], it)
		if (it.current->second == picid) {
			m_picturemap[picid->module].erase(it.current);
			break;
		}
}

/**
* create a grayed version.
*
* @param picid the PictureID ot to grayed out
* @return the gray version of the picture
*/
PictureID Graphic::create_grayed_out_pic(const PictureID & picid) {
	if (picid != get_no_picture()) {
		Surface & s = create_surface(*get_picture_surface(picid), true);
#ifdef USE_OPENGL
		upcast(SurfaceOpenGL, gl_dest, &s);
		upcast(SurfaceOpenGL, gl_src, get_picture_surface(picid));
#endif
		upcast(SurfaceSDL, sdl_s, &s);
		SDL_PixelFormat const * format = 0;
		if (sdl_s)
			format = &(sdl_s->format());
		uint32_t const w = s.get_w(), h = s.get_h();
#ifdef USE_OPENGL
		if (gl_src)
			gl_src->lock();
#endif
		s.lock();
		for (uint32_t y = 0; y < h; ++y)
			for (uint32_t x = 0; x < w; ++x) {
				uint8_t r, g, b, a;
#ifdef USE_OPENGL
				if (gl_src)
				{
					uint32_t pixel = gl_src->get_pixel(x, y);
					r = pixel & 0xFF;
					g = (pixel & 0xFF00) >> 8;
					b = (pixel & 0xFF0000) >> 16;
					a = (pixel & 0xFF000000) >> 24;
				} else
#endif
				//  FIXME need for const_cast is SDL bug #421
					SDL_GetRGBA
						(sdl_s->get_pixel(x, y),
						 const_cast<SDL_PixelFormat *>(format), &r, &g, &b, &a);

				//  Halve the opacity to give some difference for pictures that are
				//  grayscale to begin with.
				a >>= 1;

				uint8_t const gray =
					(luminance_table_r[r] +
					 luminance_table_g[g] +
					 luminance_table_b[b] +
					 8388608U) //  compensate for truncation:  .5 * 2^24
					>> 24;

				// NOTE const_cast is needed for SDL-1.2 older than revision 3008
#ifdef USE_OPENGL
				if (gl_dest)
				{
					gl_dest->set_pixel(x, y, g + (g << 8) + (g << 16) + (a << 24));
				} else
#endif
					sdl_s->set_pixel
						(x, y,
						 SDL_MapRGBA
						 	(const_cast<SDL_PixelFormat *>(format),
						 	 gray, gray, gray, a));

			}
		s.unlock();
#ifdef USE_OPENGL
		if (gl_src)
			gl_src->unlock();
#endif
		return get_picture(PicSurface, s);
	} else
		return get_no_picture();
}


/**
 * Returns the RenderTarget for the given surface
*/
RenderTarget * Graphic::get_surface_renderer(const PictureID & pic) {
	//assert(pic < m_pictures.size());
	//  assert(m_pictures[pic].module == 0xff); fails showing terrains in editor

	RenderTarget & rt = *pic->rendertarget;

	rt.reset();

	return &rt;
}

/**
 * Creates a terrain texture.
 *
 * fnametempl is a filename with possible wildcard characters '?'. The function
 * fills the wildcards with decimal numbers to get the different frames of a
 * texture animation. For example, if fnametempl is "foo_??.bmp", it tries
 * "foo_00.bmp", "foo_01.bmp" etc...
 * frametime is in milliseconds.
 * \return 0 if the texture couldn't be loaded.
 * \note Terrain textures are not reused, even if fnametempl matches.
 * These textures are freed when PicMod_Game is flushed.
*/
uint32_t Graphic::get_maptexture
	(const char & fnametempl, const uint32_t frametime)
{
	try {
		m_maptextures.push_back
			(new Texture(fnametempl, frametime, m_screen->format()));
	} catch (std::exception const & e) {
		log("Failed to load maptexture %s: %s\n", &fnametempl, e.what());
		return 0;
	}

	return m_maptextures.size(); // ID 1 is at m_maptextures[0]
}

/**
 * Advance frames for animated textures
*/
void Graphic::animate_maptextures(uint32_t time)
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i)
		m_maptextures[i]->animate(time);
}

/**
 * reset that the map texture have been animated
 */
void Graphic::reset_texture_animation_reminder()
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i)
		m_maptextures[i]->reset_was_animated();
}

/**
 * Load all animations that are registered with the AnimationManager
*/
void Graphic::load_animations(UI::ProgressWindow & loader_ui) {
	assert(m_animations.empty());

	clock_t start = clock();

	const std::string step_description = _("Loading animations: %d%% complete");
	uint32_t last_shown = 100;
	const uint32_t nr_animations = g_anim.get_nranimations();
	for (uint32_t id = 0; id < nr_animations;) {
		const uint32_t percent = 100 * id / nr_animations;
		if (percent != last_shown) {
			last_shown = percent;
			loader_ui.stepf(step_description.c_str(), percent);
		}
		++id;
		m_animations.push_back(new AnimationGfx(g_anim.get_animation(id)));
	}
	loader_ui.step(std::string());

	clock_t end = clock();
	printf
		("load_animations took %f seconds\n",
		 (float(end - start) / CLOCKS_PER_SEC));
}

/**
 * Return the number of frames in this animation
 */
AnimationGfx::Index Graphic::nr_frames(const uint32_t anim) const
{
	return get_animation(anim)->nr_frames();
}

/**
 * writes the size of an animation frame to w and h
*/
void Graphic::get_animation_size
	(uint32_t const anim, uint32_t const time, uint32_t & w, uint32_t & h) const
{
	AnimationData const * const data = g_anim.get_animation(anim);
	AnimationGfx  const * const gfx  =        get_animation(anim);

	if (!data || !gfx) {
		log("WARNING: Animation %u does not exist\n", anim);
		w = h = 0;
	} else {
		// Get the frame and its data. Ignore playerclrs.
		Surface const & frame =
			*gfx->get_frame((time / data->frametime) % gfx->nr_frames());
		w = frame.get_w();
		h = frame.get_h();
	}
}

/**
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const char & fname) const
{
	log("Save screenshot to %s\n", &fname);
	StreamWrite * sw = g_fs->OpenStreamWrite(std::string(&fname));
	save_png(*m_screen, sw);
	delete sw;
}

/**
 * A helper function for save_png.
 * Writes the compressed data to the StreamWrite.
 * @see save_png()
 */
void Graphic::m_png_write_function
	(png_structp png_ptr, png_bytep data, png_size_t length)
{
	static_cast<StreamWrite *>(png_get_io_ptr(png_ptr))->Data(data, length);
}

/**
* A helper function for save_png.
* Flush function to avoid crashes with default libpng flush function
* @see save_png()
*/
void Graphic::m_png_flush_function
	(png_structp png_ptr)
{
	static_cast<StreamWrite *>(png_get_io_ptr(png_ptr))->Flush();
}


/**
* Returns the bitmap that belongs to the given picture ID.
* May return 0 if the given picture does not exist.
*/
Surface * Graphic::get_picture_surface(const PictureID & id)
{
	return id->surface;
}

const Surface * Graphic::get_picture_surface(const PictureID & id) const
{
	return id->surface;
}

/**
 * Retrieve the animation with the given number.
 *
 * @param anim the number of the animation
 * @return the AnimationGfs object of the given number
*/
AnimationGfx * Graphic::get_animation(uint32_t const anim) const
{
	if (!anim || anim > m_animations.size())
		return 0;

	return m_animations[anim - 1];
}

/**
 * Retrieve the map texture with the given number
 * \return the actual texture data associated with the given ID.
*/
Texture * Graphic::get_maptexture_data(uint32_t id)
{
	--id; // ID 1 is at m_maptextures[0]
	if (id < m_maptextures.size())
		return m_maptextures[id];
	else
		return 0;
}

/**
 * Retrives the texture of the road type. This loads the road texture
 * if not done yet.
 * \return The road texture
*/
Surface * Graphic::get_road_texture(int32_t const roadtex)
{
	if (not m_roadtextures) {
		// Load the road textures
		m_roadtextures = new Road_Textures();
		m_roadtextures->pic_road_normal =
			get_picture(PicMod_Game, ROAD_NORMAL_PIC, false);

		m_roadtextures->pic_road_busy =
			get_picture(PicMod_Game, ROAD_BUSY_PIC, false);
	}

	return
		get_picture_surface
			(roadtex == Widelands::Road_Normal ?
			 m_roadtextures->pic_road_normal : m_roadtextures->pic_road_busy);
}
