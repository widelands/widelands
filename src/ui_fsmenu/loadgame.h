/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

	const std::string & filename() {return filename_;}

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

	UI::Table<uintptr_t const>    table_;

	bool                          is_replay_;

	UI::Textarea                  title_;
	UI::Textarea                  label_mapname_;
	UI::MultilineTextarea         ta_mapname_;  // Multiline for long names
	UI::Textarea                  label_gametime_;
	UI::MultilineTextarea         ta_gametime_; // Multiline because we want tooltips
	UI::Textarea                  label_players_;
	UI::MultilineTextarea         ta_players_;
	UI::Textarea                  label_version_;
	UI::Textarea                  ta_version_;
	UI::Textarea                  label_win_condition_;
	UI::MultilineTextarea         ta_win_condition_;

	UI::Button                    delete_;

	UI::MultilineTextarea         ta_errormessage_;

	int32_t const                 minimap_y_, minimap_w_, minimap_h_;
	UI::Icon                      minimap_icon_;
	std::unique_ptr<const Image>  minimap_image_;

	std::vector<SavegameData>     games_data_;
	std::string                   filename_;

	Widelands::Game&              game_;
	GameSettingsProvider*         settings_;
	GameController*               ctrl_;
};


#endif  // end of include guard: WL_UI_FSMENU_LOADGAME_H
