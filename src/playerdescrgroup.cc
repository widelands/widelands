/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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
#include "tribe.h"
#include "wexception.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"


struct PlayerDescriptionGroupImpl {
	GameSettingsProvider * settings;
	uint32_t plnum;

	UI::Textarea     * plr_name;
	UI::Checkbox     * btnEnablePlayer;
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
	d->plr_name = new UI::Textarea
		(this, 0, 0, w * 11 / 50, h, std::string(), Align_Left);
	d->plr_name->set_font(fname, fsize, UI_FONT_CLR_FG);
	d->btnEnablePlayer = new UI::Checkbox(this, Point(w * 29 / 125 - 23, 0));
	d->btnEnablePlayer->changedto.set
		(this, &PlayerDescriptionGroup::enable_player);
	d->btnPlayerType = new UI::Callback_Button<PlayerDescriptionGroup>
		(this,
		 w * 29 / 125, 0, w * 19 / 100, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playertype, *this,
		 std::string(), std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerTribe = new UI::Callback_Button<PlayerDescriptionGroup>
		(this,
		 w * 43 / 100, 0, w * 5 / 25, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playertribe, *this,
		 std::string(), std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerInit = new UI::Callback_Button<PlayerDescriptionGroup>
		(this,
		 w * 64 / 100, 0, w * 3 / 10, h,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &PlayerDescriptionGroup::toggle_playerinit, *this,
		 std::string(), _("Initialization"),
		 true, false,
		 fname, fsize);
	d->btnReadyPlayer = new UI::Checkbox(this, Point(w * 189 / 200, 0));
	d->btnReadyPlayer->changedto.set
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

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::stateClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerType->set_visible(false);
		d->btnPlayerType->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->btnPlayerInit->set_visible(false);
		d->btnPlayerInit->set_enabled(false);
		d->plr_name->set_text(std::string());
		d->btnReadyPlayer->set_visible(false);
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::stateOpen) {
			d->btnPlayerType->set_title(_("Open"));
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerInit ->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->btnPlayerInit ->set_enabled(false);
			d->plr_name->set_text(std::string());
			d->btnReadyPlayer->set_visible(false);
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer) {
				if (player.ai.size() == 0)
					title = _("Computer");
				else {
					title = _("AI: ");
					title += _(player.ai);
				}
				d->btnReadyPlayer->set_visible(false);
			} else { // PlayerSettings::stateHuman
				title = _("Human");
				if (settings.multiplayer)
					d->btnReadyPlayer->set_visible(true);
				else
					d->btnReadyPlayer->set_visible(false);
			}
			// get translated tribesname
			std::string tribepath("tribes/" + player.tribe);
			Profile prof((tribepath + "/conf").c_str(), 0, tribepath.c_str());
			Section & global = prof.get_safe_section("tribe");
			d->btnPlayerTribe->set_title(global.get_safe_string("name"));
			d->btnPlayerType->set_title(title);
			{
				i18n::Textdomain td(tribepath); // for translated initialisation
				for
					(struct {
					 	std::vector<TribeBasicInfo>::const_iterator       current;
					 	std::vector<TribeBasicInfo>::const_iterator const end;
					 } i = {settings.tribes.begin(), settings.tribes.end()};;
					 ++i.current)
				{
					assert(i.current < i.end);
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

			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerInit ->set_visible(true);
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
 * Show/hide player's tribe and init buttons
 * This is useful for multi player savegames, as the tribes can't be changed
 */
void PlayerDescriptionGroup::show_tribe_button(bool show)
{
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
 * The checkbox to indicate a player is ready to start
 */
void PlayerDescriptionGroup::ready_player(bool on)
{
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	if (on) {
		d->settings->setPlayerReady(d->plnum, PlayerSettings::stateReady);
	} else {
		d->settings->setPlayerReady(d->plnum, PlayerSettings::stateNotReady);
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
