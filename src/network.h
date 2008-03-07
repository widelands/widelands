/*
 * Copyright (C) 2004-2008 by the Widelands Development Team
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

#ifndef NETWORK_H
#define NETWORK_H

#include "constants.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "md5.h"
#include "widelands_streamread.h"
#include "widelands_streamwrite.h"

#include <SDL_net.h>

#include <queue>
#include <string>
#include <cstring>
#include <vector>


struct Deserializer;
struct NetHostImpl;
class RecvPacket;
class SendPacket;

struct Chat_Message {
	uint32_t plrnum;
	std::string msg;
};


struct SyncCallback {
	virtual ~SyncCallback() {}
	virtual void syncreport() = 0;
};


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

	void disconnectPlayer(uint8_t number);
	void disconnectClient(uint32_t number);
	void reaper();

	NetHostImpl* d;
};

struct NetClientImpl;

struct NetClient : public GameController, public GameSettingsProvider, private SyncCallback {
	NetClient (IPaddress*, const std::string& playername);
	virtual ~NetClient ();

	void run();

	// GameController interface
	void think();
	void sendPlayerCommand(Widelands::PlayerCommand* pc);
	int32_t getFrametime();
	std::string getGameDescription();

	// GameSettingsProvider interface
	virtual const GameSettings& settings();

	virtual bool canChangeMap();
	virtual bool canChangePlayerState(uint8_t number);
	virtual bool canChangePlayerTribe(uint8_t number);

	virtual bool canLaunch();

	virtual void setMap(const std::string& mapname, const std::string& mapfilename, uint32_t maxplayers);
	virtual void setPlayerState(uint8_t number, PlayerSettings::State state);
	virtual void nextPlayerState(uint8_t number);
	virtual void setPlayerTribe(uint8_t number, const std::string& tribe);

private:
	void syncreport();

	void handle_network ();
	void sendTime();
	void recvOnePlayer(uint8_t number, Widelands::StreamRead& packet);
	void disconnect ();

	NetClientImpl* d;
};

/**
 * Buffered StreamWrite object for assembling a packet that will be
 * sent via the \ref send() function.
 */
struct SendPacket : public Widelands::StreamWrite {
	SendPacket ();

	void send (TCPsocket);
	void reset ();

	void Data(void const * data, size_t size);

private:
	std::vector<uint8_t> buffer;
};


/**
 * One packet, as received by the deserializer.
 */
struct RecvPacket : public Widelands::StreamRead {
public:
	RecvPacket(Deserializer& des);

	size_t Data(void * data, size_t bufsize);
	bool EndOfFile() const;

private:
	std::vector<uint8_t> buffer;
	size_t m_index;
};

struct Deserializer {
	/**
	 * Read data from the given socket.
	 * \return \c false if the socket was disconnected or another error occured.
	 * \c true if some data could be read (this does not imply that \ref avail
	 * will return \c true !)
	 */
	bool read (TCPsocket);

	/**
	 * \return \c true if an entire packet has been received.
	 */
	bool avail () const;

private:
	friend class RecvPacket;
	std::vector<uint8_t> queue;
	size_t index;
};

#endif
