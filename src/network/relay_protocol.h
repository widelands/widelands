/*
 * Copyright (C) 2012-2017 by the Widelands Development Team
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

#ifndef WL_NETWORK_RELAY_PROTOCOL_H
#define WL_NETWORK_RELAY_PROTOCOL_H

#include <string>

/**
 * Before the internet gaming relay was added, (a) NetHost and (multiple) NetClient established a direct
 * connection and exchanged data packets created and processed by the GameHost/GameClient. With the
 * introduction of the relay this changes: GameHost/GameClient still create and process the data packets.
 * For LAN games, they still pass them to the NetHost/NetClient.
 * For internet games, the traffic is relayed by the metaserver. GameHost/GameClien pass their packets to
 * the new classes NetHostProxy/NetClientProxy. Those have no longer a direct connection but are both
 * connected to the relay on the metaserver. When they want to exchange messages, they send their packets
 * to the relay which forwards them to the intended recipient.
 * The relay only transport the packets, it does not run any game logic. The idea of this is that the
 * relay runs on an globally reachable computer (i.e. the one of the metaserver) and simplifies
 * connectivity for the users (i.e. no more port forwarding required).
 *
 * Below are the command codes used in the relay protocol. They are used on the connection
 * between NetHostProxy/NetClientProxy and relay.
 *
 * An example of a typical session:
 * 1) A user wants to start an internet game. His Widelands instance tells the metaserver about it.
 * 2) A relay instance is started by the metaserver. On startup of the relay, a password (random string)
 *    for the game-host position in the new game is set. Additionally, the name of the hosted game is stored.
 *    The address of the relay as well as the password is send to the Widelands instance by the metaserver.
 * 3) The GameHost is started in the Widelands instance of the user and connects to the relay.
 * 4) Now there is a reachable relay/game running somewhere. The open game can be listed in the lobby.
 * 5) Clients get the address and port of the relay by the metaserver and can connect to the game.
 *    When enough clients have connected, the GameHost can start the game.
 * 6) When a client wants to send a packet (e.g. user input) to the GameHost, its GameHost packs the packet,
 *    passed it to the local NetClientProxy which sends it to the relay. The relay relays the packet to the
 *    NetHostProxy which passes it to the GameHost.
 * 7) When the GameHost wants to send a packet to one or all clients, he also packs it and passes it to its
 *    NetHostProxy which sends it to the relay, where it is forwarded to the clients.
 */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CONSTANTS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define RELAY_PROTOCOL_VERSION 1
enum {
	/**
	 * The current version of the network protocol.
	 * Protocol versions must match on all systems.
	 * Used versions:
	 * 1: Initial version between build 19 and build 20
	 */
	RELAY_PROTOCOL_VERSION = 1,

	// This class does not need to care about timeouts, keep-alives, or reconnects
	// since higher layers (GameHost, GameClient) deal with it.
};

/**
 * The commands used by the relay.
 *
 * If a command is followed by parameters, they are listed in the description.
 * Strings are NULL-terminated.
 *
 * Most of these commands do not require any reaction by the receiver.
 * Exceptions are described at the respective commands.
 */
/**
 * Transmitting data packets.
 * 
 * The main work of the relay consists of passing unstructured data packets between GameHost
 * and GameClients. They send SendPacket's over the network and expect to receive RecvPacket's
 * (see network.h). Those packets basically consists of an array of uint8_t with the first two bytes
 * coding the length of the packet.
 * When in the following documentation the \c packet datatype is listed as payload data, such a packet
 * is meant. Since they are already containing their own length, no explicit length field or separator
 * is required to pack them. When sending they are just written out. When receiving, the Deserializer
 * has to read the length first and read the appropriate amount of bytes afterwards.
 */
// If anyone removes a command: Please leave a comment which command with which value was removed
enum : uint8_t {
	// Value 0 should not be used

	/**
	 * Commands send by/to all participants.
	 */
	/// \{
	/**
	 * Send by the relay to answer a RLYCMD_HELLO_HOST or RLYCMD_HELLO_CLIENT command.
	 * Confirms the successful connection with the relay. In the case of a connecting NetClientProxy,
	 * this does not mean that it can participate on the game. This decision is up to the GameHost as
	 * soon as it learns about the new client.
	 * Payload:
	 * \li unsigned_8: The protocol version. Should be the same as the one send in the HELLO.
	 * \li string:     The name of the hosted game as shown in the internet lobby.
	 *                 The client might want to check this.
	 *
	 * This command and its parameters are not really necessary. But it might be nice to have at least some
	 * confirmation that we have a connection to a (and the right) relay and not to some other server.
	 */
	RLYCMD_WELCOME = 1,

	/**
	 * Can be sent by any participant.
	 * Might be the result of a protocol violation, an invalid password on connect of the NetHostProxy
	 * or a regular disconnect (e.g. the game is over).
	 * After sending or receiving this command, the TCP connection should be closed.
	 */
	RLYCMD_DISCONNECT = 2,
	/// \}

	/**
	 * Communication between relay and NetHostProxy.
	 */
	/// \{
	/**
	 * Sent by the NetHostProxy on connect to the relay.
	 * Has the following payload:
	 * \li unsigned_8: Protocol version.
	 * \li string:     Game name.
	 * \li string:     Password that was set on start of the relay.
	 *
	 * Is answered by RLYCMD_WELCOME or RLYCMD_DISCONNECT (if a parameter is wrong/unknown).
	 * If there is already a NetHostProxy connected, the old one will be disconnected.
	 */
	RLYCMD_HELLO_HOST = 11,

	/**
	 * Send by the relay to the NetHostProxy to inform that a client established a connection to the relay.
	 * Payload:
	 * \li unsigned_8: An id to represent the new client.
	 */
	RLYCMD_CONNECT_CLIENT = 12,

	/**
	 * If send by the NetHostProxy, tells the relay to close the connection to a client.
	 * If send by the relay, informs the NetHostProxy that the connection to a client has been lost.
	 * Payload:
	 * \li unsigned_8: The id of the client.
	 */
	RLYCMD_DISCONNECT_CLIENT = 13,

	/**
	 * The NetHostProxy sends a message to a connected client over the relay.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	RLYCMD_TO_CLIENT = 14,

	/**
	 * The relay transmits a packet from a client to the NetHostProxy.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	RLYCMD_FROM_CLIENT = 15,

	/**
	 * Allows the GameHost to send a message to all connected clients.
	 * \li packet: THe SendPacket to broadcast.
	 */
	RLYCMD_BROADCAST = 16,
	/// \}

	/**
	 * Communication between relay and GameClient.
	 */
	/// \{

	/**
	 * Communication between relay and NetHostProxy.
	 */
	/// \{
	/**
	 * Sent by the NetClientProxy on connect to the relay.
	 * Has the following payload:
	 * \li unsigned_8: Protocol version.
	 * \li string:     Game name.
	 *
	 * Is answered by RLYCMD_WELCOME or RLYCMD_DISCONNECT (if a parameter is wrong/unknown).
	 */
	RLYCMD_HELLO_CLIENT = 11,

	/**
	 * The NetClientProxy sends a message to the NetHostProxy over the relay.
	 * Direct communication between clients is not supported.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	RLYCMD_TO_HOST = 14,

	/**
	 * The relay transmits a packet from a NetHostProxy to the NetClientProxy.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	RLYCMD_FROM_HOST = 15
	/// \}
};
