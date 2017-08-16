#include "network/nethostproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetHostProxy> NetHostProxy::connect(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password) {
	std::unique_ptr<NetHostProxy> ptr(new NetHostProxy(addresses, name, password));
	if (ptr->conn_->is_connected()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
}

NetHostProxy::~NetHostProxy() {
	if (conn_->is_connected()) {
		while (!received_.empty()) {
			close(received_.begin()->first);
		}
		conn_->close();
	}
}
/*
bool NetHostProxy::is_connected() const {
	return conn_->is_connected();
}*/

bool NetHostProxy::is_connected(const ConnectionId id) const {
	return received_.count(id) > 0;
}

void NetHostProxy::close(const ConnectionId id) {
	auto iter_client = received_.find(id);
	if (iter_client == received_.end()) {
		// Not connected anyway
		return;
	}
    conn_->send(RelayCommand::kDisconnectClient);
    conn_->send(id);
    received_.erase(iter_client);
}

bool NetHostProxy::try_accept(ConnectionId* new_id) {
	// Always read all available data into buffers
	receive_commands();

	if (accept_.empty()) {
		return false;
	}
	*new_id = accept_.front();
	accept_.pop();
	return true;
}

bool NetHostProxy::try_receive(const ConnectionId id, RecvPacket* packet) {
	receive_commands();

	// Now check whether there is data for the requested client
	if (!is_connected(id) || received_.at(id).empty())
		return false;

	std::queue<RecvPacket>& packet_list = received_.at(id);
	*packet = std::move(packet_list.front());
	packet_list.pop();
	return true;
}

void NetHostProxy::send(const ConnectionId id, const SendPacket& packet) {
	send({id}, packet);
}

void NetHostProxy::send(const std::vector<ConnectionId>& ids, const SendPacket& packet) {
	if (ids.empty()) {
		return;
	}

	conn_->send(RelayCommand::kToClients);
	for (ConnectionId id : ids) {
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

   	// Wait for answer
	// Don't like it.
   	while (!conn_->peek_cmd());

	RelayCommand cmd;
	conn_->receive(&cmd);

   	if (cmd != RelayCommand::kWelcome) {
		conn_->close();
		conn_.reset();
   	}

   	// Check version
   	while (!conn_->peek_uint8_t());
	uint8_t relay_proto_version;
	conn_->receive(&relay_proto_version);
   	if (relay_proto_version != kRelayProtocolVersion) {
		conn_->close();
		conn_.reset();
   	}

   	// Check game name
   	while (!conn_->peek_string());
	std::string game_name;
	conn_->receive(&game_name);
   	if (game_name != name) {
		conn_->close();
		conn_.reset();
   	}
}

void NetHostProxy::receive_commands() {
	if (!conn_->is_connected()) {
		return;
	}

	// Receive all available commands
	RelayCommand cmd;
	conn_->peek_reset();
	conn_->peek_cmd(&cmd);
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
                accept_.push(id);
			}
			break;
		case RelayCommand::kDisconnectClient:
			if (conn_->peek_uint8_t()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				assert(received_.count(id));
                received_.erase(id);
			}
			break;
		case RelayCommand::kFromClient:
			if (conn_->peek_uint8_t() && conn_->peek_recvpacket()) {
				conn_->receive(&cmd);
				uint8_t id;
				conn_->receive(&id);
				RecvPacket packet;
				conn_->receive(&packet);
				assert(received_.count(id));
                received_.at(id).push(std::move(packet));
			}
			break;
		default:
			// Other commands should not be possible.
			// Then is either something wrong with the protocol or there is an implementation mistake
			NEVER_HERE();
	}
}
