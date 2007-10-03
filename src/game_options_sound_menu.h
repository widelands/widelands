/*
 * Copyright (C) 2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __S__GAME_OPTIONS_SOUND_MENU_H
#define __S__GAME_OPTIONS_SOUND_MENU_H

#include "interactive_player.h"
#include <stdint.h>
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "ui_checkbox.h"
#include "ui_slider.h"

/**
 * A window with all sound options.
 */
struct GameOptionsSoundMenu : public UI::UniqueWindow {
	GameOptionsSoundMenu(Interactive_Player &, UI::UniqueWindow::Registry &);

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
	uint32_t hspacing   () const throw () {return 5;}
	uint32_t vspacing   () const throw () {return 5;}
	uint32_t vbigspacing() const throw () {return 8 + vspacing();}
	uint32_t slideh     () const throw () {return 28;}

	/// Returns the horizontal/vertical margin between edge and buttons.
	uint32_t hmargin() const throw () {return 2 * hspacing();}
	uint32_t vmargin() const throw () {return 2 * vspacing();}

	//  calbacks
	void changed_ingame_music(bool on);
	void changed_ingame_sound(bool on);
	void music_volume_changed(int32_t value);
	void sound_volume_changed(int32_t value);
};

#endif
