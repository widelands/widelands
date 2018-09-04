#include "network/crypto.h"

#include <boost/uuid/sha1.hpp>

namespace crypto {

std::string sha1(const std::string& input) {

	// Hash the input
	boost::uuids::detail::sha1 sha;
	sha.process_bytes(input.data(), input.size());
	uint32_t digest[5];
	sha.get_digest(digest);

	// Back to string
	char result[41] = {0};

	for (int i = 0; i < 5; i++) {
		std::sprintf(result + (i << 3), "%08x", digest[i]);
	}

	return std::string(result);
}
}
