#include "network/nethost.h"

#include <map>
#include <memory>

#include <SDL_net.h>

#include "base/log.h"

class NetHostImpl {
	public:

		class Client {
			public:
				Client(TCPsocket sock)
					: socket(sock), deserializer() {
				}
				TCPsocket socket;
				Deserializer deserializer;
		};

		NetHostImpl()
			: svrsock(nullptr), sockset(nullptr), next_id(1) {
		}

		TCPsocket svrsock;
		SDLNet_SocketSet sockset;
		std::map<NetHost::ConnectionId, NetHostImpl::Client> clients;
		NetHost::ConnectionId next_id;
};

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
	while (!d->clients.empty()) {
		close(d->clients.begin()->first);
	}
	SDLNet_FreeSocketSet(d->sockset);
}


bool NetHost::is_listening() const {
	return d->svrsock != nullptr;
}

bool NetHost::is_connected(const ConnectionId id) const {
	return d->clients.count(id) > 0;
}

void NetHost::stop_listening() {
	if (!is_listening())
		return;
	SDLNet_TCP_DelSocket(d->sockset, d->svrsock);
	SDLNet_TCP_Close(d->svrsock);
	d->svrsock = nullptr;
}

void NetHost::close(const ConnectionId id) {
	auto iter_client = d->clients.find(id);
	if (iter_client == d->clients.end()) {
		// Not connected anyway
		return;
	}
	SDLNet_TCP_DelSocket(d->sockset, iter_client->second.socket);
	SDLNet_TCP_Close(iter_client->second.socket);
	d->clients.erase(iter_client);
}

bool NetHost::try_accept(ConnectionId *new_id) {
	if (!is_listening())
		return false;

	TCPsocket sock = SDLNet_TCP_Accept(d->svrsock);
	// No client wants to connect
	if (sock == nullptr)
		return false;
	SDLNet_TCP_AddSocket(d->sockset, sock);
	ConnectionId id = d->next_id++;
	assert(id > 0);
	assert(d->clients.count(id) == 0);
	d->clients.insert(std::make_pair(id, NetHostImpl::Client(sock)));
	assert(d->clients.count(id) == 1);
	*new_id = id;
	return true;
}

bool NetHost::try_receive(const ConnectionId id, RecvPacket *packet) {

	// Always read all available data into buffers
	uint8_t buffer[512];
	while (SDLNet_CheckSockets(d->sockset, 0) > 0) {
		for (auto& e : d->clients) {
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
	return d->clients.at(id).deserializer.write_packet(packet);
}

void NetHost::send(const ConnectionId id, const SendPacket& packet) {
	if (is_connected(id)) {
		SDLNet_TCP_Send(d->clients.at(id).socket, packet.get_data(), packet.get_size());
	}
}

NetHost::NetHost(const uint16_t port)
	: d(new NetHostImpl) {

	IPaddress myaddr;
	SDLNet_ResolveHost(&myaddr, nullptr, port);
	d->svrsock = SDLNet_TCP_Open(&myaddr);
	// Maximal 16 sockets! This mean we can have at most 15 clients in our game (+ metaserver)
	d->sockset = SDLNet_AllocSocketSet(16);
}
