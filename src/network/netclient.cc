#include "network/netclient.h"

#include <memory>

#include "base/log.h"

std::unique_ptr<NetClient> NetClient::connect(const NetAddress& host) {

	std::unique_ptr<NetClient> ptr(new NetClient(host));
	if (ptr->is_connected()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
}

NetClient::~NetClient() {
	if (is_connected()) {
		close();
	}
}

bool NetClient::is_connected() const {
	return conn_ != nullptr && conn_->is_connected();
}

void NetClient::close() {
	assert(conn_);
	conn_->close();
}

std::unique_ptr<RecvPacket> NetClient::try_receive() {
	if (!BufferedConnection::Peeker(conn_.get()).recvpacket()) {
		return std::unique_ptr<RecvPacket>();
	}
	std::unique_ptr<RecvPacket> packet(new RecvPacket);
	conn_->receive(packet.get());
	return packet;
}

void NetClient::send(const SendPacket& packet, NetPriority priority) {
	if (!is_connected()) {
		return;
	}
	conn_->send(priority, packet);
}

NetClient::NetClient(const NetAddress& host) {

	const asio::ip::tcp::endpoint destination(host.ip, host.port);

	verb_log_info(
	   "[NetClient]: Trying to connect to %s:%u ... ", host.ip.to_string().c_str(), host.port);
	conn_ = BufferedConnection::connect(host);
	if (conn_) {
		verb_log_info("success.");
		assert(is_connected());
	} else {
		log_err(
		   "[NetClient]: Trying to connect to %s:%u failed!", host.ip.to_string().c_str(), host.port);
		assert(!is_connected());
	}
}
