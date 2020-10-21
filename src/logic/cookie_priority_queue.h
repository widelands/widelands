/*
 * Copyright (C) 2010-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_COOKIE_PRIORITY_QUEUE_H
#define WL_LOGIC_COOKIE_PRIORITY_QUEUE_H

#include <cassert>
#include <functional>
#include <limits>
#include <vector>

#include "base/macros.h"
#include "logic/map_objects/tribes/wareworker.h"

template <typename _Type> struct DefaultCookieAccessor;

template <typename _Type> struct CookiePriorityQueueBase {
	using CookieType = _Type;
	using CookieTypeVector = std::vector<CookieType*>;
	using CookieSizeType = typename CookieTypeVector::size_type;

	struct Cookie {
		Cookie() : pos(bad_pos()) {
		}
		~Cookie() {
		}

		/** Returns \c true if the cookie is currently managed by a queue */
		bool is_active() const {
			return pos != bad_pos();
		}

	private:
		friend struct CookiePriorityQueueBase<_Type>;

		CookieSizeType pos;

		DISALLOW_COPY_AND_ASSIGN(Cookie);
	};

protected:
	static CookieSizeType& cookie_pos(Cookie& cookie) {
		return cookie.pos;
	}
	static CookieSizeType bad_pos() {
		return std::numeric_limits<CookieSizeType>::max();
	}
};

/**
 * A priority queue (heap) with back-links called cookies to allow
 * key modification in logarithmic time.
 *
 * Since elements are treated as objects with an identity, we store pointers
 * unlike the usual STL container behaviour.
 *
 * The comparison type must act like a "stricly less than" comparison.
 * The "strictly" part is important.
 *
 * \ref top returns an element such that no other element in the container
 * is strictly less than the element returned by \ref top
 *
 * If an element's position with respect to the ordering is changed,
 * you have to call \ref decrease_key or \ref increase_key after the change,
 * depending on whether the change caused a decrease or increase, respectively.
 */
template <typename _Type,
          typename _Compare = std::less<_Type>,
          typename _CookieAccessor = DefaultCookieAccessor<_Type>>
struct CookiePriorityQueue : CookiePriorityQueueBase<_Type> {
	using CookieType = typename CookiePriorityQueueBase<_Type>::CookieType;
	using CookieTypeVector = typename CookiePriorityQueueBase<_Type>::CookieTypeVector;
	using CookieSizeType = typename CookiePriorityQueueBase<_Type>::CookieSizeType;
	using Cookie = typename CookiePriorityQueueBase<_Type>::Cookie;

	using BaseType = CookiePriorityQueueBase<_Type>;
	using Compare = _Compare;
	using CookieAccessor = _CookieAccessor;
	explicit CookiePriorityQueue(Widelands::WareWorker type,
	                             const Compare& comparator = Compare(),
	                             const CookieAccessor& accessor = CookieAccessor());
	~CookiePriorityQueue();

	CookieSizeType size() const;
	bool empty() const;
	CookieType* top() const;

	void push(CookieType* elt);
	void pop(CookieType* elt);
	void decrease_key(CookieType* elt);
	void increase_key(CookieType* elt);

	Widelands::WareWorker type() const {
		return type_;
	}

private:
	Widelands::WareWorker type_;

	CookieTypeVector d;
	Compare c;
	CookieAccessor ca;

	void swap(Cookie& a, Cookie& b);
	void selftest();

	static CookieSizeType& cookie_pos(Cookie& c) {
		return BaseType::cookie_pos(c);
	}
	static CookieSizeType bad_pos() {
		return BaseType::bad_pos();
	}
	static CookieSizeType parent_pos(CookieSizeType pos) {
		return (pos - 1) / 2;
	}
	static CookieSizeType left_child_pos(CookieSizeType pos) {
		return (2 * pos) + 1;
	}
	static CookieSizeType right_child_pos(CookieSizeType pos) {
		return (2 * pos) + 2;
	}
};

template <typename _Type> struct DefaultCookieAccessor {
	using Cookie = typename CookiePriorityQueueBase<_Type>::Cookie;

	Cookie& operator()(_Type* t, Widelands::WareWorker type) {
		return t->cookie(type);
	}
};

template <typename _T, typename _Cw, typename _CA>
CookiePriorityQueue<_T, _Cw, _CA>::CookiePriorityQueue(
   Widelands::WareWorker wwtype,
   const typename CookiePriorityQueue<_T, _Cw, _CA>::Compare& comparator,
   const typename CookiePriorityQueue<_T, _Cw, _CA>::CookieAccessor& accessor)
   : type_(wwtype), c(comparator), ca(accessor) {
}

template <typename _T, typename _Cw, typename _CA>
CookiePriorityQueue<_T, _Cw, _CA>::~CookiePriorityQueue() {
	for (typename CookieTypeVector::iterator it = d.begin(); it != d.end(); ++it)
		cookie_pos(ca(*it, type_)) = bad_pos();
}

template <typename _T, typename _Cw, typename _CA>
typename CookiePriorityQueue<_T, _Cw, _CA>::CookieSizeType
CookiePriorityQueue<_T, _Cw, _CA>::size() const {
	return d.size();
}

template <typename _T, typename _Cw, typename _CA>
bool CookiePriorityQueue<_T, _Cw, _CA>::empty() const {
	return d.empty();
}

template <typename _T, typename _Cw, typename _CA>
typename CookiePriorityQueue<_T, _Cw, _CA>::CookieType*
CookiePriorityQueue<_T, _Cw, _CA>::top() const {
	return *d.begin();
}

template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::push(
   typename CookiePriorityQueue<_T, _Cw, _CA>::CookieType* elt) {
	Cookie& elt_cookie(ca(elt, type_));

	assert(cookie_pos(elt_cookie) == BaseType::bad_pos());

	d.push_back(elt);
	cookie_pos(elt_cookie) = d.size() - 1;

	decrease_key(elt);
}

template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::pop(
   typename CookiePriorityQueue<_T, _Cw, _CA>::CookieType* elt) {
	Cookie& elt_cookie(ca(elt, type_));

	assert(cookie_pos(elt_cookie) < d.size());

	while (cookie_pos(elt_cookie) > 0) {
		Cookie& parent_cookie = ca(*(d.begin() + parent_pos(cookie_pos(elt_cookie))), type_);

		assert(cookie_pos(parent_cookie) == parent_pos(cookie_pos(elt_cookie)));

		swap(elt_cookie, parent_cookie);
	}

	swap(elt_cookie, ca(d.back(), type_));
	d.pop_back();
	cookie_pos(elt_cookie) = BaseType::bad_pos();

	if (!d.empty())
		increase_key(*d.begin());
}

template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::decrease_key(
   typename CookiePriorityQueue<_T, _Cw, _CA>::CookieType* elt) {
	Cookie& elt_cookie(ca(elt, type_));

	assert(cookie_pos(elt_cookie) < d.size());

	while (cookie_pos(elt_cookie) != 0) {
		CookieSizeType parent = parent_pos(cookie_pos(elt_cookie));

		if (!c(*elt, *d[parent], type_))
			break;

		Cookie& parent_cookie(ca(d[parent], type_));

		assert(cookie_pos(parent_cookie) == parent);

		swap(elt_cookie, parent_cookie);
	}

#ifndef NDEBUG
	selftest();
#endif
}

template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::increase_key(
   typename CookiePriorityQueue<_T, _Cw, _CA>::CookieType* elt) {
	Cookie& elt_cookie(ca(elt, type_));

	assert(cookie_pos(elt_cookie) < d.size());

	for (;;) {
		CookieSizeType left_child = left_child_pos(cookie_pos(elt_cookie));
		CookieSizeType right_child = right_child_pos(cookie_pos(elt_cookie));

		if (left_child >= d.size())
			break;

		Cookie& left_cookie(ca(d[left_child], type_));

		assert(cookie_pos(left_cookie) == left_child);

		if (c(**(d.begin() + left_child), *elt, type_)) {
			if (right_child >= d.size() || c(*d[left_child], *d[right_child], type_)) {
				swap(elt_cookie, left_cookie);
				continue;
			}
		}

		if (right_child >= d.size())
			break;

		Cookie& right_cookie(ca(d[right_child], type_));

		assert(cookie_pos(right_cookie) == right_child);

		if (c(*d[right_child], *elt, type_)) {
			swap(elt_cookie, right_cookie);
			continue;
		}

		break;
	}
#ifndef NDEBUG
	selftest();
#endif
}

template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::swap(
   typename CookiePriorityQueue<_T, _Cw, _CA>::Cookie& a,
   typename CookiePriorityQueue<_T, _Cw, _CA>::Cookie& b) {
	std::swap(d[cookie_pos(a)], d[cookie_pos(b)]);
	std::swap(cookie_pos(a), cookie_pos(b));
}

// Disable in release builds.
template <typename _T, typename _Cw, typename _CA>
void CookiePriorityQueue<_T, _Cw, _CA>::selftest() {
	for (CookieSizeType pos = 0; pos < d.size(); ++pos) {
		Cookie& elt_cookie(ca(d[pos], type_));

		assert(cookie_pos(elt_cookie) == pos);

		CookieSizeType left_child = left_child_pos(pos);
		CookieSizeType right_child = right_child_pos(pos);

		if (left_child < d.size()) {
			assert(!c(*d[left_child], *d[pos], type_));
		}

		if (right_child < d.size()) {
			assert(!c(*d[right_child], *d[pos], type_));
		}
	}
}

#endif  // end of include guard: WL_LOGIC_COOKIE_PRIORITY_QUEUE_H
