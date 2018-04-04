/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H
#define WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H

#include "graphic/color.h"
#include "graphic/styles/font_style.h"

namespace UI {

struct ProgressbarStyleInfo {
	UI::FontStyleInfo font;
	RGBColor low_color;
	RGBColor medium_color;
	RGBColor high_color;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H
