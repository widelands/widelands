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

	// Implements Surface
	void lock(LockMode) override;
	void unlock(UnlockMode) override;
	void setup_gl() override;
	void pixel_to_gl(float* x, float* y) const override;

	GLuint get_gl_texture() const {return m_texture;}
	const FloatRect& texture_coordinates() const {
		return m_texture_coordinates;
	}

private:
	void init(uint16_t w, uint16_t h);

	// True if we own the texture, i.e. if we need to delete it.
	bool m_owns_texture;

	// Texture coordinates in m_texture.
	FloatRect m_texture_coordinates;

	GLuint m_texture;
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
