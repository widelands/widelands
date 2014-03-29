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

#ifndef FULLSCREEN_MENU_LOADGAME_H
#define FULLSCREEN_MENU_LOADGAME_H

#include "ui_fsmenu/base.h"

#include "graphic/image.h"
#include "graphic/image_loader.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

class IImageLoader;
namespace Widelands {
class Editor_Game_Base;
class Game;
class Map;
class Map_Loader;
};
class Image;
class RenderTarget;
class GameController;
struct GameSettingsProvider;

/// Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu.
struct Fullscreen_Menu_LoadGame : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LoadGame
		(Widelands::Game &, GameSettingsProvider * gsp = nullptr, GameController * gc = nullptr);

	const std::string & filename() {return m_filename;}

	void clicked_ok    ();
	void clicked_delete();
	void map_selected  (uint32_t);
	void double_clicked(uint32_t);
	void fill_list     ();
	void think() override;

	bool handle_key(bool down, SDL_keysym code) override;

private:
	void no_selection();

	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;
	uint16_t    m_minimap_max_size;

	Widelands::Game &                               m_game;
	UI::Button                             m_back;
	UI::Button                             m_ok;
	UI::Button                             m_delete;
	UI::Listselect<const char *>                    m_list;
	UI::Textarea                                    m_title;
	UI::Textarea                                    m_label_mapname;
	UI::Textarea                                    m_tamapname;
	UI::Textarea                                    m_label_gametime;
	UI::Textarea                                    m_tagametime;
	UI::Textarea                                    m_label_players;
	UI::Textarea                                    m_ta_players;
	UI::Textarea                                    m_ta_win_condition;
	UI::Textarea                                    m_label_minimap;
	UI::Icon                                        m_minimap_icon;
	std::string                                     m_filename;

	filenameset_t                                   m_gamefiles;

	GameSettingsProvider                          * m_settings;
	GameController                                * m_ctrl;
	std::unique_ptr<const Image>                    m_minimap_image;
	std::unique_ptr<const IImageLoader>             m_image_loader;

};


#endif
