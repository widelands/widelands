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

#ifndef WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
#define WL_UI_FSMENU_LOAD_MAP_OR_GAME_H

#include "ui_fsmenu/base.h"

#include <memory>

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "wui/maptable.h"


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

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
class FullscreenMenuLoadMapOrGame : public FullscreenMenuBase {
public:
	FullscreenMenuLoadMapOrGame();

protected:
	// Updates the information display on the right-hand side.
	// Call this function when a different entry in the table gets selected.
	virtual void entry_selected() {}
	virtual void fill_table() {}

	// Returns a y coordinate that can be used to position a Panel below the Panel directly above it
	int32_t get_y_from_preceding(UI::Panel& preceding_panel);

	// Returns the width that a Panel in the right column should have, depending on its x position
	int32_t get_right_column_w(int32_t x);

	// UI coordinates and spacers
	int32_t const padding_;               // Common padding between panels
	int32_t const indent_;                // Indent for elements below labels
	int32_t const label_height_;
	int32_t const tablex_, tabley_, tablew_, tableh_;
	int32_t const right_column_margin_;   // X margins of the right column
	int32_t const right_column_x_;
	int32_t const buty_, butw_, buth_;  // Button dimensions
	int32_t const right_column_tab_;

	// Main buttons
	UI::Button    back_;
	UI::Button    ok_;
};


#endif  // end of include guard: WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
