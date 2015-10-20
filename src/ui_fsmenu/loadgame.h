/*
 * Copyright (C) 2002, 2006-2008, 2010-2011, 2013 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_LOADGAME_H
#define WL_UI_FSMENU_LOADGAME_H

#include "ui_fsmenu/base.h"

#include <memory>

#include "graphic/image.h"
#include "logic/game_controller.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/load_map_or_game.h"

namespace Widelands {
class EditorGameBase;
class Game;
class Map;
class MapLoader;
}
class Image;
class RenderTarget;
class GameController;
struct GameSettingsProvider;


/**
 * Data about a savegame/replay that we're interested in.
 */
struct SavegameData {
	std::string filename;
	std::string mapname;
	std::string wincondition;
	std::string minimap_path;
	std::string savedatestring;
	std::string errormessage;

	uint32_t gametime;
	uint32_t nrplayers;
	std::string version;
	time_t savetimestamp;
	GameController::GameType gametype;

	SavegameData() : gametime(0), nrplayers(0), savetimestamp(0),
		gametype(GameController::GameType::SINGLEPLAYER) {}
};



/// Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu.
class FullscreenMenuLoadGame : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuLoadGame
		(Widelands::Game&, GameSettingsProvider* gsp, GameController* gc = nullptr,
		 bool is_replay = false);

	const std::string & filename() {return m_filename;}

	void think() override;

	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	void clicked_ok() override;
	void entry_selected() override;
	void fill_table() override;

private:
	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();
	bool compare_date_descending(uint32_t, uint32_t);
	void clicked_delete();

	UI::Table<uintptr_t const>    m_table;

	bool                          m_is_replay;

	UI::Textarea                  m_title;
	UI::Textarea                  m_label_mapname;
	UI::MultilineTextarea         m_ta_mapname;  // Multiline for long names
	UI::Textarea                  m_label_gametime;
	UI::MultilineTextarea         m_ta_gametime; // Multiline because we want tooltips
	UI::Textarea                  m_label_players;
	UI::MultilineTextarea         m_ta_players;
	UI::Textarea                  m_label_version;
	UI::Textarea                  m_ta_version;
	UI::Textarea                  m_label_win_condition;
	UI::MultilineTextarea         m_ta_win_condition;

	UI::Button                    m_delete;

	UI::MultilineTextarea         m_ta_errormessage;

	int32_t const                 m_minimap_y, m_minimap_w, m_minimap_h;
	UI::Icon                      m_minimap_icon;
	std::unique_ptr<const Image>  m_minimap_image;

	std::vector<SavegameData>     m_games_data;
	std::string                   m_filename;

	Widelands::Game&              m_game;
	GameSettingsProvider*         m_settings;
	GameController*               m_ctrl;
};


#endif  // end of include guard: WL_UI_FSMENU_LOADGAME_H
