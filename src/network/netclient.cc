#include "network/netclient.h"

#include <memory>

#include "base/log.h"

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
	if (sockset_ != nullptr)
		SDLNet_FreeSocketSet(sockset_);
}

bool NetClient::is_connected() const {
	return sock_ != nullptr;
}

void NetClient::close() {
	if (!is_connected())
		return;
	SDLNet_TCP_DelSocket(sockset_, sock_);
	SDLNet_TCP_Close(sock_);
	sock_ = nullptr;
}

bool NetClient::try_receive(RecvPacket *packet) {
	if (!is_connected())
		return false;

	uint8_t buffer[512];
	while (SDLNet_CheckSockets(sockset_, 0) > 0) {

		const int32_t bytes = SDLNet_TCP_Recv(sock_, buffer, sizeof(buffer));
		if (bytes <= 0) {
			// Error while receiving
			close();
			return false;
		}

		deserializer_.read_data(buffer, bytes);
	}
	// Get one packet from the deserializer
	return deserializer_.write_packet(packet);
}

void NetClient::send(const SendPacket& packet) {
	if (is_connected()) {
		SDLNet_TCP_Send(sock_, packet.get_data(), packet.get_size());
	}
}

NetClient::NetClient(const std::string& ip_address, const uint16_t port)
	: sock_(nullptr), sockset_(nullptr), deserializer_() {

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, ip_address.c_str(), port) != 0) {
		log("[Client]: Failed to resolve host address %s:%u.\n", ip_address.c_str(), port);
		return;
	}
	log("[Client]: Trying to connect to %s:%u ... ", ip_address.c_str(), port);
	sock_ = SDLNet_TCP_Open(&addr);
	if (is_connected()) {
		log("success\n");
		sockset_ = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(sockset_, sock_);
	} else {
		log("failed\n");
	}
}
