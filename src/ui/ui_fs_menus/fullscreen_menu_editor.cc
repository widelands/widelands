/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "fullscreen_menu_editor.h"

#include "constants.h"
#include "i18n.h"

Fullscreen_Menu_Editor::Fullscreen_Menu_Editor() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

title(this, MENU_XRES/2, 130, _("Editor Menu"), Align_HCenter),

new_map
	(this,
	 90, 220, 200, 26,
	 1,
	 &Fullscreen_Menu_Editor::end_modal, this, New_Map,
	 _("New Map")),
load_map
	(this,
	 90, 260, 200, 26,
	 1,
	 &Fullscreen_Menu_Editor::end_modal, this, Load_Map,
	 _("Load Map")),
back
	(this,
	 90, 400, 200, 26,
	 0,
	 &Fullscreen_Menu_Editor::end_modal, this, Back,
	 _("Back"))
{title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);}
