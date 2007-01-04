/*
 * Copyright (C) 2002-2007 by the Widelands Development Team
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

#ifndef __S__TUTSELECTMAP_H
#define __S__TUTSELECTMAP_H

#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_textarea.h"

/**
 * Fullscreen Menu for The Barbarians tutorial Mission
 */
struct Fullscreen_Menu_TutorialSelectMap : public Fullscreen_Menu_Base {
      Fullscreen_Menu_TutorialSelectMap();

      const char* get_mapname(int code );

private:
	UI::Textarea                                         title;
	UI::IDButton<Fullscreen_Menu_TutorialSelectMap, int> mission_1;
	UI::IDButton<Fullscreen_Menu_TutorialSelectMap, int> mission_2;
	UI::Textarea                                         title2;
	UI::IDButton<Fullscreen_Menu_TutorialSelectMap, int> empire_1;
	UI::IDButton<Fullscreen_Menu_TutorialSelectMap, int> back;
};

#endif // __S__TUTSELECTMAP_H
