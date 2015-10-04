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
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "io/filesystem/filesystem.h"
#include "profile/profile.h"
#include "wlapplication.h"

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

	background_image_ = (boost::format("pics/%s") % bgpic).str();
}

FullscreenMenuBase::~FullscreenMenuBase()
{
}


/**
 * Draw the background / splash screen
*/
void FullscreenMenuBase::draw(RenderTarget & dst) {
	const Image* bg = g_gr->images().get(background_image_);
	dst.blitrect_scale(Rect(0, 0, get_w(), get_h()),
	                   bg,
	                   Rect(0, 0, bg->width(), bg->height()),
	                   1.,
	                   BlendMode::UseAlpha);
}

uint32_t FullscreenMenuBase::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

uint32_t FullscreenMenuBase::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

bool FullscreenMenuBase::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		switch (code.sym) {
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				clicked_ok();
				return true;
			case SDLK_ESCAPE:
				clicked_back();
				return true;
			default:
				break; // not handled
		}
	}
	return UI::Panel::handle_key(down, code);
}


void FullscreenMenuBase::clicked_back() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
}
void FullscreenMenuBase::clicked_ok() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
}
