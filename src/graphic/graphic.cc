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
#include <memory>

#include <SDL_image.h>

#include "base/deprecated.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "graphic/animation.h"
#include "graphic/diranimations.h"
#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/image_transformations.h"
#include "graphic/render/gl_surface_screen.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/rendertarget.h"
#include "graphic/surface_cache.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "logic/roadtype.h"
#include "ui_basic/progresswindow.h"

using namespace std;

Graphic * g_gr;
bool g_opengl;

constexpr int kFallbackGraphicsWidth = 800;
constexpr int kFallbackGraphicsHeight = 600;

namespace  {

/// The size of the transient (i.e. temporary) surfaces in the cache in bytes.
/// These are all surfaces that are not loaded from disk.
const uint32_t TRANSIENT_SURFACE_CACHE_SIZE = 160 << 20;   // shifting converts to MB

}  // namespace

/**
 * Initialize the SDL video mode.
*/
Graphic::Graphic()
	:
	m_fallback_settings_in_effect(false),
	m_update(true),
	surface_cache_(create_surface_cache(TRANSIENT_SURFACE_CACHE_SIZE)),
	image_cache_(new ImageCache(surface_cache_.get())),
	animation_manager_(new AnimationManager())
{
	ImageTransformations::initialize();

#ifndef _WIN32
	const std::string icon_name = "pics/wl-ico-128.png";
#else
	const std::string icon_name = "pics/wl-ico-32.png";
#endif
	m_sdlwindow = nullptr;
	m_sdl_screen = nullptr;
	m_sdl_renderer = nullptr;
	m_sdl_texture = nullptr;
	m_glcontext = nullptr;
}

void Graphic::initialize(int32_t w, int32_t h, bool fullscreen, bool opengl) {
	cleanup();

	// Set video mode using SDL. First collect the flags
	int32_t flags = 0;
	g_opengl = false;

	if (opengl) {
		log("Graphics: Trying opengl\n");
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		flags |= SDL_WINDOW_OPENGL;
	}

	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
		log("Graphics: Trying FULLSCREEN\n");
	}

	log("Graphics: Try to set Videomode %ux%u 32 Bit\n", w, h);
	// Here we actually set the video mode
	m_sdlwindow = SDL_CreateWindow("Widelands Window",
											 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
	if (opengl) {
		// TODO(sirver): this context needs to be created also for fallback settings,
		// otherwise SDL_GetWindowFlags() will return SDL_WINDOW_OPENGL,
		// though if you call any gl function, the system crashes.
		m_glcontext = SDL_GL_CreateContext(m_sdlwindow);
		if (m_glcontext) {
			SDL_GL_MakeCurrent(m_sdlwindow, m_glcontext);
		}
	}
	// If we tried opengl and it was not successful try without opengl
	if ((!m_sdlwindow || !m_glcontext) && opengl)
	{
		log("Graphics: Could not set videomode: %s, trying without opengl\n", SDL_GetError());
		flags &= ~SDL_WINDOW_OPENGL;
		m_sdlwindow = SDL_CreateWindow("Widelands Window",
												 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
	}

	if (!m_sdlwindow) {
		log
			("Graphics: Could not set videomode: %s, trying minimum graphics settings\n",
			 SDL_GetError());
		flags &= ~SDL_WINDOW_FULLSCREEN;
		m_sdlwindow = SDL_CreateWindow("Widelands Window",
												 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
												 kFallbackGraphicsWidth, kFallbackGraphicsHeight, flags);
		m_fallback_settings_in_effect = true;
		if (!m_sdlwindow) {
			throw wexception
				("Graphics: could not set video mode: %s", SDL_GetError());
		}
	}

#ifndef _WIN32
	const std::string icon_name = "pics/wl-ico-128.png";
#else
	const std::string icon_name = "pics/wl-ico-32.png";
#endif
	SDL_Surface* s = load_image_as_sdl_surface(icon_name, g_fs);
	SDL_SetWindowIcon(m_sdlwindow, s);
	SDL_FreeSurface(s);

	// setting the videomode was successful. Print some information now
	log("Graphics: Setting video mode was successful\n");

	if (opengl && 0 != (SDL_GetWindowFlags(m_sdlwindow) & SDL_GL_DOUBLEBUFFER))
		log("Graphics: OPENGL DOUBLE BUFFERING ENABLED\n");
	if (0 != (SDL_GetWindowFlags(m_sdlwindow) & SDL_WINDOW_FULLSCREEN))
		log("Graphics: FULLSCREEN ENABLED\n");

	bool use_arb = true;
	const char * extensions = nullptr;

	if (opengl && 0 != (SDL_GetWindowFlags(m_sdlwindow) & SDL_WINDOW_OPENGL)) {
		//  We have successful opened an opengl screen. Print some information
		//  about opengl and set the rendering capabilities.
		log ("Graphics: OpenGL: OpenGL enabled\n");

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			log("glewInit returns %i\nYour OpenGL installation must be __very__ broken. %s\n",
				 err, glewGetErrorString(err));
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
			flags &= ~SDL_WINDOW_OPENGL;
			m_fallback_settings_in_effect = true;
			SDL_DestroyWindow(m_sdlwindow);
			m_sdlwindow = SDL_CreateWindow("Widelands Window",
													 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
													 kFallbackGraphicsWidth, kFallbackGraphicsHeight, flags);
			m_fallback_settings_in_effect = true;
			if (!m_sdlwindow) {
				throw wexception("Graphics: could not set video mode: %s", SDL_GetError());
			}
		}
	}
	Surface::display_format_is_now_defined();

	// Redoing the check, because fallback settings might mean we no longer use OpenGL.
	if (opengl && 0 != (SDL_GetWindowFlags(m_sdlwindow) & SDL_WINDOW_OPENGL)) {
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
			(m_caps.gl.major_version < 2) &&
			(strstr(extensions, "GL_ARB_texture_non_power_of_two") == nullptr);
		log("Graphics: OpenGL: Textures ");
		log
			(m_caps.gl.tex_power_of_two?"must have a size power of two\n":
			 "may have any size\n");

		m_caps.gl.multitexture =
			 ((strstr(extensions, "GL_ARB_multitexture") != nullptr) &&
			  (strstr(extensions, "GL_ARB_texture_env_combine") != nullptr));
		log("Graphics: OpenGL: Multitexture capabilities ");
		log(m_caps.gl.multitexture ? "sufficient\n" : "insufficient, only basic terrain rendering possible\n");

DIAG_OFF("-Wold-style-cast")
		m_caps.gl.blendequation = GLEW_VERSION_1_4 || GLEW_ARB_imaging;
DIAG_ON ("-Wold-style-cast")
	}

	/* Information about the video capabilities. */
	SDL_DisplayMode disp_mode;
	SDL_GetWindowDisplayMode(m_sdlwindow, &disp_mode);
	const char * videodrvused = SDL_GetCurrentVideoDriver();
	log
		("**** GRAPHICS REPORT ****\n"
		 " VIDEO DRIVER %s\n"
		 " pixel fmt %u\n"
		 " size %d %d\n"
		 "**** END GRAPHICS REPORT ****\n",
		 videodrvused,
		 disp_mode.format,
		 disp_mode.w, disp_mode.h);

	log("Graphics: flags: %u\n", SDL_GetWindowFlags(m_sdlwindow));

	assert
			(SDL_BYTESPERPIXEL(disp_mode.format) == 2 ||
			 SDL_BYTESPERPIXEL(disp_mode.format) == 4);

	SDL_SetWindowTitle(m_sdlwindow, ("Widelands " + build_id() + '(' + build_type() + ')').c_str());

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
		SDL_GL_SetSwapInterval(1);
		SDL_GL_SwapWindow(m_sdlwindow);
		glEnable(GL_TEXTURE_2D);

		GLSurfaceTexture::initialize(use_arb);
	}

	if (g_opengl)
	{
		screen_.reset(new GLSurfaceScreen(w, h));
	}
	else {
		m_sdl_renderer =  SDL_CreateRenderer(m_sdlwindow, -1, 0);
		uint32_t rmask, gmask, bmask, amask;
		int bpp;
		SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bpp, &rmask, &gmask, &bmask, &amask);
		m_sdl_screen = SDL_CreateRGBSurface(0, w, h, bpp, rmask, gmask, bmask, amask);
		m_sdl_texture = SDL_CreateTexture(m_sdl_renderer,
													 SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
													 w, h);
		screen_.reset(new SDLSurface(m_sdl_screen, false));
	}

	m_rendertarget.reset(new RenderTarget(screen_.get()));

	pic_road_normal_.reset(load_image("world/pics/roadt_normal.png"));
	pic_road_busy_.reset(load_image("world/pics/roadt_busy.png"));
}

bool Graphic::check_fallback_settings_in_effect()
{
	return m_fallback_settings_in_effect;
}

void Graphic::cleanup() {
	m_maptextures.clear();
	surface_cache_->flush();
	// TODO(unknown): this should really not be needed, but currently is :(
	if (UI::g_fh)
		UI::g_fh->flush();

	if (g_opengl) {
		GLSurfaceTexture::cleanup();
	}
	if (m_sdl_texture) {
		SDL_DestroyTexture(m_sdl_texture);
		m_sdl_texture = nullptr;
	}
	if (m_sdl_screen) {
		SDL_FreeSurface(m_sdl_screen);
		m_sdl_screen = nullptr;
	}
	if (m_sdlwindow) {
		SDL_DestroyWindow(m_sdlwindow);
		m_sdlwindow = nullptr;
	}
	if (m_glcontext) {
		SDL_GL_DeleteContext(m_glcontext);
		m_glcontext = nullptr;
	}
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
	return SDL_GetWindowFlags(m_sdlwindow) & SDL_WINDOW_FULLSCREEN;
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
	// TODO(unknown): implement proper fullscreening here. The way it can work is to
	// recreate SurfaceCache but keeping ImageCache around. Then exiting and
	// reinitalizing the SDL Video Mode should just work: all surface are
	// recreated dynamically and correctly.
	// Note: Not all Images are cached in the ImageCache, at time of me writing
	// this, only InMemoryImage does not safe itself in the ImageCache. And this
	// should only be a problem for Images loaded from maps.
	// NOCOM: Is this still a todo?
	if (SDL_GetWindowFlags(m_sdlwindow) & SDL_WINDOW_FULLSCREEN) {
		SDL_SetWindowFullscreen(m_sdlwindow, 0);
	} else {
		SDL_SetWindowFullscreen(m_sdlwindow, SDL_WINDOW_FULLSCREEN);
	}
}


void Graphic::update() {
	m_update = true;
}

/**
 * Returns true if parts of the screen have been marked for refreshing.
*/
bool Graphic::need_update() const
{
	return  m_update;
}

/**
 * Bring the screen uptodate.
 *
 * \param force update whole screen
*/
void Graphic::refresh()
{
	if (g_opengl) {
		SDL_GL_SwapWindow(m_sdlwindow);
		m_update = false;
		return;
	}

	SDL_UpdateTexture(m_sdl_texture, nullptr, m_sdl_screen->pixels, m_sdl_screen->pitch);
	SDL_RenderClear(m_sdl_renderer);
	SDL_RenderCopy(m_sdl_renderer, m_sdl_texture, nullptr, nullptr);
	SDL_RenderPresent(m_sdl_renderer);
	m_update = false;
}


/**
 * Saves a pixel region to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(const Image* image, StreamWrite * sw) const {
	save_surface_to_png(image->surface(), sw);
}

uint32_t Graphic::new_maptexture(const std::vector<std::string>& texture_files, const uint32_t frametime)
{
	SDL_PixelFormat* pixel_fmt = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	m_maptextures.emplace_back(new Texture(texture_files, frametime, *pixel_fmt));
	if (pixel_fmt) {
		SDL_FreeFormat(pixel_fmt);
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
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const string& fname) const
{
	log("Save screenshot to %s\n", fname.c_str());
	StreamWrite * sw = g_fs->open_stream_write(fname);
	save_surface_to_png(screen_.get(), sw);
	delete sw;
}

/**
 * Retrieve the map texture with the given number
 * \return the actual texture data associated with the given ID.
 */
Texture * Graphic::get_maptexture_data(uint32_t id)
{
	--id; // ID 1 is at m_maptextures[0]

	assert(id < m_maptextures.size());
	return m_maptextures[id].get();
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
