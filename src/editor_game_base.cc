/*
 * Copyright (C) 2002 by The Widelands Development Team
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
#include "editor_game_base.h"
#include "map.h"
#include "IntPlayer.h"
#include "player.h"

/*
===============
Editor_Game_Base::recalc_for_field

Call this function whenever the field at fx/fy has changed in one of the ways:
 - height has changed
 - robust Map_Object has been added or removed
 
This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Editor_Game_Base::recalc_for_field(Coords coords, int radius)
{
   cerr << "Hier und alive!" << endl;
	Map_Region_Coords mrc;
   cerr << "Hier und alive!" << endl;
	int x, y;
   cerr << "Hier und alive!" << endl;
	Field *f;
   cerr << "Hier und alive!" << endl;
	
	// First pass
   cerr << "Hier und alive!" << endl;
	mrc.init(coords, 2+radius, m_map);
   cerr << "Hier und alive!" << endl;

	while(mrc.next(&x, &y)) {
		cerr << 1 << endl;
      f = m_map->get_field(x, y);
		cerr << 2 << endl;
		m_map->recalc_brightness(x, y, f);
		cerr << 3 << endl;
		m_map->recalc_fieldcaps_pass1(x, y, f);
		cerr << 4 << endl;
	}

   cerr << "Hier und alive!" << endl;

	// Second pass
	mrc.init(coords, 2+radius, m_map);
   cerr << "Hier und alive!" << endl;
	
	while(mrc.next(&x, &y)) {
   cerr << "Hier und alive!" << endl;
		f = m_map->get_field(x, y);
   cerr << "Hier und alive!" << endl;
		m_map->recalc_fieldcaps_pass2(x, y, f);
   cerr << "Hier und alive!" << endl;
		
   cerr << "Hier und alive!" << endl;
		if (get_ipl())
			get_ipl()->recalc_overlay(FCoords(x, y, f));
	}
   cerr << "Hier und alive!" << endl;
}

/*
===============
Editor_Game_Base::conquer_area

Conquers the given area for that player.
Additionally, it updates the visible area for that player.
===============
*/
void Editor_Game_Base::conquer_area(uchar playernr, Coords coords, int radius)
{
	Map_Region m(coords, radius, m_map);
	Field* f;

	while((f = m.next()))
	{
		if (f->get_owned_by() == playernr)
			continue;
		if (!f->get_owned_by()) {
			f->set_owned_by(playernr);
			continue;
		}
		
      // TODO: add support here what to do if some fields are already
      // occupied by another player
		// Probably the best thing to just don't grab it. Players should fight
		// for their land.
      //cerr << "warning: already occupied field is claimed by another user!" << endl;
   }
	
	Player *player = get_player(playernr);
	
	player->set_area_seen(coords.x, coords.y, radius+4, true);
	
	recalc_for_field(coords, radius);
}



/*
===============
Editor_Game_Base::remove_player

Remove the player with the given number
===============
*/
void Editor_Game_Base::remove_player(int plnum)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	
	if (m_players[plnum-1]) {
		delete m_players[plnum-1];
		m_players[plnum-1] = 0;
	}		
}


/*
===============
Editor_Game_Base::add_player

Create the player structure for the given plnum.
Note that AI player structures and the Interactive_Player are created when
the game starts. Similar for remote players.
===============
*/
void Editor_Game_Base::add_player(int plnum, int type, const char* tribe, const uchar *playercolor)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	
	if (m_players[plnum-1])
		remove_player(plnum);

	// Get the player's tribe
	uint i;
	
	for(i = 0; i < m_tribes.size(); i++)
		if (!strcmp(m_tribes[i]->get_name(), tribe))
			break;
	
	if (i == m_tribes.size())
		m_tribes.push_back(new Tribe_Descr(tribe));
	
	m_players[plnum-1] = new Player(this, type, plnum, m_tribes[i], playercolor);
}


