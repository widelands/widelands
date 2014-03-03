/*
 * Copyright (C) 2002, 2007-2010 by the Widelands Development Team
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
 *
 */

#include "ui_fsmenu/base.h"

#include <cstdio>

#include "constants.h"
#include "graphic/font.h"
#include "graphic/graphic.h"
#include "graphic/image_transformations.h"
#include "graphic/rendertarget.h"
#include "io/filesystem/filesystem.h"
#include "log.h"
#include "profile/profile.h"
#include "wexception.h"
#include "wlapplication.h"

/*
==============================================================================

Fullscreen_Menu_Base

==============================================================================
*/

struct Fullscreen_Menu_Base::Data {
	const Image* res_background;
	UI::TextStyle textstyle_small;
	UI::TextStyle textstyle_big;
};

/**
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
Fullscreen_Menu_Base::Fullscreen_Menu_Base(char const * const bgpic)
	: UI::Panel(nullptr, 0, 0, gr_x(), gr_y()),
	d(new Data)
{
	// Load background graphics
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "pics/%s", bgpic);
	d->res_background = ImageTransformations::resize(g_gr->images().get(buffer), get_w(), get_h());

	d->textstyle_small = UI::TextStyle::ui_small();
	d->textstyle_small.font = UI::Font::get(ui_fn(), fs_small());

	d->textstyle_big = UI::TextStyle::ui_big();
	d->textstyle_big.font = UI::Font::get(ui_fn(), fs_big());
}

Fullscreen_Menu_Base::~Fullscreen_Menu_Base()
{
}


/**
 * Draw the background / splash screen
*/
void Fullscreen_Menu_Base::draw(RenderTarget & dst) {
	dst.blit(Point(0, 0), d->res_background);
}


uint32_t Fullscreen_Menu_Base::gr_x() {
	return g_gr->get_xres();
}

uint32_t Fullscreen_Menu_Base::gr_y() {
	return g_gr->get_yres();
}


uint32_t Fullscreen_Menu_Base::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

uint32_t Fullscreen_Menu_Base::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

UI::TextStyle & Fullscreen_Menu_Base::ts_small()
{
	return d->textstyle_small;
}

UI::TextStyle & Fullscreen_Menu_Base::ts_big()
{
	return d->textstyle_big;
}

UI::Font * Fullscreen_Menu_Base::font_small()
{
	return d->textstyle_small.font;
}

UI::Font * Fullscreen_Menu_Base::font_big()
{
	return d->textstyle_big.font;
}
