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

#include "graphic/screen.h"

#include <algorithm>
#include <cassert>
#include <memory>

#include "base/wexception.h"
#include "graphic/gl/utils.h"
#include "graphic/texture.h"

Screen::Screen(uint16_t w, uint16_t h)
{
	m_w = w;
	m_h = h;
}

void Screen::pixel_to_gl(float* x, float* y) const {
	*x = (*x / m_w) * 2. - 1.;
	*y = 1. - (*y / m_h) * 2.;
}

void Screen::lock(Surface::LockMode mode)
{
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_w * m_h * 4]);

	if (mode == Lock_Normal) {
		// TODO(unknown): terrain dither picture somehow leave the alpha
		// channel with non-1 values, so it is cleared before
		// accessing pixels.
		glColorMask(false, false, false, true);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(true, true, true, true);
		glReadPixels(0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}
}

void Screen::unlock(Surface::UnlockMode mode)
{
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glDrawPixels(m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}

	m_pixels.reset(nullptr);
}

void Screen::setup_gl() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int Screen::get_gl_texture() const {
	throw wexception("get_gl_texture() is not implemented for Screen.");
}

const FloatRect& Screen::texture_coordinates() const {
	throw wexception("texture_coordinates() is not implemented for Screen.");
}

std::unique_ptr<Texture> Screen::to_texture() const {
	std::unique_ptr<uint8_t[]> pixels(new uint8_t[m_w * m_h * 4]);
	glReadPixels(0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

	// Swap order of rows in m_pixels, to compensate for the upside-down nature of the
	// OpenGL coordinate system.
	uint8_t* begin_row = pixels.get();
	uint8_t* end_row = pixels.get() + (m_w * (m_h - 1) * 4);
	while (begin_row < end_row) {
		for (uint16_t x = 0; x < m_w * 4; ++x) {
			std::swap(begin_row[x], end_row[x]);
		}
		begin_row += m_w * 4;
		end_row -= m_w * 4;
	}

	// Ownership of pixels is not taken here. But the Texture() transfers it to
	// the GPU, frees the SDL surface and after that we are free to free
	// 'pixels'.
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels.get(),
	                                                m_w,
	                                                m_h,
	                                                32,
	                                                m_w * 4,
	                                                0x000000ff,
	                                                0x0000ff00,
	                                                0x00ff0000,
	                                                0xff000000);

	return std::unique_ptr<Texture>(new Texture(surface));
}
