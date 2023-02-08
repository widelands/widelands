/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 *
 */

#ifndef WL_GRAPHIC_ALIGN_H
#define WL_GRAPHIC_ALIGN_H

#include <cstdint>
#include <string>

#include "base/rect.h"
#include "base/vector.h"

namespace UI {

// TODO(GunChleoc): Step 1: Clean up superfluous usages of kLeft/kTop, especially with dalls to
// Box::add and Textarea constructor.
// TODO(GunChleoc): Step 2: Rename to kBeginning, kCenter, kEnd
/// Horizontal or vertical alignment
enum class Align {
	kLeft,
	kCenter,
	kRight,
	kTop = kLeft,
	kBottom = kRight,
};

void center_vertically(uint32_t h, Vector2i* pt);
void correct_for_align(Align, uint32_t w, Vector2i* pt);
Align mirror_alignment(Align alignment, bool is_rtl);

/**
 * Position an image at the screen center, as large as possible without upscaling,
 * with or without cropping the image.
 */
Rectf fit_image(
   float img_width, float img_height, float available_width, float available_height, bool crop);

}  // namespace UI
#endif  // end of include guard: WL_GRAPHIC_ALIGN_H
