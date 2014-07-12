/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_NOTIFICATIONS_NOTIFICATIONS_H
#define WL_NOTIFICATIONS_NOTIFICATIONS_H

#include <stdint.h>

// NOCOM(#sirver): cmake does not realize changes to this file.
#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <map> // NOCOM(#sirver): probably unordered_map

#include <boost/noncopyable.hpp>

#include "base/log.h" // NOCOM(#sirver): remove again

// NOCOM(#sirver): Subscriber helper class.

class Notifications : public boost::noncopyable {
public:
	static Notifications* get();

	// NOCOM(#sirver): return a id.
	template <typename T> uint32_t subscribe(std::function<void(const T&)> callback) {
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		// NOCOM(#sirver): add constructor and emplace_back
		Subscriber new_subscriber;
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		new_subscriber.id = next_subscriber_id_;
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		new_subscriber.callback = [&callback](const void* message) {callback(*static_cast<const T*>(message));};
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		++next_subscriber_id_;
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		note_id_to_subscribers_[T::kUniqueNoteId].push_back(new_subscriber);

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		return new_subscriber.id;
	}



	template <typename T> void send(const T& message) {
		for (Subscriber& subscriber : note_id_to_subscribers_[T::kUniqueNoteId]) {
			subscriber.callback(&message);
		}
	}

	template <typename T>
	void unsubscribe(uint32_t id) {
		std::list<Subscriber>& subscribers = note_id_to_subscribers_.at(T::kUniqueNoteId);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		auto subscribers_it = std::find_if(subscribers.begin(),
		                       subscribers.end(),
		                       [id](const Subscriber& subscriber) {return subscriber.id == id;});

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		assert (subscribers_it != subscribers.end());
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		subscribers.erase(subscribers_it);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	}

private:
	struct Subscriber {
		uint32_t id;
		std::function<void (const void*)> callback;
	};
	uint32_t next_subscriber_id_;
	// NOCOM(#sirver): multiple_subscribers
	std::map<uint32_t, std::list<Subscriber>>  note_id_to_subscribers_;
	// NOCOM(#sirver): second entry is really not nice.

	Notifications();
	~Notifications();
};


#endif  // end of include guard: WL_NOTIFICATIONS_NOTIFICATIONS_H
