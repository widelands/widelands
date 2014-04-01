/*
 * Copyright (C) 2008-2012 by the Widelands Development Team
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

#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "chat.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "network/network.h"

struct NetClientImpl;

//FIXME Use composition instead of inheritance
/**
 * NetClient manages the lifetime of a network game in which this computer
 * participates as a client.
 *
 * This includes running the game setup screen and the actual game after
 * launch, as well as dealing with the actual network protocol.
 */
struct NetClient :
	public  GameController,
	public  GameSettingsProvider,
	private SyncCallback,
	public  ChatProvider
{
	NetClient (IPaddress *, const std::string & playername, bool internet = false);
	virtual ~NetClient ();

	void run();

	// GameController interface
	void think() override;
	void sendPlayerCommand(Widelands::PlayerCommand &) override;
	int32_t getFrametime() override;
	std::string getGameDescription() override;

	uint32_t realSpeed() override;
	uint32_t desiredSpeed() override;
	void setDesiredSpeed(uint32_t speed) override;
	bool isPaused() override;
	void setPaused(bool paused) override;
	void report_result
		(uint8_t player, Widelands::PlayerEndResult result, const std::string & info) override;
	// End GameController interface

	// GameSettingsProvider interface
	virtual const GameSettings & settings() override;

	virtual void setScenario(bool) override;
	virtual bool canChangeMap() override;
	virtual bool canChangePlayerState(uint8_t number) override;
	virtual bool canChangePlayerTribe(uint8_t number) override;
	virtual bool canChangePlayerInit (uint8_t number) override;
	virtual bool canChangePlayerTeam (uint8_t number) override;

	virtual bool canLaunch() override;

	virtual void setMap
		(const std::string & mapname,
		 const std::string & mapfilename,
		 uint32_t maxplayers,
		 bool savegame = false) override;
	virtual void setPlayerState    (uint8_t number, PlayerSettings::State state) override;
	virtual void setPlayerAI
		(uint8_t number, const std::string & ai, bool const random_ai = false) override;
	virtual void nextPlayerState   (uint8_t number) override;
	virtual void setPlayerTribe
		(uint8_t number, const std::string & tribe, bool const random_tribe = false) override;
	virtual void setPlayerInit     (uint8_t number, uint8_t index) override;
	virtual void setPlayerName     (uint8_t number, const std::string & name) override;
	virtual void setPlayer         (uint8_t number, PlayerSettings ps) override;
	virtual void setPlayerNumber   (uint8_t number) override;
	virtual void setPlayerTeam     (uint8_t number, Widelands::TeamNumber team) override;
	virtual void setPlayerCloseable(uint8_t number, bool closeable) override;
	virtual void setPlayerShared   (uint8_t number, uint8_t shared) override;
	virtual void setWinConditionScript   (std::string) override;
	virtual void nextWinCondition  () override;
	virtual std::string getWinConditionScript() override;

	// ChatProvider interface
	void send(const std::string & msg) override;
	const std::vector<ChatMessage> & getMessages() const override;

private:
	/// for unique backupname
	std::string backupFileName(std::string & path) {return path + "~backup";}

	NetTransferFile * file;

	void syncreport() override;

	void handle_packet(RecvPacket &);
	void handle_network ();
	void sendTime();
	void recvOnePlayer(uint8_t  number, Widelands::StreamRead &);
	void recvOneUser  (uint32_t number, Widelands::StreamRead &);
	void disconnect
		(const std::string & reason, const std::string & arg = "", bool sendreason = true, bool showmsg = true);

	NetClientImpl * d;
	bool m_internet;
	bool m_dedicated_access;
	bool m_dedicated_temp_scenario;
};

#endif
