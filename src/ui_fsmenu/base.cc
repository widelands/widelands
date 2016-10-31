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

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "io/filesystem/filesystem.h"
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
FullscreenMenuBase::FullscreenMenuBase() : UI::FullscreenWindow() {
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.width, message.height);
		   layout();
		});
}

FullscreenMenuBase::~FullscreenMenuBase() {
}

bool FullscreenMenuBase::handle_key(bool down, SDL_Keysym code) {
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
			break;  // not handled
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
