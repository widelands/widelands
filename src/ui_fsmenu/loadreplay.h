/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_LOADREPLAY_H
#define WL_UI_FSMENU_LOADREPLAY_H

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/load_map_or_game.h"

namespace Widelands
{class Game;}

/**
 * Select a replay from a list of replays.
 */
struct FullscreenMenuLoadReplay : public FullscreenMenuLoadMapOrGame {
	FullscreenMenuLoadReplay(Widelands::Game&);

	const std::string& filename() {return m_filename;}

	void clicked_ok();
	void clicked_delete();
	void replay_selected(uint32_t);
	void double_clicked(uint32_t);
	void fill_list();

	bool handle_key(bool down, SDL_keysym code) override;

private:
	void no_selection();

	UI::Textarea                  m_title;
	UI::Textarea                  m_label_mapname;
	UI::MultilineTextarea         m_ta_mapname;  // Multiline for long names
	UI::Textarea                  m_label_gametime;
	UI::MultilineTextarea         m_ta_gametime; // Multiline because we want tooltips
	UI::Textarea                  m_label_players;
	UI::MultilineTextarea         m_ta_players;
	UI::Textarea                  m_label_win_condition;
	UI::MultilineTextarea         m_ta_win_condition;
	UI::Button                    m_delete;

	UI::Listselect<std::string>   m_list;
	Widelands::Game&              m_game;
	std::string                   m_filename;
};


#endif  // end of include guard: WL_UI_FSMENU_LOADREPLAY_H
