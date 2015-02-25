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

#include <memory>

#include "base/rect.h"
#include "graphic/gl/system_headers.h"
#include "graphic/surface.h"

struct SDL_Surface;

class Texture : public Surface, public Image {
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
	int width() const override;
	int height() const override;
	void setup_gl() override;
	void pixel_to_gl(float* x, float* y) const override;

	// Implements Image.
	int get_gl_texture() const override;
	const FloatRect& texture_coordinates() const override;

	enum UnlockMode {
		/**
		 * Update mode will ensure that any changes in the pixel data
		 * will appear in subsequent operations.
		 */
		Unlock_Update = 0,

		/**
		 * NoChange mode indicates that the caller changed no pixel data.
		 *
		 * \note If the caller did change pixel data but specifies NoChange
		 * mode, the results are undefined.
		 */
		Unlock_NoChange
	};

	/// This returns the pixel format for direct pixel access.
	const SDL_PixelFormat & format() const;

	// Number of bytes per row.
	uint16_t get_pitch() const;

	// Pointer to the raw pixel data. May only be called inside lock/unlock
	// pairs.
	uint8_t * get_pixels() const;

	// Lock/Unlock pairs must guard any of the direct pixel access using the
	// functions below. Lock/Unlock pairs cannot be nested.
	void lock();
	void unlock(UnlockMode);

	// Returns the color of the pixel as a value as defined by 'format()'.
	uint32_t get_pixel(uint16_t x, uint16_t y);

	// Sets the pixel to the 'clr'.
	void set_pixel(uint16_t x, uint16_t y, uint32_t clr);

private:
	void init(uint16_t w, uint16_t h);

	// Width and height.
	int m_w, m_h;

	// True if we own the texture, i.e. if we need to delete it.
	bool m_owns_texture;

	// Texture coordinates in m_texture.
	FloatRect m_texture_coordinates;

	GLuint m_texture;

	/// Pixel data, while the texture is locked
	std::unique_ptr<uint8_t[]> m_pixels;

	DISALLOW_COPY_AND_ASSIGN(Texture);
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
