/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef NETHOST_H
#define NETHOST_H

#include "widelands.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "network.h"

struct ChatMessage;
struct NetHostImpl;

/**
 * NetHost manages the lifetime of a network game in which this computer
 * acts as the host.
 *
 * This includes running the game setup screen and the actual game after
 * launch, as well as dealing with the actual network protocol.
 */
struct NetHost : public GameController, private SyncCallback {
	NetHost (const std::string& playername);
	virtual ~NetHost ();

	void run();
	const std::string& getLocalPlayername() const;

	// GameController interface
	void think();
	void sendPlayerCommand(Widelands::PlayerCommand* pc);
	int32_t getFrametime();
	std::string getGameDescription();

	uint32_t realSpeed();
	uint32_t desiredSpeed();
	void setDesiredSpeed(uint32_t speed);
	// End GameController interface

	// Pregame-related stuff
	const GameSettings& settings();
	bool canLaunch();
	void setScenario(bool);//no scenario for multiplayer
	void setMap(const std::string& mapname, const std::string& mapfilename, uint32_t maxplayers, bool savegame = false);
	void setPlayerState (uint8_t number, PlayerSettings::State state, bool host = false);
	void setPlayerTribe (uint8_t number, const std::string& tribe);
	void setPlayerInit  (uint8_t number, uint8_t index);
	void setPlayerName  (uint8_t number, const std::string& name);
	void setPlayer      (uint8_t number, PlayerSettings ps);
	void setPlayerNumber(int32_t number);
	void setMultiplayerGameSettings();//just visible stuff for the select mapmenu

	// Chat-related stuff
	void send(const ChatMessage& msg);

private:
	void sendSystemChat(char const * fmt, ...) PRINTF_FORMAT(2, 3);
	void requestSyncReports();
	void checkSyncReports();
	void syncreport();

	void clearComputerPlayers();
	void initComputerPlayer(Widelands::Player_Number p);
	void initComputerPlayers();

	void handle_packet(uint32_t i, RecvPacket& r);
	void handle_network ();

	void checkHungClients();
	void broadcastRealSpeed(uint32_t speed);
	void updateNetworkSpeed();

	std::string getComputerPlayerName(uint32_t playernum);
	bool haveUserName(const std::string& name, int32_t ignoreplayer = -1);
	void welcomeClient(uint32_t number, const std::string& playername);
	void committedNetworkTime(int32_t time);
	void recvClientTime(uint32_t number, int32_t time);

	void broadcast(SendPacket& packet);
	void writeSettingMap(SendPacket& packet);
	void writeSettingPlayer(SendPacket& packet, uint8_t number);
	void writeSettingAllPlayers(SendPacket& packet);
	void writeSettingUser(SendPacket& packet, uint32_t number);
	void writeSettingAllUsers(SendPacket& packet);

	void disconnectPlayer(uint8_t number, const std::string& reason, bool sendreason = true);
	void disconnectClient(uint32_t number, const std::string& reason, bool sendreason = true);
	void reaper();

	NetHostImpl* d;
};


#endif // NETHOST_H
