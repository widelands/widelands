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

#ifndef __S__NETSETUP_H
#define __S__NETSETUP_H

#include "fullscreen_menu_base.h"
#include "ui_editbox.h"


class Fullscreen_Menu_NetSetup : public Fullscreen_Menu_Base {
	public:
		enum {
			CANCEL=0,
			HOSTGAME,
			JOINGAME
		};
		
		Fullscreen_Menu_NetSetup ();
		~Fullscreen_Menu_NetSetup ();

		const char* get_host_address ()
		{ return hostname->get_text(); }
	
	private:
		UIEdit_Box*	hostname;
};

#endif

