/*
 * Copyright (C) 2002-2003, 2006-2007 by the Widelands Development Team
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

#ifndef __S__INTERACTIVE_SPECTATOR_H
#define __S__INTERACTIVE_SPECTATOR_H

#include "interactive_base.h"

class Game;

/**
 * This class shows a game for somebody who is only a spectator.
 *
 * Right now, it is used for replays, but we should be able to
 * adapt it to implement spectators of network games.
 *
 * This class provides the UI, runs the game logic, etc.
 */
class Interactive_Spectator : public Interactive_Base {
public:
	Interactive_Spectator(Game*);
	~Interactive_Spectator();

	void start();
	virtual void think();

	Game* get_game();
	Player* get_player() const throw ();

	bool handle_key(bool down, int32_t code, char c);
	void end_of_game();

private:
	void exit_btn();
	void field_action();

private:
	class Internals;

	Internals* m;
};


#endif // __S__INTERACTIVE_SPECTATOR_H
