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

#include "gamecontroller.h"
#include "gamesettings.h"
#include "network.h"

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

	// GameController interface
	void think();
	void sendPlayerCommand(Widelands::PlayerCommand* pc);
	int32_t getFrametime();
	std::string getGameDescription();

	// Pregame-related stuff
	const GameSettings& settings();
	bool canLaunch();
	void setMap(const std::string& mapname, const std::string& mapfilename, uint32_t maxplayers);
	void setPlayerState(uint8_t number, PlayerSettings::State state);
	void setPlayerTribe(uint8_t number, const std::string& tribe);

private:
	void requestSyncReports();
	void checkSyncReports();
	void syncreport();

	void clearComputerPlayers();
	void initComputerPlayers();

	void handle_packet(uint32_t i, RecvPacket& r);
	void handle_network ();

	void checkHungClients();
	void setRealSpeed(uint32_t speed);

	std::string getComputerPlayerName(uint32_t playernum);
	bool havePlayerName(const std::string& name, int32_t ignoreplayer = -1);
	void welcomeClient(uint32_t number, const std::string& playername);
	void committedNetworkTime(int32_t time);
	void recvClientTime(uint32_t number, int32_t time);

	void broadcast(SendPacket& packet);
	void writeSettingMap(SendPacket& packet);
	void writeSettingPlayer(SendPacket& packet, uint8_t number);
	void writeSettingAllPlayers(SendPacket& packet);

	void disconnectPlayer(uint8_t number, const std::string& reason, bool sendreason = true);
	void disconnectClient(uint32_t number, const std::string& reason, bool sendreason = true);
	void reaper();

	NetHostImpl* d;
};


#endif // NETHOST_H
