/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

/**
 * Whether a new subscriber will be the last (default)
 * or first to be invoked when the signal is triggered.
 */
enum class SubscriberPosition { kFront, kBack };

/** Class that allows attaching callback functions to an object. */
template <typename... Args> class Signal {
public:
	/** Wrapper around a callback function. */
	class SignalSubscriber {
	public:
		explicit SignalSubscriber(const Signal& p) : parent_(p) {
		}
		~SignalSubscriber() {
			parent_.unsubscribe(this);
		}

	private:
		const Signal& parent_;
		DISALLOW_COPY_AND_ASSIGN(SignalSubscriber);
	};

	/** Invoke all the signal's subscribers' callback functions. */
	void operator()(Args... args) const {
		// Make a copy before iteration – a callback function may delete their
		// Subscriber or even the Signal itself, resulting in a heap-use-after-free.
		auto all = all_subscribers_;
		for (const auto& pair : all) {
			pair.second(args...);
		}
	}

	/**
	 * Create a subscriber with a user-defined lifetime.
	 * The caller must ensure that the subscriber is destroyed before the signal.
	 */
	std::unique_ptr<SignalSubscriber>
	subscribe(std::function<void(Args...)> callback,
	          SubscriberPosition pos = SubscriberPosition::kBack) const {
		SignalSubscriber* s = new SignalSubscriber(*this);
		switch (pos) {
		case SubscriberPosition::kBack:
			all_subscribers_.emplace_back(s, callback);
			break;
		case SubscriberPosition::kFront:
			all_subscribers_.emplace_front(s, callback);
			break;
		}
		return std::unique_ptr<SignalSubscriber>(s);
	}

	/** Create a subscriber with the same lifetime as the signal. */
	inline void connect(std::function<void(Args...)> callback,
	                    SubscriberPosition pos = SubscriberPosition::kBack) const {
		owned_subscribers_.insert(subscribe(callback, pos));
	}

	/**
	 * Create a subscriber that echoes the signal's invokations to another signal.
	 * This means that every invokation of this signal triggers the callback function
	 * of the signal `s` with the same arguments as passed to this signal.
	 *
	 * The caller is responsible for ensuring that `s` will not be destroyed before
	 * the last invokation of this signal, otherwise this will segfault.
	 * The caller is furthermore responsible for ensuring that no cycles are created.
	 */
	inline std::unique_ptr<SignalSubscriber>
	subscribe(const Signal& s, SubscriberPosition pos = SubscriberPosition::kBack) const {
		return subscribe([&s](Args... args) { s(args...); }, pos);
	}
	inline void connect(const Signal& s, SubscriberPosition pos = SubscriberPosition::kBack) const {
		connect([&s](Args... args) { s(args...); }, pos);
	}

	Signal() = default;
	~Signal() {
		owned_subscribers_.clear();
		// Any subscribers not owned by us should have been destroyed by their owner by now
		assert(all_subscribers_.empty());
	}

private:
	friend class SignalSubscriber;

	/** Called by a subscriber at the end of its lifetime. */
	void unsubscribe(const SignalSubscriber* s) const {
		for (auto it = all_subscribers_.begin(); it != all_subscribers_.end(); ++it) {
			if (it->first == s) {
				all_subscribers_.erase(it);
				return;
			}
		}
		NEVER_HERE();
	}

	/* A Signal instance can sometimes be a member of a const object, but we still want
	 * to allow new subscriptions to the object, so all member functions of Signal need
	 * to be const-qualified while still being able to add/remove subscribers to/from
	 * these lists. So these two member variables need to be declared mutable.
	 */
	mutable std::set<std::unique_ptr<SignalSubscriber>> owned_subscribers_;
	mutable std::list<std::pair<SignalSubscriber*, std::function<void(Args...)>>> all_subscribers_;

	DISALLOW_COPY_AND_ASSIGN(Signal);
};
}  // namespace Notifications

#endif  // end of include guard: WL_NOTIFICATIONS_SIGNAL_H
