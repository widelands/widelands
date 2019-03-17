/*
 * Copyright (C) 2019 by the Widelands Development Team
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

#ifndef WL_WUI_SOUND_CONTROL_H
#define WL_WUI_SOUND_CONTROL_H

#include <string>

#include "sound/constants.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"

/**
 * UI elements to set sound properties for 1 type of sounds.
 * All changes to the sound settings take effect immediately, but are not saved to config.
 */
class SoundControl : public UI::Box {
public:
/**
	 * @brief SoundControl Creates a new sound control box
	 * @param parent The parent panel
	 * @param title The localized test label to display
	 * @param type The type of sound to set the properties for
	 * @param style The User interface style for the slider
	 */
	SoundControl(UI::Box* parent, const std::string& title, SoundType type, UI::SliderStyle style);

private:
	/// Plays a system sound sample selected from the given sound type
	void play_sound_sample();
	/// Sets new enable/disable value in the sound handler for the sound type and enables/disables the volume slider accordingly
	void enable_changed(bool on);
	/// Sets the volume in the sound handler to the new 'value'
	void volume_changed(int32_t value);

	/// Enable / disable sound type
	UI::Checkbox enable_;
	/// Control the volume for the sound type
	UI::HorizontalSlider volume_;
	/// The sound type to control
	const SoundType type_;
	/// Representative sound effect to play
	FxId fx_;
};

#endif  // end of include guard: WL_WUI_SOUND_CONTROL_H
