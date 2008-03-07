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

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H


enum {
	/**
	 * The current version of the in-game network protocol. Client and host
	 * protocol versions must match.
	 */
	NETWORK_PROTOCOL_VERSION = 3,

	/**
	 * The default interval (in milliseconds) in which the host issues
	 * \ref NETCMD_SYNCREQUEST commands.
	 *
	 * This is a tunable for the host, and clients must not rely on its value.
	 */
	SYNCREPORT_INTERVAL = 1000,

	/**
	 * During the game, clients must send a \ref NETCMD_TIME command in this
	 * interval (in milliseconds). If the host does not receive such commands,
	 * it will enter a waiting state (see \ref NETCMD_WAIT).
	 *
	 * Note that a client may replace explicit \ref NETCMD_TIME commands by
	 * \ref NETCMD_PLAYERCOMMAND commands which imply a time update.
	 */
	CLIENT_TIMESTAMP_INTERVAL = 500,

	/**
	 * The default interval (in milliseconds) in which the host sends
	 * \ref NETCMD_TIME commands to update the game time to which clients
	 * may advance.
	 */
	SERVER_TIMESTAMP_INTERVAL = 100
};


/**
 * These are the command codes used in the in-game network protocol.
 *
 * The in-game network protocol is responsible for the game setup phase, i.e.
 * setting up the map, chosing tribes and so on, as well as for the actual
 * running game itself. It is \em not responsible for game discovery and
 * other "lobby" functionality.
 *
 * The network stream of the in-game network protocol is split up into
 * packets (see \ref Deserializer, \ref RecvPacket, \ref SendPacket).
 * Every packet starts with a single-byte command code.
 */
enum {
	NETCMD_UNUSED = 0,

	// Bidirectional messages
	/**
	 * Initiate a connection.
	 *
	 * The first communication across the network stream is a HELLO command
	 * sent by the client, with the following payload:
	 * \li Unsigned8: protocol version
	 * \li String: player name
	 *
	 * If the host accepts, it replies with a HELLO command with the following
	 * payload:
	 * \li Unsigned8: protocol version
	 * \li Unsigned8: 0-based player number for the client
	 *
	 * \note The host may override the client's chosen name in a subsequent
	 * \ref NETCMD_SETTING_ALLPLAYERS or \ref NETCMD_SETTING_PLAYER packet.
	 *
	 * \note For historical reasons, the HELLO command code should not be 1.
	 */
	NETCMD_HELLO = 16,

	/**
	 * Bidirectional command: Game time update. This command has the following
	 * payload:
	 * \li Signed32: game time
	 *
	 * If sent by the host, this command advises the client that it may
	 * simulate the game up to the given game time. The host guarantees that
	 * all player command with a duetime up to the given time have been sent.
	 *
	 * The client uses this command to inform the host how far its simulation
	 * has advanced.
	 */
	NETCMD_TIME = 22,

	/**
	 * Bidirectional command: Player command. This command has the payload:
	 * \li Signed32: game time
	 * \li serialized \ref Widelands::PlayerCommand
	 *
	 * If sent by the host, the client must enqueue the given \ref PlayerCommand
	 * with the sent game time as duetime. The host can send PlayerCommands for
	 * very player in the game.
	 *
	 * This command is sent by the client to request the execution of a
	 * \ref PlayerCommand. The client is only allowed to send PlayerCommands
	 * for its own player.
	 *
	 * In both cases, this command implicitly acts like a \ref NETCMD_TIME
	 * command.
	 *
	 * \note The client must never enqueue a \ref PlayerCommand that it issued
	 * itself. It must not assume that the host will accept a \ref PlaerCommand
	 * that it sends. Instead, it must rely on the host echoing back the command.
	 *
	 */
	NETCMD_PLAYERCOMMAND = 24,

	/**
	 * Bidirectional command: Terminate the connection with a given reason.
	 * Payload is:
	 * \li String: reason for disconnect
	 *
	 * Both host and client can send this command, followed by immediately
	 * closing the connection. The receiver of this command should just close
	 * the connection.
	 *
	 * Note that either party is allowed to close the connection without
	 * sending a \ref NETCMD_DISCONNECT command first (in any case, this can
	 * happen when the program crashes or network connection is lost).
	 *
	 * \note If you want to change the payload of this command, change it only
	 * by appending new items. The reason is that this is the only command
	 * that can be sent by the server even when the protocol versions differ.
	 */
	NETCMD_DISCONNECT = 27,

	// Server->Client messages
	/**
	 * During game setup, this command is sent by the host to advise clients
	 * of a map change. Payload is:
	 * \li String: human readable mapname
	 * \li String: map filename
	 */
	NETCMD_SETTING_MAP = 4,

	/**
	 * During game setup, this command is sent by the host to inform clients
	 * about the names of the tribes they may choose.
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_TRIBES = 5,

	/**
	 * During game setup, this command contains complete information about all
	 * player slots (independent of their state).
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_ALLPLAYERS = 6,

	/**
	 * During game setup, this command updates the information associated to
	 * one player slot.
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_PLAYER = 7,

	/**
	 * Sent by the host with no payload. The client must reply with a
	 * \ref NETCMD_PONG command.
	 */
	NETCMD_PING = 10,

	/**
	 * Sent by the host during game setup to indicate that the game starts.
	 *
	 * The client must load the map and setup the game. As soon as the game
	 * is fully loaded, it must behave as if a \ref NETCMD_WAIT command had
	 * been sent.
	 */
	NETCMD_LAUNCH = 20,

	/**
	 * Sent by the host to change the simulation speed. Payload is
	 * \li Unsigned16 game speed in milliseconds per second
	 *
	 * This advises the client of how fast it should simulate the game
	 * to avoid a choppy "stop-and-go" simulation. However, the client
	 * must never simulate beyond the game time allowed by (explicit or
	 * implicit) \ref NETCMD_TIME commands.
	 */
	NETCMD_SETSPEED = 21,

	/**
	 * Sent by the host when it suspects that a client has died or cannot
	 * catch up with the simulation.
	 *
	 * The client must continue simulating until it reaches the latest
	 * gametime received from the host via (explicit or implicit)
	 * \ref NETCMD_TIME commands and then send a \ref NETCMD_TIME to
	 * confirm that it is up to speed (in a sense, this time reply acts
	 * as a pong).
	 */
	NETCMD_WAIT = 23,

	/**
	 * Sent by the host to request an MD5 synchronization hash. Payload is:
	 * \li Signed32: game time at which the hash must be taken
	 *
	 * The client must reply with a \ref NETCMD_SYNCREPORT command as soon
	 * as the given game time has been reached on the client.
	 *
	 * \see Widelands::Game::get_sync_hash
	 */
	NETCMD_SYNCREQUEST = 25,

	// Client->Server messages
	/**
	 * Reply to a \ref NETCMD_PING command, with no payload.
	 */
	NETCMD_PONG = 11,

	/**
	 * During game setup, this is sent by the client to request a change
	 * to a different tribe. Payload is
	 * \li String: name of tribe
	 *
	 * The client must not assume that the host will accept this request.
	 * The host may or may not send a \ref NETCMD_SETTING_ALLPLAYERS or
	 * \ref NETCMD_SETTING_PLAYER command reflecting the changed tribe.
	 * If no such command is received, then the tribe is not changed.
	 */
	NETCMD_SETTING_CHANGETRIBE = 17,

	/**
	 * Sent by the client to reply to a \ref NETCMD_SYNCREQUEST command,
	 * with the following payload:
	 * \li Signed32: game time at which the hash was taken
	 * \li 16 bytes: MD5 hash
	 *
	 * It is solely the host's responsibility to act when desyncs are
	 * detected.
	 */
	NETCMD_SYNCREPORT = 26
};

#endif // NETWORK_PROTOCOL_H
