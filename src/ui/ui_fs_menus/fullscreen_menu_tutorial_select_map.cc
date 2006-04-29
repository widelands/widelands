/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include "ui_button.h"
#include "ui_textarea.h"
#include "constants.h"

static const char* filenames[] = {
   "campaigns/t01.wmf",
   "campaigns/t02.wmf",
};

/*
 * Array with file names for this mission
 */
Fullscreen_Menu_TutorialSelectMap::Fullscreen_Menu_TutorialSelectMap()
	: Fullscreen_Menu_Base("singleplmenu.jpg")
{
	// Text
   UITextarea* title= new UITextarea(this, MENU_XRES/2, 45, _("Tutorial Campaign: A new Kingdom"), Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 30, 85, 580, 24, 1, 1);
	b->clickedid.set(this, &Fullscreen_Menu_TutorialSelectMap::end_modal);
	b->set_title(_("Mission 1: Eyes in the Darkness - Game Basics"));

	b = new UIButton(this, 30, 115, 580, 24, 1, 2);
	b->clickedid.set(this, &Fullscreen_Menu_TutorialSelectMap::end_modal);
	b->set_title(_("Mission 2: A Place to call Home - Mining, Expanding, Advanced Productions"));

	b = new UIButton(this, (get_inner_w()-120)/2, get_inner_h()-80, 120, 24, 0, 0);
	b->clickedid.set(this, &Fullscreen_Menu_TutorialSelectMap::end_modal);
	b->set_title(_("Back"));
}

const char* Fullscreen_Menu_TutorialSelectMap::get_mapname( int code ) {
   return filenames[code-1];
}

