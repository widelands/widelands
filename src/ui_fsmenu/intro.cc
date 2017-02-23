/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_fsmenu/intro.h"

#include "base/i18n.h"

FullscreenMenuIntro::FullscreenMenuIntro()
   : FullscreenMenuBase(),

     // Text area
     message_(this,
              get_w() / 2,
              get_h() * 19 / 20,
              _("Press any key or click to continue…"),
				  UI::Align::kCenter) {
	message_.set_fontsize(fs_small() * 6 / 5);
	message_.set_color(RGBColor(192, 192, 128));
	add_overlay_image("images/loadscreens/splash.jpg", FullscreenWindow::Alignment(UI::Align::kCenter, UI::Align::kCenter));
}

bool FullscreenMenuIntro::handle_mousepress(uint8_t, int32_t, int32_t) {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	return true;
}

bool FullscreenMenuIntro::handle_key(const bool down, const SDL_Keysym) {
	if (down) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}

	return false;
}
