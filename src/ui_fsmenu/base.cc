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
#include "graphic/image_transformations.h"
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

/**
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
FullscreenMenuBase::FullscreenMenuBase(char const* const bgpic)
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()) {
	// Load background graphics
	const std::string bgpicpath = (boost::format("pics/%s") % bgpic).str();
	background_ = ImageTransformations::resize(g_gr->images().get(bgpicpath), get_w(), get_h());

	textstyle_small_ = UI::TextStyle::ui_small();
	textstyle_small_.font = UI::Font::get(ui_fn(), fs_small());

	textstyle_big_ = UI::TextStyle::ui_big();
	textstyle_big_.font = UI::Font::get(ui_fn(), fs_big());
}

FullscreenMenuBase::~FullscreenMenuBase()
{
}


/**
 * Draw the background / splash screen
*/
void FullscreenMenuBase::draw(RenderTarget & dst) {
	dst.blit(Point(0, 0), background_);
}

uint32_t FullscreenMenuBase::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

uint32_t FullscreenMenuBase::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

UI::TextStyle & FullscreenMenuBase::ts_small()
{
	return textstyle_small_;
}

UI::TextStyle & FullscreenMenuBase::ts_big()
{
	return textstyle_big_;
}

UI::Font * FullscreenMenuBase::font_small()
{
	return textstyle_small_.font;
}

UI::Font * FullscreenMenuBase::font_big()
{
	return textstyle_big_.font;
}
