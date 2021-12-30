#include "network/bufferedconnection.h"

#include <memory>

#include "base/log.h"

BufferedConnection::Peeker::Peeker(BufferedConnection* conn) : conn_(conn), peek_pointer_(0) {
	assert(conn_);
}

bool BufferedConnection::Peeker::string() {

	std::lock_guard<std::mutex> lock(conn_->mutex_receive_);

	// Simple validity check. Should always be true as long as the caller
	// has not used any receive() method.
	assert(conn_->receive_buffer_.size() >= peek_pointer_);

	if (conn_->receive_buffer_.size() < peek_pointer_ + 1) {
		return false;
	}

	// A string goes until the next \0 and might have a length of 0
	for (size_t i = peek_pointer_; i < conn_->receive_buffer_.size(); ++i) {
		if (conn_->receive_buffer_[i] == '\0') {
			peek_pointer_ = i + 1;
			return true;
		}
	}
	return false;
}

bool BufferedConnection::Peeker::cmd(RelayCommand* out) {

	std::lock_guard<std::mutex> lock(conn_->mutex_receive_);

	assert(conn_->receive_buffer_.size() >= peek_pointer_);

	if (conn_->receive_buffer_.size() > peek_pointer_) {
		if (out != nullptr) {
			*out = static_cast<RelayCommand>(conn_->receive_buffer_[peek_pointer_]);
		}
		peek_pointer_++;
		return true;
	}
	return false;
}

bool BufferedConnection::Peeker::uint8_t(::uint8_t* out) {

	std::lock_guard<std::mutex> lock(conn_->mutex_receive_);

	assert(conn_->receive_buffer_.size() >= peek_pointer_);

	// If there is any byte available, we can read an uint8
	if (conn_->receive_buffer_.size() > peek_pointer_) {
		if (out != nullptr) {
			*out = static_cast<::uint8_t>(conn_->receive_buffer_[peek_pointer_]);
		}
		peek_pointer_++;
		return true;
	}
	return false;
}

bool BufferedConnection::Peeker::recvpacket() {

	std::lock_guard<std::mutex> lock(conn_->mutex_receive_);

	assert(conn_->receive_buffer_.size() >= peek_pointer_);

	if (conn_->receive_buffer_.size() < peek_pointer_ + 2) {
		// Not even enough space for the size of the recvpacket
		return false;
	}

	// RecvPackets have their size coded in their first two bytes.
	// See SendPacket in network.cc
	const uint16_t size =
	   conn_->receive_buffer_[peek_pointer_ + 0] << 8 | conn_->receive_buffer_[peek_pointer_ + 1];
	assert(size >= 2);

	if (conn_->receive_buffer_.size() >= peek_pointer_ + size) {
		peek_pointer_ += size;
		return true;
	}
	return false;
}

std::unique_ptr<BufferedConnection> BufferedConnection::connect(const NetAddress& host) {
	assert(host.is_valid());
	std::unique_ptr<BufferedConnection> ptr(new BufferedConnection(host));
	if (!ptr->is_connected()) {
		ptr.reset();
	}

	return ptr;
}

std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*>
BufferedConnection::create_unconnected() {
	std::unique_ptr<BufferedConnection> ptr(new BufferedConnection());
	assert(!ptr->is_connected());
	return std::make_pair(std::move(ptr), &(ptr->socket_));
}

BufferedConnection::~BufferedConnection() {
	close();
	assert(!asio_thread_.joinable());
}

bool BufferedConnection::is_connected() const {
	return socket_.is_open();
}

void BufferedConnection::close() {
	std::error_code ec;
	asio::ip::tcp::endpoint remote = socket_.remote_endpoint(ec);
	if (!ec) {
		verb_log_info("[BufferedConnection] Closing network socket connected to %s:%i.",
		              remote.address().to_string().c_str(), remote.port());
	} else {
		verb_log_info("[BufferedConnection] Closing network socket.");
	}
	// Stop the thread
	io_service_.stop();
	// Not sure if that is required, wait up to one second for the io_service to stop
	for (int i = 0; i < 1000 && !io_service_.stopped(); i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	assert(io_service_.stopped());
	if (asio_thread_.joinable()) {
		try {
			asio_thread_.join();
		} catch (const std::invalid_argument&) {
			// Thread probably stopped between joinable() and join()
		}
	}
	// The thread should be stopped now
	assert(!asio_thread_.joinable());
	// Close the socket
	if (socket_.is_open()) {
		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		socket_.close(ec);
	}
}

void BufferedConnection::receive(std::string* str) {

	// Check if we can read a complete string
	assert(Peeker(this).string());

	std::lock_guard<std::mutex> lock(mutex_receive_);

	// Read the string
	str->clear();
	// No range check needed, peek_string() takes care of that
	while (receive_buffer_.front() != '\0') {
		str->push_back(receive_buffer_.front());
		receive_buffer_.pop_front();
	}
	// Pop the \0
	receive_buffer_.pop_front();
}

void BufferedConnection::receive(RelayCommand* out) {
	uint8_t i;
	receive(&i);
	*out = static_cast<RelayCommand>(i);
}

void BufferedConnection::receive(uint8_t* out) {

	std::lock_guard<std::mutex> lock(mutex_receive_);

	assert(!receive_buffer_.empty());

	*out = receive_buffer_.front();
	receive_buffer_.pop_front();
}

void BufferedConnection::receive(RecvPacket* packet) {

	assert(Peeker(this).recvpacket());

	std::lock_guard<std::mutex> lock(mutex_receive_);

	// Read the packet
	const uint16_t size = receive_buffer_[0] << 8 | receive_buffer_[1];
	assert(size >= 2);
	assert(receive_buffer_.size() >= size);

	packet->buffer.clear();
	packet->buffer.insert(
	   packet->buffer.end(), receive_buffer_.begin() + 2, receive_buffer_.begin() + size);
	packet->index_ = 0;

	receive_buffer_.erase(receive_buffer_.begin(), receive_buffer_.begin() + size);
}

// Called by send() method but will only do something if not sending yet
void BufferedConnection::start_sending() {

	std::unique_lock<std::mutex> lock(mutex_send_);

	if (currently_sending_) {
		// Already sending, don't start a second write call
		return;
	}

	// Find something to send
	std::queue<std::vector<uint8_t>>* nonempty_queue = nullptr;
	for (auto& entry : buffers_to_send_) {
		if (!entry.second.empty()) {
			nonempty_queue = &entry.second;
			break;
		}
	}

	if (nonempty_queue == nullptr) {
		// Nothing (further) to send (right now)
		return;
	}

	currently_sending_ = true;
	lock.unlock();

	// Start writing to the socket. This might block if the network buffer within
	// the operating system is currently full.
	// When done with sending, call the lambda method defined below
	asio::async_write(
	   socket_, asio::buffer(nonempty_queue->front()),
#ifndef NDEBUG
	   [this, nonempty_queue](std::error_code ec, std::size_t length) {
#else
	   [this, nonempty_queue](std::error_code ec, std::size_t /*length*/) {
#endif
		   std::unique_lock<std::mutex> lock2(mutex_send_);
		   currently_sending_ = false;
		   if (!ec) {
			   // No error: Remove the buffer from the queue
			   assert(nonempty_queue != nullptr);
			   assert(!nonempty_queue->empty());
			   assert(nonempty_queue->front().size() == length);
			   nonempty_queue->pop();
			   lock2.unlock();
			   // Try to send some more data
			   start_sending();
		   } else {
			   if (socket_.is_open()) {
				   log_err("[BufferedConnection] Error when sending packet to host (error %i: %s)\n",
				           ec.value(), ec.message().c_str());
				   log_err("[BufferedConnection] Closing socket\n");
				   socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
				   socket_.close();
			   }
		   }
	   });
}

// This method is run within a thread
void BufferedConnection::start_receiving() {

	if (!is_connected()) {
		return;
	}

	socket_.async_read_some(
	   asio::buffer(asio_receive_buffer_, kNetworkBufferSize),
	   [this](std::error_code ec, std::size_t length) {
		   if (!ec) {
			   assert(length > 0);
			   assert(length <= kNetworkBufferSize);
			   // Has read something
			   std::unique_lock<std::mutex> lock(mutex_receive_);
			   for (size_t i = 0; i < length; ++i) {
				   receive_buffer_.push_back(asio_receive_buffer_[i]);
			   }
			   lock.unlock();
			   // Try to send some more data
			   start_receiving();
		   } else {
			   if (socket_.is_open()) {
				   log_err("[BufferedConnection] Error when receiving data from host (error %i: %s)\n",
				           ec.value(), ec.message().c_str());
				   log_err("[BufferedConnection] Closing socket\n");
				   socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
				   socket_.close();
			   }
		   }
	   });
}

void BufferedConnection::reduce_send_buffer(asio::ip::tcp::socket& socket) {
	// Reduce the size of the send buffer. This will result in (slightly) slower
	// file transfers but keeps the program responsive (e.g., chat messages are
	// displayed) while transmitting files
	std::error_code ec;
	asio::socket_base::send_buffer_size send_buffer_size;
	socket.get_option(send_buffer_size, ec);
	if (!ec && send_buffer_size.value() > 20 * static_cast<int>(kNetworkBufferSize)) {
		const asio::socket_base::send_buffer_size new_buffer_size(20 * kNetworkBufferSize);
		socket.set_option(new_buffer_size, ec);
		// Ignore error. When it fails, chat messages will lag while transmitting files,
		// but nothing really bad happens
		if (ec) {
			log_warn("[BufferedConnection] Warning: Failed to reduce send buffer size\n");
		}
	}
}

BufferedConnection::BufferedConnection(const NetAddress& host)
   : socket_(io_service_), currently_sending_(false) {

	const asio::ip::tcp::endpoint destination(host.ip, host.port);

	verb_log_info("[BufferedConnection] Trying to connect to %s:%u ... ",
	              host.ip.to_string().c_str(), host.port);
	std::error_code ec;
	socket_.connect(destination, ec);
	if (!ec && is_connected()) {
		verb_log_info("success.\n");

		reduce_send_buffer(socket_);

		// start_receiving() has to be called before the thread is started,
		// otherwise the thread terminates again immediately
		start_receiving();
		asio_thread_ = std::thread([this]() {
			// The output might actually be messed up if it collides with the main thread...
			verb_log_info("[BufferedConnection] Starting networking thread\n");
			io_service_.run();
			verb_log_info("[BufferedConnection] Stopping networking thread\n");
		});
	} else {
		log_err("[BufferedConnection] Trying to connect to %s:%u failed!",
		        host.ip.to_string().c_str(), host.port);
		socket_.close();
		assert(!is_connected());
	}
}

BufferedConnection::BufferedConnection() : socket_(io_service_), currently_sending_(false) {
}

void BufferedConnection::notify_connected() {
	assert(is_connected());

	verb_log_info("[BufferedConnection] Connection to %s.",
	              socket_.remote_endpoint().address().to_string().c_str());

	reduce_send_buffer(socket_);

	start_receiving();
	asio_thread_ = std::thread([this]() {
		// The output might actually be messed up if it collides with the main thread...
		verb_log_info("[BufferedConnection] Starting networking thread");
		io_service_.run();
		verb_log_info("[BufferedConnection] Stopping networking thread");
	});
}

void BufferedConnection::ignore_rtt_response() {

	// TODO(Notabilis): Implement GUI with display of RTTs and possibility to kick lagging players
	//                  See https://github.com/widelands/widelands/issues/3236
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
