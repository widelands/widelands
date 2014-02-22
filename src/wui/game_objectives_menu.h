/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef GAME_OBJECTIVE_MENU_H
#define GAME_OBJECTIVE_MENU_H

#include "wui/interactive_player.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class Game;
struct Objective;
}
struct Interactive_Player;

///  Shows the not already fulfilled objectives.
struct GameObjectivesMenu : public UI::UniqueWindow {
	GameObjectivesMenu(Interactive_Player &, UI::UniqueWindow::Registry &);
	void think() override;

private:
	Interactive_Player & iplayer() const;
	void                 selected(uint32_t);

	typedef UI::Listselect<Widelands::Objective &> list_type;
	list_type              list;
	UI::Multiline_Textarea objectivetext;
public:
	bool victorious(bool const v = false) {
		static bool m_victory = v;
		if (v)
			m_victory = v;
		//m_claim_victory.set_enabled(m_victory);
		return m_victory;
	}
private:
	void claim_victory();
	void restart_mission();

	Interactive_Player & m_player;

};

#endif
