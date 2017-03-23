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

#ifndef WL_UI_FSMENU_INTRO_H
#define WL_UI_FSMENU_INTRO_H

#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"

/**
 * Fullscreen Menu with Splash Screen (at the moment).
 * This simply waits modal for a click and in the meantime
 * shows the splash screen
 */
class FullscreenMenuIntro : public FullscreenMenuBase {
public:
	FullscreenMenuIntro();

protected:
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym) override;

private:
	UI::Textarea message_;
};

#endif  // end of include guard: WL_UI_FSMENU_INTRO_H
