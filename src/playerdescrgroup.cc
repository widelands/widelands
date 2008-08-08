/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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
 *
 */

#include "playerdescrgroup.h"

#include "constants.h"
#include "gamesettings.h"
#include "i18n.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "wexception.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_textarea.h"


struct PlayerDescriptionGroupImpl {
	GameSettingsProvider* settings;
	uint32_t plnum;

	UI::Textarea* plr_name;
	UI::Checkbox* btnEnablePlayer;
	UI::Basic_Button* btnPlayerType;
	UI::Basic_Button* btnPlayerTribe;
};

PlayerDescriptionGroup::PlayerDescriptionGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y,
	 GameSettingsProvider * const settings,
	 uint32_t               const plnum)
:
UI::Panel(parent, x, y, 450, 20),
d(new PlayerDescriptionGroupImpl)
{
	d->settings = settings;
	d->plnum = plnum;

	d->plr_name = new UI::Textarea(this, 0, 0, 100, 20, std::string(), Align_Left);
	d->btnEnablePlayer = new UI::Checkbox(this, Point(88, 0));
	d->btnEnablePlayer->changedto.set(this, &PlayerDescriptionGroup::enable_player);
	d->btnPlayerType = new UI::Button<PlayerDescriptionGroup>
		(this,
		 116, 0, 120, 20,
		 1,
		 &PlayerDescriptionGroup::toggle_playertype, this,
		 "");
	d->btnPlayerTribe = new UI::Button<PlayerDescriptionGroup>
		(this,
		 244, 0, 120, 20,
		 1,
		 &PlayerDescriptionGroup::toggle_playertribe, this,
		 "");

	refresh();
}


PlayerDescriptionGroup::~PlayerDescriptionGroup()
{
	delete d;
	d = 0;
}


/**
 * Update display and enabled buttons based on current settings.
 */
void PlayerDescriptionGroup::refresh()
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	const PlayerSettings& player = settings.players[d->plnum];
	bool stateaccess = d->settings->canChangePlayerState(d->plnum);
	bool tribeaccess = d->settings->canChangePlayerTribe(d->plnum);

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::stateClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerType->set_visible(false);
		d->btnPlayerType->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->plr_name->set_text(std::string());
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::stateOpen) {
			d->btnPlayerType->set_title(_("Open"));
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->plr_name->set_text(std::string());
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer)
				title = _("Computer");
			else
				title = _("Human");
			// get translated tribesname
			std::string tribepath("tribes/" + player.tribe);
			i18n::grab_textdomain(tribepath);
			Profile prof((tribepath + "/conf").c_str());
			Section & global = prof.get_safe_section("tribe");
			std::string tribe(global.get_safe_string("name"));
			i18n::release_textdomain();

			d->btnPlayerType->set_title(title);
			d->btnPlayerTribe->set_title(tribe);
			d->plr_name->set_text(player.name);

			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerTribe->set_enabled(tribeaccess);
		}
	}
}


/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	if (on) {
		if (settings.players[d->plnum].state == PlayerSettings::stateClosed)
			d->settings->nextPlayerState(d->plnum);
	} else {
		if (settings.players[d->plnum].state != PlayerSettings::stateClosed)
			d->settings->setPlayerState(d->plnum, PlayerSettings::stateClosed);
	}
}


void PlayerDescriptionGroup::toggle_playertype()
{
	d->settings->nextPlayerState(d->plnum);
}

/**
 * Cycle through available tribes for the player.
 */
void PlayerDescriptionGroup::toggle_playertribe()
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const std::string& currenttribe = settings.players[d->plnum].tribe;
	std::string nexttribe = settings.tribes[0];

	for (uint32_t i = 0; i < settings.tribes.size()-1; ++i) {
		if (settings.tribes[i] == currenttribe) {
			nexttribe = settings.tribes[i+1];
			break;
		}
	}

	d->settings->setPlayerTribe(d->plnum, nexttribe);
}

