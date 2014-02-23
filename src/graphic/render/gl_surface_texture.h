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

#ifndef GL_SURFACE_TEXTURE_H
#define GL_SURFACE_TEXTURE_H

#include "graphic/render/gl_surface.h"

struct SDL_Surface;

class GLSurfaceTexture : public GLSurface {
public:
	// Call this once before using any instance of this class and Cleanup once
	// before the program exits.
	static void Initialize(bool use_arb);
	static void Cleanup();

	GLSurfaceTexture(SDL_Surface * surface, bool intensity = false);
	GLSurfaceTexture(int w, int h);

	virtual ~GLSurfaceTexture();

	/// Interface implementation
	//@{
	virtual void lock(LockMode) override;
	virtual void unlock(UnlockMode) override;
	virtual uint16_t get_pitch() const override;
	virtual const SDL_PixelFormat & format() const override;

	// Note: the following functions are reimplemented here though they
	// basically only call the functions in GLSurface wrapped in calls to
	// setup_gl(), reset_gl(). The same functionality can be achieved by making
	// those two functions virtual and calling them in GLSurface. However,
	// especially for blit which is called very often and mostly on the screen,
	// this costs two virtual function calls which makes a notable difference in
	// profiles.
	virtual void fill_rect(const Rect&, RGBAColor) override;
	virtual void draw_rect(const Rect&, RGBColor) override;
	virtual void brighten_rect(const Rect&, int32_t factor) override;
	virtual void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor&, uint8_t width) override;
	virtual void blit(const Point&, const Surface*, const Rect& srcrc, Composite cm) override;

	GLuint get_gl_texture() const {return m_texture;}
	uint16_t get_tex_w() const {return m_tex_w;}
	uint16_t get_tex_h() const {return m_tex_h;}

private:
	void init(uint16_t w, uint16_t h);
	void setup_gl();
	void reset_gl();

	static GLuint gl_framebuffer_id_;
	GLuint m_texture;

	/// Keep the size of the opengl texture. This is necessary because some
	/// systems support only a power of two for texture sizes.
	uint16_t m_tex_w, m_tex_h;
};

#endif //GL_SURFACE_TEXTURE_H
