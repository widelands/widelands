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

#include "wui/sound_control.h"

#include "base/i18n.h"
#include "sound/sound_handler.h"

namespace {
constexpr int kSliderWidth = 160;
constexpr int kCursorWidth = 28;
constexpr int kSpacing = 16;
} // namespace

SoundControl::SoundControl(UI::Box* parent, const std::string& title, SoundType type)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal),
     enable_(this, Vector2i::zero(), title),
     volume_(this,
                         0,
                         0,
                         kSliderWidth,
                         enable_.get_h(),
                         0,
                         g_sound_handler.get_max_volume(),
                         g_sound_handler.get_volume(type),
                         UI::SliderStyle::kWuiLight, _("Sound Volume"), kCursorWidth),
     type_(type) {
	set_inner_spacing(kSpacing);
	add(&volume_, UI::Box::Resizing::kFullSize);
	add(&enable_, UI::Box::Resizing::kFullSize);

	if (g_sound_handler.is_backend_disabled()) {
		enable_.set_enabled(false);
		volume_.set_enabled(false);
	} else {
		enable_.set_state(g_sound_handler.is_sound_enabled(type));
		volume_.set_enabled(g_sound_handler.is_sound_enabled(type));

		enable_.changedto.connect([this] (bool on) { enable_changed(on); });
		volume_.changedto.connect([this] (int32_t value) { volume_changed(value); });
		volume_.clicked.connect([this] { play_sound_sample(); });
	}
	set_thinks(false);
}

SoundControl::~SoundControl() {
}

void SoundControl::play_sound_sample() {
	switch (type_) {
	case SoundType::kAmbient:
			g_sound_handler.play_fx(type_, "create_construction_site");
			break;
	case SoundType::kChat:
		g_sound_handler.play_fx(type_, "lobby_chat");
		break;
	case SoundType::kMessage:
		g_sound_handler.play_fx(type_, "message");
		break;
	default:
		// UI and music take care of themselves
		break;
	}
}

void SoundControl::volume_changed(int32_t value) {
	g_sound_handler.set_volume(type_, value);
}

void SoundControl::enable_changed(bool on) {
	enable_.set_state(on);
	volume_.set_enabled(on);
	g_sound_handler.set_enable_sound(type_, on);
}

