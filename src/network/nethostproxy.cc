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
	std::lock_guard<std::mutex> lock(mutex_);
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
	std::lock_guard<std::mutex> lock(mutex_);

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
	std::lock_guard<std::mutex> lock(mutex_);

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
	std::lock_guard<std::mutex> lock(mutex_);
	if (ids.empty()) {
		return;
	}

	std::vector<uint8_t> active_ids;
	for (ConnectionId id : ids) {
		if (is_connected(id)) {
			// This should be but is not always the case. It can happen that we receive
			// a client disconnect on receive_commands() and the GameHost did not
			// have the chance to react to it yet.
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

void NetHostProxy::request_rtt_update() {
	std::lock_guard<std::mutex> lock(mutex_);
	conn_->send(NetPriority::kNormal, RelayCommand::kRoundTripTimeRequest);
}

uint8_t NetHostProxy::get_client_rtt(ConnectionId id) {
	std::lock_guard<std::mutex> lock(mutex_);
	return pings_.get_rtt(id);
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
	const time_t endtime = time(nullptr) + 10;
	RelayCommand cmd;
	for (;;) {
		if (time(nullptr) > endtime) {
			log_err("[NetHostProxy] Handshaking error (1): No welcome from relay server in time\n");
			conn_->close();
			conn_.reset();
			return;
		}
		// Peeker has to be created inside the loop to reset it to the beginning of the input buffer
		BufferedConnection::Peeker peek(conn_.get());
		if (!(peek.cmd(&cmd) && peek.uint8_t())) {
			// Not enough received yet. Do a "mini sleep" and try again
			std::this_thread::yield();
			continue;
		}
		if (cmd == RelayCommand::kPing) {
			// Got a ping before the welcome. Handle it
			conn_->receive(&cmd);
			uint8_t seq;
			conn_->receive(&seq);
			// Reply with a pong
			conn_->send(NetPriority::kPing, RelayCommand::kPong, seq);
		} else if (cmd == RelayCommand::kWelcome) {
			// It is the expected welcome message. Check whether it is complete
			if (!peek.string()) {
				// Command not complete yet
				std::this_thread::yield();
				continue;
			}
			// It is complete! Handle it
			// Receive the command code for real (that is, remove it from the buffer)
			conn_->receive(&cmd);
			// Check version
			uint8_t relay_proto_version;
			conn_->receive(&relay_proto_version);
			if (relay_proto_version != kRelayProtocolVersion) {
				log_err("[NetHostProxy] Handshaking error (2): Relay server uses protocol "
				        "version %i instead of our version %i\n",
				        static_cast<uint8_t>(relay_proto_version),
				        static_cast<uint8_t>(kRelayProtocolVersion));
				conn_->close();
				conn_.reset();
				return;
			}
			// Check game name
			std::string game_name;
			conn_->receive(&game_name);
			if (game_name != name) {
				log_err("[NetHostProxy] Handshaking error (3): Relay wants to connect us to "
				        "game '%s' instead of our game '%s'\n",
				        game_name.c_str(), name.c_str());
				conn_->close();
				conn_.reset();
				return;
			}
			// Everything is fine, leave the loop
			break;
		} else {
			// Unexpected command. Maybe it is no relay server?
			log_err("[NetHostProxy] Handshaking error (4): Received command code %i from "
			        "relay server instead of welcome (%i)\n",
			        static_cast<uint8_t>(cmd), static_cast<uint8_t>(RelayCommand::kWelcome));
			conn_->close();
			conn_.reset();
			return;
		}
	}  // End for (;;) waiting for kWelcome

	conn_->data_received.connect([this]() { receive_commands(); });
	receive_commands();

	log_info("[NetHostProxy] Handshaking with relay server done\n");
}

void NetHostProxy::receive_commands() {
	std::lock_guard<std::mutex> lock(mutex_);
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
	for (;;) {
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
				break;
			} else {
				// It will be a kDisconnect, but it isn't complete yet. Abort for now.
				return;
			}
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
				break;
			} else {
				return;
			}
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
				break;
			} else {
				return;
			}
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
					log_warn("[NetHostProxy] Received packet from unknown client");
				}
				break;
			} else {
				return;
			}
		case RelayCommand::kPing:
			if (peek.uint8_t()) {
				conn_->receive(&cmd);
				uint8_t seq;
				conn_->receive(&seq);
				// Reply with a pong
				conn_->send(NetPriority::kPing, RelayCommand::kPong, seq);
				break;
			} else {
				return;
			}
		case RelayCommand::kRoundTripTimeResponse:
			uint8_t length_list;
			// Check if enough data is available
			if (peek.uint8_t(&length_list)) {
				for (uint8_t i = 0; i < length_list * 3; i++) {
					if (!peek.uint8_t()) {
						return;
					}
				}
			} else {
				return;
			}
			// Enough data is there, get it
			uint8_t id;
			uint8_t rtt;
			uint8_t last;
			conn_->receive(&cmd);
			conn_->receive(&length_list);
			for (uint8_t i = 0; i < length_list; i++) {
				conn_->receive(&id);
				conn_->receive(&rtt);
				conn_->receive(&last);
				// We are fetching the data once per second. So if the time of the last
				// received ping is greater than one second, ignore the old rtt result and
				// add a "missed ping" rtt of 255
				if (last >= 1) {
					pings_.register_rtt(id, 255);
				} else {
					pings_.register_rtt(id, rtt);
				}
			}
			break;
		default:
			// Other commands should not be possible.
			// Then is either something wrong with the protocol or there is an implementation mistake
			log_err("[NetHostProxy] Received command code %i from relay server, "
			        " do not know what to do with it\n",
			        static_cast<uint8_t>(cmd));
			NEVER_HERE();
		}
	}
}
