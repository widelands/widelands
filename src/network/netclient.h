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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_NETWORK_NETCLIENT_H
#define WL_NETWORK_NETCLIENT_H

#include <memory>

#include "network/bufferedconnection.h"
#include "network/netclient_interface.h"
#include "network/network.h"

/**
 * NetClient manages the network connection for a network game in which this computer
 * participates as a client.
 * This class only tries to create a single socket, either for IPv4 and IPv6.
 * Which is used depends on what kind of address is given on call to connect().
 */
// This class is currently only an interface wrapper for BufferedConnection
class NetClient : public NetClientInterface {
public:
	/**
	 * Tries to establish a connection to the given host.
	 * \param host The host to connect to.
	 * \return A pointer to a connected \c NetClient object or a \c nullptr if the connection failed.
	 */
	static std::unique_ptr<NetClient> connect(const NetAddress& host);

	~NetClient() override;

	// Inherited from NetClientInterface
	bool is_connected() const override;
	void close() override;
	std::unique_ptr<RecvPacket> try_receive() override;
	void send(const SendPacket& packet, NetPriority priority = NetPriority::kNormal) override;

private:
	/**
	 * Tries to establish a connection to the given host.
	 * If the connection attempt failed, is_connected() will return \c false.
	 * \param host The host to connect to.
	 */
	explicit NetClient(const NetAddress& host);

	std::unique_ptr<BufferedConnection> conn_;
};

#endif  // end of include guard: WL_NETWORK_NETCLIENT_H
