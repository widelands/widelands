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

#include "graphic/render/gl_surface_screen.h"

#include <algorithm>
#include <cassert>

#include "graphic/render/gl_utils.h"

GLSurfaceScreen::GLSurfaceScreen(uint16_t w, uint16_t h)
{
	m_w = w;
	m_h = h;
}


/**
 * Swap order of rows in m_pixels, to compensate for the upside-down nature of the
 * OpenGL coordinate system.
 */
void GLSurfaceScreen::swap_rows()
{
	uint8_t * begin_row = m_pixels.get();
	uint8_t * end_row = m_pixels.get() + (m_w * (m_h - 1) * 4);

	while (begin_row < end_row) {
		for (uint16_t x = 0; x < m_w * 4; ++x)
			std::swap(begin_row[x], end_row[x]);

		begin_row += m_w * 4;
		end_row -= m_w * 4;
	}
}

const SDL_PixelFormat & GLSurfaceScreen::format() const {
	return gl_rgba_format();
}

void GLSurfaceScreen::lock(Surface::LockMode mode)
{
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_w * m_h * 4]);

	if (mode == Lock_Normal) {
		// FIXME: terrain dither picture somehow leave the alpha
		// channel with non-1 values, so it is cleared before
		// accessing pixels.
		glColorMask(false, false, false, true);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(true, true, true, true);
		glReadPixels(0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
		swap_rows();
	}
}

void GLSurfaceScreen::unlock(Surface::UnlockMode mode)
{
	assert(m_pixels);

	if (mode == Unlock_Update) {
		swap_rows();
		glDrawPixels(m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}

	m_pixels.reset(nullptr);
}

uint16_t GLSurfaceScreen::get_pitch() const {
	return 4 * m_w;
}
