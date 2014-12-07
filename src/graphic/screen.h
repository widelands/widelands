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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef WL_GRAPHIC_SCREEN_H
#define WL_GRAPHIC_SCREEN_H

#include <memory>

#include "graphic/surface.h"

/**
 * The screen.
 */
class Screen : public Surface {
public:
	Screen(uint16_t w, uint16_t h);
	virtual ~Screen() {}

	// Implements Surface
	void lock(LockMode) override;
	void unlock(UnlockMode) override;
	void setup_gl() override;
	void pixel_to_gl(float* x, float* y) const override;
	int get_gl_texture() const override;
	const FloatRect& texture_coordinates() const override;

	// Reads out the current pixels in the framebuffer and returns them as a
	// texture for screenshots. This is a very slow process, so use with care.
	std::unique_ptr<Texture> to_texture() const;
};

#endif  // end of include guard: WL_GRAPHIC_SCREEN_H
