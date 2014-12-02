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

#include <boost/format.hpp>

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/font.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "io/filesystem/filesystem.h"
#include "profile/profile.h"
#include "wlapplication.h"
#include "wui/text_constants.h"

/*
==============================================================================

FullscreenMenuBase

==============================================================================
*/

struct FullscreenMenuBase::Data {
	UI::TextStyle textstyle_small;
	UI::TextStyle textstyle_big;
	std::string background_image;
};

/**
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
FullscreenMenuBase::FullscreenMenuBase(char const * const bgpic)
	: UI::Panel(nullptr, 0, 0, gr_x(), gr_y()),
	d(new Data)
{
	d->textstyle_small = UI::TextStyle::ui_small();
	d->textstyle_small.font = UI::Font::get(ui_fn(), fs_small());

	d->textstyle_big = UI::TextStyle::ui_big();
	d->textstyle_big.font = UI::Font::get(ui_fn(), fs_big());
	d->background_image = (boost::format("pics/%s") % bgpic).str();
}

FullscreenMenuBase::~FullscreenMenuBase()
{
}


/**
 * Draw the background / splash screen
*/
void FullscreenMenuBase::draw(RenderTarget & dst) {
	const Image* bg = g_gr->images().get(d->background_image);
	dst.blitrect_scale(Rect(0, 0, get_w(), get_h()), bg, Rect(0, 0, bg->width(), bg->height()));
}


uint32_t FullscreenMenuBase::gr_x() {
	return g_gr->get_xres();
}

uint32_t FullscreenMenuBase::gr_y() {
	return g_gr->get_yres();
}


uint32_t FullscreenMenuBase::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

uint32_t FullscreenMenuBase::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

UI::TextStyle & FullscreenMenuBase::ts_small()
{
	return d->textstyle_small;
}

UI::TextStyle & FullscreenMenuBase::ts_big()
{
	return d->textstyle_big;
}

UI::Font * FullscreenMenuBase::font_small()
{
	return d->textstyle_small.font;
}

UI::Font * FullscreenMenuBase::font_big()
{
	return d->textstyle_big.font;
}
