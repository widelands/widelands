/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "network/nethost.h"

#include <memory>

#include "base/log.h"

namespace {

/**
 * Returns the IP version.
 * \param acceptor The acceptor socket to get the IP version for.
 * \return Either 4 or 6, depending on the version of the given acceptor.
 */
int get_ip_version(const asio::ip::tcp::acceptor& acceptor) {
	assert(acceptor.is_open());
	if (acceptor.local_endpoint().protocol() == asio::ip::tcp::v4()) {
		return 4;
	} else {
		return 6;
	}
}
}  // namespace

std::unique_ptr<NetHost> NetHost::listen(const uint16_t port) {
	std::unique_ptr<NetHost> ptr(new NetHost(port));
	if (ptr->is_listening()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
}

NetHost::~NetHost() {
	stop_listening();
	assert(!asio_thread_.joinable());
	while (!clients_.empty()) {
		close(clients_.begin()->first);
	}
}

bool NetHost::is_listening() const {
	return acceptor_v4_.is_open() || acceptor_v6_.is_open();
}

bool NetHost::is_connected(const ConnectionId id) const {
	assert(!clients_.count(id) || clients_.at(id));
	return clients_.count(id) > 0 && clients_.at(id)->is_connected();
}

void NetHost::stop_listening() {

	// Stop the thread
	io_service_.stop();
	for (int i = 0; i < 1000 && !io_service_.stopped(); i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	assert(io_service_.stopped());
	if (asio_thread_.joinable()) {
		try {
			asio_thread_.join();
		} catch (const std::invalid_argument&) {
			// Most likely the thread ended between joinable() and join()
		}
	}
	// The thread should be stopped now
	assert(!asio_thread_.joinable());

	static const auto do_stop = [](asio::ip::tcp::acceptor& acceptor) {
		std::error_code ec;
		if (acceptor.is_open()) {
			verb_log_info("[NetHost] Closing a listening IPv%d socket.", get_ip_version(acceptor));
			acceptor.close(ec);
		}
		// Ignore errors
	};

	do_stop(acceptor_v4_);
	do_stop(acceptor_v6_);
}

void NetHost::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
	iter_client->second->close();
	clients_.erase(iter_client);
}

bool NetHost::try_accept(ConnectionId* new_id) {

	std::lock_guard<std::mutex> lock(mutex_accept_);
	if (accept_queue_.empty()) {
		return false;
	}

	ConnectionId id = next_id_++;
	assert(id > 0);
	assert(clients_.count(id) == 0);
	clients_.insert(std::make_pair(id, std::move(accept_queue_.front())));
	accept_queue_.pop();
	assert(clients_.count(id) == 1);
	*new_id = id;
	return true;
}

std::unique_ptr<RecvPacket> NetHost::try_receive(const ConnectionId id) {

	// Check whether there is data for the requested client
	if (!is_connected(id)) {
		return std::unique_ptr<RecvPacket>();
	}

	// Try to get one packet from the connection
	if (!BufferedConnection::Peeker(clients_.at(id).get()).recvpacket()) {
		return std::unique_ptr<RecvPacket>();
	}
	std::unique_ptr<RecvPacket> packet(new RecvPacket);
	clients_.at(id)->receive(packet.get());
	return packet;
}

void NetHost::send(const ConnectionId id, const SendPacket& packet, NetPriority priority) {
	if (!is_connected(id)) {
		return;
	}
	clients_.at(id)->send(priority, packet);
}

void NetHost::send(const std::vector<ConnectionId>& ids, const SendPacket& packet, NetPriority) {
	for (ConnectionId id : ids) {
		send(id, packet);
	}
}

// This method is run within a thread
void NetHost::start_accepting(
   asio::ip::tcp::acceptor& acceptor,
   std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*>& pair) {

	if (!is_listening()) {
		return;
	}

	if (!pair.first) {
		assert(pair.second == nullptr);
		pair = BufferedConnection::create_unconnected();
	}

	acceptor.async_accept(*(pair.second), [this, &acceptor, &pair](const std::error_code& ec) {
		if (!ec) {
			// No error occurred, so we have establish a (TCP) connection.
			// We can't say whether it is valid Widelands client yet
			pair.first->notify_connected();
			assert(pair.first->is_connected());
			std::lock_guard<std::mutex> lock(mutex_accept_);
			accept_queue_.push(std::move(pair.first));
			// pair.first is cleared by the std::move
			pair.second = nullptr;
		}
		// Wait for the next client
		start_accepting(acceptor, pair);
	});
}

NetHost::NetHost(const uint16_t port)
   : next_id_(1), acceptor_v4_(io_service_), acceptor_v6_(io_service_) {

	if (open_acceptor(&acceptor_v4_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))) {
		verb_log_info("[NetHost] Opening a listening IPv4 socket on TCP port %u", port);
	}
	if (open_acceptor(&acceptor_v6_, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port))) {
		verb_log_info("[NetHost] Opening a listening IPv6 socket on TCP port %u", port);
	}

	verb_log_info("[NetHost] Starting to listen for network connections");
	start_accepting(acceptor_v4_, accept_pair_v4_);
	start_accepting(acceptor_v6_, accept_pair_v6_);

	asio_thread_ = std::thread([this]() {
		verb_log_info("[NetHost] Starting networking thread");
		io_service_.run();
		verb_log_info("[NetHost] Stopping networking thread");
	});
}

bool NetHost::open_acceptor(asio::ip::tcp::acceptor* acceptor,
                            const asio::ip::tcp::endpoint& endpoint) {
	try {
		acceptor->open(endpoint.protocol());
		const asio::socket_base::reuse_address option_reuse(true);
		acceptor->set_option(option_reuse);
		if (endpoint.protocol() == asio::ip::tcp::v6()) {
			const asio::ip::v6_only option_v6only(true);
			acceptor->set_option(option_v6only);
		}
		acceptor->bind(endpoint);
		acceptor->listen(asio::socket_base::max_connections);
		return true;
	} catch (const std::system_error&) {
		return false;
	}
}
