/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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

/**
 * Before the internet gaming relay was added, (a) NetHost and (multiple) NetClient established a
 * direct
 * connection and exchanged data packets created and processed by the GameHost/GameClient. With the
 * introduction of the relay this changed: GameHost/GameClient still create and process the data
 * packets.
 * For LAN games, they still pass them to the NetHost/NetClient.
 * For internet games, the traffic is relayed by the wlnr binary in the metaserver repository.
 * GameHost/GameClient pass their packets to the new classes NetHostProxy/NetClientProxy.
 * Those no longer have a direct connection but are both connected to the relay on the metaserver.
 * When they want to exchange messages, they send their packets to the relay which forwards them to
 * the
 * intended recipient.
 * The relay only transport the packets, it does not run any game logic. The idea of this is that
 * the
 * relay runs on an globally reachable computer (i.e. the one of the metaserver) and simplifies
 * connectivity for the users (i.e. no more port forwarding required).
 *
 * Below are the command codes used in the relay protocol. They are used on the connection
 * between NetHostProxy/NetClientProxy and relay.
 *
 * An example of a typical session:
 * 1) A user wants to start an internet game. The Widelands instance tells the metaserver about it.
 * 2) A relay instance is started by the metaserver. On startup of the relay, the nonce of the
 * player is set
 *    as password for the game-host position in the new game. Additionally, the name of the hosted
 * game
 *    is stored. The IP address of the relay is send to the Widelands instance by the metaserver.
 * 3) The Widelands instance of the user connects to the relay.
 * 4) Now there is a reachable relay/game running somewhere. The open game can be listed in the
 * lobby.
 * 5) Clients get the address and port of the relay by the metaserver and can connect to the game.
 *    When enough clients have connected, the GameHost can start the game.
 * 6) When a client wants to send a packet (e.g. user input) to the GameHost, its GameClient packs
 * the packet,
 *    passes it to the local NetClientProxy which sends it to the relay. The relay relays the packet
 * to the
 *    NetHostProxy which passes it to the GameHost.
 * 7) When the GameHost wants to send a packet to one or all clients, it also packs it and passes it
 * to its
 *    NetHostProxy which sends it to the relay, where it is forwarded to the client(s).
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CONSTANTS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * The current version of the network protocol.
 * Protocol versions must match on all systems.
 * Used versions:
 * 1: Initial version introduced between build 19 and build 20
 */
constexpr uint8_t kRelayProtocolVersion = 1;

/**
 * The commands used by the relay.
 *
 * If a command is followed by parameters, they are listed in the description.
 * Most of these commands do not require any reaction by the receiver.
 * Exceptions are described at the respective commands.
 *
 * Parameter types:
 *
 * Strings are NULL-terminated.
 *
 * Transmitting data packets:
 *
 * The main work of the relay consists of passing opaque data packets between GameHost
 * and GameClients. They send SendPacket's over the network and expect to receive RecvPacket's
 * (see network.h). Those packets basically consists of an array of uint8_t with the first two bytes
 * coding the length of the packet.
 * When in the following documentation the \c packet datatype is listed as payload data, such a
 * packet
 * is meant. Since they are already containing their own length, no explicit length field or
 * separator
 * is required to pack them. When sending they are just written out. When receiving, the
 * Deserializer
 * has to read the length first and read the appropriate amount of bytes afterwards.
 */
/**
 * This protocol is a binary protocol (as in: not human readable). The receiver reads only the first
 * byte
 * of the received data to determine which command has been send. Based on that, it knows whether to
 * expect
 * a string / an uint8 / a SendPacket. Based on this, it can read until NULL / one byte / as much
 * as the next two bytes specify. When all parameters have been read the next command starts
 * without any separator.
 */

// If anyone removes a command: Please leave a comment which command with which value was removed
enum class RelayCommand : uint8_t {
	// Value 0 should not be used

	/**
	 * Commands send by/to all participants.
	 */
	/// \{
	/**
	 * Sent by the NetHostProxy or NetClientProxy on connect to the relay.
	 * Has the following payload:
	 * \li unsigned_8: Protocol version.
	 * \li string:     Game name.
	 * \li string:     For the host: Password that was set on start of the relay. Is the "solution"
	 *                 for the challenge send by the metaserver on IGPCMD_GAME_OPEN
	 *                 For clients/observers: String "client".
	 *
	 * Is answered by kWelcome or kDisconnect (if a parameter is wrong/unknown).
	 */
	kHello = 1,

	/**
	 * Send by the relay to answer a kHello command.
	 * Confirms the successful connection with the relay. In the case of a connecting NetClientProxy,
	 * this does not mean that it can participate on the game. This decision is up to the GameHost as
	 * soon as it learns about the new client.
	 * Payload:
	 * \li unsigned_8: The protocol version. Should be the same as the one send in the kHello.
	 * \li string:     The name of the hosted game as shown in the internet lobby.
	 *                 The client might want to check this.
	 *
	 * This command and its parameters are not really necessary. But it might be nice to have at
	 * least some
	 * confirmation that we have a connection to a (and the right) relay and not to some other
	 * server.
	 */
	kWelcome = 2,

	/**
	 * Can be sent by any participant.
	 * Might be the result of a protocol violation, an invalid password on connect of the
	 * NetHostProxy
	 * or a regular disconnect (e.g. the game is over).
	 * After sending or receiving this command, the TCP connection should be closed.
	 * \note When the game host sends its kDisconnect message, the relay will shut down.
	 * Payload:
	 * \li string: An error code describing the reason of the disconnect. Valid values:
	 *             NORMAL: Regular disconnect (game has ended, host leaves, client leaves, ...);
	 *             PROTOCOL_VIOLATION: Some protocol error (unknown command, invalid parameters,
	 * ...);
	 *             WRONG_VERSION: The version in the kHello packet is not supported;
	 *             GAME_UNKNOWN: Game name provided in kHello packet is unknown;
	 *             NO_HOST: No host is connected to the relay yet;
	 *             INVALID_CLIENT: Host tried to send a message to a non-existing client
	 */
	kDisconnect = 3,

	/**
	 * The relay sends this message to check for presence and to measure the round-trip-time.
	 * Has to be answered by kPong immediately.
	 * Payload:
	 * \li unsigned_8: A sequence number for this ping request.
	 */
	kPing = 4,

	/**
	 * Send to the relay to answer a kPing message.
	 * Payload:
	 * \li unsigned_8: Should be the sequence number found in the ping request.
	 */
	kPong = 5,

	/**
	 * Send to the relay to request the newest ping results.
	 * No payload.
	 */
	kRoundTripTimeRequest = 6,

	/**
	 * Send by the relay as an answer to the kRoundTripTimeRequest with the following payload:
	 * \li unsigned_8: Length of the list.
	 * A list of
	 * \li unsigned_8: Id of the client.
	 * \li unsigned_8: The RTT in milliseconds. Capped to max. 255ms.
	 * \li unsigned_8: Seconds since the last kPong has been received by the relay. Capped to max.
	 * 255ms.
	 */
	// TODO(Notabilis): Above documentation is broken. Seconds or Milliseconds? Or something else?
	kRoundTripTimeResponse = 7,
	/// \}

	/**
	 * Communication between relay and NetHostProxy.
	 */
	/// \{

	/**
	 * Send by the relay to the NetHostProxy to inform that a client established a connection to the
	 * relay.
	 * Payload:
	 * \li unsigned_8: An id to represent the new client.
	 */
	kConnectClient = 11,

	/**
	 * If send by the NetHostProxy, tells the relay to close the connection to a client.
	 * If send by the relay, informs the NetHostProxy that the connection to a client has been lost.
	 * Payload:
	 * \li unsigned_8: The id of the client.
	 */
	kDisconnectClient = 12,

	/**
	 * The NetHostProxy sends a message to a connected client over the relay.
	 * Payload:
	 * \li NULL terminated list of unsigned_8: The ids of the clients.
	 * \li packet: The SendPacket to relay.
	 */
	kToClients = 13,

	/**
	 * The relay transmits a packet from a client to the NetHostProxy.
	 * Payload:
	 * \li unsigned_8: The id of the client.
	 * \li packet: The SendPacket to relay.
	 */
	kFromClient = 14,
	/// \}

	/**
	 * Communication between relay and NetClientProxy.
	 */
	/// \{

	/**
	 * The NetClientProxy sends a message to the NetHostProxy over the relay.
	 * Direct communication between clients is not supported.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	kToHost = 21,

	/**
	 * The relay transmits a packet from a NetHostProxy to the NetClientProxy.
	 * Payload:
	 * \li packet: The SendPacket to relay.
	 */
	kFromHost = 22
	/// \}
};

#endif  // end of include guard: WL_NETWORK_RELAY_PROTOCOL_H
