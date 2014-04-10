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

#ifndef NETHOST_H
#define NETHOST_H

#include "gamecontroller.h"
#include "gamesettings.h"
#include "logic/widelands.h"
#include "network/network.h"

struct ChatMessage;
struct NetHostImpl;
struct Client;

/**
 * NetHost manages the lifetime of a network game in which this computer
 * acts as the host.
 *
 * This includes running the game setup screen and the actual game after
 * launch, as well as dealing with the actual network protocol.
 */
struct NetHost : public GameController, private SyncCallback {
	NetHost (const std::string & playername, bool internet = false);
	virtual ~NetHost ();

	void run(bool autostart = false);
	const std::string & getLocalPlayername() const;
	int16_t getLocalPlayerposition();

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
	// End GameController interface

	// Pregame-related stuff
	const GameSettings & settings();
	bool canLaunch();
	void setScenario(bool);
	void setMap
		(const std::string & mapname,
		 const std::string & mapfilename,
		 uint32_t            maxplayers,
		 bool                savegame = false);
	void setPlayerState    (uint8_t number, PlayerSettings::State state, bool host = false);
	void setPlayerTribe    (uint8_t number, const std::string & tribe, bool const random_tribe = false);
	void setPlayerInit     (uint8_t number, uint8_t index);
	void setPlayerAI       (uint8_t number, const std::string & name, bool const random_ai = false);
	void setPlayerName     (uint8_t number, const std::string & name);
	void setPlayer         (uint8_t number, PlayerSettings);
	void setPlayerNumber   (uint8_t number);
	void setPlayerTeam     (uint8_t number, Widelands::TeamNumber team);
	void setPlayerCloseable(uint8_t number, bool closeable);
	void setPlayerShared   (uint8_t number, uint8_t shared);
	void switchToPlayer    (uint32_t user,  uint8_t number);
	void setWinConditionScript   (std::string);

	// just visible stuff for the select mapmenu
	void setMultiplayerGameSettings();

	// Chat-related stuff
	void send(ChatMessage msg);

	//  Host command related stuff
	int32_t checkClient(std::string name);
	void kickUser(uint32_t, std::string);
	void splitCommandArray
		(const std::string & cmdarray, std::string & cmd, std::string & arg1, std::string & arg2);
	void handle_dserver_command(std::string, std::string);
	void dserver_send_maps_and_saves(Client &);

	void report_result(uint8_t player, Widelands::PlayerEndResult result, const std::string & info) override;

	void forcePause() {
		m_forced_pause = true;
		updateNetworkSpeed();
	}

	void endForcedPause() {
		m_forced_pause = false;
		updateNetworkSpeed();
	}

	bool forcedPause() {return m_forced_pause;}

	bool isDedicated() {return m_is_dedicated;}

private:
	NetTransferFile * file;

	void sendSystemMessageCode
		(const std::string &,
		 const std::string & a = "", const std::string & b = "", const std::string & c = "");
	void requestSyncReports();
	void checkSyncReports();
	void syncreport() override;

	void clearComputerPlayers();
	void initComputerPlayer(Widelands::Player_Number p);
	void initComputerPlayers();

	void handle_packet(uint32_t i, RecvPacket &);
	void handle_network ();
	void sendFilePart(TCPsocket, uint32_t);

	void checkHungClients();
	void broadcastRealSpeed(uint32_t speed);
	void updateNetworkSpeed();

	std::string getComputerPlayerName(uint8_t playernum);
	bool haveUserName
		(const std::string & name,
		 uint8_t             ignoreplayer = UserSettings::none());
	void welcomeClient(uint32_t number, std::string & playername);
	void committedNetworkTime(int32_t time);
	void recvClientTime(uint32_t number, int32_t time);

	void broadcast(SendPacket &);
	void writeSettingMap(SendPacket &);
	void writeSettingPlayer(SendPacket &, uint8_t number);
	void writeSettingAllPlayers(SendPacket &);
	void writeSettingUser(SendPacket &, uint32_t number);
	void writeSettingAllUsers(SendPacket &);
	bool writeMapTransferInfo(SendPacket &, std::string);

	void disconnectPlayerController
		(uint8_t number,
		 const std::string & name);
	void disconnectClient
		(uint32_t number,
		 const std::string & reason,
		 bool sendreason = true,
		 const std::string & arg = "");
	void reaper();

	NetHostImpl * d;
	bool m_internet;
	bool m_is_dedicated;
	std::string m_password;
	std::string m_dedicated_motd;
	bool m_forced_pause;
};


#endif
