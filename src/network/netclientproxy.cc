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

void NetClientProxy::send(const SendPacket& packet, NetPriority priority) {

	conn_->send(priority, RelayCommand::kToHost, packet);
}

NetClientProxy::NetClientProxy(const NetAddress& address, const std::string& name)
   : conn_(BufferedConnection::connect(address)) {
	if (conn_ == nullptr || !conn_->is_connected()) {
		return;
	}

	conn_->send(NetPriority::kNormal, RelayCommand::kHello, kRelayProtocolVersion, name, "client");

	// Wait 10 seconds for an answer
	const time_t endtime = time(nullptr) + 10;
	RelayCommand cmd;
	for (;;) {
		if (time(nullptr) > endtime) {
			log_err("[NetClientProxy] Handshaking error (1): No welcome from relay server in time\n");
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
				log_err("[NetClientProxy] Handshaking error (2): Relay server uses protocol "
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
				log_err("[NetClientProxy] Handshaking error (3): Relay wants to connect us to "
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
			log_err("[NetClientProxy] Handshaking error (4): Received command code %i from "
			        "relay server instead of welcome (%i)\n",
			        static_cast<uint8_t>(cmd), static_cast<uint8_t>(RelayCommand::kWelcome));
			conn_->close();
			conn_.reset();
			return;
		}
	}  // End for (;;) waiting for kWelcome

	log_info("[NetClientProxy] Handshaking with relay server done\n");
}

void NetClientProxy::receive_commands() {
	if (!conn_->is_connected()) {
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
		// TODO(Notabilis): Move the ping logic inside the receive-thread of BufferedConnection?
		// Should speed up the measured RTT and could prevent disconnects while, e.g., loading a map.
		// Would break the separation of the network layers, though.
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
		log_err("Received command code %i from relay server, do not know what to do with it\n",
		        static_cast<uint8_t>(cmd));
		NEVER_HERE();
	}
}
