/*
 * Copyright 2010-2011 by the Widelands Development Team
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
 */

#ifndef WL_GRAPHIC_GL_SURFACE_TEXTURE_H
#define WL_GRAPHIC_GL_SURFACE_TEXTURE_H

#include "graphic/gl/surface.h"

struct SDL_Surface;

class GLSurfaceTexture : public GLSurface {
public:
	GLSurfaceTexture(SDL_Surface * surface, bool intensity = false);
	GLSurfaceTexture(int w, int h);

	virtual ~GLSurfaceTexture();

	/// Interface implementation
	//@{
	void lock(LockMode) override;
	void unlock(UnlockMode) override;
	uint16_t get_pitch() const override;
	const SDL_PixelFormat & format() const override;

	// Note: the following functions are reimplemented here though they
	// basically only call the functions in GLSurface wrapped in calls to
	// setup_gl(), reset_gl(). The same functionality can be achieved by making
	// those two functions virtual and calling them in GLSurface. However,
	// especially for blit which is called very often and mostly on the screen,
	// this costs two virtual function calls which makes a notable difference in
	// profiles.
	void fill_rect(const Rect&, const RGBAColor&) override;
	void draw_rect(const Rect&, const RGBColor&) override;
	void brighten_rect(const Rect&, int32_t factor) override;
	virtual void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor&, uint8_t width) override;
	void blit(const Point&, const Surface*, const Rect& srcrc, Composite cm) override;

	GLuint get_gl_texture() const {return m_texture;}

private:
	void pixel_to_gl(float* x, float* y) override;
	void init(uint16_t w, uint16_t h);

	GLuint m_texture;
};

#endif  // end of include guard: WL_GRAPHIC_GL_SURFACE_TEXTURE_H
