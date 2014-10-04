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

#ifndef WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
#define WL_UI_FSMENU_LOAD_MAP_OR_GAME_H

#include "ui_fsmenu/base.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

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

/// Select a Map, Saved Game or Replay in Fullscreen Mode. It's a superclass for modal fullscreen menus.
struct FullscreenMenuLoadMapOrGame : public FullscreenMenuBase {
	FullscreenMenuLoadMapOrGame() :
		FullscreenMenuBase("choosemapmenu.jpg"),

		// Values for alignment and size
		m_padding (5),
		m_space   (25),
		m_margin_right(15),
		m_maplistx(get_w() *  47 / 2500),
		m_maplisty(get_h() * 17 / 50),
		m_maplistw(get_w() * 711 / 1250),
		m_maplisth(get_h() * 6083 / 10000),
		m_butx (m_maplistx + m_maplistw + m_margin_right),
		m_buty (get_h() * 9 / 10),
		m_butw ((get_w() - m_butx - m_margin_right) / 2 - m_padding),
		m_buth (get_h() * 9 / 200),
		m_nr_players_width(35),
		m_description_column_tab(get_w() - m_margin_right - m_butw),

		// Main buttons
		m_back
		  (this, "back",
			m_butx, m_buty, m_butw, m_buth,
			g_gr->images().get("pics/but0.png"),
			_("Back"), std::string(), true, false),
		m_ok
		  (this, "ok",
			get_w() - m_margin_right - m_butw, m_buty, m_butw, m_buth,
			g_gr->images().get("pics/but2.png"),
			_("Load"), std::string(), false, false),

		// Savegame / Map / Replay list
		m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth)
	{}

	const std::string & filename() {return m_filename;}

	bool handle_key(bool down, SDL_keysym code) override {return FullscreenMenuBase::handle_key(down, code);}

protected:
	int32_t const    m_padding;
	int32_t const    m_space;
	int32_t const    m_margin_right;
	int32_t const    m_maplistx, m_maplisty, m_maplistw, m_maplisth;
	int32_t const    m_butx, m_buty, m_butw, m_buth;
	int32_t const    m_nr_players_width;
	int32_t const    m_description_column_tab;

	UI::Button                    m_back;
	UI::Button                    m_ok;

	UI::Listselect<const char *>  m_list;
	std::string                   m_filename;
	FilenameSet                   m_gamefiles;
};


#endif  // end of include guard: WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
