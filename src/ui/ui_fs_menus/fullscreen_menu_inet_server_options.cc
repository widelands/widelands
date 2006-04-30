/*
 * Copyright (C) 2004 by the Widelands Development Team
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

#include "fullscreen_menu_inet_server_options.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "i18n.h"
#include "constants.h"

Fullscreen_Menu_InetServerOptions::Fullscreen_Menu_InetServerOptions ()
	:Fullscreen_Menu_Base("singleplmenu.jpg") // change this
{
	// Text
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 140, _("Internet Options"), Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 60, 170, 174, 24, 1, 1);
	b->clickedid.set(this, &Fullscreen_Menu_InetServerOptions::end_modal);
	b->set_title(_("Run Game").c_str());

	b = new UIButton(this, 60, 250, 174, 24, 0, 0);
	b->clickedid.set(this, &Fullscreen_Menu_InetServerOptions::end_modal);
	b->set_title(_("Back").c_str());

	// Hostname
	hostname=new UIEdit_Box(this, 288, 210, 174, 24, 2, 0);
	hostname->set_text("localhost");
	// Player
	playername=new UIEdit_Box(this, 288, 240, 174, 24, 2, 0);
	playername->set_text(_("nobody").c_str());
}

Fullscreen_Menu_InetServerOptions::~Fullscreen_Menu_InetServerOptions ()
{
}

