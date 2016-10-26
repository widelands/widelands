/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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

#include "wui/playerdescrgroup.h"

#include <string>

#include <boost/format.hpp>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"

struct PlayerDescriptionGroupImpl {
	GameSettingsProvider* settings;
	uint32_t plnum;

	UI::Textarea* plr_name;
	UI::Checkbox* btnEnablePlayer;
	UI::Button* btnPlayerTeam;
	UI::Button* btnPlayerType;
	UI::Button* btnPlayerTribe;
	UI::Button* btnPlayerInit;
};

PlayerDescriptionGroup::PlayerDescriptionGroup(UI::Panel* const parent,
                                               int32_t const x,
                                               int32_t const y,
                                               int32_t const w,
                                               int32_t const h,
                                               GameSettingsProvider* const settings,
                                               uint32_t const plnum)
   : UI::Panel(parent, x, y, w, h), d(new PlayerDescriptionGroupImpl) {
	d->settings = settings;
	d->plnum = plnum;

	int32_t xplrname = 0;
	int32_t xplayertype = w * 35 / 125;
	int32_t xplayerteam = w * 35 / 125;
	int32_t xplayertribe = w * 80 / 125;
	int32_t xplayerinit = w * 55 / 125;
	d->plr_name = new UI::Textarea(this, xplrname, 0, xplayertype - xplrname, h);
	d->btnEnablePlayer = new UI::Checkbox(this, Vector2i(xplayertype - 23, 0), "");
	d->btnEnablePlayer->changedto.connect(
	   boost::bind(&PlayerDescriptionGroup::enable_player, this, _1));
	d->btnPlayerType =
	   new UI::Button(this, "player_type", xplayertype, 0, xplayertribe - xplayertype - 2, h / 2,
	                  g_gr->images().get("images/ui_basic/but1.png"), "");
	d->btnPlayerType->sigclicked.connect(
	   boost::bind(&PlayerDescriptionGroup::toggle_playertype, boost::ref(*this)));
	d->btnPlayerTeam =
	   new UI::Button(this, "player_team", xplayerteam, h / 2, xplayerinit - xplayerteam - 2, h / 2,
	                  g_gr->images().get("images/ui_basic/but1.png"), "");
	d->btnPlayerTeam->sigclicked.connect(
	   boost::bind(&PlayerDescriptionGroup::toggle_playerteam, boost::ref(*this)));
	d->btnPlayerTribe = new UI::Button(this, "player_tribe", xplayertribe, 0, w - xplayertribe,
	                                   h / 2, g_gr->images().get("images/ui_basic/but1.png"), "");
	d->btnPlayerTribe->sigclicked.connect(
	   boost::bind(&PlayerDescriptionGroup::toggle_playertribe, boost::ref(*this)));
	d->btnPlayerInit =
	   new UI::Button(this, "player_initialization", xplayerinit, h / 2, w - xplayerinit, h / 2,
	                  g_gr->images().get("images/ui_basic/but1.png"), "", _("Initialization"));
	d->btnPlayerInit->sigclicked.connect(
	   boost::bind(&PlayerDescriptionGroup::toggle_playerinit, boost::ref(*this)));

	refresh();
}

PlayerDescriptionGroup::~PlayerDescriptionGroup() {
	delete d;
	d = nullptr;
}

/**
 * Update display and enabled buttons based on current settings.
 */
void PlayerDescriptionGroup::refresh() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	const PlayerSettings& player = settings.players[d->plnum];
	bool stateaccess = d->settings->can_change_player_state(d->plnum);
	bool tribeaccess = d->settings->can_change_player_tribe(d->plnum);
	bool const initaccess = d->settings->can_change_player_init(d->plnum);
	bool teamaccess = d->settings->can_change_player_team(d->plnum);

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::stateClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerTeam->set_visible(false);
		d->btnPlayerTeam->set_enabled(false);
		d->btnPlayerType->set_visible(false);
		d->btnPlayerType->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->btnPlayerInit->set_visible(false);
		d->btnPlayerInit->set_enabled(false);
		d->plr_name->set_text(std::string());
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::stateOpen) {
			d->btnPlayerType->set_title(_("Open"));
			d->btnPlayerTeam->set_visible(false);
			d->btnPlayerTeam->set_visible(false);
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerInit->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->btnPlayerInit->set_enabled(false);
			d->plr_name->set_text(std::string());
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer) {
				if (player.ai.empty())
					title = _("Computer");
				else {
					if (player.random_ai) {
						title += _("Random AI");
					} else {
						const ComputerPlayer::Implementation* impl =
						   ComputerPlayer::get_implementation(player.ai);
						title = _(impl->descname);
					}
				}
			} else {  // PlayerSettings::stateHuman
				title = _("Human");
			}
			d->btnPlayerType->set_title(title);

			TribeBasicInfo info = Widelands::Tribes::tribeinfo(player.tribe);
			if (!tribenames_[player.tribe].size()) {
				// Tribe's localized name
				tribenames_[player.tribe] = info.descname;
			}
			if (player.random_tribe) {
				d->btnPlayerTribe->set_title(pgettext("tribe", "Random"));
				d->btnPlayerTribe->set_tooltip(_("The tribe will be set at random."));
			} else {
				i18n::Textdomain td("tribes");
				d->btnPlayerTribe->set_title(_(tribenames_[player.tribe]));
				d->btnPlayerTribe->set_tooltip(info.tooltip);
			}

			{
				i18n::Textdomain td("tribes");  // for translated initialisation
				for (const TribeBasicInfo& tribeinfo : settings.tribes) {
					if (tribeinfo.name == player.tribe) {
						d->btnPlayerInit->set_title(
						   _(tribeinfo.initializations.at(player.initialization_index).descname));
						d->btnPlayerInit->set_tooltip(
						   _(tribeinfo.initializations.at(player.initialization_index).tooltip));
						break;
					}
				}
			}
			d->plr_name->set_text(player.name);

			if (player.team) {
				d->btnPlayerTeam->set_title(std::to_string(static_cast<unsigned int>(player.team)));
			} else {
				d->btnPlayerTeam->set_title("--");
			}

			d->btnPlayerTeam->set_visible(true);
			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerInit->set_visible(true);
			d->btnPlayerTeam->set_enabled(teamaccess);
			d->btnPlayerTribe->set_enabled(tribeaccess);
			d->btnPlayerInit->set_enabled(initaccess);
		}
	}
}

/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on) {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	if (on) {
		if (settings.players[d->plnum].state == PlayerSettings::stateClosed)
			d->settings->next_player_state(d->plnum);
	} else {
		if (settings.players[d->plnum].state != PlayerSettings::stateClosed)
			d->settings->set_player_state(d->plnum, PlayerSettings::stateClosed);
	}
}

void PlayerDescriptionGroup::toggle_playertype() {
	d->settings->next_player_state(d->plnum);
}

/**
 * Cycle through available tribes for the player.
 */
void PlayerDescriptionGroup::toggle_playertribe() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players.at(d->plnum);
	const std::string& currenttribe = player.tribe;
	std::string nexttribe = settings.tribes.at(0).name;
	bool random_tribe = false;
	uint32_t num_tribes = settings.tribes.size();

	if (player.random_tribe) {
		nexttribe = settings.tribes.at(0).name;
	} else if (player.tribe == settings.tribes.at(num_tribes - 1).name) {
		nexttribe = "Random";
		random_tribe = true;
	} else {
		for (uint32_t i = 0; i < num_tribes - 1; ++i) {
			if (settings.tribes[i].name == currenttribe) {
				nexttribe = settings.tribes.at(i + 1).name;
				break;
			}
		}
	}

	d->settings->set_player_tribe(d->plnum, nexttribe, random_tribe);
}

/**
 * Cycle through team numbers.
 *
 * \note Since each "useful" team needs at least two players, the maximum
 * number of teams is the number of players divided by two. Lonely players
 * can just have team number set to 0.
 */
void PlayerDescriptionGroup::toggle_playerteam() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	Widelands::TeamNumber currentteam = settings.players[d->plnum].team;
	Widelands::TeamNumber maxteam = settings.players.size() / 2;
	Widelands::TeamNumber newteam;

	if (currentteam >= maxteam)
		newteam = 0;
	else
		newteam = currentteam + 1;

	d->settings->set_player_team(d->plnum, newteam);
}

/// Cycle through available initializations for the player's tribe.
void PlayerDescriptionGroup::toggle_playerinit() {
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players[d->plnum];

	for (const TribeBasicInfo& tribeinfo : settings.tribes) {
		if (tribeinfo.name == player.tribe) {
			return d->settings->set_player_init(
			   d->plnum, (player.initialization_index + 1) % tribeinfo.initializations.size());
		}
	}
	NEVER_HERE();
}
