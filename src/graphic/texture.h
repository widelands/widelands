/*
 * Copyright (C) 2002-2004, 2006, 2008-2010, 2012 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXTURE_H
#define WL_GRAPHIC_TEXTURE_H

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

#include "graphic/colormap.h"
#include "graphic/render/gl_surface_texture.h"

/// Textures have a fixed size and are squares.
/// TEXTURE_HEIGHT is just defined for easier understanding of the code.
#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT TEXTURE_WIDTH

/** struct Texture
*
* Texture represents are terrain texture, which is strictly
* TEXTURE_WIDTH by TEXTURE_HEIGHT pixels in size. It uses 8 bit color, and
* a pointer to the corresponding palette and color lookup table is
* provided.
*
* Currently, this is initialized from a 16 bit bitmap. This should be
* changed to load 8 bit bitmaps directly.
*/
struct Texture {
	Texture(const std::vector<std::string>& texture_files,
	        uint32_t frametime,
	        const SDL_PixelFormat&);
	~Texture();

	const std::string & get_texture_image() const {return m_texture_image;}

	uint8_t * get_pixels   () const {return m_pixels;}
	uint8_t * get_curpixels() const {return m_curframe;}
	void    * get_colormap () const {return m_colormap->get_colormap();}

	RGBColor get_minimap_color(int8_t shade);

	void animate(uint32_t time);
	uint32_t getTexture() const
		{return m_glFrames.at(m_frame_num)->get_gl_texture();}

private:
	std::unique_ptr<Colormap> m_colormap;
	uint8_t   * m_pixels;
	RGBColor    m_mmap_color[256];
	uint8_t   * m_curframe;
	int32_t     m_frame_num;
	std::string m_texture_image;
	uint32_t    m_nrframes;
	uint32_t    m_frametime;
	std::vector<std::unique_ptr<GLSurfaceTexture>> m_glFrames;
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_H
