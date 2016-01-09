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

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "build_info.h"
#include "graphic/align.h"
#include "graphic/animation.h"
#include "graphic/build_texture_atlas.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/gl/system_headers.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/screen.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "notifications/notifications.h"

using namespace std;

Graphic * g_gr;

namespace  {

// Sets the icon for the application.
void set_icon(SDL_Window* sdl_window) {
#ifndef _WIN32
	const std::string icon_name = "pics/wl-ico-128.png";
#else
	const std::string icon_name = "pics/wl-ico-32.png";
#endif
	SDL_Surface* s = load_image_as_sdl_surface(icon_name, g_fs);
	SDL_SetWindowIcon(sdl_window, s);
	SDL_FreeSurface(s);
}

void show_preload_text(const std::string& text) {
		if (UI::g_fh && UI::g_fh1) {
			auto *target = g_gr->get_render_target();
			UI::g_fh->draw_text(
					*target, UI::TextStyle::makebold(UI::Font::get(UI::g_fh1->fontset().serif(), 15),
						RGBColor(204, 204, 0)),
					Point(target->width() / 2, target->height() / 2),
					text, UI::Align_Center);
		   g_gr->refresh();
	   }
}

}  // namespace

Graphic::Graphic() : image_cache_(new ImageCache()), animation_manager_(new AnimationManager()) {
}

/**
 * Initialize the SDL video mode.
 */
void Graphic::initialize(int window_mode_w, int window_mode_h, bool init_fullscreen) {
	m_window_mode_width = window_mode_w;
	m_window_mode_height = window_mode_h;
	m_requires_update = true;

	// Request an OpenGL 2 context with double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	log("Graphics: Try to set Videomode %ux%u\n", m_window_mode_width, m_window_mode_height);
	m_sdl_window =
	   SDL_CreateWindow("Widelands Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                    m_window_mode_width, m_window_mode_height, SDL_WINDOW_OPENGL);

	resolution_changed();
	set_fullscreen(init_fullscreen);

	SDL_SetWindowTitle(m_sdl_window, ("Widelands " + build_id() + '(' + build_type() + ')').c_str());
	set_icon(m_sdl_window);

	m_glcontext = SDL_GL_CreateContext(m_sdl_window);
	SDL_GL_MakeCurrent(m_sdl_window, m_glcontext);

#ifdef USE_GLBINDING
	glbinding::Binding::initialize();
#else
	// See graphic/gl/system_headers.h for an explanation of the next line.
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		log("glewInit returns %i\nYour OpenGL installation must be __very__ broken. %s\n", err,
		    glewGetErrorString(err));
		throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
	}
#endif

	log(
	   "Graphics: OpenGL: Version \"%s\"\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

	GLboolean glBool;
	glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
	log("Graphics: OpenGL: Double buffering %s\n", (glBool == GL_TRUE) ? "enabled" : "disabled");

	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	log("Graphics: OpenGL: Max texture size: %u\n", max_texture_size);

	glDrawBuffer(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_sdl_window);

	/* Information about the video capabilities. */
	{
		SDL_DisplayMode disp_mode;
		SDL_GetWindowDisplayMode(m_sdl_window, &disp_mode);
		log("**** GRAPHICS REPORT ****\n"
		    " VIDEO DRIVER %s\n"
		    " pixel fmt %u\n"
		    " size %d %d\n"
		    "**** END GRAPHICS REPORT ****\n",
		    SDL_GetCurrentVideoDriver(), disp_mode.format, disp_mode.w, disp_mode.h);
		assert(SDL_BYTESPERPIXEL(disp_mode.format) == 4);
	}

	if (!is_texture_atlas_current()) {
		if (UI::g_fh && UI::g_fh1) {
			show_preload_text(
			   _("Building texture atlas...\nThis one time operation can take up to 5 minutes."));
		}
		make_texture_atlas(max_texture_size);
	}
	show_preload_text(_("Loading images..."));
	image_cache_->fill_with_texture_atlas();
}

Graphic::~Graphic()
{
	// TODO(unknown): this should really not be needed, but currently is :(
	if (UI::g_fh)
		UI::g_fh->flush();

	if (m_sdl_window) {
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = nullptr;
	}
	if (m_glcontext) {
		SDL_GL_DeleteContext(m_glcontext);
		m_glcontext = nullptr;
	}
}

/**
 * Return the screen x resolution
*/
int Graphic::get_xres()
{
	return screen_->width();
}

/**
 * Return the screen x resolution
*/
int Graphic::get_yres()
{
	return screen_->height();
}

void Graphic::change_resolution(int w, int h) {
	m_window_mode_width = w;
	m_window_mode_height = h;

	if (!fullscreen()) {
		SDL_SetWindowSize(m_sdl_window, w, h);
		resolution_changed();
	}
}

void Graphic::resolution_changed() {
	int new_w, new_h;
	SDL_GetWindowSize(m_sdl_window, &new_w, &new_h);

	screen_.reset(new Screen(new_w, new_h));
	m_rendertarget.reset(new RenderTarget(screen_.get()));

	Notifications::publish(GraphicResolutionChanged{new_w, new_h});

	update();
}

/**
 * Return a pointer to the RenderTarget representing the screen
*/
RenderTarget * Graphic::get_render_target()
{
	m_rendertarget->reset();
	return m_rendertarget.get();
}

bool Graphic::fullscreen()
{
	uint32_t flags = SDL_GetWindowFlags(m_sdl_window);
	return (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void Graphic::set_fullscreen(const bool value)
{
	if (value == fullscreen()) {
		return;
	}

	// Widelands is not resolution agnostic, so when we set fullscreen, we want
	// it at the full resolution of the desktop and we want to know about the
	// true resolution (SDL supports hiding the true resolution from the
	// application). Since SDL ignores requests to change the size of the window
	// whet fullscreen, we do it when in windowed mode.
	if (value) {
		SDL_DisplayMode display_mode;
		SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(m_sdl_window), &display_mode);
		SDL_SetWindowSize(m_sdl_window, display_mode.w, display_mode.h);

		SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(m_sdl_window, 0);

		// Next line does not work. See comment in refresh().
		SDL_SetWindowSize(m_sdl_window, m_window_mode_width, m_window_mode_height);
	}
	resolution_changed();
}


void Graphic::update() {
	m_requires_update = true;
}

/**
 * Returns true if parts of the screen have been marked for refreshing.
*/
bool Graphic::need_update() const {
	return m_requires_update;
}

/**
 * Bring the screen uptodate.
 *
 * \param force update whole screen
*/
void Graphic::refresh()
{
	RenderQueue::instance().draw(screen_->width(), screen_->height());

	// Setting the window size immediately after going out of fullscreen does
	// not work properly. We work around this issue by resizing the window in
	// refresh() when in window mode.
	if (!fullscreen()) {
		int true_width, true_height;
		SDL_GetWindowSize(m_sdl_window, &true_width, &true_height);
		if (true_width != m_window_mode_width || true_height != m_window_mode_height) {
			SDL_SetWindowSize(m_sdl_window, m_window_mode_width, m_window_mode_height);
		}
	}

	SDL_GL_SwapWindow(m_sdl_window);
	m_requires_update = false;
}


/**
 * Saves a pixel region to a png. This can be a file or part of a stream.
 *
 * @param surf The Surface to save
 * @param sw a StreamWrite where the png is written to
 */
void Graphic::save_png(Texture* texture, StreamWrite * sw) const {
	save_to_png(texture, sw, ColorType::RGBA);
}

/**
 * Save a screenshot to the given file.
*/
void Graphic::screenshot(const string& fname) const
{
	log("Save screenshot to %s\n", fname.c_str());
	std::unique_ptr<StreamWrite> sw(g_fs->open_stream_write(fname));
	save_to_png(screen_->to_texture().get(), sw.get(), ColorType::RGB);
}
