/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "chat.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "network.h"

struct NetClientImpl;

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
	NetClient (IPaddress *, std::string const & playername, bool ggz = false);
	virtual ~NetClient ();

	void run();

	// GameController interface
	void think();
	void sendPlayerCommand(Widelands::PlayerCommand &);
	int32_t getFrametime();
	std::string getGameDescription();

	uint32_t realSpeed();
	uint32_t desiredSpeed();
	void setDesiredSpeed(uint32_t speed);
	// End GameController interface

	// GameSettingsProvider interface
	virtual GameSettings const & settings();

	virtual void setScenario(bool); //  FIXME no scenario for multiplayer
	virtual bool canChangeMap();
	virtual bool canChangePlayerState(uint8_t number);
	virtual bool canChangePlayerTribe(uint8_t number);
	virtual bool canChangePlayerInit (uint8_t number);

	virtual bool canLaunch();

	virtual void setMap
		(std::string const & mapname,
		 std::string const & mapfilename,
		 uint32_t maxplayers,
		 bool savegame = false);
	virtual void setPlayerState   (uint8_t number, PlayerSettings::State state);
	virtual void setPlayerAI      (uint8_t number, std::string const & ai);
	virtual void nextPlayerState  (uint8_t number);
	virtual void setPlayerTribe   (uint8_t number, std::string const & tribe);
	virtual void setPlayerInit    (uint8_t number, uint8_t index);
	virtual void setPlayerName    (uint8_t number, std::string const & name);
	virtual void setPlayer        (uint8_t number, PlayerSettings ps);
	virtual void setPlayerNumber  (uint8_t number);
	virtual void setPlayerReady   (uint8_t number, bool ready);
	virtual bool getPlayerReady   (uint8_t number);

	// ChatProvider interface
	void send(std::string const & msg);
	std::vector<ChatMessage> const & getMessages() const;

private:
	void syncreport();

	void handle_packet(RecvPacket &);
	void handle_network ();
	void sendTime();
	void recvOnePlayer(uint8_t  number, Widelands::StreamRead &);
	void recvOneUser  (uint32_t number, Widelands::StreamRead &);
	void disconnect
		(std::string const & reason,
		 bool sendreason = true, bool showmsg = true);

	NetClientImpl * d;
	bool use_ggz;
};

#endif // NETCLIENT_H
