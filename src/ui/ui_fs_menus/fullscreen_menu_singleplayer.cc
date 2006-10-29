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

#include "constants.h"
#include "error.h"
#include "fullscreen_menu_singleplayer.h"
#include "i18n.h"
#include "ui_button.h"
#include "ui_textarea.h"

Fullscreen_Menu_SinglePlayer::Fullscreen_Menu_SinglePlayer()
	: Fullscreen_Menu_Base("singleplmenu.jpg")
{
	// Text
   UITextarea* title= new UITextarea(this, MENU_XRES/2, 130, _("Single Player Menu"), Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 90, 220, 200, 26, 1, New_Game);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title(_("New Game").c_str());

	b = new UIButton(this, 90, 260, 200, 26, 1, Tutorial_Campaign);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title(_("Tutorial Campaign").c_str());

	b = new UIButton(this, 90, 300, 200, 26, 1, Load_Game);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title(_("Load Game").c_str());

	b = new UIButton(this, 90, 400, 200, 26, 0, Back);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title(_("Back").c_str());
}
