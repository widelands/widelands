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

#include <boost/algorithm/string.hpp>

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

/**
 * Author data for a map or scenario.
 */
struct MapAuthorData {

	const std::string& get_names() const {return m_names;}
	size_t get_number()            const {return m_number;}

	// Parses author list string into localized contatenated list string.
	// Use , as list separator and no whitespaces between author names.
	explicit MapAuthorData(const std::string& author_list) {
		std::vector<std::string> authors;
		boost::split(authors, author_list, boost::is_any_of(","));
		m_names = localize_item_list(authors, i18n::ConcatenateWith::AMPERSAND);
		m_number = authors.size();
	}

private:
	std::string m_names;
	size_t      m_number;
};

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
struct FullscreenMenuLoadMapOrGame : public FullscreenMenuBase {
	FullscreenMenuLoadMapOrGame() :
		FullscreenMenuBase("choosemapmenu.jpg"),

		// Values for alignment and size
		m_padding(4),
		m_indent(10),
		m_label_height(20),
		m_maplistx(get_w() *  47 / 2500),
		m_maplisty(get_h() * 17 / 50),
		m_maplistw(get_w() * 711 / 1250),
		m_maplisth(get_h() * 6083 / 10000),
		m_right_column_margin(15),
		m_right_column_x(m_maplistx + m_maplistw + m_right_column_margin),
		m_buty (get_h() * 9 / 10),
		m_butw ((get_w() - m_right_column_x - m_right_column_margin) / 2 - m_padding),
		m_buth (get_h() * 9 / 200),
		m_right_column_tab(get_w() - m_right_column_margin - m_butw),

		// Main buttons
		m_back
		  (this, "back",
			m_right_column_x, m_buty, m_butw, m_buth,
			g_gr->images().get("pics/but0.png"),
			_("Back"), std::string(), true, false),
		m_ok
		  (this, "ok",
			get_w() - m_right_column_margin - m_butw, m_buty, m_butw, m_buth,
			g_gr->images().get("pics/but2.png"),
			_("OK"), std::string(), false, false)
	{}

	bool handle_key(bool down, SDL_keysym code) override {

		if (!down)
			return false;

		switch (code.sym)
		{
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				clicked_ok();
				return true;
			case SDLK_ESCAPE:
				clicked_back();
				return true;
			default:
				break; // not handled
		}
		return FullscreenMenuBase::handle_key(down, code);
	}

protected:
	virtual void clicked_ok() {end_modal(1);}
	void clicked_back() {end_modal(0);}

	// Returns a y coordinate that can be used to position a Panel below the Panel directly above it
	int32_t get_y_from_preceding(UI::Panel& preceding_panel) {
		return preceding_panel.get_y() + preceding_panel.get_h();
	}

	// Returns the width that a Panel in the right column should have, depending on its x position
	int32_t get_right_column_w(int32_t x) {
		return get_w() - m_right_column_margin - x;
	}

	// UI coordinates and spacers
	int32_t const m_padding;               // Common padding between panels
	int32_t const m_indent;                // Indent for elements below labels
	int32_t const m_label_height;
	int32_t const m_maplistx, m_maplisty, m_maplistw, m_maplisth;
	int32_t const m_right_column_margin;   // X margins of the right column
	int32_t const m_right_column_x;
	int32_t const m_buty, m_butw, m_buth;  // Button dimensions
	int32_t const m_right_column_tab;

	// Main buttons
	UI::Button    m_back;
	UI::Button    m_ok;
};


#endif  // end of include guard: WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
