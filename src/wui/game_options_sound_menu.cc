/*
 * Copyright (C) 2007-2018 by the Widelands Development Team
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

#include "wui/game_options_sound_menu.h"

#include "base/i18n.h"
#include "sound/sound_handler.h"
#include "ui_basic/multilinetextarea.h"

GameOptionsSoundMenu::GameOptionsSoundMenu(InteractiveGameBase& gb,
                                           UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&gb, "sound_options_menu", &registry, 160, 160, _("Sound Options")),
     ingame_music(this, Vector2i(hmargin(), vmargin()), _("Enable music")),
     ingame_sound(this,
                  Vector2i(hmargin(), vmargin() + kStateboxSize + vspacing()),
                  _("Enable sound effects")),
     ingame_music_volume_label(this,
                               hmargin(),
                               vmargin() + 2 * (kStateboxSize + vspacing()) + vbigspacing(),
                               _("Music volume")),
     ingame_music_volume(this,
                         hmargin(),
                         vmargin() + 2 * (kStateboxSize + vspacing()) + vbigspacing() +
                            1 * vspacing() + ingame_music_volume_label.get_h(),
                         get_inner_w() - 2 * hmargin(),
                         slideh(),
                         0,
                         g_sound_handler.get_max_volume(),
                         g_sound_handler.get_music_volume(),
                         UI::SliderStyle::kWuiLight),
     ingame_sound_volume_label(this,
                               hmargin(),
                               vmargin() + 2 * (kStateboxSize + vspacing()) + vbigspacing() +
                                  2 * vspacing() + slideh() + ingame_music_volume_label.get_h(),
                               _("Sound effects volume")),
     ingame_sound_volume(this,
                         hmargin(),
                         vmargin() + 2 * (kStateboxSize + vspacing()) + vbigspacing() +
                            3 * vspacing() + slideh() + ingame_music_volume_label.get_h() +
                            ingame_music_volume_label.get_h(),
                         get_inner_w() - 2 * hmargin(),
                         slideh(),
                         0,
                         g_sound_handler.get_max_volume(),
                         g_sound_handler.get_fx_volume(),
                         UI::SliderStyle::kWuiLight) {
	ingame_music.set_state(!g_sound_handler.get_disable_music());
	ingame_sound.set_state(!g_sound_handler.get_disable_fx());

	uint32_t boxes_width =
	   kStateboxSize + hspacing() + std::max(ingame_music.get_w(), ingame_sound.get_w());
	uint32_t labels_width =
	   std::max(ingame_music_volume_label.get_w(), ingame_sound_volume_label.get_w());

	set_inner_size(std::max(static_cast<uint32_t>(get_inner_w()),
	                        2 * hmargin() + std::max(boxes_width, labels_width)),
	               2 * vmargin() + 2 * (kStateboxSize + vspacing()) + vbigspacing() +
	                  3 * vspacing() + 2 * slideh() + ingame_music_volume_label.get_h() +
	                  ingame_music_volume_label.get_h());

	if (g_sound_handler.is_backend_disabled()) {  //  disabling sound options
		ingame_music.set_enabled(false);
		ingame_sound.set_enabled(false);
		ingame_music_volume.set_enabled(false);
		ingame_sound_volume.set_enabled(false);

		UI::MultilineTextarea* sound_warning = new UI::MultilineTextarea(
		   this, hmargin(), ingame_sound_volume.get_y() + ingame_sound_volume.get_h() + vspacing(),
		   get_inner_w() - 2 * hmargin(), 0, UI::PanelStyle::kWui,
		   _("Sound is disabled due to a problem with the sound driver"), UI::Align::kLeft,
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);

		sound_warning->set_color(UI_FONT_CLR_WARNING);
		set_size(get_w(), get_h() + sound_warning->get_h() + vspacing());

	} else {  // initial widget states
		ingame_music.set_state(!g_sound_handler.get_disable_music());
		ingame_sound.set_state(!g_sound_handler.get_disable_fx());
		ingame_music_volume.set_enabled(!g_sound_handler.get_disable_music());
		ingame_sound_volume.set_enabled(!g_sound_handler.get_disable_fx());
	}

	//  ready signals
	ingame_music.changedto.connect(
	   boost::bind(&GameOptionsSoundMenu::changed_ingame_music, this, _1));
	ingame_sound.changedto.connect(
	   boost::bind(&GameOptionsSoundMenu::changed_ingame_sound, this, _1));
	ingame_music_volume.changedto.connect(
	   boost::bind(&GameOptionsSoundMenu::music_volume_changed, this, _1));
	ingame_sound_volume.changedto.connect(
	   boost::bind(&GameOptionsSoundMenu::sound_volume_changed, this, _1));

	if (get_usedefaultpos())
		center_to_parent();
}

/**
 * \brief The music checkbox has been toggled.
 */
void GameOptionsSoundMenu::changed_ingame_music(bool on) {
	ingame_music_volume.set_enabled(on);
	g_sound_handler.set_disable_music(!on);
}

/**
 * \brief The FX checkbox has been toggled.
 */
void GameOptionsSoundMenu::changed_ingame_sound(bool on) {
	ingame_sound_volume.set_enabled(on);
	g_sound_handler.set_disable_fx(!on);
}

/**
 * \brief Callback for the music volume slider.
 *
 * \param value The new music volume.
 */
void GameOptionsSoundMenu::music_volume_changed(int32_t value) {
	g_sound_handler.set_music_volume(value);
}

/**
 * \brief Callback for the sound volume slider.
 *
 * \param value The new sound volume value.
 */
void GameOptionsSoundMenu::sound_volume_changed(int32_t value) {
	g_sound_handler.set_fx_volume(value);
}
