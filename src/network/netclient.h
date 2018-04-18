/*
 * Copyright (C) 2008-2018 by the Widelands Development Team
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

#include "network/netclient_interface.h"
#include "network/network.h"

/**
 * NetClient manages the network connection for a network game in which this computer
 * participates as a client.
 * This class only tries to create a single socket, either for IPv4 and IPv6.
 * Which is used depends on what kind of address is given on call to connect().
 */
class NetClient : public NetClientInterface {
public:
	/**
	 * Tries to establish a connection to the given host.
	 * \param host The host to connect to.
	 * \return A pointer to a connected \c NetClient object or a \c nullptr if the connection failed.
	 */
	static std::unique_ptr<NetClient> connect(const NetAddress& host);

	~NetClient() override;

	/**
	 * Returns the ip and port of the remote host we are connected to.
	 * \param addr A pointer to a NetAddress structure to write the address to.
	 * \return Returns \c false when addr could not be filled in.
	 *  This should only happen when the client is not connected.
	 */
	bool get_remote_address(NetAddress* addr) const;

	// Inherited from NetClientInterface
	bool is_connected() const override;
	void close() override;
	std::unique_ptr<RecvPacket> try_receive() override;
	void send(const SendPacket& packet) override;

private:
	/**
	 * Tries to establish a connection to the given host.
	 * If the connection attempt failed, is_connected() will return \c false.
	 * \param host The host to connect to.
	 */
	explicit NetClient(const NetAddress& host);

	/// An io_service needed by boost.asio. Primarily needed for asynchronous operations.
	boost::asio::io_service io_service_;

	/// The socket that connects us to the host.
	boost::asio::ip::tcp::socket socket_;

	/// Deserializer acts as a buffer for packets (splitting stream to packets)
	Deserializer deserializer_;
};

#endif  // end of include guard: WL_NETWORK_NETCLIENT_H
