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
 */

#include "wui/sound_options.h"

#include "base/i18n.h"
#include "sound/sound_handler.h"
#include "ui_basic/multilinetextarea.h"
#include "wui/sound_control.h"

namespace {
constexpr int kSpacing = 12;
} // namespace

SoundOptions::SoundOptions(UI::Panel& parent, UI::SliderStyle style)
   : UI::Box(&parent, 0, 0, UI::Box::Vertical) {

	set_inner_spacing(kSpacing);

	add(new SoundControl(this, pgettext("sound_options", "Music"), SoundType::kMusic, style), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, pgettext("sound_options", "Chat Messages"), SoundType::kChat, style), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, pgettext("sound_options", "Game Messages"), SoundType::kMessage, style), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, pgettext("sound_options", "User Interface"), SoundType::kUI, style), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, pgettext("sound_options", "Ambient Sounds"), SoundType::kAmbient, style), UI::Box::Resizing::kFullSize);

	// TODO(GunChleoc): There's a bug (probably somewhere in Box) that will hide the bottom SoundControl if the textearea is not added to the box. So, we create and add it even if its text is empty.
	UI::MultilineTextarea* sound_warning = new UI::MultilineTextarea(
	   this, 0, 0, 100, 0, UI::PanelStyle::kWui,
	   "", UI::Align::kLeft,
	   UI::MultilineTextarea::ScrollMode::kNoScrolling);
	add(sound_warning, UI::Box::Resizing::kExpandBoth);

	if (SoundHandler::is_backend_disabled()) {
		sound_warning->set_text(_("Sound is disabled either due to a problem with the sound driver, or because it was switched off at the command line."));
	}
}
