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
#include "ui_basic/textarea.h"

class SoundControl : public UI::Box {
public:
	SoundControl(UI::Box* parent, const std::string& title, SoundType type);
	~SoundControl() override;

	void enable_changed(bool on);
	void volume_changed(int32_t value);

private:
	UI::Checkbox enable_;
	UI::HorizontalSlider volume_;
	SoundType type_;
};

#endif  // end of include guard: WL_WUI_SOUND_CONTROL_H
