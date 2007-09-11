/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "fullscreen_menu_main.h"

#include "../../build_id.h"
#include "i18n.h"

/*
==============================================================================

Fullscreen_Menu_Main

==============================================================================
*/
Fullscreen_Menu_Main::Fullscreen_Menu_Main()
:
Fullscreen_Menu_Base("mainmenu.jpg"),
	// UI::Buttons

singleplayer
(this,
 100, 140, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_singleplayer,
 _("Single Player")),

multiplayer
(this,
 100, 180, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_multiplayer,
 _("Multi Player")),

replay
(this,
 100, 220, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_replay,
 _("Watch Replay")),

options
(this,
 100, 260, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_options,
 _("Options")),

editor
(this,
 100, 300, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_editor,
 _("Editor")),

readme
(this,
 100, 340, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_readme,
 _("View Readme")),

license
(this,
 100, 380, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_license,
 _("License")),

exit
(this,
 100, 440, 220, 26,
 3,
 &Fullscreen_Menu_Main::end_modal, this, mm_exit,
 _("Exit Game")),

	// Text
version(this, MENU_XRES-25, MENU_YRES-29, _("Version")+" "+BUILD_ID, Align_Right),
copyright
(this,
 15, MENU_YRES - 29,
 _("(C) 2002-2007 by the Widelands Development Team"), Align_TopLeft)
{}
