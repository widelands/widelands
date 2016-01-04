/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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
 *
 */

#ifndef WL_GRAPHIC_GL_BLIT_PROGRAM_H
#define WL_GRAPHIC_GL_BLIT_PROGRAM_H

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/gl/blit_data.h"
#include "graphic/gl/system_headers.h"

class BlitProgram;


class VanillaBlitProgram {
public:
	struct Arguments {
		FloatRect destination_rect;
		float z_value;
		BlitData texture;
		float opacity;
		BlendMode blend_mode;
	};

	// Returns the (singleton) instance of this class.
	static VanillaBlitProgram& instance();
	~VanillaBlitProgram();

	// Draws the rectangle 'gl_src_rect' from the texture with the name
	// 'texture' to 'gl_dest_rect' in the currently bound framebuffer. All alpha
	// values are multiplied by 'opacity' during the blit.
	// All coordinates are in the OpenGL frame. The 'blend_mode' defines if the
	// values are copied or if alpha values are used.
	void draw(const FloatRect& gl_dest_rect,
				 const float z_value,
				 const BlitData& texture,
				 float opacity,
	          const BlendMode blend_mode);

	// Draws a bunch of items at once.
	void draw(const std::vector<Arguments>& arguments);

private:
	VanillaBlitProgram();

	std::unique_ptr<BlitProgram> blit_program_;

	DISALLOW_COPY_AND_ASSIGN(VanillaBlitProgram);
};

class MonochromeBlitProgram {
public:
	struct Arguments {
		FloatRect destination_rect;
		float z_value;
		BlitData texture;
		RGBAColor blend;
		BlendMode blend_mode;
	};

	// Returns the (singleton) instance of this class.
	static MonochromeBlitProgram& instance();
	~MonochromeBlitProgram();

	// Draws the rectangle 'gl_src_rect' from the texture with the name
	// 'texture' to 'gl_dest_rect' in the currently bound framebuffer. All
	// coordinates are in the OpenGL frame. The image is first converted to
	// luminance, then all values are multiplied with blend.
	void draw(const FloatRect& gl_dest_rect,
				 const float z_value,
				 const BlitData& blit_source,
				 const RGBAColor& blend);

	// Draws a bunch of items at once.
	void draw(const std::vector<Arguments>& arguments);

private:
	MonochromeBlitProgram();

	std::unique_ptr<BlitProgram> blit_program_;

	DISALLOW_COPY_AND_ASSIGN(MonochromeBlitProgram);
};

class BlendedBlitProgram {
public:
	struct Arguments {
		FloatRect destination_rect;
		float z_value;
		BlitData texture;
		BlitData mask;
		RGBAColor blend;
		BlendMode blend_mode;
	};

	// Returns the (singleton) instance of this class.
	static BlendedBlitProgram& instance();
	~BlendedBlitProgram();

	// Draws the rectangle 'gl_src_rect' from the texture with the name
	// 'gl_texture_image' to 'gl_dest_rect' in the currently bound framebuffer. All
	// coordinates are in the OpenGL frame. The 'texture_mask' is used to selectively apply
	// the 'blend'. This is used for blitting player colored images.
	void draw(const FloatRect& gl_dest_rect,
	          const float z_value,
				 const BlitData& texture,
				 const BlitData& mask,
	          const RGBAColor& blend);

	// Draws a bunch of items at once.
	void draw(const std::vector<Arguments>& arguments);

private:
	BlendedBlitProgram();

	std::unique_ptr<BlitProgram> blit_program_;

	DISALLOW_COPY_AND_ASSIGN(BlendedBlitProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_BLIT_PROGRAM_H
