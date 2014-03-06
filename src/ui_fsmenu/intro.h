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

#ifndef FULLSCREEN_MENU_INTRO_H
#define FULLSCREEN_MENU_INTRO_H

#include "ui_fsmenu/base.h"
#include "ui_basic/textarea.h"

/**
 * Fullscreen Menu with Splash Screen (at the moment).
 * This simply waits modal for a click and in the meantime
 * shows the splash screen
 */
struct Fullscreen_Menu_Intro : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Intro();

protected:
	virtual bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;
	virtual bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_keysym) override;
private:
	UI::Textarea m_message;
};

#endif
