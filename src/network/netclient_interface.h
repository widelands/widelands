/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_NETWORK_NETCLIENT_INTERFACE_H
#define WL_NETWORK_NETCLIENT_INTERFACE_H

#include <memory>

#include "network/network.h"

/**
 * NetClient manages the network connection for a network game in which this computer
 * participates as a client.
 *
 * This class provides the interface all NetClient implementation have to follow.
 * Currently two implementations exists: A "real" NetClient for local games and a
 * NetClientProxy which relays commands over a relay server.
 */
class NetClientInterface {
public:
	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message to the host, do so before freeing the object.
	 */
	virtual ~NetClientInterface() {
	}

	/**
	 * Returns whether the client is connected.
	 *
	 * \return \c true if the connection is open, \c false otherwise.
	 */
	virtual bool is_connected() const = 0;

	/**
	 * Closes the connection.
	 *
	 * If you want to send a goodbye-message to the host, do so before calling this.
	 */
	virtual void close() = 0;

	/**
	 * Tries to receive a packet.
	 *
	 * \return A pointer to a packet if one packet is available, an invalid pointer otherwise.
	 *   Calling this on a closed connection will return an invalid pointer.
	 */
	virtual std::unique_ptr<RecvPacket> try_receive() = 0;

	/**
	 * Sends a packet.
	 *
	 * Calling this on a closed connection will silently fail.
	 * \param packet The packet to send.
	 */
	virtual void send(const SendPacket& packet, NetPriority priority = NetPriority::kNormal) = 0;
};

#endif  // end of include guard: WL_NETWORK_NETCLIENT_INTERFACE_H
