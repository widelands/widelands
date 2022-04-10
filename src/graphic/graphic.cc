/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/graphic.h"

#include <memory>

#include <SDL_messagebox.h>
#include <SDL_video.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "build_info.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/build_texture_atlas.h"
#include "graphic/default_resolution.h"
#include "graphic/gl/initialize.h"
#include "graphic/gl/system_headers.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "graphic/note_graphic_resolution_changed.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/screen.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "notifications/notifications.h"

#ifndef RESIZABLE_WINDOW
#define SDL_SetWindowResizable(window, resizable)
#endif

Graphic* g_gr;

namespace {

// Sets the icon for the application.
void set_icon(SDL_Window* sdl_window) {
#ifndef _WIN32
	const std::string icon_name = "images/logos/wl-ico-128.png";
#else
	const std::string icon_name = "images/logos/wl-ico-32.png";
#endif
	SDL_Surface* s = load_image_as_sdl_surface(icon_name, g_fs);
	SDL_SetWindowIcon(sdl_window, s);
	SDL_FreeSurface(s);
}

}  // namespace

Graphic::Graphic() {
	g_image_cache = new ImageCache();
	g_animation_manager = new AnimationManager();
}

/**
 * Initialize the SDL video mode.
 */
void Graphic::initialize(const TraceGl& trace_gl,
                         int window_mode_w,
                         int window_mode_h,
                         bool init_fullscreen,
                         bool init_maximized) {
	window_mode_width_ = window_mode_w;
	window_mode_height_ = window_mode_h;

	if (SDL_GL_LoadLibrary(nullptr) == -1) {
		throw wexception("SDL_GL_LoadLibrary failed: %s", SDL_GetError());
	}

	log_info("Graphics: Try to set Videomode %ux%u\n", window_mode_width_, window_mode_height_);
	uint32_t window_flags = SDL_WINDOW_OPENGL;
#ifdef RESIZABLE_WINDOW
	window_flags |= SDL_WINDOW_RESIZABLE;
#endif
	sdl_window_ =
	   SDL_CreateWindow("Widelands Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                    window_mode_width_, window_mode_height_, window_flags);
	SDL_SetWindowMinimumSize(sdl_window_, kMinimumResolutionW, kMinimumResolutionH);

	GLint max;
	// LeakSanitizer reports a memory leak which is triggered somewhere in this function call,
	// probably coming from the gaphics drivers
	gl_context_ = Gl::initialize(
	   trace_gl == TraceGl::kYes ? Gl::Trace::kYes : Gl::Trace::kNo, sdl_window_, &max);

	max_texture_size_ = static_cast<int>(max);

	set_fullscreen(init_fullscreen);
	if (init_maximized) {
		set_maximized(true);
	}
	resolution_changed();

	SDL_SetWindowTitle(sdl_window_, ("Widelands " + build_id() + '(' + build_type() + ')').c_str());
	set_icon(sdl_window_);

	SDL_GL_SwapWindow(sdl_window_);

	/* Information about the video capabilities. */
	const char* drv = SDL_GetCurrentVideoDriver();
	log_info("**** GRAPHICS REPORT ****\n");
#ifdef WL_USE_GLVND
	log_info("VIDEO DRIVER GLVND %s\n", drv ? drv : "NONE");
#else
	log_info("VIDEO DRIVER %s\n", drv ? drv : "NONE");
#endif
	SDL_DisplayMode disp_mode;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
		if (SDL_GetCurrentDisplayMode(i, &disp_mode) == 0) {
			log_info("Display #%d: %dx%d @ %dhz %s\n", i, disp_mode.w, disp_mode.h,
			         disp_mode.refresh_rate, SDL_GetPixelFormatName(disp_mode.format));
		} else {
			log_warn("Couldn't get display mode for display #%d: %s\n", i, SDL_GetError());
		}
	}
	verb_log_info("**** END GRAPHICS REPORT ****\n");
	rebuild_texture_atlas();
}

void Graphic::rebuild_texture_atlas() const {
	verb_log_info("Rebuilding texture atlas");
	std::map<std::string, std::unique_ptr<Texture>> textures_in_atlas;
	auto texture_atlases = build_texture_atlas(max_texture_size_, &textures_in_atlas);
	g_image_cache->fill_with_texture_atlases(
	   std::move(texture_atlases), std::move(textures_in_atlas));
}

Graphic::~Graphic() {
	delete g_animation_manager;
	g_animation_manager = nullptr;
	delete g_image_cache;
	g_image_cache = nullptr;
	if (sdl_window_ != nullptr) {
		SDL_DestroyWindow(sdl_window_);
		sdl_window_ = nullptr;
	}
	if (gl_context_ != nullptr) {
		SDL_GL_DeleteContext(gl_context_);
		gl_context_ = nullptr;
	}
}

/**
 * Return the screen x resolution
 */
int Graphic::get_xres() const {
	return screen_->width();
}

/**
 * Return the screen x resolution
 */
int Graphic::get_yres() const {
	return screen_->height();
}

int Graphic::get_window_mode_xres() const {
	return window_mode_width_;
}

int Graphic::get_window_mode_yres() const {
	return window_mode_height_;
}

void Graphic::change_resolution(int w, int h, bool resize_window) {
	window_mode_width_ = w;
	window_mode_height_ = h;

	if (!fullscreen() && resize_window) {
		set_window_size(w, h);
	}

	resolution_changed();
}

void Graphic::set_window_size(int w, int h) {
	// Calling SDL_SetWindowSize() when the window is resizable or maximized can sometimes cause
	// SDL (and consequently us) to lose track of the real window size, causing nasty glitches.
	// To work around this we temporarily set the window as not resizable. It's set back to
	// resizable later in refresh().
	SDL_SetWindowResizable(sdl_window_, SDL_FALSE);

	if (maximized()) {
		SDL_RestoreWindow(sdl_window_);
	}

	SDL_SetWindowSize(sdl_window_, w, h);
}

void Graphic::resolution_changed() {
	int old_w = screen_ ? screen_->width() : 0;
	int old_h = screen_ ? screen_->height() : 0;

	int new_w;
	int new_h;
	SDL_GetWindowSize(sdl_window_, &new_w, &new_h);

	if (old_w == new_w && old_h == new_h) {
		return;
	}

	screen_.reset(new Screen(new_w, new_h));
	render_target_.reset(new RenderTarget(screen_.get()));

	Notifications::publish(GraphicResolutionChanged{old_w, old_h, new_w, new_h});
}

/**
 * Return a pointer to the RenderTarget representing the screen
 */
RenderTarget* Graphic::get_render_target() {
	render_target_->reset();
	return render_target_.get();
}

int Graphic::max_texture_size_for_font_rendering() const {
// Test with minimum supported size in debug builds.
#ifndef NDEBUG
	return kMinimumSizeForTextures;
#else
	return max_texture_size_;
#endif
}

bool Graphic::maximized() const {
	uint32_t flags = SDL_GetWindowFlags(sdl_window_);
	return (flags & SDL_WINDOW_MAXIMIZED) != 0u;
}

void Graphic::set_maximized(const bool to_maximize) {
	window_mode_maximized_ = to_maximize;
	if (fullscreen() || maximized() == to_maximize) {
		return;
	}
	if (to_maximize) {
		// Maximizing only works if the window is resizable.
		SDL_SetWindowResizable(sdl_window_, SDL_TRUE);
		SDL_MaximizeWindow(sdl_window_);
	} else {
		// Avoid glitches. See the comment in set_window_size().
		SDL_SetWindowResizable(sdl_window_, SDL_FALSE);
		SDL_RestoreWindow(sdl_window_);
	}
}

bool Graphic::fullscreen() const {
	uint32_t flags = SDL_GetWindowFlags(sdl_window_);
	return ((flags & SDL_WINDOW_FULLSCREEN) != 0u) ||
	       ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0u);
}

void Graphic::set_fullscreen(const bool value) {
	if (value == fullscreen()) {
		return;
	}

	// Avoid glitches. See the comment in set_window_size().
	SDL_SetWindowResizable(sdl_window_, SDL_FALSE);

	// Use the desktop resolution in fullscreen mode.
	// SDL will resize the window for us.
	if (value) {
		window_mode_maximized_ = maximized();
		SDL_SetWindowFullscreen(sdl_window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(sdl_window_, 0);
	}
	resolution_changed();
}

/**
 * Bring the screen uptodate.
 */
void Graphic::refresh() {
	RenderQueue::instance().draw(screen_->width(), screen_->height());

	if (!fullscreen()) {
		// Set the window to our preferred size if it goes out of sync.
		// Not sure if this is still needed, leaving it just in case.
		int true_width;
		int true_height;
		SDL_GetWindowSize(sdl_window_, &true_width, &true_height);

		if (true_width != window_mode_width_ || true_height != window_mode_height_) {
			set_window_size(window_mode_width_, window_mode_height_);
			set_maximized(window_mode_maximized_);
			resolution_changed();
		}
		// See the comment in set_window_size().
		SDL_SetWindowResizable(sdl_window_, SDL_TRUE);
	}

	// The backbuffer now contains the current frame. If we want a screenshot,
	// we should better take it now, before this is swapped out to the
	// frontbuffer and becomes inaccessible to us.
	if (!screenshot_filename_.empty()) {
		log_info("Save screenshot to %s\n", screenshot_filename_.c_str());
		std::unique_ptr<StreamWrite> sw(g_fs->open_stream_write(screenshot_filename_));
		save_to_png(screen_->to_texture().get(), sw.get(), ColorType::RGB);
		screenshot_filename_.clear();
	}

	SDL_GL_SwapWindow(sdl_window_);
}

/**
 * Save a screenshot to the given file.
 */
void Graphic::screenshot(const std::string& fname) {
	screenshot_filename_ = fname;
}
