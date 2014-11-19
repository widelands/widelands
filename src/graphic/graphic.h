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

#include <map>
#include <memory>
#include <vector>

#include <SDL.h>

#include "base/rect.h"
#include "graphic/image_cache.h"

#define MAX_RECTS 20

class AnimationManager;
class RenderTarget;
class Surface;
class SurfaceCache;
struct SDL_Surface;
class StreamWrite;
struct Texture;

/**
 * This class is a kind of Swiss Army knife for your graphics need. It
 * initializes the graphic system and provides access to resolutions. It has an
 * Animation, Image and Surface cache and owns the road textures. It also
 * offers functionality to save a screenshot.
 */
class Graphic {
public:
	Graphic();
	~Graphic();

	// Initialize or reinitialize the graphics system. Throws on error.
	void initialize
		(int32_t w, int32_t h, bool fullscreen, bool opengl);

	int32_t get_xres();
	int32_t get_yres();
	bool is_fullscreen();

	RenderTarget * get_render_target();
	void toggle_fullscreen();
	void update();
	bool need_update() const;
	void refresh();
	SDL_Window* get_sdlwindow() {return m_sdl_window;}

	SurfaceCache& surfaces() const {return *surface_cache_.get();}
	ImageCache& images() const {return *image_cache_.get();}
	AnimationManager& animations() const {return *animation_manager_.get();}

	void save_png(const Image*, StreamWrite*) const;

	// Creates a new Texture() with the given 'frametime' and using the given
	// 'texture_files' as the images for it and returns it id.
	uint32_t new_maptexture(const std::vector<std::string>& texture_files, uint32_t frametime);
	void animate_maptextures(uint32_t time);

	void screenshot(const std::string& fname) const;
	Texture * get_maptexture_data(uint32_t id);

	Surface& get_road_texture(int32_t roadtex);

	bool check_fallback_settings_in_effect();

private:
	void cleanup();

	bool m_fallback_settings_in_effect;

	/// This is the main screen Surface.
	/// A RenderTarget for this can be retrieved with get_render_target()
	std::unique_ptr<Surface> screen_;
	/// This saves a copy of the screen SDL_Surface. This is needed for
	/// opengl rendering as the SurfaceOpenGL does not use it. It allows
	/// manipulation the screen context.
	SDL_Surface * m_sdl_screen;
	SDL_Renderer * m_sdl_renderer;
	SDL_Window * m_sdl_window;
	SDL_Texture * m_sdl_texture;
	SDL_GLContext m_glcontext;
	/// A RenderTarget for screen_. This is initialized during init()
	std::unique_ptr<RenderTarget> m_rendertarget;
	/// This marks the complete screen for updating.
	bool m_update;

	/// Volatile cache of Hardware dependant surfaces.
	std::unique_ptr<SurfaceCache> surface_cache_;
	/// Non-volatile cache of hardware independent images. The use the
	/// surface_cache_ to cache their pixel data.
	std::unique_ptr<ImageCache> image_cache_;
	/// This holds all animations.
	std::unique_ptr<AnimationManager> animation_manager_;

	// The texture needed to draw roads.
	std::unique_ptr<Surface> pic_road_normal_;
	std::unique_ptr<Surface> pic_road_busy_;

	std::vector<std::unique_ptr<Texture>> m_maptextures;
};

extern Graphic * g_gr;
extern bool g_opengl;

#endif  // end of include guard: WL_GRAPHIC_GRAPHIC_H
