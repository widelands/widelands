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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "network/nethostproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetHostProxy>
NetHostProxy::connect(const std::pair<NetAddress, NetAddress>& addresses,
                      const std::string& name,
                      const std::string& password) {
	std::unique_ptr<NetHostProxy> ptr(new NetHostProxy(addresses, name, password));
	if (ptr->conn_ == nullptr || !ptr->conn_->is_connected()) {
		ptr.reset();
	}
	return ptr;
}

NetHostProxy::~NetHostProxy() {
	if (conn_ && conn_->is_connected()) {
		while (!clients_.empty()) {
			close(clients_.begin()->first);
			clients_.erase(clients_.begin());
		}
		conn_->close();
	}
}

bool NetHostProxy::is_connected(const ConnectionId id) const {
	return clients_.count(id) > 0 && clients_.at(id).state_ == Client::State::kConnected;
}

void NetHostProxy::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
	conn_->send(NetPriority::kNormal, RelayCommand::kDisconnectClient, id);
	if (iter_client->second.received_.empty()) {
		// No pending messages, remove the client
		clients_.erase(iter_client);
	} else {
		// Still messages pending. Keep the structure so the host can receive them
		iter_client->second.state_ = Client::State::kDisconnected;
	}
}

bool NetHostProxy::try_accept(ConnectionId* new_id) {
	// Always read all available data into buffers
	receive_commands();

	for (auto& entry : clients_) {
		if (entry.second.state_ == Client::State::kConnecting) {
			*new_id = entry.first;
			entry.second.state_ = Client::State::kConnected;
			return true;
		}
	}
	return false;
}

std::unique_ptr<RecvPacket> NetHostProxy::try_receive(const ConnectionId id) {
	receive_commands();

	// Check whether client is not (yet) connected
	if (clients_.count(id) == 0 || clients_.at(id).state_ == Client::State::kConnecting) {
		return std::unique_ptr<RecvPacket>();
	}

	std::queue<std::unique_ptr<RecvPacket>>& packet_list = clients_.at(id).received_;

	// Now check whether there is data for the requested client
	if (packet_list.empty()) {
		// If the client is already disconnected it should not be in the map anymore
		assert(clients_.at(id).state_ == Client::State::kConnected);
		return std::unique_ptr<RecvPacket>();
	}

	std::unique_ptr<RecvPacket> packet = std::move(packet_list.front());
	packet_list.pop();
	if (packet_list.empty() && clients_.at(id).state_ == Client::State::kDisconnected) {
		// If the receive buffer is empty now, remove client
		clients_.erase(id);
	}
	return packet;
}

void NetHostProxy::send(const ConnectionId id, const SendPacket& packet, NetPriority priority) {
	std::vector<ConnectionId> vec;
	vec.push_back(id);
	send(vec, packet, priority);
}

void NetHostProxy::send(const std::vector<ConnectionId>& ids,
                        const SendPacket& packet,
                        NetPriority priority) {
	if (ids.empty()) {
		return;
	}

	receive_commands();

	std::vector<uint8_t> active_ids;
	for (ConnectionId id : ids) {
		if (is_connected(id)) {
			// This should be but is not always the case. It can happen that we receive a client
			// disconnect
			// on receive_commands() above and the GameHost did not have the chance to react to it yet.
			active_ids.push_back(id);
		}
	}
	if (active_ids.empty()) {
		// Oops, no clients left to send to
		return;
	}

	active_ids.push_back(0);
	conn_->send(priority, RelayCommand::kToClients, active_ids, packet);
}

NetHostProxy::NetHostProxy(const std::pair<NetAddress, NetAddress>& addresses,
                           const std::string& name,
                           const std::string& password)
   : conn_(BufferedConnection::connect(addresses.first)) {

	if ((conn_ == nullptr || !conn_->is_connected()) && addresses.second.is_valid()) {
		conn_ = BufferedConnection::connect(addresses.second);
	}

	if (conn_ == nullptr || !conn_->is_connected()) {
		return;
	}

	conn_->send(NetPriority::kNormal, RelayCommand::kHello, kRelayProtocolVersion, name, password);

	// Wait 10 seconds for an answer
	time_t endtime = time(nullptr) + 10;
	while (!BufferedConnection::Peeker(conn_.get()).cmd()) {
		if (time(nullptr) > endtime) {
			log_err("[NetHostProxy] Handshaking error (1): No message from relay server in time\n");
			conn_->close();
			conn_.reset();
			return;
		}
	}

	RelayCommand cmd;
	conn_->receive(&cmd);

	if (cmd != RelayCommand::kWelcome) {
		log_err("[NetHostProxy] Handshaking error (2): Received command code %i from relay server "
		        "instead of Welcome (%i)\n",
		        static_cast<uint8_t>(cmd), static_cast<uint8_t>(RelayCommand::kWelcome));
		conn_->close();
		conn_.reset();
		return;
	}

	// Check version
	endtime = time(nullptr) + 10;
	while (!BufferedConnection::Peeker(conn_.get()).uint8_t()) {
		if (time(nullptr) > endtime) {
			log_err("[NetHostProxy] Handshaking error (3): No message from relay server in time\n");
			conn_->close();
			conn_.reset();
			return;
		}
	}
	uint8_t relay_proto_version;
	conn_->receive(&relay_proto_version);
	if (relay_proto_version != kRelayProtocolVersion) {
		log_err(
		   "[NetHostProxy] Handshaking error (4): Relay server uses protocol version %i instead of "
		   "our version %i\n",
		   static_cast<uint8_t>(relay_proto_version), static_cast<uint8_t>(kRelayProtocolVersion));
		conn_->close();
		conn_.reset();
		return;
	}

	// Check game name
	endtime = time(nullptr) + 10;
	while (!BufferedConnection::Peeker(conn_.get()).string()) {
		if (time(nullptr) > endtime) {
			log_err("[NetHostProxy] Handshaking error (5): No message from relay server in time\n");
			conn_->close();
			conn_.reset();
			return;
		}
	}
	std::string game_name;
	conn_->receive(&game_name);
	if (game_name != name) {
		log_err(
		   "[NetHostProxy] Handshaking error (6): Relay wants to connect us to game '%s' instead of "
		   "our game '%s'\n",
		   game_name.c_str(), name.c_str());
		conn_->close();
		conn_.reset();
		return;
	}
	verb_log_info("[NetHostProxy] Handshaking with relay server done");
}

void NetHostProxy::receive_commands() {
	if (!conn_->is_connected()) {
		// Seems the connection broke at some time. Set all clients to disconnected
		for (auto iter_client = clients_.begin(); iter_client != clients_.end();) {
			if (iter_client->second.received_.empty()) {
				// No pending messages, remove the client
				clients_.erase(iter_client++);
			} else {
				// Still messages pending. Keep the structure so the host can receive them
				iter_client->second.state_ = Client::State::kDisconnected;
				++iter_client;
			}
		}
		return;
	}

	// Receive all available commands
	RelayCommand cmd;
	BufferedConnection::Peeker peek(conn_.get());
	if (!peek.cmd(&cmd)) {
		// No command to receive
		return;
	}
	switch (cmd) {
	case RelayCommand::kDisconnect:
		if (peek.string()) {
			// Command is completely in the buffer, handle it
			conn_->receive(&cmd);
			std::string reason;
			conn_->receive(&reason);
			conn_->close();
			// Set all clients to offline
			for (auto& entry : clients_) {
				entry.second.state_ = Client::State::kDisconnected;
			}
		}
		break;
	case RelayCommand::kConnectClient:
		if (peek.uint8_t()) {
			conn_->receive(&cmd);
			uint8_t id;
			conn_->receive(&id);
#ifndef NDEBUG
			auto result = clients_.emplace(
			   std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
			assert(result.second);
#else
			clients_.emplace(
			   std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
#endif
		}
		break;
	case RelayCommand::kDisconnectClient:
		if (peek.uint8_t()) {
			conn_->receive(&cmd);
			uint8_t id;
			conn_->receive(&id);
			const auto client = clients_.find(id);
			if (client != clients_.end()) {
				// As of a race condition this may not always work
				client->second.state_ = Client::State::kDisconnected;
			}
		}
		break;
	case RelayCommand::kFromClient:
		if (peek.uint8_t() && peek.recvpacket()) {
			conn_->receive(&cmd);
			uint8_t id;
			conn_->receive(&id);
			std::unique_ptr<RecvPacket> packet(new RecvPacket);
			conn_->receive(packet.get());
			const auto client = clients_.find(id);
			if (client != clients_.end()) {
				client->second.received_.push(std::move(packet));
			} else {
				log_warn("[NetHostProxy] Received packed from unknown client");
			}
		}
		break;
	case RelayCommand::kPing:
		if (peek.uint8_t()) {
			conn_->receive(&cmd);
			uint8_t seq;
			conn_->receive(&seq);
			// Reply with a pong
			conn_->send(NetPriority::kPing, RelayCommand::kPong, seq);
		}
		break;
	case RelayCommand::kRoundTripTimeResponse:
		conn_->ignore_rtt_response();
		break;
	default:
		// Other commands should not be possible.
		// Then is either something wrong with the protocol or there is an implementation mistake
		log_err(
		   "[NetHostProxy] Received command code %i from relay server, do not know what to do with "
		   "it\n",
		   static_cast<uint8_t>(cmd));
		NEVER_HERE();
	}
}
