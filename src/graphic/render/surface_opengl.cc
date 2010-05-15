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
#include "graphic/graphic.h"

#include <cassert>
#include <cmath>

#ifdef USE_OPENGL

long unsigned int pix_used = 0;
long unsigned int pix_aloc = 0;
long unsigned int num_tex = 0;

#define handle_glerror() \
	_handle_glerror(__FILE__, __LINE__)

SDL_PixelFormat * rgbafmt = NULL;

GLenum _handle_glerror(const char * file, unsigned int line)
{
	GLenum err = glGetError();
	if (err==GL_NO_ERROR)
		return err;

	log("%s:%d: OpenGL ERROR: ", file, line);

	switch (err)
	{
	case GL_INVALID_VALUE:
		log("invalid value\n");
		break;
	case GL_INVALID_ENUM:
		log("invalid enum\n");
		break;
	case GL_INVALID_OPERATION:
		log("invalid operation\n");
		break;
	case GL_STACK_OVERFLOW:
		log("stack overflow\n");
		break;
	case GL_STACK_UNDERFLOW:
		log("stack undeflow\n");
		break;
	case GL_OUT_OF_MEMORY:
		log("out of memory\n");
		break;
	case GL_TABLE_TOO_LARGE:
		log("table too large\n");
		break;
	default:
		log("unknown\n");
	}
	return err;
}

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

	handle_glerror();

	surface = &par_surface;

	if (g_gr->caps().gl.tex_power_of_two)
	{
		unsigned int wexp = log2(surface->w);
		unsigned int hexp = log2(surface->h);
		if (pow(2,wexp) < surface->w)
			wexp++;
		if (pow(2,hexp) < surface->h)
			hexp++;

		m_tex_w = pow(2,wexp);
		m_tex_h = pow(2,hexp);
	} else {
		m_tex_w = surface->w;
		m_tex_h = surface->h;
	}

	if
		(surface->format->palette or (surface->format->colorkey > 0) or
		 m_tex_w != static_cast<uint32_t>(surface->w) or 
		 m_tex_h != static_cast<uint32_t>(surface->h))
	{
		log("SurfaceOpenGL: convert surface for opengl\n");
		surface = SDL_CreateRGBSurface
			(SDL_SWSURFACE, m_tex_w, m_tex_h,
			 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		assert(surface);
		//SDL_DisplayFormatAlpha(&par_surface);
		SDL_SetAlpha(surface, 0, 0);
		SDL_SetAlpha(&par_surface, 0, 0);
		SDL_BlitSurface(&par_surface, 0, surface, 0);
		SDL_FreeSurface(&par_surface);
	}

	SDL_PixelFormat const & fmt = *surface->format;
	Bpp = fmt.BytesPerPixel;

	/*
	log
		("SurfaceOpenGL::SurfaceOpenGL(SDL_Surface) Size: (%d, %d) %db(%dB) ", m_tex_w, m_tex_h,
		 fmt.BitsPerPixel, Bpp);

	log("R:%X, G:%X, B:%X, A:%X", fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
	*/

	if(Bpp==4) {
		if(fmt.Rmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Bmask==0x00ff0000) {
			if(fmt.Amask==0xff000000) {
				pixels_format=GL_RGBA; //log(" RGBA 8888 ");
			} else {
				pixels_format=GL_RGB; //log(" RGB 8880 ");
			}
		} else if(fmt.Bmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Rmask==0x00ff0000) {
			if(fmt.Amask==0xff000000) { 
				pixels_format=GL_BGRA; //log(" BGRA 8888 ");
			} else {
				pixels_format=GL_BGR; //log(" BGRA 8888 ");
			}
		} else
			assert(false);
		pixels_type=GL_UNSIGNED_BYTE;
	} else if (Bpp==3) {
		if(fmt.Rmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Bmask==0x00ff0000) {
			pixels_format=GL_RGB; //log(" RGB 888 ");
		} else if (fmt.Bmask==0x000000ff and fmt.Gmask==0x0000ff00 and fmt.Rmask==0x00ff0000) {
			pixels_format=GL_BGR;
		}else
			assert(false);
		pixels_type=GL_UNSIGNED_BYTE;
	} else if (Bpp==2) {
		if((fmt.Rmask==0xF800) and (fmt.Gmask==0x7E0) and (fmt.Bmask==0x1F)) {
			pixels_format=GL_RGB; //log(" RGB 565"); 
		} else if ((fmt.Bmask==0xF800) and (fmt.Gmask==0x7E0) and (fmt.Rmask==0x1F)) {
			pixels_format=GL_BGR; //log(" BGR 565"); 
		} else
			assert(false);
		pixels_type = GL_UNSIGNED_SHORT_5_6_5;
	} else
		assert(false);
	//log("\n");

	// Let OpenGL create a texture object
	glGenTextures( 1, &texture );
	handle_glerror();

	// selcet the texture object
	glBindTexture( GL_TEXTURE_2D, texture );
	handle_glerror();

	// set texture filter to siply take the  nearest pixel.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	handle_glerror();

	SDL_LockSurface(surface);

	glTexImage2D( GL_TEXTURE_2D, 0, WL_GLINTERNALFORMAT, m_tex_w, m_tex_h, 0,
	pixels_format, pixels_type, surface->pixels );
	handle_glerror();

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);

	pix_used += m_w * m_h;
	pix_aloc += m_tex_w * m_tex_h;
	num_tex++;
	log
		("texture stats: num: %lu, used: %lu (%luM), "
		 "alocated: %lu (%luM) ++\n",
		 num_tex, pix_used * 4, pix_used * 4 / (1024 * 1024),
		 pix_aloc * 4, pix_aloc * 4/ (1024 * 1024));

	assert(glIsTexture(texture));

	m_texture = new oglTexture(texture);
	m_glTexUpdate = false;
}


SurfaceOpenGL::~SurfaceOpenGL() {
	if(m_texture) {
		pix_used -= m_w * m_h;
		pix_aloc -= m_tex_w * m_tex_h;
		num_tex--;
		delete m_texture;
	}
	log
		("~SurfaceOpenGL(): texture stats: num: %lu, "
		 "used: %lu (%luM), alocated: %lu (%luM) --\n",
		 num_tex, pix_used * 4, pix_used * 4 / (1024 * 1024),
		 pix_aloc * 4, pix_aloc * 4/ (1024 * 1024));
	delete[] m_pixels;
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
	if (g_gr and g_gr->caps().gl.tex_power_of_two)
	{
		unsigned int wexp = log2(w);
		unsigned int hexp = log2(h);
		if (pow(2,wexp) < w)
			wexp++;
		if (pow(2,hexp) < h)
			hexp++;

		m_tex_w = pow(2, wexp);
		m_tex_h = pow(2, hexp);
	} else {
		m_tex_w = w;
		m_tex_h = h;
	}
	log("SurfaceOpenGL::SurfaceOpenGL(%d, %d): texture (%d, %d)", w, h, m_tex_w, m_tex_h);
}

const SDL_PixelFormat * SurfaceOpenGL::get_format() const
{
	if (rgbafmt)
		return rgbafmt;
	rgbafmt = new SDL_PixelFormat;
	rgbafmt->BitsPerPixel = 32;
	rgbafmt->BytesPerPixel = 4;
	rgbafmt->Rmask = 0x000000ff; rgbafmt->Gmask = 0x0000ff00;
	rgbafmt->Bmask = 0x00ff0000; rgbafmt->Amask = 0xff000000;
	rgbafmt->Ashift = 24; rgbafmt->Bshift = 16; rgbafmt->Gshift = 8;
	rgbafmt->Ashift = 0; rgbafmt->palette = NULL;
	return rgbafmt;
}


void SurfaceOpenGL::lock() {
	if (m_locked)
		return;
	try {
		if(m_surf_type == SURFACE_SCREEN)
			m_pixels = new uint8_t[m_w * m_h * 4];
		else
			m_pixels = new uint8_t[m_tex_w * m_tex_h * 4];
	} catch (std::bad_alloc) {
		return;
	}
	if (m_surf_type == SURFACE_SCREEN)
		glReadPixels
			( 0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels );
	else if (m_texture) {
		assert(glIsTexture(m_texture->id()));
		glBindTexture( GL_TEXTURE_2D, m_texture->id());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels);
		m_glTexUpdate = false;
	} else
		m_glTexUpdate = true;
	log("locked opengl surface(%d, %d)\n", m_tex_w, m_tex_h);
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
			log("unlock opengl surface: create new texture (%d, %d)\n", m_tex_w, m_tex_h);
			GLuint texture;
			glGenTextures( 1, &texture );

			// selcet the texture object
			glBindTexture( GL_TEXTURE_2D, texture );

			// set texture filter to siply take the nearest pixel.
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			pix_used += m_w * m_h;
			pix_aloc += m_tex_w * m_tex_h;
			num_tex++;

			m_texture = new oglTexture(texture);
		}
		glBindTexture( GL_TEXTURE_2D, m_texture->id());
		log("unlock opengl surface: (%d, %d)\n", m_tex_w, m_tex_h);
		glTexImage2D
			(GL_TEXTURE_2D, 0, WL_GLINTERNALFORMAT, m_tex_w, m_tex_h, 0, GL_RGBA,
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