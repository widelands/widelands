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
#include "ui_basic/table.h"
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

	// Parses author list string into localized contatenated list
	// string. Use , as list separator and no whitespaces between
	// author names.
	void parse(const std::string& author_list) {
		std::vector<std::string> authors;
		boost::split(authors, author_list, boost::is_any_of(","));
		m_names = i18n::localize_list(authors, i18n::ConcatenateWith::AMPERSAND);
		m_number = authors.size();
	}

private:
	std::string m_names;
	size_t      m_number;
};

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
class FullscreenMenuLoadMapOrGame : public FullscreenMenuBase {
public:
	FullscreenMenuLoadMapOrGame(bool sortdesc = false);

protected:
	// Updates the information display on the right-hand side.
	// Call this function when a different entry in the table gets selected.
	virtual void entry_selected() {}
	virtual void fill_table() {}

	// Updates buttons and text labels and returns whether a table entry is selected.
	virtual bool set_has_selection() {
		bool has_selection = m_table.has_selection();
		m_ok.set_enabled(has_selection);
		return has_selection;
	}

	// Returns a y coordinate that can be used to position a Panel below the Panel directly above it
	int32_t get_y_from_preceding(UI::Panel& preceding_panel);

	// Returns the width that a Panel in the right column should have, depending on its x position
	int32_t get_right_column_w(int32_t x);

	// UI coordinates and spacers
	int32_t const m_padding;               // Common padding between panels
	int32_t const m_indent;                // Indent for elements below labels
	int32_t const m_label_height;
	int32_t const m_tablex, m_tabley, m_tablew, m_tableh;
	int32_t const m_right_column_margin;   // X margins of the right column
	int32_t const m_right_column_x;
	int32_t const m_buty, m_butw, m_buth;  // Button dimensions
	int32_t const m_right_column_tab;

	// Main buttons
	UI::Button    m_back;
	UI::Button    m_ok;

	UI::Table<uintptr_t const>    m_table;
};


#endif  // end of include guard: WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
