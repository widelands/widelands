#include "network/nethostproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetHostProxy> NetHostProxy::connect(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password) {
	std::unique_ptr<NetHostProxy> ptr(new NetHostProxy(addresses, name, password));
	if (ptr->conn_ != nullptr && ptr->conn_->is_connected()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
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
/*
bool NetHostProxy::is_connected() const {
	return conn_->is_connected();
}*/

bool NetHostProxy::is_connected(const ConnectionId id) const {
	return clients_.count(id) > 0 && clients_.at(id).state_ == Client::State::kConnected;
}

void NetHostProxy::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
    conn_->send(RelayCommand::kDisconnectClient);
    conn_->send(id);
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

bool NetHostProxy::try_receive(const ConnectionId id, RecvPacket* packet) {
	receive_commands();

	// Check whether client is or was connected
	if (clients_.count(id) == 0 || clients_.at(id).state_ == Client::State::kConnecting)
		return false;

	std::queue<RecvPacket>& packet_list = clients_.at(id).received_;

	// Now check whether there is data for the requested client
	if (packet_list.empty()) {
		// If the client is already disconnected it should not be in the map anymore
		assert(clients_.at(id).state_ == Client::State::kConnected);
		return false;
	}

	*packet = std::move(packet_list.front());
	packet_list.pop();
	if (packet_list.empty() && clients_.at(id).state_ == Client::State::kDisconnected) {
		// If the receive bufffer is empty now, remove client
		clients_.erase(id);
	}
	return true;
}

void NetHostProxy::send(const ConnectionId id, const SendPacket& packet) {
	//printf("NetHostProxy::send() to %i\n", id);
	std::vector<ConnectionId> vec;
	vec.push_back(id);
	send(vec, packet);
}

void NetHostProxy::send(const std::vector<ConnectionId>& ids, const SendPacket& packet) {
	if (ids.empty()) {
		return;
	}

	conn_->send(RelayCommand::kToClients);
	for (ConnectionId id : ids) {
		//printf("NetHostProxy::send({}) to %i\n", id);
		assert(is_connected(id));
		conn_->send(id);
	}
	conn_->send(0);
	conn_->send(packet);
}

NetHostProxy::NetHostProxy(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password)
	: conn_(NetRelayConnection::connect(addresses.first)) {

	if ((conn_ == nullptr || !conn_->is_connected()) && addresses.second.is_valid()) {
		conn_ = NetRelayConnection::connect(addresses.second);
	}

   	if (conn_ == nullptr || !conn_->is_connected()) {
		return;
   	}

   	conn_->send(RelayCommand::kHello);
   	conn_->send(kRelayProtocolVersion);
   	conn_->send(name);
   	conn_->send(password);
   	conn_->send(password);

   	// Wait for answer
	// TODO(Notabilis): Don't like it.
   	while (!conn_->peek_cmd())
		printf("while (!conn_->peek_cmd())\n")
		;

	RelayCommand cmd;
	conn_->receive(&cmd);

   	if (cmd != RelayCommand::kWelcome) {
		conn_->close();
		conn_.reset();
		return;
   	}

   	// Check version
   	while (!conn_->peek_uint8_t())
		printf("while (!conn_->peek_cmd())\n")
		;
	uint8_t relay_proto_version;
	conn_->receive(&relay_proto_version);
   	if (relay_proto_version != kRelayProtocolVersion) {
		conn_->close();
		conn_.reset();
		return;
   	}

   	// Check game name
   	while (!conn_->peek_string())
		printf("while (!conn_->peek_string())\n")
		;
	std::string game_name;
	conn_->receive(&game_name);
   	if (game_name != name) {
		conn_->close();
		conn_.reset();
		return;
   	}
}

void NetHostProxy::receive_commands() {
	if (!conn_->is_connected()) {
		return;
	}

	// Receive all available commands
	RelayCommand cmd;
	conn_->peek_reset();
	if (!conn_->peek_cmd(&cmd)) {
		// No command to receive
		return;
	}
	switch (cmd) {
		case RelayCommand::kDisconnect:
			if (conn_->peek_string()) {
				// Command is completely in the buffer, handle it
				conn_->receive(&cmd);
				std::string reason;
				conn_->receive(&reason);
				// TODO(Notabilis): Handle the reason for the disconnect
				conn_->close();
			}
			break;
		case RelayCommand::kConnectClient:
			if (conn_->peek_uint8_t()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				// Kind of hacky, but create a new Client object inplace.
				// insert() and emplace() do not work since they call the (deleted) copy constructor
				// (operator[] returns the object when it exists, otherwise a new one is created)
				assert(clients_.count(id) == 0);
                clients_[id];
                assert(clients_.count(id) == 1);
			}
			break;
		case RelayCommand::kDisconnectClient:
			if (conn_->peek_uint8_t()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				assert(clients_.count(id));
				clients_.at(id).state_ = Client::State::kDisconnected;
			}
			break;
		case RelayCommand::kFromClient:
			if (conn_->peek_uint8_t() && conn_->peek_recvpacket()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				RecvPacket packet;
				conn_->receive(&packet);
				assert(clients_.count(id));
				clients_.at(id).received_.push(std::move(packet));
			}
			break;
		default:
			// Other commands should not be possible.
			// Then is either something wrong with the protocol or there is an implementation mistake
			log("Received command code %i from relay server, do not know what to do with it\n",
					static_cast<uint8_t>(cmd));
			NEVER_HERE();
	}
}
