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
#include "fullscreen_menu_singleplayer.h"
#include "ui_button.h"
#include "ui_textarea.h"


Fullscreen_Menu_SinglePlayer::Fullscreen_Menu_SinglePlayer()
	: Fullscreen_Menu_Base("singleplmenu.jpg")
{
	// Text
	new UITextarea(this, MENU_XRES/2, 140, "Single Player Menu", Align_HCenter);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 60, 170, 174, 24, 1, sp_skirmish);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title("Single Map");

	b = new UIButton(this, 60, 210, 174, 24, 1);
	b->clicked.set(this, &Fullscreen_Menu_SinglePlayer::not_supported);
	b->set_title("Campaign");

	b = new UIButton(this, 60, 250, 174, 24, 1);
	b->clicked.set(this, &Fullscreen_Menu_SinglePlayer::not_supported);
	b->set_title("Load Game");

	b = new UIButton(this, 60, 370, 174, 24, 0, sp_back);
	b->clickedid.set(this, &Fullscreen_Menu_SinglePlayer::end_modal);
	b->set_title("Back");
}

void Fullscreen_Menu_SinglePlayer::not_supported()
{
	critical_error("This is not yet supported. You can safely click on continue.");
}
