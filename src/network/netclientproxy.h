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

#ifndef WL_NETWORK_NETCLIENTPROXY_H
#define WL_NETWORK_NETCLIENTPROXY_H

#include <memory>

#include "network/bufferedconnection.h"
#include "network/netclient_interface.h"

/**
 * Represents a client in-game, but talks through the 'wlnr' relay binary.
 */
class NetClientProxy : public NetClientInterface {
public:
	/**
	 * Tries to connect to the relay at the given address.
	 * \param address The address to connect to.
	 * \param name The name of the game.
	 * \return A pointer to a ready \c NetClientProxy object or a nullptr if the connection failed.
	 */
	static std::unique_ptr<NetClientProxy> connect(const NetAddress& address,
	                                               const std::string& name);

	/**
	 * Closes the server.
	 */
	~NetClientProxy() override;

	// Inherited from NetClientInterface
	bool is_connected() const override;
	void close() override;
	std::unique_ptr<RecvPacket> try_receive() override;
	void send(const SendPacket& packet, NetPriority priority = NetPriority::kNormal) override;

private:
	/**
	 * Tries to connect to the relay at the given address.
	 * If it fails, is_connected() will return \c false.
	 * \param address The address to connect to.
	 * \param name The name of the game.
	 */
	NetClientProxy(const NetAddress& address, const std::string& name);

	void receive_commands();

	std::unique_ptr<BufferedConnection> conn_;

	/// For each connected client, the packages that have been received from him.
	std::queue<std::unique_ptr<RecvPacket>> received_;
};

#endif  // end of include guard: WL_NETWORK_NETCLIENTPROXY_H
