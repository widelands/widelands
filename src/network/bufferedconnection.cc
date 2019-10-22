#include "network/bufferedconnection.h"

#include <memory>
#include <thread>

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

#if BOOST_VERSION >= 106600
std::unique_ptr<BufferedConnection>
BufferedConnection::accept(boost::asio::ip::tcp::acceptor& acceptor) {
	assert(acceptor.is_open());
	std::unique_ptr<BufferedConnection> ptr(new BufferedConnection(acceptor));
	if (!ptr->is_connected()) {
		ptr.reset();
	}
	return ptr;
}
#else
std::pair<std::unique_ptr<BufferedConnection>, boost::asio::ip::tcp::socket*>
BufferedConnection::create_unconnected() {
	std::unique_ptr<BufferedConnection> ptr(new BufferedConnection());
	assert(!ptr->is_connected());
	return std::make_pair(std::move(ptr), &(ptr->socket_));
}
#endif

BufferedConnection::~BufferedConnection() {
	close();
	assert(!asio_thread_.joinable());
}

bool BufferedConnection::is_connected() const {
	return socket_.is_open();
}

void BufferedConnection::close() {
	if (!is_connected()) {
		return;
	}
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint remote = socket_.remote_endpoint(ec);
	if (!ec) {
		log("[BufferedConnection] Closing network socket connected to %s:%i.\n",
		    remote.address().to_string().c_str(), remote.port());
	} else {
		log("[BufferedConnection] Closing network socket.\n");
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
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_.close(ec);
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
	boost::asio::async_write(
	   socket_, boost::asio::buffer(nonempty_queue->front()),
#ifndef NDEBUG
	   [this, nonempty_queue](boost::system::error_code ec, std::size_t length) {
#else
	   [this, nonempty_queue](boost::system::error_code ec, std::size_t /*length*/) {
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
			   throw wexception(
			      "[BufferedConnection] Error when sending packet to host (error %i: %s)", ec.value(),
			      ec.message().c_str());
		   }
	   });
}

// This method is run within a thread
void BufferedConnection::start_receiving() {

	if (!is_connected()) {
		return;
	}

	socket_.async_read_some(
	   boost::asio::buffer(asio_receive_buffer_, kNetworkBufferSize),
	   [this](boost::system::error_code ec, std::size_t length) {
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
		   } else if (ec == boost::asio::error::eof) {
			   // Connection has been closed, nothing more to do here
		   } else {
			   throw wexception(
			      "[BufferedConnection] Error when receiving data from host (error %i: %s)",
			      ec.value(), ec.message().c_str());
		   }
	   });
}

BufferedConnection::BufferedConnection(const NetAddress& host)
   : io_service_(), socket_(io_service_), receive_buffer_(), currently_sending_(false) {

	const boost::asio::ip::tcp::endpoint destination(host.ip, host.port);

	log("[BufferedConnection] Trying to connect to %s:%u ... ", host.ip.to_string().c_str(),
	    host.port);
	boost::system::error_code ec;
	socket_.connect(destination, ec);
	if (!ec && is_connected()) {
		log("success.\n");

		// start_receiving() has to be called before the thread is started,
		// otherwise the thread terminates again immediately
		start_receiving();
		asio_thread_ = std::thread([this]() {
			// The output might actually be messed up if it collides with the main thread...
			log("[BufferedConnection] Starting networking thread\n");
			io_service_.run();
			log("[BufferedConnection] Stopping networking thread\n");
		});
	} else {
		log("failed.\n");
		socket_.close();
		assert(!is_connected());
	}
}

#if BOOST_VERSION >= 106600
BufferedConnection::BufferedConnection(boost::asio::ip::tcp::acceptor& acceptor)
   : io_service_(), socket_(io_service_), receive_buffer_(), currently_sending_(false) {

	boost::system::error_code ec;
	acceptor.accept(socket_, ec);
	assert(ec != boost::asio::error::would_block);
	if (ec) {
		// Some error
		log("[BufferedConnection] Error when trying to accept connection: %s.\n",
		    ec.message().c_str());
		assert(!is_connected());
		return;
	}
	assert(is_connected());

	log("[BufferedConnection] Accepting connection from %s.\n",
	    socket_.remote_endpoint().address().to_string().c_str());

	start_receiving();
	asio_thread_ = std::thread([this]() {
		// The output might actually be messed up if it collides with the main thread...
		log("[BufferedConnection] Starting networking thread\n");
		io_service_.run();
		log("[BufferedConnection] Stopping networking thread\n");
	});
}
#else
BufferedConnection::BufferedConnection()
   : io_service_(), socket_(io_service_), receive_buffer_(), currently_sending_(false) {
}

void BufferedConnection::notify_connected() {
	assert(is_connected());

	log("[BufferedConnection] Connection to %s.\n",
	    socket_.remote_endpoint().address().to_string().c_str());

	start_receiving();
	asio_thread_ = std::thread([this]() {
		// The output might actually be messed up if it collides with the main thread...
		log("[BufferedConnection] Starting networking thread\n");
		io_service_.run();
		log("[BufferedConnection] Stopping networking thread\n");
	});
}
#endif

void BufferedConnection::ignore_rtt_response() {

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
