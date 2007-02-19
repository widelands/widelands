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

#include "error.h"
#include "fullscreen_menu_tutorial_select_map.h"
#include "i18n.h"
#include "constants.h"

static const char* filenames[] = {
   "campaigns/t01.wmf",
   "campaigns/t02.wmf",
   "campaigns/emp01.wmf",
   "campaigns/emp02.wmf",
};

/*
 * Array with file names for this mission
 */
Fullscreen_Menu_TutorialSelectMap::Fullscreen_Menu_TutorialSelectMap() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

// Barbarian Tutorials

	// Text
title
(this, MENU_XRES / 2, 45, _("Tutorial Campaign: A new Kingdom"), Align_HCenter),

	// UI::Buttons

mission_1
(this,
 80, 100, 640, 26,
 1,
 &Fullscreen_Menu_TutorialSelectMap::end_modal, this, 1,
 _("Mission 1: Eyes in the Darkness - Game Basics")),

mission_2
(this,
 80, 130, 640, 26,
 1,
 &Fullscreen_Menu_TutorialSelectMap::end_modal, this, 2,
 _("Mission 2: A Place to call Home - Mining, Expanding, Advanced "
   "Productions")),

// Empire Tutorials

	// Text
title2
(this, MENU_XRES / 2, 300, _("Empire Tutorial Campaign"), Align_HCenter),

	// UI::Buttons

empire_1
(this,
 80, 350, 640, 26,
 1,
 &Fullscreen_Menu_TutorialSelectMap::end_modal, this, 3,
 _("Empire 1: The Strands of Malac' Mor")),
 
 empire_2
(this,
 80, 380, 640, 26,
 1,
 &Fullscreen_Menu_TutorialSelectMap::end_modal, this, 4,
 _("Empire 2: An outpost for exile")),

back
(this,
 (get_inner_w() - 160) / 2, get_inner_h() - 80, 160, 26,
 0,
 &Fullscreen_Menu_TutorialSelectMap::end_modal, this, 0,
 _("Back"))

{title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
title2.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);}

const char* Fullscreen_Menu_TutorialSelectMap::get_mapname( int code ) {
   return filenames[code-1];
}
