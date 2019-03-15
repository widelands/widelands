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

SoundOptions::SoundOptions(UI::Panel& parent)
   : UI::Box(&parent, 0, 0, UI::Box::Vertical) {

	set_inner_spacing(kSpacing);

	add(new SoundControl(this, _("Music"), SoundType::kMusic), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, _("Chat Messages"), SoundType::kChat), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, _("Game Messages"), SoundType::kMessage), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, _("User Interface"), SoundType::kUI), UI::Box::Resizing::kFullSize);
	add(new SoundControl(this, _("Ambient Sounds"), SoundType::kAmbient), UI::Box::Resizing::kFullSize);

	UI::MultilineTextarea* sound_warning = new UI::MultilineTextarea(
	   this, 0, 0, 100, 0, UI::PanelStyle::kWui,
	   "", UI::Align::kLeft,
	   UI::MultilineTextarea::ScrollMode::kNoScrolling);
	add(sound_warning, UI::Box::Resizing::kExpandBoth);

	if (g_sound_handler.is_backend_disabled()) {
		sound_warning->set_text(_("Sound is disabled either due to a problem with the sound driver, or because it was switched off at the command line."));
	}
}
