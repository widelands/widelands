#include "network/netclientproxy.h"

#include <memory>

#include "base/log.h"
#include "network/relay_protocol.h"

std::unique_ptr<NetClientProxy> NetClientProxy::connect(const NetAddress& address,
                                                        const std::string& name) {
	std::unique_ptr<NetClientProxy> ptr(new NetClientProxy(address, name));
	if (ptr->conn_ == nullptr || !ptr->conn_->is_connected()) {
		ptr.reset();
	}
	return ptr;
}

NetClientProxy::~NetClientProxy() {
	close();
}

bool NetClientProxy::is_connected() const {
	return conn_ && conn_->is_connected();
}

void NetClientProxy::close() {
	if (conn_ && conn_->is_connected()) {
		conn_->close();
	}
}

std::unique_ptr<RecvPacket> NetClientProxy::try_receive() {
	receive_commands();

	// Now check whether there is data
	if (received_.empty()) {
		return std::unique_ptr<RecvPacket>();
	}

	std::unique_ptr<RecvPacket> packet = std::move(received_.front());
	received_.pop();
	return packet;
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

	// Wait 10 seconds for an answer
	time_t endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_.get()).cmd()) {
		if (time(nullptr) > endtime) {
			// No message received in time
			conn_->close();
			conn_.reset();
			return;
		}
	}

	RelayCommand cmd;
	conn_->receive(&cmd);

	if (cmd != RelayCommand::kWelcome) {
		conn_->close();
		conn_.reset();
		return;
	}

	// Check version
	endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_.get()).uint8_t()) {
		if (time(nullptr) > endtime) {
			conn_->close();
			conn_.reset();
			return;
		}
	}
	uint8_t relay_proto_version;
	conn_->receive(&relay_proto_version);
	if (relay_proto_version != kRelayProtocolVersion) {
		conn_->close();
		conn_.reset();
	}

	// Check game name
	endtime = time(nullptr) + 10;
	while (!NetRelayConnection::Peeker(conn_.get()).string()) {
		if (time(nullptr) > endtime) {
			conn_->close();
			conn_.reset();
			return;
		}
	}
	std::string game_name;
	conn_->receive(&game_name);
	if (game_name != name) {
		conn_->close();
		conn_.reset();
		return;
	}
}

void NetClientProxy::receive_commands() {
	if (!conn_->is_connected()) {
		return;
	}

	// Receive all available commands
	RelayCommand cmd;
	NetRelayConnection::Peeker peek(conn_.get());
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
			// TODO(Notabilis): Handle the reason for the disconnect
			conn_->close();
		}
		break;
	case RelayCommand::kFromHost:
		if (peek.recvpacket()) {
			conn_->receive(&cmd);
			std::unique_ptr<RecvPacket> packet(new RecvPacket);
			conn_->receive(packet.get());
			received_.push(std::move(packet));
		}
		break;
	case RelayCommand::kPing:
		if (peek.uint8_t()) {
			conn_->receive(&cmd);
			uint8_t seq;
			conn_->receive(&seq);
			// Reply with a pong
			conn_->send(RelayCommand::kPong);
			conn_->send(seq);
		}
		break;
	case RelayCommand::kRoundTripTimeResponse:
		conn_->ignore_rtt_response();
		break;
	default:
		// Other commands should not be possible.
		// Then is either something wrong with the protocol or there is an implementation mistake
		log("Received command code %i from relay server, do not know what to do with it\n",
		    static_cast<uint8_t>(cmd));
		NEVER_HERE();
	}
}
