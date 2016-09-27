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

#ifndef WL_GRAPHIC_GRAPHIC_H
#define WL_GRAPHIC_GRAPHIC_H

#include <memory>

#include <SDL.h>

#include "graphic/image_cache.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"

class AnimationManager;
class RenderTarget;
class Screen;
class StreamWrite;

// Will be send whenever the resolution changes.
struct GraphicResolutionChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::GraphicResolutionChanged)

	// New width and height in pixels.
	int width;
	int height;
};

/**
 * This class is a kind of Swiss Army knife for your graphics need.
 * It initializes the graphic system and provides access to
 * resolutions. It owns an Animation, Image and Surface cache. It
 * also offers functionality to save a screenshot.
 */
class Graphic {
public:
	// Creates a new Graphic object. Must call initialize before first use.
	Graphic();
	~Graphic();

	// Initializes with the given resolution if fullscreen is false, otherwise a
	// window that fills the screen. The 'trace_gl' parameter gets passed on to
	// 'Gl::initialize'.
	enum class TraceGl { kNo, kYes };
	void
	initialize(const TraceGl& trace_gl, int window_mode_w, int window_mode_height, bool fullscreen);

	// Gets and sets the resolution.
	void change_resolution(int w, int h);
	int get_xres();
	int get_yres();

	// Changes the window to be fullscreen or not.
	bool fullscreen();
	void set_fullscreen(bool);

	RenderTarget* get_render_target();
	void refresh();
	SDL_Window* get_sdlwindow() {
		return sdl_window_;
	}

	int max_texture_size() const {
		return max_texture_size_;
	}

	ImageCache& images() const {
		return *image_cache_.get();
	}
	AnimationManager& animations() const {
		return *animation_manager_.get();
	}

	// Requests a screenshot being taken on the next frame.
	void screenshot(const std::string& fname);

private:
	// Called when the resolution (might) have changed.
	void resolution_changed();

	// The height & width of the window should we be in window mode.
	int window_mode_width_;
	int window_mode_height_;

	/// This is the main screen Surface.
	/// A RenderTarget for this can be retrieved with get_render_target()
	std::unique_ptr<Screen> screen_;

	/// This saves a copy of the screen SDL_Surface. This is needed for
	/// opengl rendering as the SurfaceOpenGL does not use it. It allows
	/// manipulation the screen context.
	SDL_Window* sdl_window_;
	SDL_GLContext gl_context_;

	/// The maximum width or height a texture can have.
	int max_texture_size_;

	/// A RenderTarget for screen_. This is initialized during init()
	std::unique_ptr<RenderTarget> render_target_;

	/// Non-volatile cache of independent images.
	std::unique_ptr<ImageCache> image_cache_;

	/// This holds all animations.
	std::unique_ptr<AnimationManager> animation_manager_;

	/// Screenshot filename. If a screenshot is requested, this will be set to
	/// the requested filename. On the next frame the screenshot will be written
	/// out and this will be clear()ed again.
	std::string screenshot_filename_;
};

extern Graphic* g_gr;

#endif  // end of include guard: WL_GRAPHIC_GRAPHIC_H
