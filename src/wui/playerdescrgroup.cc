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

#include <boost/format.hpp>

#include "constants.h"
#include "gamesettings.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "wexception.h"


struct PlayerDescriptionGroupImpl {
	GameSettingsProvider * settings;
	uint32_t plnum;

	UI::Textarea     * plr_name;
	UI::Checkbox     * btnEnablePlayer;
	UI::Button * btnPlayerTeam;
	UI::Button * btnPlayerType;
	UI::Button * btnPlayerTribe;
	UI::Button * btnPlayerInit;
};

PlayerDescriptionGroup::PlayerDescriptionGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t               const plnum,
	 UI::Font * font)
:
UI::Panel(parent, x, y, w, h),
d(new PlayerDescriptionGroupImpl)
{
	d->settings = settings;
	d->plnum = plnum;

	int32_t xplrname = 0;
	int32_t xplayertype = w * 35 / 125;
	int32_t xplayerteam = w * 35 / 125;
	int32_t xplayertribe = w * 80 / 125;
	int32_t xplayerinit = w * 55 / 125;
	d->plr_name = new UI::Textarea(this, xplrname, 0, xplayertype - xplrname, h);
	d->plr_name->set_textstyle(UI::TextStyle::makebold(font, UI_FONT_CLR_FG));
	d->btnEnablePlayer = new UI::Checkbox(this, Point(xplayertype - 23, 0));
	d->btnEnablePlayer->changedto.connect
		(boost::bind(&PlayerDescriptionGroup::enable_player, this, _1));
	d->btnPlayerType = new UI::Button
		(this, "player_type",
		 xplayertype, 0, xplayertribe - xplayertype - 2, h / 2,
		 g_gr->images().get("pics/but1.png"),
		 std::string(), std::string(),
		 true, false);
	d->btnPlayerType->sigclicked.connect
		(boost::bind(&PlayerDescriptionGroup::toggle_playertype, boost::ref(*this)));
	d->btnPlayerType->set_font(font);
	d->btnPlayerTeam = new UI::Button
		(this, "player_team",
		 xplayerteam, h / 2, xplayerinit - xplayerteam - 2, h / 2,
		 g_gr->images().get("pics/but1.png"),
		 std::string(), std::string(),
		 true, false);
	d->btnPlayerTeam->sigclicked.connect
		(boost::bind(&PlayerDescriptionGroup::toggle_playerteam, boost::ref(*this)));
	d->btnPlayerTeam->set_font(font);
	d->btnPlayerTribe = new UI::Button
		(this, "player_tribe",
		 xplayertribe, 0, w - xplayertribe, h / 2,
		 g_gr->images().get("pics/but1.png"),
		 std::string(), std::string(),
		 true, false);
	d->btnPlayerTribe->sigclicked.connect
		(boost::bind(&PlayerDescriptionGroup::toggle_playertribe, boost::ref(*this)));
	d->btnPlayerTribe->set_font(font);
	d->btnPlayerInit = new UI::Button
		(this, "player_initialization",
		 xplayerinit, h / 2, w - xplayerinit, h / 2,
		 g_gr->images().get("pics/but1.png"),
		 std::string(), _("Initialization"),
		 true, false);
	d->btnPlayerInit->sigclicked.connect
		(boost::bind(&PlayerDescriptionGroup::toggle_playerinit, boost::ref(*this)));
	d->btnPlayerInit->set_font(font);

	refresh();
}


PlayerDescriptionGroup::~PlayerDescriptionGroup()
{
	delete d;
	d = nullptr;
}


/**
 * Update display and enabled buttons based on current settings.
 */
void PlayerDescriptionGroup::refresh()
{
	const GameSettings & settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	const PlayerSettings & player = settings.players[d->plnum];
	bool stateaccess = d->settings->canChangePlayerState(d->plnum);
	bool tribeaccess = d->settings->canChangePlayerTribe(d->plnum);
	bool const initaccess  = d->settings->canChangePlayerInit(d->plnum);
	bool teamaccess = d->settings->canChangePlayerTeam(d->plnum);

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::stateClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerTeam ->set_visible(false);
		d->btnPlayerTeam ->set_enabled(false);
		d->btnPlayerType ->set_visible(false);
		d->btnPlayerType ->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->btnPlayerInit ->set_visible(false);
		d->btnPlayerInit ->set_enabled(false);
		d->plr_name->set_text(std::string());
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::stateOpen) {
			d->btnPlayerType ->set_title(_("Open"));
			d->btnPlayerTeam ->set_visible(false);
			d->btnPlayerTeam ->set_visible(false);
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerInit ->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->btnPlayerInit ->set_enabled(false);
			d->plr_name->set_text(std::string());
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer) {
				if (player.ai.empty())
					title = _("Computer");
				else {
					if (player.random_ai) {
						title += _("AI: Random");
					} else {
						/** TRANSLATORS %s = AI type, e.g. 'Agressive' */
						title += (boost::format(_("AI: %s")) % _(player.ai)).str();
					}
				}
			} else { // PlayerSettings::stateHuman
				title = _("Human");
			}
			d->btnPlayerType->set_title(title);
			std::string tribepath("tribes/" + player.tribe);
			if (!m_tribenames[player.tribe].size()) {
				// get translated tribesname
				Profile prof
					((tribepath + "/conf").c_str(), nullptr, "tribe_" + player.tribe);
				Section & global = prof.get_safe_section("tribe");
				m_tribenames[player.tribe] = global.get_safe_string("name");
			}
			if (player.random_tribe) {
				d->btnPlayerTribe->set_title(_("Random"));
			} else {
				d->btnPlayerTribe->set_title(m_tribenames[player.tribe]);
			}

			{
				i18n::Textdomain td(tribepath); // for translated initialisation
				container_iterate_const
					 (std::vector<TribeBasicInfo>, settings.tribes, i)
				{
					if (i.current->name == player.tribe) {
						d->btnPlayerInit->set_title
							(_
								(i.current->initializations.at
									(player.initialization_index)
								 .second));
						break;
					}
				}
			}
			d->plr_name->set_text(player.name);

			if (player.team) {
				char buf[64];
				snprintf(buf, sizeof(buf), "%i", player.team);
				d->btnPlayerTeam->set_title(buf);
			} else {
				d->btnPlayerTeam->set_title("--");
			}

			d->btnPlayerTeam ->set_visible(true);
			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerInit ->set_visible(true);
			d->btnPlayerTeam ->set_enabled(teamaccess);
			d->btnPlayerTribe->set_enabled(tribeaccess);
			d->btnPlayerInit ->set_enabled(initaccess);
		}
	}
}


/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	const GameSettings & settings = d->settings->settings();

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
	const GameSettings & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const PlayerSettings & player = settings.players.at(d->plnum);
	const std::string & currenttribe = player.tribe;
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

	d->settings->setPlayerTribe(d->plnum, nexttribe, random_tribe);
}

/**
 * Cycle through team numbers.
 *
 * \note Since each "useful" team needs at least two players, the maximum
 * number of teams is the number of players divided by two. Lonely players
 * can just have team number set to 0.
 */
void PlayerDescriptionGroup::toggle_playerteam()
{
	const GameSettings & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	Widelands::TeamNumber currentteam = settings.players[d->plnum].team;
	Widelands::TeamNumber maxteam = settings.players.size() / 2;
	Widelands::TeamNumber newteam;

	if (currentteam >= maxteam)
		newteam = 0;
	else
		newteam = currentteam + 1;

	d->settings->setPlayerTeam(d->plnum, newteam);
}

/// Cycle through available initializations for the player's tribe.
void PlayerDescriptionGroup::toggle_playerinit()
{
	const GameSettings & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const PlayerSettings & player = settings.players[d->plnum];
	container_iterate_const(std::vector<TribeBasicInfo>, settings.tribes, j)
		if (j.current->name == player.tribe)
			return
				d->settings->setPlayerInit
					(d->plnum,
					 (player.initialization_index + 1)
					 %
					 j.current->initializations.size());
	assert(false);
}
