/*
 * Copyright (C) 2004, 2006-2008 by the Widelands Development Team
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

#include "constants.h"
#include "i18n.h"

Fullscreen_Menu_InetServerOptions::Fullscreen_Menu_InetServerOptions ()
:
Fullscreen_Menu_Base("singleplmenu.jpg"), // change this

title(this, MENU_XRES/2, 140, _("Internet Options"), Align_HCenter),

rungame
(this,
 60, 170, 174, 24,
 1,
 &Fullscreen_Menu_InetServerOptions::end_modal, this, 1,
 _("Run Game")),

back
(this,
 60, 250, 174, 24,
 0,
 &Fullscreen_Menu_InetServerOptions::end_modal, this, 0,
 _("Back")),

playername(this, 288, 240, 174, 24, 2, 0),
hostname  (this, 288, 210, 174, 24, 2, 0)

{
	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	hostname  .set_text("localhost");
	playername.set_text(_("nobody"));
}

Fullscreen_Menu_InetServerOptions::~Fullscreen_Menu_InetServerOptions () {}
