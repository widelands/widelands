/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#ifndef __S__INTRO_H
#define __S__INTRO_H

#include "fullscreen_menu_base.h"

/**
 * Fullscreen Menu with Splash Screen (at the moment).
 * This simply waits modal for a click and in the meantime
 * shows the splash screen
 */
struct Fullscreen_Menu_Intro : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Intro();

protected:
	virtual bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);
	virtual bool handle_key(bool down, int32_t code, char c);
};

#endif // __S__INTRO_H
