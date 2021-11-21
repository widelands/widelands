/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_NOTIFICATIONS_SIGNAL_H
#define WL_NOTIFICATIONS_SIGNAL_H

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <set>

#include "base/macros.h"
#include "base/wexception.h"

namespace Notifications {

/** Class that allows attaching callback functions to an object. */
template <typename... Args> class Signal {
public:
	/** Wrapper around a callback function. */
	struct SignalSubscriber {
		explicit SignalSubscriber(const Signal& p, const std::function<void(Args...)>& c)
		   : parent_(p), callback_(c) {
		}
		~SignalSubscriber() {
			parent_.unsubscribe(this);
		}

		const Signal& parent_;
		const std::function<void(Args...)> callback_;

		DISALLOW_COPY_AND_ASSIGN(SignalSubscriber);
	};

	/** Invoke all the signal's subscribers' callback functions. */
	void operator()(Args... args) const {
		for (const auto& s : all_subscribers_) {
			s->callback_(args...);
		}
	}

	/** Create a subscriber with a user-defined lifetime. */
	std::unique_ptr<SignalSubscriber> subscribe(const std::function<void(Args...)>& callback,
	                                            bool at_front = false) const {
		SignalSubscriber* s = new SignalSubscriber(*this, callback);
		if (at_front) {
			all_subscribers_.push_front(s);
		} else {
			all_subscribers_.push_back(s);
		}
		return std::unique_ptr<SignalSubscriber>(s);
	}

	/** Create a subscriber with the same lifetime as the signal. */
	inline void connect(const std::function<void(Args...)>& callback, bool at_front = false) const {
		owned_subscribers_.insert(subscribe(callback, at_front));
	}

	/** Create a subscriber that echoes the signal's invokations to another signal. */
	inline std::unique_ptr<SignalSubscriber> subscribe(const Signal& s,
	                                                   bool at_front = false) const {
		return subscribe([&s](Args... args) { s(args...); }, at_front);
	}
	inline void connect(const Signal& s, bool at_front = false) const {
		connect([&s](Args... args) { s(args...); }, at_front);
	}

	/** Called by a subscriber at the end of its lifetime. */
	void unsubscribe(const SignalSubscriber* s) const {
		for (auto it = all_subscribers_.begin(); it != all_subscribers_.end(); ++it) {
			if (*it == s) {
				all_subscribers_.erase(it);
				return;
			}
		}
		NEVER_HERE();
	}

	Signal() = default;
	~Signal() {
		owned_subscribers_.clear();
		// Any subscribers not owned by us should have been destroyed by their owner by now
		assert(all_subscribers_.empty());
	}

private:
	mutable std::set<std::unique_ptr<SignalSubscriber>> owned_subscribers_;
	mutable std::list<SignalSubscriber*> all_subscribers_;

	DISALLOW_COPY_AND_ASSIGN(Signal);
};
}  // namespace Notifications

#endif  // end of include guard: WL_NOTIFICATIONS_SIGNAL_H
