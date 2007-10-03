/*
 * Copyright (C) 2004. 2007 by the Widelands Development Team
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

#ifndef __S__INET_SERVER_OPTIONS_H
#define __S__INET_SERVER_OPTIONS_H

#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_textarea.h"


struct Fullscreen_Menu_InetServerOptions : public Fullscreen_Menu_Base {
		Fullscreen_Menu_InetServerOptions ();
		~Fullscreen_Menu_InetServerOptions ();

		const char* get_player_name()
	{return playername.get_text();}
		const char* get_server_name()
	{return hostname.get_text();}

private:
	UI::Textarea                                         title;
	UI::IDButton<Fullscreen_Menu_InetServerOptions, int32_t> rungame;
	UI::IDButton<Fullscreen_Menu_InetServerOptions, int32_t> back;
	UI::Edit_Box                                         playername;
	UI::Edit_Box                                         hostname;
};

#endif
