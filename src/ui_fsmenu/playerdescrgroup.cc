/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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
#include "logic/player.h"
#include "profile/profile.h"
#include "logic/tribe.h"
#include "wexception.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"


struct PlayerDescriptionGroupImpl {
	GameSettingsProvider * settings;
	uint32_t plnum;

	UI::Textarea     * plr_name;
	UI::Checkbox     * btnEnablePlayer;
	UI::Callback_Button<PlayerDescriptionGroup> * btnPlayerTeam;
	UI::Callback_Button<PlayerDescriptionGroup> * btnPlayerType;
	UI::Callback_Button<PlayerDescriptionGroup> * btnPlayerTribe;
	UI::Callback_Button<PlayerDescriptionGroup> * btnPlayerInit;
	UI::Checkbox     * btnReadyPlayer;
};

PlayerDescriptionGroup::PlayerDescriptionGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t               const plnum,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
d(new PlayerDescriptionGroupImpl)
{
	d->settings = settings;
	d->plnum = plnum;

	int32_t xplrname = 0;
	int32_t xplayertype = w * 28 / 125;
	int32_t xplayerteam = w * 55 / 125;
	int32_t xplayertribe = w * 60 / 125;
	int32_t xplayerinit = w * 85 / 125;
	int32_t xplayerready = w - 23;
	d->plr_name = new UI::Textarea(this, xplrname, 0, xplayertype - xplrname, h);
	d->plr_name->set_font(fname, fsize, UI_FONT_CLR_FG);
	d->btnEnablePlayer = new UI::Checkbox(this, Point(xplayertype - 23, 0));
	d->btnEnablePlayer->changedto.set
		(this, &PlayerDescriptionGroup::enable_player);
	d->btnPlayerType = new UI::Callback_Button<PlayerDescriptionGroup>
		(this, "player_type",
		 xplayertype, 0, xplayerteam - xplayertype - 2, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playertype, *this,
		 std::string(), std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerTeam = new UI::Callback_Button<PlayerDescriptionGroup>
		(this, "player_team",
		 xplayerteam, 0, xplayertribe - xplayerteam - 2, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playerteam, *this,
		 std::string(), std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerTribe = new UI::Callback_Button<PlayerDescriptionGroup>
		(this, "player_tribe",
		 xplayertribe, 0, xplayerinit - xplayertribe - 2, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playertribe, *this,
		 std::string(), std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerInit = new UI::Callback_Button<PlayerDescriptionGroup>
		(this, "player_initialization",
		 xplayerinit, 0, xplayerready - xplayerinit - 2, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playerinit, *this,
		 std::string(), _("Initialization"),
		 true, false,
		 fname, fsize);
	d->btnReadyPlayer = new UI::Checkbox(this, Point(xplayerready, 0));
	d->btnReadyPlayer->clickedto.set
		(this, &PlayerDescriptionGroup::ready_player);

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
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	PlayerSettings const & player = settings.players[d->plnum];
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
		d->btnReadyPlayer->set_visible(false);
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
			d->btnReadyPlayer->set_visible(false);
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer) {
				if (player.ai.empty())
					title = _("Computer");
				else {
					title = _("AI: ");
					title += _(player.ai);
				}
				d->btnReadyPlayer->set_visible(false);
			} else { // PlayerSettings::stateHuman
				title = _("Human");
				if (settings.multiplayer) {
					d->btnReadyPlayer->set_visible(true);
					d->btnReadyPlayer->set_enabled(tribeaccess);
					d->btnReadyPlayer->set_state(player.ready);
				} else
					d->btnReadyPlayer->set_visible(false);
			}
			std::string tribepath("tribes/" + player.tribe);
			if(!m_tribenames[player.tribe].size())
			{
				// get translated tribesname
				Profile prof
					((tribepath + "/conf").c_str(), 0, "tribe_" + player.tribe);
				Section & global = prof.get_safe_section("tribe");
				m_tribenames[player.tribe] = global.get_safe_string("name");
			}
			d->btnPlayerTribe->set_title(m_tribenames[player.tribe]);
			d->btnPlayerType->set_title(title);
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
 * Enables/Disables a playerposition completely
 * This is useful for multi player savegames, where specific player positions
 * were not used in the saved game.
 */
void PlayerDescriptionGroup::enable_pdg(bool enable)
{
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	enable_player(enable);
	d->btnEnablePlayer->set_enabled(enable);
	d->btnEnablePlayer->set_visible(enable);
	d->btnPlayerType  ->set_enabled(enable);
	d->btnPlayerType  ->set_visible(enable);
	d->btnPlayerTribe ->set_visible(enable);
	d->btnPlayerTribe ->set_enabled(enable);
	d->btnPlayerInit  ->set_visible(enable);
	d->btnPlayerInit  ->set_enabled(enable);
	if (!enable)
		d->plr_name->set_text(std::string());
}


/**
 * Show/hide player's tribe team and init buttons
 * This is useful for multi player savegames, as these can't be changed there.
 */
void PlayerDescriptionGroup::show_tribe_button(bool show)
{
	d->btnPlayerTeam  ->set_visible(show);
	d->btnPlayerTeam  ->set_enabled(show);
	d->btnPlayerTribe ->set_visible(show);
	d->btnPlayerTribe ->set_enabled(show);
	d->btnPlayerInit  ->set_visible(show);
	d->btnPlayerInit  ->set_enabled(show);
}


/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	GameSettings const & settings = d->settings->settings();

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

/**
 * The checkbox to indicate whether player is ready to start
 */
void PlayerDescriptionGroup::ready_player(bool const ready)
{
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	d->settings->setPlayerReady(d->plnum, ready);
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
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	std::string const & currenttribe = settings.players[d->plnum].tribe;
	std::string nexttribe = settings.tribes.at(0).name;

	for (uint32_t i = 0; i < settings.tribes.size() - 1; ++i)
		if (settings.tribes[i].name == currenttribe) {
			nexttribe = settings.tribes.at(i + 1).name;
			break;
		}

	d->settings->setPlayerTribe(d->plnum, nexttribe);
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
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	PlayerSettings const & player = settings.players[d->plnum];
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
