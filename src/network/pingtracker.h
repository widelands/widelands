/*
 * Copyright (C) 2008-2021 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_NETWORK_PINGTRACKER_H
#define WL_NETWORK_PINGTRACKER_H

#include <chrono>
#include <list>
#include <map>

/**
 * Stores the state of the currently active pings and calculates RTTs.
 */
class PingTracker {
public:
	/**
	 * Registers that a PING has been send to a client.
	 * @param clientnum An arbitrary number representing the client
	 * @param seq The number/id of the ping
	 */
	void register_request(size_t clientnum, uint8_t seq);

	/**
	 * Registers that a PONG has been received from a client.
	 * There should have been a previous PING been registered with matching client and number.
	 * @param clientnum The client the PONG has been received from
	 * @param seq The number/id contained in the PONG
	 */
	void register_response(size_t clientnum, uint8_t seq);

	/**
	 * Register a complete RTT instead of a request/response pair
	 * @param clientnum The client to register the RTT for
	 * @param rtt The time in milliseconds the PING/PONG pair needed
	 */
	void register_rtt(size_t clientnum, uint8_t rtt);

	/**
	 * Returns the smoothed RTT as calculated over the last few PING/PONG pairs.
	 * @param clientnum The client to get the RTT for
	 * @return The RTT in milliseconds, up to 254. 255 means the RTT is unknown or too large
	 */
	uint8_t get_rtt(size_t clientnum);

private:
	/**
	 * Closes open PING requests that are so old that they (would) result in an RTT of 255.
	 * @param clientnum The client to check
	 */
	void close_old_entries(size_t clientnum);

	/// The data stored for each open ping request
	struct PingData {
		/// The sequence number of the ping
		uint8_t seq;
		/// The time the ping was send
		std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
	};

	/// The data stored about a client
	struct ClientData {
		/// A list of not yet answered pings
		std::list<PingData> open_requests;
		/// The RTTs from the answered pings
		std::list<uint8_t> rtts;
	};

	/// A map listing the known clients and their data
	std::map<size_t, ClientData> clients_;
};

#endif  // end of include guard: WL_NETWORK_PINGTRACKER_H
