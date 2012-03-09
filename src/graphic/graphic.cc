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
#include "picture.h"
#include "rendertarget.h"
#include "texture.h"

#include "render/surface_sdl.h"
#include "render/gl_picture_texture.h"
#include "render/gl_surface_screen.h"

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
	m_caps.offscreen_rendering = true;

#ifdef USE_OPENGL
	if (0 != (sdlsurface->flags & SDL_OPENGL)) {
		//  We have successful opened an opengl screen. Print some information
		//  about opengl and set the rendering capabilities.
		log ("Graphics: OpenGL: OpenGL enabled\n");

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			log("glewInit returns %i\nYour OpenGL installation must be __very__ broken.\n", err);
			throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
		}

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

		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &glInt);
		log("Graphics: OpenGL: Maximum number of textures for multitextures: %u\n", glInt);
		m_caps.gl.max_tex_combined = glInt;

		str = reinterpret_cast<const char *>(glGetString(GL_VERSION));
		m_caps.gl.major_version = atoi(str);
		m_caps.gl.minor_version = strstr(str, ".")?atoi(strstr(str, ".") + 1):0;
		log
			("Graphics: OpenGL: Version %d.%d \"%s\"\n",
			 m_caps.gl.major_version, m_caps.gl.minor_version, str);

		const char * extensions = reinterpret_cast<const char *>(glGetString (GL_EXTENSIONS));

		m_caps.gl.tex_power_of_two =
			(m_caps.gl.major_version < 2) and
			(strstr(extensions, "GL_ARB_texture_non_power_of_two") == 0);
		log("Graphics: OpenGL: Textures ");
		log
			(m_caps.gl.tex_power_of_two?"must have a size power of two\n":
			 "may have any size\n");

		m_caps.gl.multitexture =
			 ((strstr(extensions, "GL_ARB_multitexture") != 0) and
			  (strstr(extensions, "GL_ARB_texture_env_combine") != 0))
			and (m_caps.gl.max_tex_combined >= 6);
		log("Graphics: OpenGL: Multitexture capabilities ");
		log(m_caps.gl.multitexture ? "sufficient\n" : "insufficient, only basic terrain rendering possible\n");

		m_caps.offscreen_rendering = false;

		m_caps.gl.blendequation = GLEW_VERSION_1_4 || GLEW_ARB_imaging;
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
	{
		m_screen.reset(new GLSurfaceScreen(w, h));
	}
	else
#endif
	{
		boost::shared_ptr<SurfaceSDL> screen(new SurfaceSDL(*sdlsurface));
		screen->set_isscreen(true);
		m_screen = screen;
	}

	m_sdl_screen = sdlsurface;
	m_rendertarget = new RenderTarget(m_screen);
}

/**
 * Free the surface
*/
Graphic::~Graphic()
{
	delete m_rendertarget;
	delete m_roadtextures;

	// Remove traces of cached pictures
	UI::g_fh->flush_cache();
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
 * Clear all cached resources from the given module.
 *
 * \note This only removes the cache entries. If the corresonding resources
 * are still in use somewhere, they will not be freed.
 */
void Graphic::flush(PicMod const module)
{
	std::vector<std::string> eraselist;

	for (pmit it = m_picturemap.begin(); it != m_picturemap.end(); ++it) {
		it->second.modules &= ~(1 << module);
		if (!it->second.modules)
			eraselist.push_back(it->first);
	}

	while (!eraselist.empty()) {
		m_picturemap.erase(eraselist.back());
		eraselist.pop_back();
	}

	if (module == PicMod_UI) // Flush the cached Fontdatas
		UI::g_fh->flush_cache();
}


/// flushes the animations in m_animations
void Graphic::flush_animations() {
	container_iterate_const(std::vector<AnimationGfx *>, m_animations, i)
		delete *i.current;
	m_animations.clear();
}


PictureID Graphic::load_image(std::string const & fname, bool const alpha) {
	//log("Graphic::LoadImage(\"%s\")\n", fname.c_str());
	FileRead fr;
	SDL_Surface * sdlsurf;

	//fastOpen tries to use mmap
	fr.fastOpen(*g_fs, fname.c_str());

	sdlsurf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

	if (!sdlsurf)
		throw wexception("%s", IMG_GetError());

	return convert_sdl_surface_to_picture(sdlsurf, alpha);
}

/**
 * Retrieves the picture ID of the picture with the given filename.
 * If the picture has already been loaded, the old ID is reused.
 * The picture is placed into the module(s) given by mod.
 *
 * \return 0 (a null-picture) if the picture cannot be loaded.
*/
const PictureID & Graphic::get_picture
	(PicMod const module, const std::string & fname, bool alpha)
{
	//  Check if the picture is already loaded.
	pmit it = m_picturemap.find(fname);

	if (it == m_picturemap.end()) {
		PictureRec rec;

		try {
			rec.picture = load_image(fname, alpha);
			rec.modules = 0;
			//log("Graphic::get_picture(): loading picture '%s'\n", fname.c_str());
		} catch (std::exception const & e) {
			log("WARNING: Could not open %s: %s\n", fname.c_str(), e.what());
			return get_no_picture();
		}

		it = m_picturemap.insert(std::make_pair(fname, rec)).first;
	}

	it->second.modules |= 1 << module;
	return it->second.picture;
}

/**
 * Add the given picture to the cache under the given name.
 *
 * This overwrites pre-existing cache entries, if any.
 */
void Graphic::add_picture_to_cache(PicMod module, const std::string & name, PictureID pic)
{
	PictureRec rec;
	rec.picture = pic;
	rec.modules = 1 << module;
	m_picturemap.insert(std::make_pair(name, rec));
}

/**
 * \return an empty, invalid, null picture
 */
const PictureID & Graphic::get_no_picture() const
{
	return IPicture::null();
}

/**
 * Produces a resized version of the specified picture
 *
 * Might return same id if dimensions are the same
 */
PictureID Graphic::get_resized_picture
	(PictureID src,
	 uint32_t const w, uint32_t const h,
	 ResizeMode const mode)
{
	if (src->get_w() == w and src->get_h() == h)
		return src;

	// First step: compute scaling factors
	Rect srcrect;
	Rect destrect;

	if (mode == ResizeMode_Loose) {
		srcrect = Rect(Point(0, 0), src->get_w(), src->get_h());
		destrect = Rect(Point(0, 0), w, h);
	} else {
		const double ratio_x = double(w) / src->get_w();
		const double ratio_y = double(h) / src->get_h();
		double ratio;

		if (ResizeMode_Clip == mode)
			ratio = std::max(ratio_x, ratio_y);
		else if (ResizeMode_LeaveBorder == mode)
			ratio = std::min(ratio_x, ratio_y);
		else // average
			ratio = (ratio_x + ratio_y) / 2;

		uint32_t fullwidth(src->get_w() * ratio);
		uint32_t fullheight(src->get_h() * ratio);

		if (fullwidth <= w) {
			srcrect.x = 0;
			srcrect.w = src->get_w();
			destrect.x = (w - fullwidth) / 2;
			destrect.w = fullwidth;
		} else {
			srcrect.w = std::min(src->get_w(), uint32_t(w / ratio));
			srcrect.x = (src->get_w() - srcrect.w) / 2;
			destrect.x = 0;
			destrect.w = w;
		}

		if (fullheight <= h) {
			srcrect.y = 0;
			srcrect.h = src->get_h();
			destrect.y = (h - fullheight) / 2;
			destrect.h = fullheight;
		} else {
			srcrect.h = std::min(src->get_h(), uint32_t(h / ratio));
			srcrect.y = (src->get_h() - srcrect.h) / 2;
			destrect.y = 0;
			destrect.h = h;
		}
	}

	// Second step: get source material
	SDL_Surface * srcsdl = 0;
	bool free_source = true;

	if (upcast(SurfaceSDL, srcsurf, src.get())) {
		if
			(srcrect.x != 0 || srcrect.w != uint32_t(srcsurf->get_w()) ||
			 srcrect.y != 0 || srcrect.h != uint32_t(srcsurf->get_h()))
		{
			const SDL_PixelFormat & fmt = *srcsurf->get_sdl_surface()->format;
			srcsdl = SDL_CreateRGBSurface
				(SDL_SWSURFACE, srcrect.w, srcrect.h,
				 fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
			SDL_Rect srcrc = {srcrect.x, srcrect.y, srcrect.w, srcrect.h};
			SDL_Rect dstrc = {0, 0, 0, 0};
			bool alpha = srcsurf->get_sdl_surface()->flags & SDL_SRCALPHA;
			uint8_t alphaval = srcsurf->get_sdl_surface()->format->alpha;
			SDL_SetAlpha(srcsurf->get_sdl_surface(), 0, 0);
			SDL_BlitSurface(srcsurf->get_sdl_surface(), &srcrc, srcsdl, &dstrc);
			SDL_SetAlpha(srcsurf->get_sdl_surface(), alpha ? SDL_SRCALPHA : 0, alphaval);
		} else {
			srcsdl = srcsurf->get_sdl_surface();
			free_source = false;
		}
	} else {
		srcsdl = extract_sdl_surface(src->pixelaccess(), srcrect);
	}

	// Third step: perform the zoom and placement
	SDL_Surface * zoomed = zoomSurface
		(srcsdl, double(destrect.w) / srcsdl->w, double(destrect.h) / srcsdl->h, 1);
	if (free_source)
		SDL_FreeSurface(srcsdl);

	if (uint32_t(zoomed->w) != w || uint32_t(zoomed->h) != h) {
		const SDL_PixelFormat & fmt = *zoomed->format;
		SDL_Surface * placed = SDL_CreateRGBSurface
			(SDL_SWSURFACE, w, h,
			 fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
		SDL_Rect srcrc = {0, 0, zoomed->w, zoomed->h};
		SDL_Rect dstrc = {destrect.x, destrect.y};
		SDL_SetAlpha(zoomed, 0, 0);
		SDL_BlitSurface(zoomed, &srcrc, placed, &dstrc);

		Uint32 fillcolor = SDL_MapRGBA(zoomed->format, 0, 0, 0, 255);

		if (destrect.x > 0) {
			dstrc.x = 0;
			dstrc.y = destrect.y;
			dstrc.w = destrect.x;
			dstrc.h = zoomed->h;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}
		if (destrect.x + zoomed->w < placed->w) {
			dstrc.x = destrect.x + zoomed->w;
			dstrc.y = destrect.y;
			dstrc.w = placed->w - destrect.x - zoomed->w;
			dstrc.h = zoomed->h;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}
		if (destrect.y > 0) {
			dstrc.x = 0;
			dstrc.y = 0;
			dstrc.w = placed->w;
			dstrc.h = destrect.y;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}
		if (destrect.y + zoomed->h < placed->h) {
			dstrc.x = 0;
			dstrc.y = destrect.y + zoomed->h;
			dstrc.w = placed->w;
			dstrc.h = placed->h - destrect.y - zoomed->h;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}

		SDL_FreeSurface(zoomed);
		zoomed = placed;
	}

	return convert_sdl_surface_to_picture(zoomed);
}

/**
 * Create and return an \ref SDL_Surface that contains the given sub-rectangle
 * of the given pixel region.
 */
SDL_Surface * Graphic::extract_sdl_surface(IPixelAccess & pix, Rect srcrect)
{
	assert(srcrect.x >= 0);
	assert(srcrect.y >= 0);
	assert(srcrect.x + srcrect.w <= pix.get_w());
	assert(srcrect.y + srcrect.h <= pix.get_h());

	const SDL_PixelFormat & fmt = pix.format();
	SDL_Surface * dest = SDL_CreateRGBSurface
		(SDL_SWSURFACE, srcrect.w, srcrect.h,
		 fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);

	pix.lock(IPixelAccess::Lock_Normal);
	SDL_LockSurface(dest);

	uint32_t srcpitch = pix.get_pitch();
	uint32_t rowsize = srcrect.w * fmt.BytesPerPixel;
	uint8_t * srcpix = pix.get_pixels() + srcpitch * srcrect.y + fmt.BytesPerPixel * srcrect.x;
	uint8_t * dstpix = static_cast<uint8_t *>(dest->pixels);

	for (uint32_t y = 0; y < srcrect.h; ++y) {
		memcpy(dstpix, srcpix, rowsize);
		srcpix += srcpitch;
		dstpix += dest->pitch;
	}

	SDL_UnlockSurface(dest);
	pix.unlock(IPixelAccess::Unlock_NoChange);

	return dest;
}

/**
 * Stores the picture size in pw and ph.
 * Throws an exception if the picture doesn't exist.
*/
void Graphic::get_picture_size
	(const PictureID & pic, uint32_t & w, uint32_t & h) const
{
	w = pic->get_w();
	h = pic->get_h();
}

/**
 * This is purely a convenience function intended to allow casting
 * pointers without including a whole bunch of headers.
 */
PictureID Graphic::get_offscreen_picture(OffscreenSurfacePtr surface) const
{
	return surface;
}


/**
 * Saves a pixel region to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(IPixelAccess & pix, StreamWrite * sw) const
{
	// Save a png
	png_structp png_ptr =
		png_create_write_struct
			(PNG_LIBPNG_VER_STRING, static_cast<png_voidp>(0), 0, 0);

	if (!png_ptr)
		throw wexception("Graphic::save_png: could not create png struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(0));
		throw wexception("Graphic::save_png: could not create png info struct");
	}

	// Set jump for error
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw wexception("Graphic::save_png: Error writing PNG!");
	}

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

	// Fill info struct
	png_set_IHDR
		(png_ptr, info_ptr, pix.get_w(), pix.get_h(),
		 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Start writing
	png_write_info(png_ptr, info_ptr);
	{
		uint32_t surf_w = pix.get_w();
		uint32_t surf_h = pix.get_h();
		uint32_t row_size = 4 * surf_w;

		boost::scoped_array<png_byte> row(new png_byte[row_size]);

		//Write each row
		const SDL_PixelFormat & fmt = pix.format();
		pix.lock(IPixelAccess::Lock_Normal);

		// Write each row
		for (uint32_t y = 0; y < surf_h; ++y) {
			for (uint32_t x = 0; x < surf_w; ++x) {
				RGBAColor color;
				color.set(fmt, pix.get_pixel(x, y));
				row[4 * x] = color.r;
				row[4 * x + 1] = color.g;
				row[4 * x + 2] = color.b;
				row[4 * x + 3] = color.a;
			}

			png_write_row(png_ptr, row.get());
		}

		pix.unlock(IPixelAccess::Unlock_NoChange);
	}

	// End write
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

/**
 * Saves a surface to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(SurfacePtr surf, StreamWrite * sw) const
{
	save_png(surf->pixelaccess(), sw);
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
	save_png(pic_index->pixelaccess(), sw);
}

/**
 * Create a Picture from an SDL_Surface.
 *
 * @param surf a SDL_Surface from which the Surface will be created; this function
 * takes ownership of surf
 * @param alpha if true the surface is created with alpha channel
 * @return the new Surface created from the SDL_Surface
 */
PictureID Graphic::convert_sdl_surface_to_picture(SDL_Surface * surf, bool alpha)
{
	if (g_opengl)
	{
#ifdef USE_OPENGL
		return PictureID(new GLPictureTexture(surf));
#endif
	} else {
		SDL_Surface * surface;
		if (alpha)
			surface = SDL_DisplayFormatAlpha(surf);
		else
			surface = SDL_DisplayFormat(surf);
		SDL_FreeSurface(surf);
		return PictureID(new SurfaceSDL(*surface));
	}
}

/**
 * Create a empty offscreen surface of specified size.
 *
 * \note Offscreen surfaces with an alpha channel are not supported due to
 * limitations in the SDL blitter.
 *
 * @param w width of the new surface
 * @param h height of the new surface
 * @return the new created surface
 */
OffscreenSurfacePtr Graphic::create_offscreen_surface(int32_t w, int32_t h)
{
#ifdef USE_OPENGL
	if (g_opengl)
	{
		throw wexception("OpenGL mode does not support offscreen surfaces");
	}
	else
#endif
	{
		const SDL_PixelFormat & format = *m_sdl_screen->format;
		SDL_Surface & tsurf = *SDL_CreateRGBSurface
			(SDL_SWSURFACE,
			 w, h,
			 format.BitsPerPixel,
			 format.Rmask, format.Gmask, format.Bmask, format.Amask);
		return OffscreenSurfacePtr(new SurfaceSDL(tsurf));
	}
}

/**
 * Create a picture with initially undefined contents.
 *
 * Use \ref IPicture::pixelaccess to upload image data afterwards.
 *
 * @param w width of the new surface
 * @param h height of the new surface
 * @param alpha if true the surface is created with alpha channel
 * @return the new created surface
 */
PictureID Graphic::create_picture(int32_t w, int32_t h, bool alpha)
{
#ifdef USE_OPENGL
	if (g_opengl)
	{
		return PictureID(new GLPictureTexture(w, h));
	}
	else
#endif
	{
		const SDL_PixelFormat & format = *m_sdl_screen->format;
		SDL_Surface & tsurf = *SDL_CreateRGBSurface
			(SDL_SWSURFACE,
			 w, h,
			 format.BitsPerPixel,
			 format.Rmask, format.Gmask, format.Bmask, format.Amask);
		if (alpha) {
			SDL_Surface & surf = *SDL_DisplayFormatAlpha(&tsurf);
			SDL_FreeSurface(&tsurf);
			return OffscreenSurfacePtr(new SurfaceSDL(surf));
		}
		return PictureID(new SurfaceSDL(tsurf));
	}
}


/**
 * Create a grayed version of the given picture.
 *
 * @param picid the PictureID ot to grayed out
 * @return the gray version of the picture
 */
PictureID Graphic::create_grayed_out_pic(const PictureID & picid)
{
	if (!picid || !picid->valid())
		return get_no_picture();

	IPixelAccess & origpix = picid->pixelaccess();
	uint32_t w = picid->get_w();
	uint32_t h = picid->get_h();
	const SDL_PixelFormat & origfmt = origpix.format();

	PictureID destpicture = create_picture(w, h, origfmt.Amask);
	IPixelAccess & destpix = destpicture->pixelaccess();
	const SDL_PixelFormat & destfmt = destpix.format();

	origpix.lock(IPixelAccess::Lock_Normal);
	destpix.lock(IPixelAccess::Lock_Discard);
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			RGBAColor color;

			color.set(origfmt, origpix.get_pixel(x, y));

			//  Halve the opacity to give some difference for pictures that are
			//  grayscale to begin with.
			color.a >>= 1;

			uint8_t const gray =
				(luminance_table_r[color.r] +
				 luminance_table_g[color.g] +
				 luminance_table_b[color.b] +
				 8388608U) //  compensate for truncation:  .5 * 2^24
				>> 24;

			color.r = color.g = color.b = gray;

			destpix.set_pixel(x, y, color.map(destfmt));
		}
	}
	origpix.unlock(IPixelAccess::Unlock_NoChange);
	destpix.unlock(IPixelAccess::Unlock_Update);

	return destpicture;
}

/**
 * Creates an picture with changed luminosity from the given picture.
 *
 * @param picid the PictureID of the picture to modify
 * @param factor the factor the luminosity should be changed by
 * @param half_alpha whether the opacity should be halved or not
 * @return a new picture with 50% luminosity
 */
PictureID Graphic::create_changed_luminosity_pic
	(const PictureID & picid, const float factor, const bool halve_alpha)
{
	if (!picid || !picid->valid())
		return get_no_picture();

	IPixelAccess & origpix = picid->pixelaccess();
	uint32_t w = picid->get_w();
	uint32_t h = picid->get_h();
	const SDL_PixelFormat & origfmt = origpix.format();

	PictureID destpicture = create_picture(w, h, origfmt.Amask);
	IPixelAccess & destpix = destpicture->pixelaccess();
	const SDL_PixelFormat & destfmt = destpix.format();

	origpix.lock(IPixelAccess::Lock_Normal);
	destpix.lock(IPixelAccess::Lock_Discard);
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			RGBAColor color;

			color.set(origfmt, origpix.get_pixel(x, y));

			if (halve_alpha)
				color.a >>= 1;

			color.r = color.r * factor > 255 ? 255 : color.r * factor;
			color.g = color.g * factor > 255 ? 255 : color.g * factor;
			color.b = color.b * factor > 255 ? 255 : color.b * factor;

			destpix.set_pixel(x, y, color.map(destfmt));
		}
	}
	origpix.unlock(IPixelAccess::Unlock_NoChange);
	destpix.unlock(IPixelAccess::Unlock_Update);

	return destpicture;
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
			(new Texture(fnametempl, frametime, *m_sdl_screen->format));
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
	for (uint32_t i = 0; i < m_maptextures.size(); ++i) {
		m_maptextures[i]->animate(time);
	}
}

/**
 * reset that the map texture have been animated
 */
void Graphic::reset_texture_animation_reminder()
{
	for (uint32_t i = 0; i < m_maptextures.size(); ++i) {
		m_maptextures[i]->reset_was_animated();
	}
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
		const PictureID & frame =
			gfx->get_frame((time / data->frametime) % gfx->nr_frames());
		w = frame->get_w();
		h = frame->get_h();
	}
}

/**
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const char & fname) const
{
	log("Save screenshot to %s\n", &fname);
	StreamWrite * sw = g_fs->OpenStreamWrite(std::string(&fname));
	save_png(m_screen, sw);
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
 * Sets the name of the current world and loads the fitting road and edge textures
 */
void Graphic::set_world(std::string worldname) {
	char buf[255];

	if (m_roadtextures)
		delete m_roadtextures;

	// Load the road textures
	m_roadtextures = new Road_Textures();
	snprintf(buf, sizeof(buf), "worlds/%s/pics/roadt_normal.png", worldname.c_str());
	m_roadtextures->pic_road_normal = get_picture(PicMod_Game, buf, false);
	snprintf(buf, sizeof(buf), "worlds/%s/pics/roadt_busy.png", worldname.c_str());
	m_roadtextures->pic_road_busy = get_picture(PicMod_Game, buf, false);

	// load edge texture
	snprintf(buf, sizeof(buf), "worlds/%s/pics/edge.png", worldname.c_str());
	m_edgetexture = get_picture(PicMod_Game, buf, false);
}

/**
 * Retrives the texture of the road type. This loads the road texture
 * if not done yet.
 * \return The road texture
 */
PictureID Graphic::get_road_texture(int32_t const roadtex)
{
	return
		(roadtex == Widelands::Road_Normal ? m_roadtextures->pic_road_normal : m_roadtextures->pic_road_busy);
}

/**
 * Returns the alpha mask texture for edges.
 * \return The edge texture (alpha mask)
 */
PictureID Graphic::get_edge_texture()
{
	return m_edgetexture;
}
