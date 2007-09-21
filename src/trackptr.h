/*
 * Copyright (C) 2004 by the Widelands Development Team
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

#ifndef included_trackptr_h
#define included_trackptr_h

#include <stdint.h>

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
		uint32_t        m_refcount;
		Trackable * m_ptr;

	public:
		Tracker(Trackable* p) : m_refcount(0), m_ptr(p) {}

		void addref() {
			++m_refcount;
			assert(m_refcount > 0);
		}
		void deref() {
			assert(m_refcount > 0);
			--m_refcount;
			if (!m_refcount && !m_ptr)
				delete this;
		}
		void clear() {
			m_ptr = 0;
			if (!m_refcount)
				delete this;
		}

		Trackable* get() {return m_ptr;}

	protected: // putting private here causes a compiler warning, even though
	           // we use delete this
		~Tracker() {}
	};

public:
	Trackable() {m_tracker = new Tracker(this);}
	virtual ~Trackable() {m_tracker->clear();}

private:
	Tracker * m_tracker;
};


/*
BaseTrackPtr cannot be used directly. It contains the bookkeeping logic
for TrackPtrs.
TrackPtr is a template that derives from BaseTrackPtr and provides a
type-safe interface.
*/
class BaseTrackPtr {
	mutable Trackable::Tracker * m_tracker;

protected:
	BaseTrackPtr() : m_tracker(0) {}
	~BaseTrackPtr() {
		if (m_tracker)
			m_tracker->deref();
	}
	BaseTrackPtr(Trackable* t) {
		if (t) {
			m_tracker = t->m_tracker;
			m_tracker->addref();
		} else
			m_tracker = 0;
	}
	BaseTrackPtr(const BaseTrackPtr& o) {
		m_tracker = o.m_tracker;
		if (m_tracker)
			m_tracker->addref();
	}

	void set(const BaseTrackPtr& o) {
		if (m_tracker)
			m_tracker->deref();

		m_tracker = o.m_tracker;
		if (m_tracker)
			m_tracker->addref();
	}

	void set(Trackable* t)
	{
		if (m_tracker)
			m_tracker->deref();

		if (t) {
			m_tracker = t->m_tracker;
			m_tracker->addref();
		} else
			m_tracker = 0;
	}

	Trackable* get() const
	{
		if (m_tracker) {
			Trackable* t = m_tracker->get();

			if (t)
				return t;

			m_tracker->deref();
			m_tracker = 0;
		}

		return 0;
	}
};


/*
A TrackPtr is a smart pointer to a T*.
It automatically becomes 0 when the object it points to is deleted.
Class T, i.e. the object that the TrackPtr points to, must be a class
derived from Trackable.
*/
template<class T>
class TrackPtr : BaseTrackPtr {
public:
	TrackPtr() {}
	~TrackPtr() {}

	TrackPtr(T* ptr) : BaseTrackPtr(ptr) {}
	TrackPtr(const TrackPtr<T>& o) : BaseTrackPtr(o) {}

	TrackPtr& operator=(const TrackPtr<T>& o) {set(o); return *this;}
	TrackPtr& operator=(T* ptr) {set(ptr); return *this;}

	operator T*() const {return static_cast<T*>(get());}
	T* operator->() const {return static_cast<T*>(get());}
};


#endif // included_trackptr_h
