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
	if (is_connected())
		close();
}

bool NetClient::is_connected() const {
	return socket_.is_open();
}

void NetClient::close() {
	if (!is_connected())
		return;
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

bool NetClient::try_receive(RecvPacket* packet) {
	if (!is_connected())
		return false;

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
		return false;
	}
	// Get one packet from the deserializer
	return deserializer_.write_packet(packet);
}

void NetClient::send(const SendPacket& packet) {
	if (!is_connected())
		return;

	boost::system::error_code ec;
#ifdef NDEBUG
	boost::asio::write(socket_, boost::asio::buffer(packet.get_data(), packet.get_size()), ec);
#else
	size_t written =
	   boost::asio::write(socket_, boost::asio::buffer(packet.get_data(), packet.get_size()), ec);
#endif

	// TODO(Notabilis): This one is an assertion of mine, I am not sure if it will hold
	// If it doesn't, set the socket to blocking before writing
	// If it does, remove this comment after build 20
	assert(ec != boost::asio::error::would_block);
	assert(written == packet.get_size() || ec);
	if (ec) {
		log("[NetClient] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
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
	}
}
