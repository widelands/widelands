#include "network/netclientproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetClientProxy> NetClientProxy::connect(const NetAddress& address, const std::string& name) {
	std::unique_ptr<NetClientProxy> ptr(new NetClientProxy(address, name));
	if (ptr->conn_->is_connected()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
}

NetClientProxy::~NetClientProxy() {
	close();
}


bool NetClientProxy::is_connected() const {
	return conn_->is_connected();
}

void NetClientProxy::close() {
	if (conn_->is_connected()) {
		conn_->close();
	}
}

bool NetClientProxy::try_receive(RecvPacket* packet) {
	receive_commands();

	// Now check whether there is data
	if (received_.empty())
		return false;

	*packet = std::move(received_.front());
	received_.pop();
	return true;
}

void NetClientProxy::send(const SendPacket& packet) {
	conn_->send(RelayCommand::kToHost);
	conn_->send(packet);
}

NetClientProxy::NetClientProxy(const NetAddress& address, const std::string& name)
	: conn_(NetRelayConnection::connect(address)) {

   	if (conn_ == nullptr || !conn_->is_connected()) {
		return;
   	}

   	conn_->send(RelayCommand::kHello);
   	conn_->send(kRelayProtocolVersion);
   	conn_->send(name);
   	conn_->send("client");

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

void NetClientProxy::receive_commands() {
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
		case RelayCommand::kFromHost:
			if (conn_->peek_recvpacket()) {
				conn_->receive(&cmd);
				RecvPacket packet;
				conn_->receive(&packet);
                received_.push(std::move(packet));
			}
			break;
		default:
			// Other commands should not be possible.
			// Then is either something wrong with the protocol or there is an implementation mistake
			NEVER_HERE();
	}
}
