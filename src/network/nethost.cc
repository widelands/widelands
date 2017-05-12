#include "network/nethost.h"

#include <memory>

#include "base/log.h"

NetHost::Client::Client(TCPsocket sock) : socket(sock), deserializer() {
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
	SDLNet_FreeSocketSet(sockset_);
}

bool NetHost::is_listening() const {
	return svrsock_ != nullptr;
}

bool NetHost::is_connected(const ConnectionId id) const {
	return clients_.count(id) > 0;
}

void NetHost::stop_listening() {
	if (!is_listening())
		return;
	SDLNet_TCP_DelSocket(sockset_, svrsock_);
	SDLNet_TCP_Close(svrsock_);
	svrsock_ = nullptr;
}

void NetHost::close(const ConnectionId id) {
	auto iter_client = clients_.find(id);
	if (iter_client == clients_.end()) {
		// Not connected anyway
		return;
	}
	SDLNet_TCP_DelSocket(sockset_, iter_client->second.socket);
	SDLNet_TCP_Close(iter_client->second.socket);
	clients_.erase(iter_client);
}

bool NetHost::try_accept(ConnectionId* new_id) {
	if (!is_listening())
		return false;

	TCPsocket sock = SDLNet_TCP_Accept(svrsock_);
	// No client wants to connect
	if (sock == nullptr)
		return false;
	SDLNet_TCP_AddSocket(sockset_, sock);
	ConnectionId id = next_id_++;
	assert(id > 0);
	assert(clients_.count(id) == 0);
	clients_.insert(std::make_pair(id, Client(sock)));
	assert(clients_.count(id) == 1);
	*new_id = id;
	return true;
}

bool NetHost::try_receive(const ConnectionId id, RecvPacket* packet) {

	// Always read all available data into buffers
	uint8_t buffer[512];
	while (SDLNet_CheckSockets(sockset_, 0) > 0) {
		for (auto& e : clients_) {
			if (SDLNet_SocketReady(e.second.socket)) {
				const int32_t bytes = SDLNet_TCP_Recv(e.second.socket, buffer, sizeof(buffer));
				if (bytes <= 0) {
					// Error while receiving
					close(e.first);
					// We have to run the for-loop again since we modified the map
					break;
				}

				e.second.deserializer.read_data(buffer, bytes);
			}
		}
	}

	// Now check whether there is data for the requested client
	if (!is_connected(id))
		return false;

	// Get one packet from the deserializer
	return clients_.at(id).deserializer.write_packet(packet);
}

void NetHost::send(const ConnectionId id, const SendPacket& packet) {
	if (is_connected(id)) {
		SDLNet_TCP_Send(clients_.at(id).socket, packet.get_data(), packet.get_size());
	}
}

NetHost::NetHost(const uint16_t port) : svrsock_(nullptr), sockset_(nullptr), next_id_(1) {

	IPaddress myaddr;
	SDLNet_ResolveHost(&myaddr, nullptr, port);
	svrsock_ = SDLNet_TCP_Open(&myaddr);
	// Maximal 16 sockets! This mean we can have at most 15 clients_ in our game (+ metaserver)
	sockset_ = SDLNet_AllocSocketSet(16);
}
