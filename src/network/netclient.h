/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETCLIENT_H
#define WL_NETWORK_NETCLIENT_H

#include <memory>

#include <SDL_net.h>

#include "network/network.h"

/**
 * NetClient manages the network connection for a network game in which this computer
 * participates as a client.
 */
class NetClient {
public:
	/**
	 * Tries to establish a connection to the given host.
	 * @param ip_address A hostname or an IPv4 address as string.
	 * @param port The port to connect to.
	 * @return A pointer to a connected \c NetClient object or a nullptr if the connection failed.
	 */
	static std::unique_ptr<NetClient> connect(const std::string& ip_address, const uint16_t port);

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message to the host, do so before freeing the object.
	 */
	~NetClient();

	/**
	 * Returns whether the client is connected.
	 * @return \c true if the connection is open, \c false otherwise.
	 */
	bool is_connected() const;

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message to the host, do so before calling this.
	 */
	void close();

	/**
	 * Tries to receive a packet.
	 * @param packet A packet that should be overwritten with the received data.
	 * @return \c true if a packet is available, \c false otherwise.
	 *   The given packet is only modified when \c true is returned.
	 *   Calling this on a closed connection will return false.
	 */
	bool try_receive(RecvPacket* packet);

	/**
	 * Sends a packet.
	 * Calling this on a closed connection will silently fail.
	 * @param packet The packet to send.
	 */
	void send(const SendPacket& packet);

private:
	NetClient(const std::string& ip_address, const uint16_t port);

	/// The socket that connects us to the host
	TCPsocket sock_;

	/// Socket set used for selection
	SDLNet_SocketSet sockset_;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer deserializer_;
};

#endif  // end of include guard: WL_NETWORK_NETCLIENT_H
