/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_ECONOMY_TRACKPTR_H
#define WL_ECONOMY_TRACKPTR_H

#include <cassert>
#include <cstdint>

#include "base/macros.h"

class BaseTrackPtr;

/*
Trackable must be the base class of any class that is used in
conjunction with TrackPtr.
The class deriving from Trackable needs no special logic.

Trackables work like this: there's a Tracker object (allocated on the heap)
for every Trackable. It contains a pointer to the Trackable and it keeps
track of how many TrackPtrs are referencing it.
The Trackable destructor notifies the Tracker of its destruction, but it will
not delete the Tracker itself: the Tracker will stick around until it is no
longer referenced by any TrackPtrs.
*/
class Trackable {
	friend class BaseTrackPtr;

	class Tracker {
		uint32_t refcount_;
		Trackable* ptr_;

	public:
		explicit Tracker(Trackable* const p) : refcount_(0), ptr_(p) {
		}

		void addref() {
			++refcount_;
			assert(refcount_ > 0);
		}
		void deref() {
			assert(refcount_ > 0);
			--refcount_;
			if (!refcount_ && !ptr_)
				delete this;
		}
		void clear() {
			ptr_ = nullptr;
			if (!refcount_)
				delete this;
		}

		Trackable* get() {
			return ptr_;
		}

		//  Putting "private:" here causes a compiler warning, even though we use
		//  delete this.
	protected:
		~Tracker() {
		}
	};

public:
	Trackable() {
		tracker_ = new Tracker(this);
	}

	DISALLOW_COPY_AND_ASSIGN(Trackable);

	virtual ~Trackable() {
		tracker_->clear();
	}

private:
	Tracker* tracker_;
};

/*
BaseTrackPtr cannot be used directly. It contains the bookkeeping logic
for TrackPtrs.
TrackPtr is a template that derives from BaseTrackPtr and provides a
type-safe interface.
*/
class BaseTrackPtr {
	mutable Trackable::Tracker* tracker_;

protected:
	BaseTrackPtr() : tracker_(nullptr) {
	}
	~BaseTrackPtr() {
		if (tracker_)
			tracker_->deref();
	}
	explicit BaseTrackPtr(Trackable* const t) {
		if (t) {
			tracker_ = t->tracker_;
			tracker_->addref();
		} else
			tracker_ = nullptr;
	}
	BaseTrackPtr(const BaseTrackPtr& o) {
		tracker_ = o.tracker_;
		if (tracker_)
			tracker_->addref();
	}

	void set(const BaseTrackPtr& o) {
		if (tracker_)
			tracker_->deref();

		tracker_ = o.tracker_;
		if (tracker_)
			tracker_->addref();
	}

	void set(Trackable* const t) {
		if (tracker_)
			tracker_->deref();

		if (t) {
			tracker_ = t->tracker_;
			tracker_->addref();
		} else
			tracker_ = nullptr;
	}

	Trackable* get() const {
		if (tracker_) {
			if (Trackable* const t = tracker_->get())
				return t;

			tracker_->deref();
			tracker_ = nullptr;
		}

		return nullptr;
	}
};

/*
A TrackPtr is a smart pointer to a T*.
It automatically becomes 0 when the object it points to is deleted.
Class T, i.e. the object that the TrackPtr points to, must be a class
derived from Trackable.
*/
template <class T> struct TrackPtr : BaseTrackPtr {
	TrackPtr() = default;

	explicit TrackPtr(T* ptr) : BaseTrackPtr(ptr) {
	}
	explicit TrackPtr(const TrackPtr<T>& o) : BaseTrackPtr(o) {
	}

	TrackPtr& operator=(const TrackPtr<T>& o) {
		set(o);
		return *this;
	}
	TrackPtr& operator=(T* const p) {
		set(p);
		return *this;
	}

	operator T*() const {
		return static_cast<T*>(get());
	}
	T* operator->() const {
		return static_cast<T*>(get());
	}
};

#endif  // end of include guard: WL_ECONOMY_TRACKPTR_H
