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
	Texture(SDL_Surface* surface, bool intensity = false);

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

	// Implements Image.
	const BlitData& blit_data() const override;

	enum LockMode {
		/**
		* Previously existing pixel data will be discarded.
		*
		* The contents of the texture will be undefined unless all pixels
		* values are explicitly set and \ref unlock is called in Unlock_Update
		* mode.
		*/
		Lock_Discard = 0,

		/**
		* The existing data in the texture will be preserved.
		*
		* Avoid this when possible, since the texture may have to be
		* re-downloaded from the GPU which involves a graphics pipeline stall.
		*/
		Lock_Preserve
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

	// Lock/Unlock pairs must guard any of the direct pixel access using the
	// functions below. Lock/Unlock pairs cannot be nested.
	void lock(LockMode);
	void unlock(UnlockMode);

	// Returns the color of the pixel.
	RGBAColor get_pixel(uint16_t x, uint16_t y);

	// Sets the pixel to the 'clr'.
	void set_pixel(uint16_t x, uint16_t y, const RGBAColor& color);

private:
	// Configures OpenGL to draw to this surface.
	void setup_gl();
	void init(uint16_t w, uint16_t h);

	// Implements surface.
	void do_blit(const FloatRect& dst_rect,
	             const BlitData& texture,
	             float opacity,
	             BlendMode blend_mode) override;
	void do_blit_blended(const FloatRect& dst_rect,
	                     const BlitData& texture,
	                     const BlitData& mask,
	                     const RGBColor& blend) override;
	void do_blit_monochrome(const FloatRect& dst_rect,
	                        const BlitData& texture,
	                        const RGBAColor& blend) override;
	void do_draw_line_strip(std::vector<DrawLineProgram::PerVertexData> vertices) override;
	void
	do_fill_rect(const FloatRect& dst_rect, const RGBAColor& color, BlendMode blend_mode) override;

	// True if we own the texture, i.e. if we need to delete it.
	bool owns_texture_;

	BlitData blit_data_;
	/// Pixel data, while the texture is locked
	std::unique_ptr<uint8_t[]> pixels_;

	DISALLOW_COPY_AND_ASSIGN(Texture);
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
