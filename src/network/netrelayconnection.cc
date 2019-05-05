#include "network/netrelayconnection.h"

#include <memory>

#include "base/log.h"

// Not so great: Quite some duplicated code between this class and NetClient.

NetRelayConnection::Peeker::Peeker(NetRelayConnection* conn) : conn_(conn), peek_pointer_(0) {
	assert(conn_);
}

bool NetRelayConnection::Peeker::string() {

	// Simple validity check. Should always be true as long as the caller has not used any receive()
	// method.
	assert(conn_->buffer_.size() >= peek_pointer_);

	conn_->try_network_receive();

	if (conn_->buffer_.size() < peek_pointer_ + 1) {
		return false;
	}

	// A string goes until the next \0 and might have a length of 0
	for (size_t i = peek_pointer_; i < conn_->buffer_.size(); ++i) {
		if (conn_->buffer_[i] == '\0') {
			peek_pointer_ = i + 1;
			return true;
		}
	}
	return false;
}

bool NetRelayConnection::Peeker::cmd(RelayCommand* out) {

	assert(conn_->buffer_.size() >= peek_pointer_);

	conn_->try_network_receive();

	if (conn_->buffer_.size() > peek_pointer_) {
		if (out != nullptr) {
			*out = static_cast<RelayCommand>(conn_->buffer_[peek_pointer_]);
		}
		peek_pointer_++;
		return true;
	}
	return false;
}

bool NetRelayConnection::Peeker::uint8_t(::uint8_t* out) {

	assert(conn_->buffer_.size() >= peek_pointer_);

	conn_->try_network_receive();

	// If there is any byte available, we can read an uint8
	if (conn_->buffer_.size() > peek_pointer_) {
		if (out != nullptr) {
			*out = static_cast<::uint8_t>(conn_->buffer_[peek_pointer_]);
		}
		peek_pointer_++;
		return true;
	}
	return false;
}

bool NetRelayConnection::Peeker::recvpacket() {

	assert(conn_->buffer_.size() >= peek_pointer_);

	conn_->try_network_receive();

	if (conn_->buffer_.size() < peek_pointer_ + 2) {
		// Not even enough space for the size of the recvpacket
		return false;
	}

	// RecvPackets have their size coded in their first two bytes
	const uint16_t size = conn_->buffer_[peek_pointer_ + 0] << 8 | conn_->buffer_[peek_pointer_ + 1];
	assert(size >= 2);

	if (conn_->buffer_.size() >= peek_pointer_ + size) {
		peek_pointer_ += size;
		return true;
	}
	return false;
}

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

void NetRelayConnection::receive(std::string* str) {
	// Try to receive something from the network.
	try_network_receive();

#ifndef NDEBUG
	// Check if we can read a complete string
	assert(Peeker(this).string());
#endif  // NDEBUG

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

void NetRelayConnection::receive(RelayCommand* out) {
	try_network_receive();

	// No complete peeker required here, we only want one byte
	assert(!buffer_.empty());

	uint8_t i;
	receive(&i);
	*out = static_cast<RelayCommand>(i);
}

void NetRelayConnection::receive(uint8_t* out) {
	try_network_receive();

	assert(!buffer_.empty());

	*out = buffer_.front();
	buffer_.pop_front();
}

void NetRelayConnection::receive(RecvPacket* packet) {
	try_network_receive();

#ifndef NDEBUG
	assert(Peeker(this).recvpacket());
#endif  // NDEBUG

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
		log("[NetRelayConnection] Error when trying to receive some data: %s.\n",
		    ec.message().c_str());
		close();
		return false;
	}
	return true;
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
		throw wexception("[NetRelayConnection] Unable to send complete string to relay (only %" PRIuS
		                 " bytes of %" PRIuS ")",
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
	if (ec) {
		log("[NetRelayConnection] Error when trying to send some data: %s.\n", ec.message().c_str());
		close();
		return;
	}
	if (written < packet.get_size()) {
		throw wexception("[NetRelayConnection] Unable to send complete packet to relay (only %" PRIuS
		                 " bytes of %" PRIuS ")",
		                 written, packet.get_size());
	}
}

NetRelayConnection::NetRelayConnection(const NetAddress& host)
   : io_service_(), socket_(io_service_), buffer_() {

	assert(host.is_valid());
	const boost::asio::ip::tcp::endpoint destination(host.ip, host.port);

	log("[NetRelayConnection] Trying to connect to %s:%u ... ", host.ip.to_string().c_str(),
	    host.port);
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

void NetRelayConnection::ignore_rtt_response() {

	// TODO(Notabilis): Implement GUI with display of RTTs and possibility to kick lagging players
	//                  See https://bugs.launchpad.net/widelands/+bug/1734673
	// TODO(Notabilis): Move this method somewhere where it makes sense.

	uint8_t length_list = 0;
	RelayCommand cmd;
	uint8_t tmp;

	Peeker peek(this);
	peek.cmd(&cmd);
	assert(cmd == RelayCommand::kRoundTripTimeResponse);

	bool data_complete = peek.uint8_t(&length_list);
	// Each list element consists of three uint8_t
	for (uint8_t i = 0; i < length_list * 3; i++) {
		data_complete = data_complete && peek.uint8_t();
	}
	if (!data_complete) {
		// Some part of this packet is still missing. Try again later
		return;
	}

	// Packet completely in buffer, fetch it and ignore it
	receive(&cmd);  // Cmd
	receive(&tmp);  // Length
	for (uint8_t i = 0; i < length_list * 3; i++) {
		receive(&tmp);  // Parts of the list. See relay_protocol.h
	}
}
