/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "graphic/graphic.h"

#include <cstring>
#include <iostream>

#include <SDL_image.h>
#include <boost/foreach.hpp>
#include <config.h>

#include "build_info.h"
#include "compile_diagnostics.h"
#include "constants.h"
#include "container_iterate.h"
#include "graphic/animation.h"
#include "graphic/diranimations.h"
#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/image_loader_impl.h"
#include "graphic/image_transformations.h"
#include "graphic/render/gl_surface_screen.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/rendertarget.h"
#include "graphic/surface_cache.h"
#include "graphic/texture.h"
#include "i18n.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "log.h"
#include "logic/roadtype.h"
#include "logic/widelands_fileread.h"
#include "ui_basic/progresswindow.h"
#include "upcast.h"
#include "wexception.h"

using namespace std;

Graphic * g_gr;
bool g_opengl;

/**
 * Initialize the SDL video mode.
*/
Graphic::Graphic()
	:
	m_fallback_settings_in_effect (false),
	m_nr_update_rects  (0),
	m_update_fullscreen(true),
	image_loader_(new ImageLoaderImpl()),
	surface_cache_(create_surface_cache(TRANSIENT_SURFACE_CACHE_SIZE)),
	image_cache_(create_image_cache(image_loader_.get(), surface_cache_.get())),
	animation_manager_(new AnimationManager())
{
	ImageTransformations::initialize();

	//fastOpen tries to use mmap
	FileRead fr;
#ifndef _WIN32
	fr.fastOpen(*g_fs, "pics/wl-ico-128.png");
#else
	fr.fastOpen(*g_fs, "pics/wl-ico-32.png");
#endif
	SDL_Surface * s = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
	SDL_WM_SetIcon(s, nullptr);
	SDL_FreeSurface(s);
}

void Graphic::initialize(int32_t w, int32_t h, bool fullscreen, bool opengl) {
	cleanup();

	// Set video mode using SDL. First collect the flags
	int32_t flags = 0;
	g_opengl = false;
	SDL_Surface * sdlsurface = nullptr;

	if (opengl) {
		log("Graphics: Trying opengl\n");
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		flags |= SDL_OPENGL;
	}

	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
		log("Graphics: Trying FULLSCREEN\n");
	}

	log("Graphics: Try to set Videomode %ux%u 32 Bit\n", w, h);
	// Here we actually set the video mode
	sdlsurface = SDL_SetVideoMode(w, h, 32, flags);

	// If we tried opengl and it was not successful try without opengl
	if (!sdlsurface and opengl)
	{
		log("Graphics: Could not set videomode: %s, trying without opengl\n", SDL_GetError());
		flags &= ~SDL_OPENGL;
		sdlsurface = SDL_SetVideoMode(w, h, 32, flags);
	}

	if (!sdlsurface)
	{
		log
			("Graphics: Could not set videomode: %s, trying minimum graphics settings\n",
			 SDL_GetError());
		flags &= ~SDL_FULLSCREEN;
		sdlsurface = SDL_SetVideoMode
			(FALLBACK_GRAPHICS_WIDTH, FALLBACK_GRAPHICS_HEIGHT, FALLBACK_GRAPHICS_DEPTH, flags);
		m_fallback_settings_in_effect = true;
		if (!sdlsurface)
			throw wexception
				("Graphics: could not set video mode: %s", SDL_GetError());
	}

	// setting the videomode was successful. Print some information now
	log("Graphics: Setting video mode was successful\n");

	if (opengl and 0 != (sdlsurface->flags & SDL_GL_DOUBLEBUFFER))
		log("Graphics: OPENGL DOUBLE BUFFERING ENABLED\n");
	if (0 != (sdlsurface->flags & SDL_FULLSCREEN))
		log("Graphics: FULLSCREEN ENABLED\n");

	bool use_arb = true;
	const char * extensions;

	if (0 != (sdlsurface->flags & SDL_OPENGL)) {
		//  We have successful opened an opengl screen. Print some information
		//  about opengl and set the rendering capabilities.
		log ("Graphics: OpenGL: OpenGL enabled\n");

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			log("glewInit returns %i\nYour OpenGL installation must be __very__ broken.\n", err);
			throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
		}

		extensions = reinterpret_cast<const char *>(glGetString (GL_EXTENSIONS));

		if (strstr(extensions, "GL_ARB_framebuffer_object") != nullptr) {
			use_arb = true;
		} else if (strstr(extensions, "GL_EXT_framebuffer_object") != nullptr) {
			use_arb = false;
		} else {
			log
			("Graphics: Neither GL_ARB_framebuffer_object or GL_EXT_framebuffer_object supported! "
			"Switching off OpenGL!\n"
			);
			flags &= ~SDL_OPENGL;
			m_fallback_settings_in_effect = true;

			// One must never free the screen surface of SDL (using
			// SDL_FreeSurface) as it is owned by SDL itself, therefore the next
			// call does not leak memory.
			sdlsurface = SDL_SetVideoMode
				(FALLBACK_GRAPHICS_WIDTH, FALLBACK_GRAPHICS_HEIGHT, FALLBACK_GRAPHICS_DEPTH, flags);
			m_fallback_settings_in_effect = true;
			if (!sdlsurface)
				throw wexception("Graphics: could not set video mode: %s", SDL_GetError());
		}
	}

	// Redoing the check, because fallback settings might mean we no longer use OpenGL.
	if (0 != (sdlsurface->flags & SDL_OPENGL)) {
		//  We now really have a working opengl screen...
		g_opengl = true;

		GLboolean glBool;
		glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
		log
			("Graphics: OpenGL: Double buffering %s\n",
			 (glBool == GL_TRUE)?"enabled":"disabled");

		GLint glInt;
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

		const char * str = reinterpret_cast<const char *>(glGetString(GL_VERSION));
		m_caps.gl.major_version = atoi(str);
		m_caps.gl.minor_version = strstr(str, ".")?atoi(strstr(str, ".") + 1):0;
		log
			("Graphics: OpenGL: Version %d.%d \"%s\"\n",
			 m_caps.gl.major_version, m_caps.gl.minor_version, str);

		// extensions will be valid if we ever succeeded in runnning glewInit.
		m_caps.gl.tex_power_of_two =
			(m_caps.gl.major_version < 2) and
			(strstr(extensions, "GL_ARB_texture_non_power_of_two") == nullptr);
		log("Graphics: OpenGL: Textures ");
		log
			(m_caps.gl.tex_power_of_two?"must have a size power of two\n":
			 "may have any size\n");

		m_caps.gl.multitexture =
			 ((strstr(extensions, "GL_ARB_multitexture") != nullptr) and
			  (strstr(extensions, "GL_ARB_texture_env_combine") != nullptr));
		log("Graphics: OpenGL: Multitexture capabilities ");
		log(m_caps.gl.multitexture ? "sufficient\n" : "insufficient, only basic terrain rendering possible\n");

GCC_DIAG_OFF("-Wold-style-cast")
		m_caps.gl.blendequation = GLEW_VERSION_1_4 || GLEW_ARB_imaging;
GCC_DIAG_ON ("-Wold-style-cast")
	}

	/* Information about the video capabilities. */
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
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

	if (g_opengl) {
		glViewport(0, 0, w, h);

		// Set up OpenGL projection matrix. This transforms opengl coordinates to
		// screen coordinates. We set up a simple Orthogonal view which takes just
		// the x, y coordinates and ignores the z coordinate. Note that the top and
		// bottom values are interchanged. This is to invert the y axis to get the
		// same coordinates as with opengl. The exact values of near and far
		// clipping plane are not important. We draw everything with z = 0. They
		// just must not be null and have different sign.
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
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
		glEnable(GL_TEXTURE_2D);

		GLSurfaceTexture::Initialize(use_arb);
	}

	if (g_opengl)
	{
		screen_.reset(new GLSurfaceScreen(w, h));
	}
	else
	{
		screen_.reset(new SDLSurface(sdlsurface, false));
	}

	m_sdl_screen = sdlsurface;
	m_rendertarget.reset(new RenderTarget(screen_.get()));
}

bool Graphic::check_fallback_settings_in_effect()
{
	return m_fallback_settings_in_effect;
}

void Graphic::cleanup() {
	flush_maptextures();
	surface_cache_->flush();
	// TODO: this should really not be needed, but currently is :(
	if (UI::g_fh)
		UI::g_fh->flush();

	if (g_opengl)
		GLSurfaceTexture::Cleanup();
}

Graphic::~Graphic()
{
	cleanup();
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_xres()
{
	return screen_->width();
}

/**
 * Return the screen x resolution
*/
int32_t Graphic::get_yres()
{
	return screen_->height();
}

bool Graphic::is_fullscreen()
{
	return m_sdl_screen->flags & SDL_FULLSCREEN;
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget * Graphic::get_render_target()
{
	m_rendertarget->reset();

	return m_rendertarget.get();
}

/**
 * Switch from fullscreen to windowed mode or vice-versa
*/
void Graphic::toggle_fullscreen()
{
	log("Try SDL_WM_ToggleFullScreen...\n");
	// TODO: implement proper fullscreening here. The way it can work is to
	// recreate SurfaceCache but keeping ImageCache around. Then exiting and
	// reinitalizing the SDL Video Mode should just work: all surface are
	// recreated dynamically and correctly.
	// Note: Not all Images are cached in the ImageCache, at time of me writing
	// this, only InMemoryImage does not safe itself in the ImageCache. And this
	// should only be a problem for Images loaded from maps.
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
	if (g_opengl) {
		SDL_GL_SwapBuffers();
		m_update_fullscreen = false;
		m_nr_update_rects = 0;
		return;
	}

	if (force or m_update_fullscreen) {
		//flip defaults to SDL_UpdateRect(m_surface, 0, 0, 0, 0);
		SDL_Flip(m_sdl_screen);
	} else
		SDL_UpdateRects
			(m_sdl_screen, m_nr_update_rects, m_update_rects);

	m_update_fullscreen = false;
	m_nr_update_rects = 0;
}


/**
 * Saves a pixel region to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(const Image* image, StreamWrite * sw) const {
	save_png_(*image->surface(), sw);
}
void Graphic::save_png_(Surface & surf, StreamWrite * sw) const
{
	// Save a png
	png_structp png_ptr =
		png_create_write_struct
			(PNG_LIBPNG_VER_STRING, static_cast<png_voidp>(nullptr), nullptr, nullptr);

	if (!png_ptr)
		throw wexception("Graphic::save_png: could not create png struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(nullptr));
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
		(png_ptr, info_ptr, surf.width(), surf.height(),
		 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Start writing
	png_write_info(png_ptr, info_ptr);
	{
		uint16_t surf_w = surf.width();
		uint16_t surf_h = surf.height();
		uint32_t row_size = 4 * surf_w;

		std::unique_ptr<png_byte[]> row(new png_byte[row_size]);

		//Write each row
		const SDL_PixelFormat & fmt = surf.format();
		surf.lock(Surface::Lock_Normal);

		// Write each row
		for (uint32_t y = 0; y < surf_h; ++y) {
			for (uint32_t x = 0; x < surf_w; ++x) {
				RGBAColor color;
				color.set(fmt, surf.get_pixel(x, y));
				row[4 * x] = color.r;
				row[4 * x + 1] = color.g;
				row[4 * x + 2] = color.b;
				row[4 * x + 3] = color.a;
			}

			png_write_row(png_ptr, row.get());
		}

		surf.unlock(Surface::Unlock_NoChange);
	}

	// End write
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void Graphic::flush_maptextures()
{
	BOOST_FOREACH(Texture* texture, m_maptextures)
		delete texture;
	m_maptextures.clear();
}

/**
 * Creates a terrain texture.
 *
 * fnametempl is a filename with possible wildcard characters '?'. The function
 * fills the wildcards with decimal numbers to get the different frames of a
 * texture animation. For example, if fnametempl is "foo_??.png", it tries
 * "foo_00.png", "foo_01.png" etc...
 * frametime is in milliseconds.
 * \return 0 if the texture couldn't be loaded.
 * \note Terrain textures are not reused, even if fnametempl matches.
*/
uint32_t Graphic::get_maptexture(const string& fnametempl, uint32_t frametime)
{
	try {
		m_maptextures.push_back
			(new Texture(fnametempl, frametime, *m_sdl_screen->format));
	} catch (exception& e) {
		log("Failed to load maptexture %s: %s\n", fnametempl.c_str(), e.what());
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
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const string& fname) const
{
	log("Save screenshot to %s\n", fname.c_str());
	StreamWrite * sw = g_fs->OpenStreamWrite(fname);
	Surface& screen = *screen_.get();
	save_png_(screen, sw);
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
 * Retrieve the map texture with the given number
 * \return the actual texture data associated with the given ID.
 */
Texture * Graphic::get_maptexture_data(uint32_t id)
{
	--id; // ID 1 is at m_maptextures[0]
	if (id < m_maptextures.size())
		return m_maptextures[id];
	else
		return nullptr;
}


/**
 * Sets the name of the current world and loads the fitting road and edge textures
 */
void Graphic::set_world(string worldname) {
	char buf[255];

	// Load the road textures
	snprintf(buf, sizeof(buf), "worlds/%s/pics/roadt_normal.png", worldname.c_str());
	pic_road_normal_.reset(image_loader_->load(buf));
	snprintf(buf, sizeof(buf), "worlds/%s/pics/roadt_busy.png", worldname.c_str());
	pic_road_busy_.reset(image_loader_->load(buf));
}

/**
 * Retrives the texture of the road type.
 * \return The road texture
 */
Surface& Graphic::get_road_texture(int32_t roadtex)
{
	return
		roadtex == Widelands::Road_Normal ? *pic_road_normal_.get() : *pic_road_busy_.get();
}
