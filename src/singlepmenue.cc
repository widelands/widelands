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

#include "widelands.h"
#include "graphic.h"
#include "ui.h"
#include "cursor.h"
#include "font.h"
#include "mainmenue.h"
#include "menuecommon.h"
#include "game.h"
#include "singlepmenue.h"
#include "mapselectmenue.h"

/*
==============================================================================

SinglePlayerMenu

==============================================================================
*/

enum {
	sp_skirmish,
	//sp_campaign, // BIG TODO
	//sp_loadgame,
	sp_back
};

class SinglePlayerMenu : public BaseMenu {
public:
	SinglePlayerMenu();

	void not_supported();
};

SinglePlayerMenu::SinglePlayerMenu()
	: BaseMenu("singleplmenu.bmp")
{
	// Text
	new Textarea(this, MENU_XRES/2, 140, "Single Player Menu", Textarea::H_CENTER);

	// Buttons
	Button* b;

	b = new Button(this, 60, 170, 174, 24, 1, sp_skirmish);
	b->clickedid.set(this, &SinglePlayerMenu::end_modal);
	b->set_pic(g_fh.get_string("Single Map", 0));

	b = new Button(this, 60, 210, 174, 24, 1);
	b->clicked.set(this, &SinglePlayerMenu::not_supported);
	b->set_pic(g_fh.get_string("Campaign", 0));

	b = new Button(this, 60, 250, 174, 24, 1);
	b->clicked.set(this, &SinglePlayerMenu::not_supported);
	b->set_pic(g_fh.get_string("Load Game", 0));

	b = new Button(this, 60, 370, 174, 24, 0, sp_back);
	b->clickedid.set(this, &SinglePlayerMenu::end_modal);
	b->set_pic(g_fh.get_string("Back", 0));
}

void SinglePlayerMenu::not_supported()
{
	critical_error("This is not yet supported. You can savly click on continue.");
}


/** void single_player_menue(void)
 *
 * This takes care for the single player menue, chosing campaign or
 * single player maps and so on, so on.
 */
void single_player_menue(void)
{
	for(;;)
	{
		SinglePlayerMenu *sp = new SinglePlayerMenu;
		int code = sp->run();
		delete sp;

		switch(code) {
		case sp_skirmish:
		{
			Game *g = new Game;
			bool ran = g->run();
			delete g;
			if (ran)
				return;
			break;
		}
		
		default:
		case sp_back:
			return;
		}
	}
}
