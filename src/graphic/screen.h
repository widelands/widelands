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

#ifndef WL_GRAPHIC_SCREEN_H
#define WL_GRAPHIC_SCREEN_H

#include <memory>

#include "base/macros.h"
#include "graphic/surface.h"
#include "graphic/texture.h"

/**
 * The screen.
 */
class Screen : public Surface {
public:
	Screen(int w, int h);
	~Screen() override {
	}

	// Implements Surface.
	int width() const override;
	int height() const override;

	// Reads out the current pixels in the framebuffer and returns
	// them as a texture for screenshots. This is a very slow process,
	// so use with care.
	std::unique_ptr<Texture> to_texture() const;

private:
	void do_blit(const Rectf& dst_rect,
	             const BlitData& texture,
	             float opacity,
	             BlendMode blend_mode) override;
	void do_blit_blended(const Rectf& dst_rect,
	                     const BlitData& texture,
	                     const BlitData& mask,
	                     const RGBColor& blend) override;
	void do_blit_monochrome(const Rectf& dst_rect,
	                        const BlitData& texture,
	                        const RGBAColor& blend) override;
	void do_draw_line_strip(std::vector<DrawLineProgram::PerVertexData> vertices) override;
	void do_fill_rect(const Rectf& dst_rect, const RGBAColor& color, BlendMode blend_mode) override;

	const int w_, h_;

	DISALLOW_COPY_AND_ASSIGN(Screen);
};

#endif  // end of include guard: WL_GRAPHIC_SCREEN_H
