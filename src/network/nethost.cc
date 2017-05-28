#include "network/nethost.h"

#include <memory>

#include "base/log.h"

constexpr size_t kNetworkBufferSize = 512;

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
}


NetHost::Client::Client(boost::asio::ip::tcp::socket&& sock) : socket(std::move(sock)), deserializer() {
}

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
	while (!clients_.empty()) {
		close(clients_.begin()->first);
	}
}

bool NetHost::is_listening() const {
	return acceptor_v4_.is_open() || acceptor_v6_.is_open();
}

bool NetHost::is_connected(const ConnectionId id) const {
	return clients_.count(id) > 0;
}

void NetHost::stop_listening() {
	static const auto do_stop = [](boost::asio::ip::tcp::acceptor& acceptor) {
		boost::system::error_code ec;
		if (acceptor.is_open()) {
			log("[NetHost]: Closing a listening IPv%d socket.\n", get_ip_version(acceptor));
			acceptor.close(ec);
		}
		// Ignore errors
	};

	do_stop(acceptor_v4_);
	do_stop(acceptor_v6_);
}

void NetHost::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint remote = iter_client->second.socket.remote_endpoint(ec);
	if (!ec) {
		log("[NetHost] Closing network connection to %s:%i.\n",
			remote.address().to_string().c_str(), remote.port());
	} else {
		log("[NetHost] Closing network connection to some client.\n");
	}
	iter_client->second.socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	iter_client->second.socket.close(ec);
	clients_.erase(iter_client);
}

bool NetHost::try_accept(ConnectionId* new_id) {
	if (!is_listening())
		return false;
	boost::asio::ip::tcp::socket socket(io_service_);

	const auto do_try_accept = [&socket](boost::asio::ip::tcp::acceptor& acceptor) {
		boost::system::error_code ec;
		if (acceptor.is_open()) {
			acceptor.accept(socket, ec);
			if (ec == boost::asio::error::would_block) {
				// No client wants to connect
				// New socket don't need to be closed since it isn't open yet
			} else if (ec) {
				// Some other error, close the acceptor
				log("[NetHost] No longer listening for IPv%d connections due to error: %s.\n",
					get_ip_version(acceptor), ec.message().c_str());
				acceptor.close(ec);
			} else {
				log("[NetHost]: Accepting IPv%d connection from %s.\n",
					get_ip_version(acceptor), socket.remote_endpoint().address().to_string().c_str());
			}
		}
	};

	do_try_accept(acceptor_v4_);
	if (!socket.is_open())
		do_try_accept(acceptor_v6_);

	if (!socket.is_open()) {
		// No new connection
		return false;
	}

	socket.non_blocking(true);

	ConnectionId id = next_id_++;
	assert(id > 0);
	assert(clients_.count(id) == 0);
	clients_.insert(std::make_pair(id, Client(std::move(socket))));
	assert(clients_.count(id) == 1);
	*new_id = id;
	return true;
}

bool NetHost::try_receive(const ConnectionId id, RecvPacket* packet) {
	// Always read all available data into buffers
	uint8_t buffer[kNetworkBufferSize];

	boost::system::error_code ec;
	for (auto it = clients_.begin(); it != clients_.end(); ) {
		size_t length = it->second.socket.read_some(boost::asio::buffer(buffer, kNetworkBufferSize), ec);
		if (ec == boost::asio::error::would_block) {
			// Nothing to read
			assert(length == 0);
			++it;
			continue;
		} else if (ec) {
			assert(length == 0);
			// Connection closed or some error, close the socket
			// close() will remove the client from the map so we have to increment the iterator first
			ConnectionId id_to_remove = it->first;
			++it;
			log("[NetHost] Error when receiving from a client, closing connection: %s.\n",
				ec.message().c_str());
			close(id_to_remove);
			continue;
		}
		assert(length > 0);
		assert(length <= kNetworkBufferSize);
		// Read something
		it->second.deserializer.read_data(buffer, length);
		++it;
	}

	// Now check whether there is data for the requested client
	if (!is_connected(id))
		return false;

	// Try to get one packet from the deserializer
	return clients_.at(id).deserializer.write_packet(packet);
}

void NetHost::send(const ConnectionId id, const SendPacket& packet) {
	boost::system::error_code ec;
	if (is_connected(id)) {
		size_t written = boost::asio::write(clients_.at(id).socket,
											boost::asio::buffer(packet.get_data(), packet.get_size()), ec);
		// TODO(Notabilis): This one is an assertion of mine, I am not sure if it will hold
		// If it doesn't, set the socket to blocking before writing
		// If it does, remove this comment after build 20
		assert(ec != boost::asio::error::would_block);
		assert(written == packet.get_size() || ec);
		if (ec) {
			log("[NetHost] Error when sending to a client, closing connection: %s.\n", ec.message().c_str());
			close(id);
		}
	}
}

NetHost::NetHost(const uint16_t port)
	: clients_(), next_id_(1), io_service_(), acceptor_v4_(io_service_), acceptor_v6_(io_service_) {

	if (open_acceptor(&acceptor_v4_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))) {
		log("[NetHost]: Opening a listening IPv4 socket on TCP port %u\n", port);
	}
	if (open_acceptor(&acceptor_v6_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), port))) {
		log("[NetHost]: Opening a listening IPv6 socket on TCP port %u\n", port);
	}
}

bool NetHost::open_acceptor(boost::asio::ip::tcp::acceptor *acceptor,
						const boost::asio::ip::tcp::endpoint& endpoint) {
	try {
		acceptor->open(endpoint.protocol());
		acceptor->non_blocking(true);
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
