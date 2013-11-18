/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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
 */

#ifndef GAME_OPTIONS_SOUND_MENU_H
#define GAME_OPTIONS_SOUND_MENU_H

#include "wui/interactive_gamebase.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

/**
 * A window with all sound options.
 */
struct GameOptionsSoundMenu : public UI::UniqueWindow {
	GameOptionsSoundMenu(Interactive_GameBase &, UI::UniqueWindow::Registry &);

private:
	UI::Checkbox         ingame_music;
	UI::Textarea         ingame_music_label;
	UI::Checkbox         ingame_sound;
	UI::Textarea         ingame_sound_label;

	UI::Textarea         ingame_music_volume_label;
	UI::HorizontalSlider ingame_music_volume;
	UI::Textarea         ingame_sound_volume_label;
	UI::HorizontalSlider ingame_sound_volume;

	/// Returns the horizontal/vertical spacing between widgets.
	uint32_t hspacing   () const {return 5;}
	uint32_t vspacing   () const {return 5;}
	uint32_t vbigspacing() const {return 8 + vspacing();}
	uint32_t slideh     () const {return 28;}

	/// Returns the horizontal/vertical margin between edge and buttons.
	uint32_t hmargin() const {return 2 * hspacing();}
	uint32_t vmargin() const {return 2 * vspacing();}

	//  calbacks
	void changed_ingame_music(bool on);
	void changed_ingame_sound(bool on);
	void music_volume_changed(int32_t value);
	void sound_volume_changed(int32_t value);
};

#endif
