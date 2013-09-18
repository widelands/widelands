/*
 * Copyright (C) 2007-2011 by the Widelands Development Team
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

#include "graphic/graphic.h"
#include "i18n.h"
#include "sound/sound_handler.h"

GameOptionsSoundMenu::GameOptionsSoundMenu
	(Interactive_GameBase & gb, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&gb, "sound_options_menu", &registry, 160, 160, _("Sound Options")),
ingame_music(this, Point(hmargin(), vmargin())),
ingame_music_label
	(this,
	 hmargin() + STATEBOX_WIDTH + hspacing(), vmargin(),
	 _("Ingame Music")),
ingame_sound(this, Point(hmargin(), vmargin() + STATEBOX_HEIGHT + vspacing())),
ingame_sound_label
	(this,
	 hmargin() + STATEBOX_WIDTH  + hspacing(),
	 vmargin() + STATEBOX_HEIGHT + vspacing(),
	 _("Sound FX")),
ingame_music_volume_label
	(this,
	 hmargin(), vmargin() + 2 * (STATEBOX_HEIGHT + vspacing()) + vbigspacing(),
	 _("Ingame Music Volume")),
ingame_music_volume
	(this,
	 hmargin(),
	 vmargin() + 2 * (STATEBOX_HEIGHT + vspacing()) + vbigspacing()
	 + 1 * vspacing() + ingame_music_volume_label.get_h(),
	 get_inner_w() - 2 * hmargin(), slideh(),
	 0, g_sound_handler.get_max_volume(), g_sound_handler.get_music_volume(),
	 g_gr->images().get("pics/but1.png")),
ingame_sound_volume_label
	(this,
	 hmargin(),
	 vmargin() + 2 * (STATEBOX_HEIGHT + vspacing()) + vbigspacing()
	 + 2 * vspacing() + slideh() + ingame_music_volume_label.get_h(),
	 _("Sound FX Volume")),
ingame_sound_volume
	(this,
	 hmargin(),
	 vmargin() + 2 * (STATEBOX_HEIGHT + vspacing()) + vbigspacing()
	 + 3 * vspacing() + slideh()
	 + ingame_music_volume_label.get_h() + ingame_music_volume_label.get_h(),
	 get_inner_w() - 2 * hmargin(), slideh(),
	 0, g_sound_handler.get_max_volume(), g_sound_handler.get_fx_volume(),
	 g_gr->images().get("pics/but1.png"))
{
	ingame_music.set_state(not g_sound_handler.get_disable_music());
	ingame_sound.set_state(not g_sound_handler.get_disable_fx   ());

	if (g_sound_handler.m_lock_audio_disabling) { //  disabling sound options
		ingame_music       .set_enabled(false);
		ingame_sound       .set_enabled(false);
		ingame_music_volume.set_enabled(false);
		ingame_sound_volume.set_enabled(false);
	} else { // initial widget states
		ingame_music.set_state         (not g_sound_handler.get_disable_music());
		ingame_sound.set_state         (not g_sound_handler.get_disable_fx   ());
		ingame_music_volume.set_enabled(not g_sound_handler.get_disable_music());
		ingame_sound_volume.set_enabled(not g_sound_handler.get_disable_fx   ());
	}

	//  ready signals
	ingame_music.changedto.connect
		(boost::bind(&GameOptionsSoundMenu::changed_ingame_music, this, _1));
	ingame_sound.changedto.connect
		(boost::bind(&GameOptionsSoundMenu::changed_ingame_sound, this, _1));
	ingame_music_volume.changedto.connect
		(boost::bind(&GameOptionsSoundMenu::music_volume_changed, this, _1));
	ingame_sound_volume.changedto.connect
		(boost::bind(&GameOptionsSoundMenu::sound_volume_changed, this, _1));

	uint32_t boxes_width =
		STATEBOX_WIDTH + hspacing() +
		std::max(ingame_music_label.get_w(), ingame_sound_label.get_w());
	uint32_t labels_width = std::max
		(ingame_music_volume_label.get_w(), ingame_sound_volume_label.get_w());

	set_inner_size
		(std::max
		 	(static_cast<uint32_t>(get_inner_w()),
		 	 2 * hmargin() + std::max(boxes_width, labels_width)),
		 2 * vmargin() + 2 * (STATEBOX_HEIGHT + vspacing())
		 + vbigspacing() + 3 * vspacing() + 2 * slideh()
		 + ingame_music_volume_label.get_h() + ingame_music_volume_label.get_h());

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
