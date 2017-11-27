#include "network/netrelayconnection.h"

#include <memory>

#include "base/log.h"

// Not so great: Quite some duplicated code between this class and NetClient.

std::unique_ptr<NetRelayConnection> NetRelayConnection::connect(const NetAddress& host) {
	std::unique_ptr<NetRelayConnection> ptr(new NetRelayConnection(host));
	if (!ptr->is_connected()) {
		ptr.reset();
	}
	return ptr;
}

NetRelayConnection::~NetRelayConnection() {
	if (is_connected()) {
		close();
	}
}

bool NetRelayConnection::is_connected() const {
	return socket_.is_open();
}

void NetRelayConnection::close() {
	if (!is_connected()) {
		return;
	}
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint remote = socket_.remote_endpoint(ec);
	if (!ec) {
		log("[NetRelayConnection] Closing network socket connected to %s:%i.\n",
		    remote.address().to_string().c_str(), remote.port());
	} else {
		log("[NetRelayConnection] Closing network socket.\n");
	}
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_.close(ec);
}

bool NetRelayConnection::peek_string() {
	try_network_receive();

	if (buffer_.size() < peek_pointer_ + 1) {
		return false;
	}

	// A string goes until the next \0 and might have a length of 0
	for (size_t i = peek_pointer_; i < buffer_.size(); ++i) {
		if (buffer_[i] == '\0') {
			peek_pointer_ = i + 1;
			return true;
		}
	}
	return false;
}

void NetRelayConnection::receive(std::string *str) {
	// Try to receive something from the network.
	try_network_receive();

	// Reset the peek pointer
	peek_reset();

	#ifndef NDEBUG
	// Check if we can read a complete string
	assert(peek_string());
	peek_reset();
	#endif // NDEBUG

	// Read the string
	str->clear();
	// No range check needed, peek_string() takes care of that
	while (buffer_.front() != '\0') {
		str->push_back(buffer_.front());
		buffer_.pop_front();
	}
	// Pop the \0
	buffer_.pop_front();
}

bool NetRelayConnection::peek_cmd(RelayCommand *cmd) {
	try_network_receive();

	if (buffer_.size() > peek_pointer_) {
		if (cmd != nullptr) {
			*cmd = static_cast<RelayCommand>(buffer_[peek_pointer_]);
		}
		peek_pointer_++;
		return true;
	} else {
		return false;
	}
}

void NetRelayConnection::receive(RelayCommand *out) {
	try_network_receive();

	peek_reset();
	assert(!buffer_.empty());

	uint8_t i;
	receive(&i);
	*out = static_cast<RelayCommand>(i);
}

bool NetRelayConnection::peek_uint8_t() {
	try_network_receive();

	// If there is any byte available, we can read an uint8
	if (buffer_.size() > peek_pointer_) {
		peek_pointer_++;
		return true;
	} else {
		return false;
	}
}

void NetRelayConnection::receive(uint8_t *out) {
	try_network_receive();

	peek_reset();
	assert(!buffer_.empty());

	*out = buffer_.front();
	buffer_.pop_front();
}

bool NetRelayConnection::peek_recvpacket() {
	try_network_receive();

	if (buffer_.size() < peek_pointer_ + 2) {
		// Not even enough space for the size
		return false;
	}

	// RecvPackets have their size coded in their first two bytes
	const uint16_t size = buffer_[peek_pointer_ + 0] << 8 | buffer_[peek_pointer_ + 1];
	assert(size >= 2);

	if (buffer_.size() >= peek_pointer_ + size) {
		peek_pointer_ += size;
		return true;
	} else {
		return false;
	}
}

void NetRelayConnection::receive(RecvPacket *packet) {
	try_network_receive();

	peek_reset();

	#ifndef NDEBUG
	assert(peek_recvpacket());
	peek_reset();
	#endif // NDEBUG

	// Read the packet (adapted copy from Deserializer)
	const uint16_t size = buffer_[0] << 8 | buffer_[1];
	assert(size >= 2);
	assert(buffer_.size() >= size);

	packet->buffer.clear();
	packet->buffer.insert(packet->buffer.end(), buffer_.begin() + 2, buffer_.begin() + size);
	packet->index_ = 0;

	buffer_.erase(buffer_.begin(), buffer_.begin() + size);
}

bool NetRelayConnection::try_network_receive() {
	if (!is_connected()) {
		return false;
	}

	unsigned char buffer[kNetworkBufferSize];
	boost::system::error_code ec;
	size_t length = socket_.read_some(boost::asio::buffer(buffer, kNetworkBufferSize), ec);
	if (!ec) {
		assert(length > 0);
		assert(length <= kNetworkBufferSize);
		// Has read something
		for (size_t i = 0; i < length; ++i) {
			buffer_.push_back(buffer[i]);
		}
	}

	if (ec && ec != boost::asio::error::would_block) {
		// Connection closed or some error, close the socket
		log("[NetRelayConnection] Error when trying to receive some data: %s.\n", ec.message().c_str());
		close();
		return false;
	}
	return true;
}

void NetRelayConnection::peek_reset() {
	peek_pointer_ = 0;
}

void NetRelayConnection::send(const RelayCommand data) {
	send(static_cast<uint8_t>(data));
}

void NetRelayConnection::send(const uint8_t data) {
	if (!is_connected()) {
		return;
	}

	uint8_t buf[1];
	buf[0] = data;

	boost::system::error_code ec;
	size_t written = boost::asio::write(socket_, boost::asio::buffer(buf, 1), ec);

	if (ec == boost::asio::error::would_block) {
		throw wexception("[NetRelayConnection] Socket connected to relay would block when writing");
	}
	if (written < 1) {
		throw wexception("[NetRelayConnection] Unable to send byte to relay");
	}
	if (ec) {
		log("[NetRelayConnection] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
	}
}

void NetRelayConnection::send(const std::string& str) {
	if (!is_connected()) {
		return;
	}

	// Append \0
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(str));
	buffers.push_back(boost::asio::buffer("\0", 1));

	boost::system::error_code ec;
	size_t written = boost::asio::write(socket_, buffers, ec);

	if (ec == boost::asio::error::would_block) {
		throw wexception("[NetRelayConnection] Socket connected to relay would block when writing");
	}
	if (written < str.length() + 1) {
		throw wexception(
					"[NetRelayConnection] Unable to send complete string to relay (only %lu bytes of %lu)",
					written, str.length() + 1);
	}
	if (ec) {
		log("[NetRelayConnection] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
	}
}

void NetRelayConnection::send(const SendPacket& packet) {
	if (!is_connected()) {
		return;
	}

	boost::system::error_code ec;
	size_t written =
		boost::asio::write(socket_, boost::asio::buffer(packet.get_data(), packet.get_size()), ec);

	if (ec == boost::asio::error::would_block) {
		throw wexception("[NetRelayConnection] Socket connected to relay would block when writing");
	}
	if (written < packet.get_size()) {
		throw wexception(
					"[NetRelayConnection] Unable to send complete packet to relay (only %lu bytes of %lu)",
					written, packet.get_size());
	}
	if (ec) {
		log("[NetRelayConnection] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
	}
}

NetRelayConnection::NetRelayConnection(const NetAddress& host)
   : io_service_(), socket_(io_service_), buffer_(), peek_pointer_(0) {

	assert(host.is_valid());
	const boost::asio::ip::tcp::endpoint destination(host.ip, host.port);

	log("[NetRelayConnection]: Trying to connect to %s:%u ... ", host.ip.to_string().c_str(), host.port);
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
