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
	time_t endtime = time(nullptr) + 10;
	while (!BufferedConnection::Peeker(conn_.get()).cmd()) {
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
	while (!BufferedConnection::Peeker(conn_.get()).uint8_t()) {
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
	while (!BufferedConnection::Peeker(conn_.get()).string()) {
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
