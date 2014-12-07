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
	void setup_gl() override;
	void pixel_to_gl(float* x, float* y) const override;

	// NOCOM(#sirver): do not implement this here.
	int get_gl_texture() const override {
		return m_texture;
	}
	const FloatRect& texture_coordinates() const override {
		return m_texture_coordinates;
	}

	/// The functions below are for direct pixel access. This should be used
	/// only very sparingly as / it is potentially expensive (especially for
	/// OpenGL). At the moment, only the code inside graphic / is actually using
	/// this.
	enum LockMode {
		/**
		 * Normal mode preserves pre-existing pixel data so that it can
		 * be read or modified.
		 */
		Lock_Normal = 0,

		/**
		 * Discard mode discards pre-existing pixel data. All pixels
		 * will be undefined unless they are re-written.
		 */
		Lock_Discard
	};

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

	/**
	 * \return Pitch of the raw pixel data, i.e. the number of bytes
	 * contained in each image row. This can be strictly larger than
	 * bytes per pixel times the width.
	 */
	uint16_t get_pitch() const;

	/**
	 * \return Pointer to the raw pixel data.
	 *
	 * \warning May only be called inside lock/unlock pairs.
	 */
	uint8_t * get_pixels() const;

	/**
	 * Lock/Unlock pairs must guard any of the direct pixel access using the
	 * functions below.
	 *
	 * \note Lock/Unlock pairs cannot be nested.
	 */
	void lock(LockMode);
	void unlock(UnlockMode);

	uint32_t get_pixel(uint16_t x, uint16_t y);
	void set_pixel(uint16_t x, uint16_t y, uint32_t clr);


private:
	void init(uint16_t w, uint16_t h);

	// True if we own the texture, i.e. if we need to delete it.
	bool m_owns_texture;

	// Texture coordinates in m_texture.
	FloatRect m_texture_coordinates;

	GLuint m_texture;

	/// Pixel data, while the texture is locked
	std::unique_ptr<uint8_t[]> m_pixels;
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
