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

	std::lock_guard<std::mutex> lock(mutex_);
	if (packets_.empty()) {
		return std::unique_ptr<RecvPacket>();
	}
	std::unique_ptr<RecvPacket> packet = std::move(packets_.front());
	packets_.pop();
	return packet;
}

void NetClient::send(const SendPacket& packet, NetPriority priority) {
	std::lock_guard<std::mutex> lock(mutex_);
	if (!is_connected()) {
		return;
	}
	conn_->send(priority, packet);
}

NetClient::NetClient(const NetAddress& host) {

	const boost::asio::ip::tcp::endpoint destination(host.ip, host.port);

	log_info("[NetClient]: Trying to connect to %s:%u ... ", host.ip.to_string().c_str(), host.port);
	conn_ = BufferedConnection::connect(host);
	if (conn_) {
		log_info("success.\n");
		assert(is_connected());
		conn_->data_received.connect([this]() { handle_data(); });
		handle_data();
	} else {
		log_err("failed.\n");
		assert(!is_connected());
	}
}

void NetClient::handle_data() {
	assert(is_connected());
	std::lock_guard<std::mutex> lock(mutex_);

	// Try to get one packet from the connection
	while (BufferedConnection::Peeker(conn_.get()).recvpacket()) {
		// Enough data for a packet
		// Check whether it is a ping or pong
		BufferedConnection::Peeker peek(conn_.get());
		// Ignore first two bytes, they are the packet length
		peek.uint8_t();
		peek.uint8_t();
		uint8_t cmd;
		peek.uint8_t(&cmd);
		if (cmd == NETCMD_PING) {
			uint8_t seq;
			peek.uint8_t(&seq);
			// Send pong immediately
			SendPacket s;
			s.unsigned_8(NETCMD_PONG);
			s.unsigned_8(seq);
			mutex_.unlock();
			send(s, NetPriority::kPing);
			mutex_.lock();
		}
		// Get the packet and add it to the queue
		std::unique_ptr<RecvPacket> packet(new RecvPacket);
		conn_->receive(packet.get());
		packets_.push(std::move(packet));
	}
}
