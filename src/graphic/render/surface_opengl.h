/*
 * Copyright 2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if !defined(SURFACE_OPENGL_H) and defined(USE_OPENGL)
#define SURFACE_OPENGL_H

#include "rgbcolor.h"
#include "rect.h"
#include "graphic/surface.h"

#include <SDL_opengl.h>
#include <assert.h>

namespace Widelands {
struct Editor_Game_Base;
struct Field;
struct Player;
}

namespace UI {
struct Font_Handler;
}

struct Vertex;

/**
 * This was formerly called struct Bitmap. But now it manages an
 * SDL_Surface as it's core.
 *
 * Represents a simple bitmap without managing its memory. The rendering
 * functions do NOT perform any clipping; this is up to the caller.
*/
class SurfaceOpenGL : public Surface {
public:
	class oglTexture
	{
	public:
		oglTexture() {}
		oglTexture(GLuint id): m_textureID(id) {}
		~oglTexture()
			{glDeleteTextures( 1, &m_textureID);}
		GLuint id() {return m_textureID;}
	private:
		GLuint m_textureID;
	};

	SurfaceOpenGL(SDL_Surface & surface);
	SurfaceOpenGL(int w = 0, int h = 0);
	~SurfaceOpenGL();

	/// Get width and height
	uint32_t get_w() const {return m_w;}
	uint32_t get_h() const {return m_h;}
	uint32_t get_dest_w() const {return m_dest_w;}
	uint32_t get_dest_h() const {return m_dest_h;}
	void update();

	/// Save a bitmap of this to a file
	//void save_bmp(const char & fname) const;

	GLuint get_texture()
	{
#if defined(DEBUG)
		if (m_surf_type != SURFACE_SOURCE)
			throw wexception("Try to get opengl texture id but not a source surface");
		if (not m_texture)
			throw wexception("no texture");
#endif
		return m_texture->id();
	}

/*	// For the bravest: Direct Pixel access. Use carefully
	/// Needed if you want to blit directly to the screen by memcpy
	void force_disable_alpha()*/
/*
	const SDL_PixelFormat * get_format() const
	{
		SDL_PixelFormat fmt;
		fmt.BitsPerPixel = 32;
		fmt.BytesPerPixel = 4;
		fmt.Rmask=0x000000ff; fmt.Gmask=0x0000ff00;
		fmt.Bmask=0x00ff0000; fmt.Amask=0xff000000;
		fmt.Ashift=24;fmt.Bshift=16;fmt.Gshift=8;
		return fmt;
	}
	const SDL_PixelFormat & format() const
	{ return *get_format();}
*/
	uint16_t get_pitch() const {return m_w*4;}
	uint8_t * get_pixels() const
	{
		assert(m_locked);
		assert(m_pixels);
		return m_pixels;
	}

	/// Lock: The Surface need to be locked before get or set pixel
	void lock();
	void unlock();

	/// For the slowest: Indirect pixel access
	uint32_t get_pixel(uint32_t x, uint32_t y);
	void set_pixel(uint32_t x, uint32_t y, Uint32 clr);

	void clear();
	void draw_rect(Rect, RGBColor);
	void fill_rect(Rect, RGBAColor);
	void brighten_rect(Rect, int32_t factor);

	void blit(Point, Surface *, Rect srcrc, bool enable_alpha = true);
	void blit(Rect dstrc, Surface *, Rect srcrc, bool enable_alpha = true);
	//void fast_blit(Surface *);
	
	void set_resized(unsigned int w, unsigned int h){
		m_dest_w = w; m_dest_h = h;
	}

	oglTexture & getTexture(){return *m_texture;}

private:
	SurfaceOpenGL & operator= (SurfaceOpenGL const &);
        explicit SurfaceOpenGL(SurfaceOpenGL const &);
	
	oglTexture * m_texture;
	bool m_glTexUpdate;

	void set_subwin(Rect r);
	void unset_subwin();

	uint8_t * m_pixels;
	bool m_locked;

	uint32_t m_dest_w, m_dest_h;
};

#endif
