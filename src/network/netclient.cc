#include "network/netclient.h"

#include <memory>

class NetClientImpl {
	public:
		NetClientImpl()
			: sock(nullptr), sockset(nullptr), deserializer() {
		}

		/// The socket that connects us to the host
		TCPsocket sock;

		/// Socket set used for selection
		SDLNet_SocketSet sockset;

		/// Deserializer acts as a buffer for packets (reassembly/splitting up)
		Deserializer deserializer;
};

std::unique_ptr<NetClient> NetClient::connect(const std::string& ip_address, const uint16_t port) {
	std::unique_ptr<NetClient> ptr(new NetClient(ip_address, port));
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
	if (d->sockset != nullptr)
		SDLNet_FreeSocketSet(d->sockset);
}

bool NetClient::is_connected() const {
	return d->sock != nullptr;
}

void NetClient::close() {
	if (!is_connected())
		return;
	SDLNet_TCP_DelSocket(d->sockset, d->sock);
	SDLNet_TCP_Close(d->sock);
	d->sock = nullptr;
}

bool NetClient::try_receive(RecvPacket& packet) {
	if (!is_connected())
		return false;

	uint8_t buffer[512];
	while (SDLNet_CheckSockets(d->sockset, 0) > 0) {

		const int32_t bytes = SDLNet_TCP_Recv(d->sock, buffer, sizeof(buffer));
		if (bytes <= 0) {
			// Error while receiving
			close();
			return false;
		}

		d->deserializer.read_data(buffer, bytes);
	}
	// Get one packet from the deserializer
	if (d->deserializer.write_packet(packet)) {
		return true;
	} else {
		return false;
	}
}

void NetClient::send(const SendPacket& packet) {
	if (is_connected()) {
		SDLNet_TCP_Send(d->sock, packet.get_data(), packet.size());
	}
}

NetClient::NetClient(const std::string& ip_address, const uint16_t port)
	: d(new NetClientImpl) {

	IPaddress addr;
	SDLNet_ResolveHost(&addr, ip_address.c_str(), port);
	log("[Client]: Trying to connect to %s:%u ... ", ip_address.c_str(), port);
	d->sock = SDLNet_TCP_Open(&addr);
	if (is_connected()) {
		log("success\n");
		d->sockset = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(d->sockset, d->sock);
	} else {
		log("failed\n");
	}
}
