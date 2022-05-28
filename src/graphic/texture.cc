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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "graphic/texture.h"

#include <cassert>

#include <SDL_surface.h>

#include "base/macros.h"
#include "base/multithreading.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/utils.h"
#include "graphic/sdl_utils.h"
#include "graphic/surface.h"

namespace {

namespace {

/**
 * \return the standard 32-bit RGBA format that we use for our textures.
 */
const SDL_PixelFormat& rgba_format() {
	static SDL_PixelFormat format;
	static bool init = false;
	if (init) {
		return format;
	}

	init = true;
	memset(&format, 0, sizeof(format));
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
	format.Rmask = 0x000000ff;
	format.Gmask = 0x0000ff00;
	format.Bmask = 0x00ff0000;
	format.Amask = 0xff000000;
	format.Rshift = 0;
	format.Gshift = 8;
	format.Bshift = 16;
	format.Ashift = 24;
	return format;
}

}  // namespace

class GlFramebuffer {
public:
	static GlFramebuffer& instance() {
		static GlFramebuffer gl_framebuffer;
		return gl_framebuffer;
	}

	~GlFramebuffer() {
		glDeleteFramebuffers(1, &gl_framebuffer_id_);
	}

	GLuint id() const {
		return gl_framebuffer_id_;
	}

private:
	GlFramebuffer() {
		// Generate the framebuffer for Offscreen rendering.
		glGenFramebuffers(1, &gl_framebuffer_id_);
	}

	GLuint gl_framebuffer_id_;

	DISALLOW_COPY_AND_ASSIGN(GlFramebuffer);
};

bool is_bgr_surface(const SDL_PixelFormat& fmt) {
	return (fmt.Bmask == 0x000000ff && fmt.Gmask == 0x0000ff00 && fmt.Rmask == 0x00ff0000);
}

}  // namespace

Texture::Texture(int w, int h) : owns_texture_(false) {
	init(w, h);

	if (blit_data_.texture_id == 0) {
		return;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA), width(), height(), 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, nullptr);
}

Texture::Texture(SDL_Surface* surface, bool intensity) : owns_texture_(false) {
	init(surface->w, surface->h);

	// Convert image data. BGR Surface support is an extension for
	// OpenGL ES 2, which we rather not rely on. So we convert our
	// surfaces in software.
	// TODO(sirver): SDL_TTF returns all data in BGR format. If we
	// use freetype directly we might be able to avoid that.
	uint8_t bpp = surface->format->BytesPerPixel;

	if ((surface->format->palette != nullptr) || width() != surface->w || height() != surface->h ||
	    (bpp != 3 && bpp != 4) || is_bgr_surface(*surface->format)) {
		SDL_Surface* converted = empty_sdl_surface(width(), height());
		assert(converted);
		SDL_SetSurfaceAlphaMod(converted, SDL_ALPHA_OPAQUE);
		SDL_SetSurfaceBlendMode(converted, SDL_BLENDMODE_NONE);
		SDL_SetSurfaceAlphaMod(surface, SDL_ALPHA_OPAQUE);
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		SDL_BlitSurface(surface, nullptr, converted, nullptr);
		SDL_FreeSurface(surface);
		surface = converted;
		bpp = surface->format->BytesPerPixel;
	}

	const GLenum pixels_format = bpp == 4 ? GL_RGBA : GL_RGB;

	SDL_LockSurface(surface);

	Gl::swap_rows(width(), height(), surface->pitch, bpp, static_cast<uint8_t*>(surface->pixels));

	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(intensity ? GL_INTENSITY : GL_RGBA), width(),
	             height(), 0, pixels_format, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);
}

Texture::Texture(const GLuint texture, const Recti& subrect, int parent_w, int parent_h)
   : owns_texture_(false) {
	if (parent_w == 0 || parent_h == 0) {
		throw wexception("Created a sub Texture with zero height and width parent.");
	}

	blit_data_ = BlitData{
	   texture,
	   parent_w,
	   parent_h,
	   subrect.cast<float>(),
	};
}

Texture::~Texture() {
	if (owns_texture_) {
		uint32_t texture_id = blit_data_.texture_id;
		NoteThreadSafeFunction::instantiate(
		   [texture_id]() { Gl::State::instance().delete_texture(texture_id); }, false);
	}
}

int Texture::width() const {
	return blit_data_.rect.w;
}

int Texture::height() const {
	return blit_data_.rect.h;
}

void Texture::init(uint16_t w, uint16_t h) {
	assert(is_initializer_thread());

	blit_data_ = {
	   0,  // initialized below
	   w,
	   h,
	   Rectf(0.f, 0.f, w, h),
	};
	if (w * h == 0) {
		return;
	}

	owns_texture_ = true;
	glGenTextures(1, &blit_data_.texture_id);
	Gl::State::instance().bind(GL_TEXTURE0, blit_data_.texture_id);

	// set texture filter to use linear filtering. This looks nicer for resized
	// texture. Most textures and images are not resized so the filtering
	// makes no difference.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
}

void Texture::lock() {
	assert(is_initializer_thread());

	if (blit_data_.texture_id == 0) {
		return;
	}

	if (pixels_) {
		throw wexception("Called lock() on locked surface.");
	}
	if (!owns_texture_) {
		throw wexception("A surface that does not own its pixels can not be locked..");
	}

	pixels_.reset(new uint8_t[width() * height() * 4]);

	Gl::State::instance().bind(GL_TEXTURE0, blit_data_.texture_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_.get());
}

void Texture::unlock(UnlockMode mode) {
	assert(is_initializer_thread());

	if (width() <= 0 || height() <= 0) {
		return;
	}
	assert(pixels_);

	if (mode == Unlock_Update) {
		Gl::State::instance().bind(GL_TEXTURE0, blit_data_.texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA), width(), height(), 0, GL_RGBA,
		             GL_UNSIGNED_BYTE, pixels_.get());
	}

	pixels_.reset(nullptr);
}

RGBAColor Texture::get_pixel(uint16_t x, uint16_t y) {
	assert(pixels_);
	assert(x < width());
	assert(y < height());

	RGBAColor color;

	SDL_GetRGBA(*reinterpret_cast<uint32_t*>(&pixels_[(height() - y - 1) * 4 * width() + 4 * x]),
	            &rgba_format(), &color.r, &color.g, &color.b, &color.a);
	return color;
}

void Texture::set_pixel(uint16_t x, uint16_t y, const RGBAColor& color) {
	assert(pixels_);
	assert(x < width());
	assert(y < height());

	uint8_t* data = &pixels_[(height() - y - 1) * 4 * width() + 4 * x];
	uint32_t packed_color = SDL_MapRGBA(&rgba_format(), color.r, color.g, color.b, color.a);
	*(reinterpret_cast<uint32_t*>(data)) = packed_color;
}

void Texture::setup_gl() const {
	assert(blit_data_.texture_id != 0);
	Gl::State::instance().bind_framebuffer(GlFramebuffer::instance().id(), blit_data_.texture_id);
	glViewport(0, 0, width(), height());
}

void Texture::do_blit(const Rectf& dst_rect,
                      const BlitData& texture,
                      float opacity,
                      BlendMode blend_mode) {
	if (blit_data_.texture_id == 0) {
		return;
	}
	setup_gl();
	BlitProgram::instance().draw(dst_rect, 0.f, texture, BlitData{0, 0, 0, Rectf()},
	                             RGBAColor(0, 0, 0, 255 * opacity), blend_mode);
}

void Texture::do_blit_blended(const Rectf& dst_rect,
                              const BlitData& texture,
                              const BlitData& mask,
                              const RGBColor& blend) {

	if (blit_data_.texture_id == 0) {
		return;
	}
	setup_gl();
	BlitProgram::instance().draw(dst_rect, 0.f, texture, mask, blend, BlendMode::UseAlpha);
}

void Texture::do_blit_monochrome(const Rectf& dst_rect,
                                 const BlitData& texture,
                                 const RGBAColor& blend) {
	if (blit_data_.texture_id == 0) {
		return;
	}
	setup_gl();
	BlitProgram::instance().draw_monochrome(dst_rect, 0.f, texture, blend);
}

void Texture::do_draw_line_strip(std::vector<DrawLineProgram::PerVertexData> vertices) {
	if (blit_data_.texture_id == 0) {
		return;
	}
	setup_gl();
	DrawLineProgram::instance().draw(
	   {DrawLineProgram::Arguments{vertices, 0.f, BlendMode::UseAlpha}});
}

void Texture::do_fill_rect(const Rectf& dst_rect, const RGBAColor& color, BlendMode blend_mode) {
	if (blit_data_.texture_id == 0) {
		return;
	}
	setup_gl();
	FillRectProgram::instance().draw(dst_rect, 0.f, color, blend_mode);
}

const BlitData& Texture::blit_data() const {
	return blit_data_;
}
