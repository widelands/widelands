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

#ifndef WL_GRAPHIC_TEXTURE_H
#define WL_GRAPHIC_TEXTURE_H

#include "base/rect.h"
#include "graphic/gl/system_headers.h"
#include "graphic/surface.h"

struct SDL_Surface;

class Texture : public Surface {
public:
	// Create a new surface from an SDL_Surface. If intensity is true, an GL_INTENSITY texture
	// is created. Ownership is taken.
	Texture(SDL_Surface * surface, bool intensity = false);

	// Create a new empty (that is randomly filled) Surface with the given
	// dimensions.
	Texture(int w, int h);

	// Create a logical texture that is a 'subrect' (in Pixel) in
	// another texture. Ownership of 'texture' is not taken.
	Texture(const GLuint texture, const Rect& subrect, int parent_w, int parent_h);

	virtual ~Texture();

	/// Interface implementation
	//@{
	void lock(LockMode) override;
	void unlock(UnlockMode) override;

	// Note: the following functions are reimplemented here though they
	// basically only call the functions in Surface wrapped in calls to
	// setup_gl(), reset_gl(). The same functionality can be achieved by making
	// those two functions virtual and calling them in Surface. However,
	// especially for blit which is called very often and mostly on the screen,
	// this costs two virtual function calls which makes a notable difference in
	// profiles.
	void fill_rect(const Rect&, const RGBAColor&) override;
	void draw_rect(const Rect&, const RGBColor&) override;
	void brighten_rect(const Rect&, int32_t factor) override;
	virtual void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor&, uint8_t width) override;
	void blit(const Rect& dstretc,
	          const Texture*,
	          const Rect& srcrc,
	          BlendMode blend_mode = BlendMode::UseAlpha) override;

	GLuint get_gl_texture() const {return m_texture;}

	const FloatRect& texture_coordinates() const {
		return m_texture_coordinates;
	}

private:
	void pixel_to_gl(float* x, float* y) const override;
	void init(uint16_t w, uint16_t h);

	// True if we own the texture, i.e. if we need to delete it.
	bool m_owns_texture;

	// Texture coordinates in m_texture.
	FloatRect m_texture_coordinates;

	GLuint m_texture;
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
