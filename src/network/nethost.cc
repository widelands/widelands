#include "network/nethost.h"

#include <memory>

#include "base/log.h"

namespace {

/**
 * Returns the IP version.
 * \param acceptor The acceptor socket to get the IP version for.
 * \return Either 4 or 6, depending on the version of the given acceptor.
 */
int get_ip_version(const boost::asio::ip::tcp::acceptor& acceptor) {
	assert(acceptor.is_open());
	if (acceptor.local_endpoint().protocol() == boost::asio::ip::tcp::v4()) {
		return 4;
	} else {
		return 6;
	}
}
}  // namespace

std::unique_ptr<NetHost> NetHost::listen(const uint16_t port) {
	std::unique_ptr<NetHost> ptr(new NetHost(port));
	if (ptr->is_listening()) {
		return ptr;
	} else {
		ptr.reset();
		return ptr;
	}
}

NetHost::~NetHost() {
	stop_listening();
	assert(!asio_thread_.joinable());
	while (!clients_.empty()) {
		close(clients_.begin()->first);
	}
}

bool NetHost::is_listening() const {
	return acceptor_v4_.is_open() || acceptor_v6_.is_open();
}

bool NetHost::is_connected(const ConnectionId id) const {
	assert(!clients_.count(id) || clients_.at(id).conn);
	return clients_.count(id) > 0 && clients_.at(id).conn->is_connected();
}

void NetHost::stop_listening() {

	// Stop the thread
	io_service_.stop();
	for (int i = 0; i < 1000 && !io_service_.stopped(); i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	assert(io_service_.stopped());
	if (asio_thread_.joinable()) {
		try {
			asio_thread_.join();
		} catch (const std::invalid_argument&) {
			// Most likely the thread ended between joinable() and join()
		}
	}
	// The thread should be stopped now
	assert(!asio_thread_.joinable());

	static const auto do_stop = [](boost::asio::ip::tcp::acceptor& acceptor) {
		boost::system::error_code ec;
		if (acceptor.is_open()) {
			log_info("[NetHost] Closing a listening IPv%d socket.\n", get_ip_version(acceptor));
			acceptor.close(ec);
		}
		// Ignore errors
	};

	do_stop(acceptor_v4_);
	do_stop(acceptor_v6_);
}

void NetHost::register_ping(ConnectionId id, uint8_t seq) {
	std::lock_guard<std::mutex> lock(mutex_pings_);
	pings_.register_request(id, seq);
}

uint8_t NetHost::get_rtt(ConnectionId id) {
	std::lock_guard<std::mutex> lock(mutex_pings_);
	return pings_.get_rtt(id);
}

void NetHost::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
	std::lock_guard<std::mutex> lock(clients_.at(id).mutex);
	iter_client->second.conn->close();
	clients_.erase(iter_client);
}

bool NetHost::try_accept(ConnectionId* new_id) {

	std::lock_guard<std::mutex> lock(mutex_accept_);
	if (accept_queue_.empty()) {
		return false;
	}

	ConnectionId id = next_id_++;
	assert(id > 0);
	assert(clients_.count(id) == 0);
	clients_[id].conn = std::move(accept_queue_.front());
	accept_queue_.pop();
	clients_[id].conn->data_received.connect([this, id]() { handle_data(id); });
	handle_data(id);
	assert(clients_.count(id) == 1);
	*new_id = id;
	return true;
}

std::unique_ptr<RecvPacket> NetHost::try_receive(const ConnectionId id) {
	// Check whether there is data for the requested client
	if (!is_connected(id)) {
		return std::unique_ptr<RecvPacket>();
	}
	std::lock_guard<std::mutex> lock(clients_.at(id).mutex);
	if (clients_.at(id).packets.empty()) {
		return std::unique_ptr<RecvPacket>();
	}
	std::unique_ptr<RecvPacket> packet = std::move(clients_.at(id).packets.front());
	clients_.at(id).packets.pop();
	return packet;
}

void NetHost::send(const ConnectionId id, const SendPacket& packet, NetPriority priority) {
	if (!is_connected(id)) {
		return;
	}
	std::lock_guard<std::mutex> lock(clients_.at(id).mutex);
	clients_.at(id).conn->send(priority, packet);
}

void NetHost::send(const std::vector<ConnectionId>& ids,
                   const SendPacket& packet,
                   NetPriority priority) {
	for (ConnectionId id : ids) {
		send(id, packet, priority);
	}
}

// This method is run within a thread
void NetHost::start_accepting(
   boost::asio::ip::tcp::acceptor& acceptor,
   std::pair<std::unique_ptr<BufferedConnection>, boost::asio::ip::tcp::socket*>& pair) {

	if (!is_listening()) {
		return;
	}

	if (!pair.first) {
		assert(pair.second == nullptr);
		pair = BufferedConnection::create_unconnected();
	}

	acceptor.async_accept(
	   *(pair.second), [this, &acceptor, &pair](const boost::system::error_code& ec) {
		   if (!ec) {
			   // No error occurred, so we have establish a (TCP) connection.
			   // We can't say whether it is valid Widelands client yet
			   pair.first->notify_connected();
			   assert(pair.first->is_connected());
			   std::lock_guard<std::mutex> lock(mutex_accept_);
			   accept_queue_.push(std::move(pair.first));
			   // pair.first is cleared by the std::move
			   pair.second = nullptr;
		   }
		   // Wait for the next client
		   start_accepting(acceptor, pair);
	   });
}

void NetHost::handle_data(ConnectionId id) {
	assert(is_connected(id));
	Client& client = clients_.at(id);

	std::lock_guard<std::mutex> lock(client.mutex);

	uint8_t length;
	uint8_t cmd;
	uint8_t seq;

	// Try to get one packet from the connection
	while (BufferedConnection::Peeker(client.conn.get()).recvpacket()) {
		// Enough data for a packet
		// Check whether it is a ping or pong
		BufferedConnection::Peeker peek(client.conn.get());
		// Make sure we can read enough bytes for the length, check if the lower byte of
		// the length decode to 2 (we should check the higher byte as well. But if it is a pong,
		// it should be 0 anyway), check if it is a PONG, try to get the sequence number

		if (peek.uint8_t() && peek.uint8_t(&length) && length == 4 && peek.uint8_t(&cmd) &&
		    cmd == NETCMD_PONG && peek.uint8_t(&seq)) {
			// All checks passed, register the pong
			std::lock_guard<std::mutex> lock_pings(mutex_pings_);
			pings_.register_response(id, seq);
		}
		// Get the packet and add it to the queue
		std::unique_ptr<RecvPacket> packet(new RecvPacket);
		client.conn->receive(packet.get());
		client.packets.push(std::move(packet));
	}
}

NetHost::NetHost(const uint16_t port)
   : clients_(), next_id_(1), io_service_(), acceptor_v4_(io_service_), acceptor_v6_(io_service_) {

	if (open_acceptor(
	       &acceptor_v4_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))) {
		log_info("[NetHost] Opening a listening IPv4 socket on TCP port %u\n", port);
	}
	if (open_acceptor(
	       &acceptor_v6_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), port))) {
		log_info("[NetHost] Opening a listening IPv6 socket on TCP port %u\n", port);
	}

	log_info("[NetHost] Starting to listen for network connections\n");
	start_accepting(acceptor_v4_, accept_pair_v4_);
	start_accepting(acceptor_v6_, accept_pair_v6_);

	asio_thread_ = std::thread([this]() {
		log_info("[NetHost] Starting networking thread\n");
		io_service_.run();
		log_info("[NetHost] Stopping networking thread\n");
	});
}

bool NetHost::open_acceptor(boost::asio::ip::tcp::acceptor* acceptor,
                            const boost::asio::ip::tcp::endpoint& endpoint) {
	try {
		acceptor->open(endpoint.protocol());
		const boost::asio::socket_base::reuse_address option_reuse(true);
		acceptor->set_option(option_reuse);
		if (endpoint.protocol() == boost::asio::ip::tcp::v6()) {
			const boost::asio::ip::v6_only option_v6only(true);
			acceptor->set_option(option_v6only);
		}
		acceptor->bind(endpoint);
		acceptor->listen(boost::asio::socket_base::max_connections);
		return true;
	} catch (const boost::system::system_error&) {
		return false;
	}
}
