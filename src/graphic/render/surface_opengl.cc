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

#include "surface_opengl.h"
#include "log.h"

#include <cassert>

#ifdef USE_OPENGL

SurfaceOpenGL::SurfaceOpenGL(SDL_Surface & par_surface): 
	Surface(par_surface.w, par_surface.h, SURFACE_SOURCE),
	m_glTexUpdate(false),
	m_pixels(NULL),
	m_locked(false),
	m_dest_w(0),
	m_dest_h(0)
{
	GLuint texture;
	SDL_Surface *surface;
	GLenum pixels_format, pixels_type;
	GLint  Bpp;

	surface = &par_surface;

	if(surface->format->palette or (surface->format->colorkey > 0))
	{
		log("Warning: trying to use a paletted picture for opengl texture\n");
		surface = SDL_DisplayFormatAlpha(&par_surface);
		SDL_BlitSurface(surface, 0, &par_surface, 0);
		SDL_FreeSurface(&par_surface);
	}

	SDL_PixelFormat const & fmt = *surface->format;
	Bpp = fmt.BytesPerPixel;

	log
		("SurfaceOpenGL::SurfaceOpenGL(SDL_Surface) Size: (%d, %d) %db(%dB) ", get_w(), get_h(),
		 fmt.BitsPerPixel, Bpp);

	log("R:%X, G:%X, B:%X, A:%X", fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
		 
	if(Bpp==4) {
		if(fmt.Rmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Bmask==0x00ff0000) {
			if(fmt.Amask==0xff000000) {
				pixels_format=GL_RGBA; log(" RGBA 8888 ");
			} else {
				pixels_format=GL_RGB; log(" RGB 8880 ");
			}
		} else if(fmt.Bmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Rmask==0x00ff0000) {
			if(fmt.Amask==0xff000000) { 
				pixels_format=GL_BGRA; log(" RGBA 8888 ");
			} else {
				pixels_format=GL_BGR; log(" RGBA 8888 ");
			}
		} else
			assert(false);
		pixels_type=GL_UNSIGNED_INT_8_8_8_8_REV;
	} else if (Bpp==3) {
		if(fmt.Rmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Bmask==0x00ff0000) {
			pixels_format=GL_RGB; log(" RGB 888 ");
		} else
			assert(false);
		pixels_type=GL_UNSIGNED_BYTE;
	} else if (Bpp==2) {
		if((fmt.Rmask==0xF800) and (fmt.Gmask==0x7E0) and (fmt.Bmask==0x1F)) {
			pixels_format=GL_RGB; log(" RGB 565"); 
		} else if ((fmt.Bmask==0xF800) and (fmt.Gmask==0x7E0) and (fmt.Rmask==0x1F)) {
			pixels_format=GL_BGR; log(" BGR 565"); 
		} else
			assert(false);
		pixels_type = GL_UNSIGNED_SHORT_5_6_5;
	} else
		assert(false);
	log("\n");

	// Let OpenGL create a texture object
	glGenTextures( 1, &texture );

	// selcet the texture object
	glBindTexture( GL_TEXTURE_2D, texture );

	// set texture filter to siply take the nearest pixel.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	SDL_LockSurface(surface);
	glTexImage2D( GL_TEXTURE_2D, 0, Bpp, surface->w, surface->h, 0,
	pixels_format, pixels_type, surface->pixels );
	SDL_UnlockSurface(surface);

	SDL_FreeSurface(surface);

	m_texture = new oglTexture(texture);
	m_glTexUpdate = false;
}


SurfaceOpenGL::~SurfaceOpenGL() {
	if(m_texture)
		delete m_texture;
}


SurfaceOpenGL::SurfaceOpenGL(int w, int h):
	Surface(w, h, SURFACE_SOURCE),
	m_texture(0),
	m_glTexUpdate(false),
	m_pixels(NULL),
	m_locked(false),
	m_dest_w(0),
	m_dest_h(0)
{
	log("SurfaceOpenGL::SurfaceOpenGL(%d, %d)", w, h);
}


void SurfaceOpenGL::lock() {
	if (m_locked)
		return;
	try {
		m_pixels = new uint8_t[m_w * m_h * 4];
	} catch (std::bad_alloc) {
		return;
	}

	if (m_surf_type == SURFACE_SCREEN)
		glReadPixels
			( 0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels );
	else if (m_texture) {
		glBindTexture( GL_TEXTURE_2D, m_texture->id());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels);
		m_glTexUpdate = false;
	} else
		m_glTexUpdate = true;

	m_locked = true;
}


void SurfaceOpenGL::unlock() {
	if (not m_locked)
		return;
	assert(m_pixels);

	if (m_glTexUpdate) {
		assert(m_surf_type != SURFACE_SCREEN);
		if (!m_texture)
		{
			GLuint texture;
			glGenTextures( 1, &texture );

			// selcet the texture object
			glBindTexture( GL_TEXTURE_2D, texture );

			// set texture filter to siply take the nearest pixel.
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			m_texture = new oglTexture(texture);
		}
		glBindTexture( GL_TEXTURE_2D, m_texture->id());
		glTexImage2D
			(GL_TEXTURE_2D, 0, 4, m_w, m_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels);
	}

	delete[] m_pixels;
	m_pixels = NULL;
	m_locked = false;
}


uint32_t SurfaceOpenGL::get_pixel(uint32_t x, uint32_t y) {
	x += m_offsx;
	y += m_offsy;

	assert(x < get_w());
	assert(y < get_h());
	assert(m_locked);

	return *reinterpret_cast<uint32_t *>(m_pixels + y*get_pitch() + x*4);
}


void SurfaceOpenGL::set_pixel(uint32_t x, uint32_t y, const Uint32 clr) {
	x += m_offsx;
	y += m_offsy;

	assert(x < get_w());
	assert(y < get_h());
	assert(m_locked);
	m_glTexUpdate = true;

	*reinterpret_cast<uint32_t *>(m_pixels + y*get_pitch() + x*4) = clr;
}

#endif //USE_OPENGL