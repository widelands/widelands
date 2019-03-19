#include "network/crypto.h"

#if BOOST_VERSION > 106700
#include <boost/uuid/detail/sha1.hpp>
#else
#include <boost/uuid/sha1.hpp>
#endif

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
}  // namespace crypto
