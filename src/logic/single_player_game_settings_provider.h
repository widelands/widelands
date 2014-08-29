/*
 * Copyright (C) 2014 by the Widelands Development Team
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

#ifndef WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H
#define WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H

#include "logic/game_settings.h"

/**
 * The settings provider for normal singleplayer games:
 * The user can change everything, except that they are themselves human.
 */
struct SinglePlayerGameSettingsProvider : public GameSettingsProvider {
	SinglePlayerGameSettingsProvider();

	void setScenario(bool const set) override;

	const GameSettings & settings() override;

	bool canChangeMap() override;
	bool canChangePlayerState(uint8_t number) override;
	bool canChangePlayerTribe(uint8_t) override;
	bool canChangePlayerInit (uint8_t) override;
	bool canChangePlayerTeam(uint8_t) override;
	bool canLaunch() override;

	virtual std::string getMap();
	virtual void setMap(const std::string & mapname, const std::string & mapfilename,
		uint32_t const maxplayers, bool const savegame) override;

	void setPlayerState(uint8_t const number, PlayerSettings::State state) override;
	void setPlayerAI(uint8_t const number, const std::string & ai, bool const random_ai) override;
	void nextPlayerState(uint8_t const number) override;
	void setPlayerTribe(uint8_t const number, const std::string & tribe, bool random_tribe) override;
	void setPlayerInit(uint8_t const number, uint8_t const index) override;
	void setPlayerTeam(uint8_t number, Widelands::TeamNumber team) override;
	void setPlayerCloseable(uint8_t, bool) override;
	void setPlayerShared(uint8_t, uint8_t) override;
	void setPlayerName(uint8_t const number, const std::string & name) override;
	void setPlayer(uint8_t const number, PlayerSettings const ps) override;
	void setPlayerNumber(uint8_t const number) override;

	std::string getWinConditionScript() override;
	void setWinConditionScript(std::string wc) override;
	void nextWinCondition() override;

private:
	GameSettings s;
};

#endif  // end of include guard: WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H
