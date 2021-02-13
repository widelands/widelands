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

#include "network/pingtracker.h"

#include <cassert>

/// The number of RTTs to store. If there are more RTTs, old ones will be dropped
static const size_t MEASUREMENT_COUNT = 10;

void PingTracker::register_request(size_t clientnum, uint8_t seq) {
	close_old_entries(clientnum);
	const auto now = std::chrono::high_resolution_clock::now();
	// Add the new entry
	clients_[clientnum].open_requests.push_back({seq, now});
}

void PingTracker::register_response(size_t clientnum, uint8_t seq) {
	close_old_entries(clientnum);
	const auto now = std::chrono::high_resolution_clock::now();
	ClientData& client = clients_[clientnum];
	for (auto iter = client.open_requests.begin(); iter != client.open_requests.end(); ++iter) {
		if (iter->seq != seq) {
			continue;
		}
		// Found the entry, record the time
		const long int rtt =
		   std::chrono::duration_cast<std::chrono::milliseconds>(now - iter->timestamp).count();
		if (rtt <= 255) {
			client.rtts.push_back(rtt);
		} else {
			client.rtts.push_back(255);
		}
		while (client.rtts.size() > MEASUREMENT_COUNT) {
			client.rtts.pop_front();
		}
		client.open_requests.erase(iter);
		break;
	}
}

void PingTracker::register_rtt(size_t clientnum, uint8_t rtt) {
	close_old_entries(clientnum);
	ClientData& client = clients_[clientnum];
	client.rtts.push_back(rtt);
	while (client.rtts.size() > MEASUREMENT_COUNT) {
		client.rtts.pop_front();
	}
}

uint8_t PingTracker::get_rtt(size_t clientnum) {
	close_old_entries(clientnum);
	const auto client = clients_.find(clientnum);
	if (client == clients_.end()) {
		// Client is unknown, return the maximum value.
		// Probably it is so new that there haven't been any pings yet
		return 255;
	}
	const size_t size = client->second.rtts.size();
	if (size == 0) {
		// No entries registered yet
		return 255;
	}
	// Calculate and return a floating average over the last measurements
	// The algorithm used is pretty arbitrary chosen, feel free to replace it
	assert(size <= MEASUREMENT_COUNT);
	uint16_t sum = 0;
	size_t i = 0;
	size_t sum_weights = 0;
	for (const uint8_t rtt : client->second.rtts) {
		const size_t weight = size - i;
		sum_weights += weight;
		sum += rtt * weight;
		++i;
	}
	sum /= sum_weights;
	assert(sum <= 255);
	return sum;
}

void PingTracker::close_old_entries(size_t clientnum) {
	// Clear up old entries
	ClientData& client = clients_[clientnum];
	const auto now = std::chrono::high_resolution_clock::now();
	while (!client.open_requests.empty()) {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(
		       now - client.open_requests.front().timestamp)
		       .count() >= 255) {
			// This entry is too old. Drop it and add the timeout to the rtt list
			client.open_requests.pop_front();
			client.rtts.push_back(255);
			while (client.rtts.size() > MEASUREMENT_COUNT) {
				client.rtts.pop_front();
			}
		} else {
			// This entry has been added recent enough.
			// Since all further entries are even newer, break
			break;
		}
	}
}
