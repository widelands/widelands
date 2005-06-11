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

#include <list>
#include <string>

#include "fullscreen_menu_base.h"

class UIEdit_Box;
class UITable;
class UITable_Entry;
class UIButton;

class LAN_Game_Finder;
class LAN_Open_Game;
struct LAN_Game_Info;

class Fullscreen_Menu_NetSetup : public Fullscreen_Menu_Base {
	public:
		enum {
			CANCEL=0,
			HOSTGAME,
			JOINGAME,
			INTERNETGAME,
			HOSTGGZGAME,
			JOINGGZGAME
		};
		
		Fullscreen_Menu_NetSetup ();
		~Fullscreen_Menu_NetSetup ();

		virtual void think();
	
		bool get_host_address (ulong&, ushort&);
		// return true if the selected or entered hostname is valid

		//bool is_internetgame();
		// return true if game should be played over GGZ

		void fill(std::list<std::string> tables);
	
	private:
		UIEdit_Box*		hostname;
		UITable*		opengames;
		LAN_Game_Finder*	discovery;
		UIButton*		networktype;
		bool			internetgame;
		
		void game_selected (int);
		
		static void discovery_callback (int, const LAN_Open_Game*, void*);

		void game_opened (const LAN_Open_Game*);
		void game_closed (const LAN_Open_Game*);
		void game_updated (const LAN_Open_Game*);
		
		void update_game_info (UITable_Entry*, const LAN_Game_Info&);

		void toggle_networktype(int code);
		void joingame(int code);
		void hostgame(int code);
};

#endif

