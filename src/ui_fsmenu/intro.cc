/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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
	: FullscreenMenuBase("splash.jpg"),

// Text area
m_message
	(this,
	 get_w() / 2, get_h() * 19 / 20,
	 _("Press any key or click to continue ..."), UI::Align_HCenter)
{
	m_message.set_font(ui_fn(), fs_small() * 6 / 5, RGBColor(192, 192, 128));
}

bool FullscreenMenuIntro::handle_mousepress  (uint8_t, int32_t, int32_t)
{
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);

	return true;
}
bool FullscreenMenuIntro::handle_mouserelease(uint8_t, int32_t, int32_t)
{
	return true;
}

bool FullscreenMenuIntro::handle_key(const bool down, const SDL_Keysym)
{
	if (down) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}

	return false;
}
