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

bool NetClient::get_remote_address(NetAddress* addr) const {
	if (!is_connected()) {
		return false;
	}
	boost::asio::ip::tcp::endpoint remote = socket_.remote_endpoint();
	addr->ip = remote.address();
	addr->port = remote.port();
	return true;
}

bool NetClient::is_connected() const {
	return socket_.is_open();
}

void NetClient::close() {
	if (!is_connected()) {
		return;
	}
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint remote = socket_.remote_endpoint(ec);
	if (!ec) {
		log("[NetClient] Closing network socket connected to %s:%i.\n",
		    remote.address().to_string().c_str(), remote.port());
	} else {
		log("[NetClient] Closing network socket.\n");
	}
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_.close(ec);
}

std::unique_ptr<RecvPacket> NetClient::try_receive() {
	if (is_connected()) {
		// If we are connected, try to receive some data

		uint8_t buffer[kNetworkBufferSize];
		boost::system::error_code ec;
		size_t length = socket_.read_some(boost::asio::buffer(buffer, kNetworkBufferSize), ec);
		if (!ec) {
			assert(length > 0);
			assert(length <= kNetworkBufferSize);
			// Has read something
			deserializer_.read_data(buffer, length);
		}

		if (ec && ec != boost::asio::error::would_block) {
			// Connection closed or some error, close the socket
			log("[NetClient] Error when trying to receive some data: %s.\n", ec.message().c_str());
			close();
		}
	}
	// Try to get one packet from the deserializer
	std::unique_ptr<RecvPacket> packet(new RecvPacket);
	if (deserializer_.write_packet(packet.get())) {
		return packet;
	} else {
		return std::unique_ptr<RecvPacket>();
	}
}

void NetClient::send(const SendPacket& packet) {
	if (!is_connected()) {
		return;
	}

	boost::system::error_code ec;
	size_t written =
	   boost::asio::write(socket_, boost::asio::buffer(packet.get_data(), packet.get_size()), ec);

	if (ec == boost::asio::error::would_block) {
		throw wexception("[NetClient] Socket connected to relay would block when writing");
	}
	if (ec) {
		log("[NetClient] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
		return;
	}
	if (written < packet.get_size()) {
		throw wexception("[NetClient] Unable to send complete packet to relay (only %" PRIuS
		                 " bytes of %" PRIuS ")",
		                 written, packet.get_size());
	}
}

NetClient::NetClient(const NetAddress& host)
   : io_service_(), socket_(io_service_), deserializer_() {

	assert(host.is_valid());
	const boost::asio::ip::tcp::endpoint destination(host.ip, host.port);

	log("[NetClient]: Trying to connect to %s:%u ... ", host.ip.to_string().c_str(), host.port);
	boost::system::error_code ec;
	socket_.connect(destination, ec);
	if (!ec && is_connected()) {
		log("success.\n");
		socket_.non_blocking(true);
	} else {
		log("failed.\n");
		socket_.close();
		assert(!is_connected());
	}
}
