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

#ifndef __S__PLAYER_DESCR_GROUP
#define __S__PLAYER_DESCR_GROUP

class Game;

/** class PlayerDescriptionGroup
 *
 * - checkbox to enable/disable player
 * - button to switch between: Human, Remote, AI
 */
class PlayerDescriptionGroup : public UIPanel {
	Game* m_game;
	int m_plnum;
	
	bool m_enabled; // is this player allowed at all (map-dependent)
	
	UICheckbox* m_btnEnablePlayer;
	int m_playertype;
	UIButton* m_btnPlayerType;

public:
	PlayerDescriptionGroup(UIPanel* parent, int x, int y, Game* game, int plnum);

	UISignal changed;
		
	void set_enabled(bool enable);

private:	
	void enable_player(bool on);
	void toggle_playertype();
};


#endif
