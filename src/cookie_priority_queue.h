/*
 * Copyright (C) 2010, 2012 by the Widelands Development Team
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

#ifndef COOKIE_PRIORITY_QUEUE_H
#define COOKIE_PRIORITY_QUEUE_H

#include <cassert>
#include <functional>
#include <limits>
#include <vector>

#include <boost/noncopyable.hpp>

template<typename _Type>
struct default_cookie_accessor;

template<typename _Type>
struct cookie_priority_queue_base {
	typedef _Type type;
	typedef std::vector<type *> container;
	typedef typename container::size_type size_type;

	struct cookie : boost::noncopyable {
		cookie() : pos(bad_pos()) {}
		~cookie() {}

		/** Returns \c true if the cookie is currently managed by a queue */
		bool is_active() const {return pos != bad_pos();}

	private:
		friend struct cookie_priority_queue_base<_Type>;

		size_type pos;
	};

protected:
	static size_type & cookie_pos(cookie & cookie) {return cookie.pos;}
	static size_type bad_pos() {return std::numeric_limits<size_type>::max();}
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
template
	<typename _Type,
	 typename _Compare = std::less<_Type>,
	 typename _CookieAccessor = default_cookie_accessor<_Type> >
struct cookie_priority_queue : cookie_priority_queue_base<_Type> {
	typedef typename cookie_priority_queue_base<_Type>::type type;
	typedef typename cookie_priority_queue_base<_Type>::container container;
	typedef typename cookie_priority_queue_base<_Type>::size_type size_type;
	typedef typename cookie_priority_queue_base<_Type>::cookie cookie;

	typedef cookie_priority_queue_base<_Type> base_type;
	typedef _Compare compare;
	typedef _CookieAccessor cookie_accessor;

	cookie_priority_queue(const compare & _c = compare(), const cookie_accessor & _a = cookie_accessor());
	~cookie_priority_queue();

	size_type size() const;
	bool empty() const;
	type * top() const;

	void push(type * elt);
	void pop(type * elt);
	void decrease_key(type * elt);
	void increase_key(type * elt);

private:
	container d;
	compare c;
	cookie_accessor ca;

	void swap(cookie & a, cookie & b);
	void selftest();

	static size_type & cookie_pos(cookie & c) {return base_type::cookie_pos(c);}
	static size_type bad_pos() {return base_type::bad_pos();}
	static size_type parent_pos(size_type pos) {return (pos - 1) / 2;}
	static size_type left_child_pos(size_type pos) {return (2 * pos) + 1;}
	static size_type right_child_pos(size_type pos) {return (2 * pos) + 2;}
};

template<typename _Type>
struct default_cookie_accessor {
	typedef typename cookie_priority_queue_base<_Type>::cookie cookie;

	cookie & operator()(_Type * t) {
		return t->cookie();
	}
};


template<typename _T, typename _Cw, typename _CA>
cookie_priority_queue<_T, _Cw, _CA>::cookie_priority_queue
	(const typename cookie_priority_queue<_T, _Cw, _CA>::compare & _c,
	 const typename cookie_priority_queue<_T, _Cw, _CA>::cookie_accessor & _a)
: c(_c), ca(_a)
{
}

template<typename _T, typename _Cw, typename _CA>
cookie_priority_queue<_T, _Cw, _CA>::~cookie_priority_queue()
{
	for (typename container::iterator it = d.begin(); it != d.end(); ++it)
		cookie_pos(ca(*it)) = bad_pos();
}

template<typename _T, typename _Cw, typename _CA>
typename cookie_priority_queue<_T, _Cw, _CA>::size_type cookie_priority_queue<_T, _Cw, _CA>::size() const
{
	return d.size();
}

template<typename _T, typename _Cw, typename _CA>
bool cookie_priority_queue<_T, _Cw, _CA>::empty() const
{
	return d.empty();
}

template<typename _T, typename _Cw, typename _CA>
typename cookie_priority_queue<_T, _Cw, _CA>::type * cookie_priority_queue<_T, _Cw, _CA>::top() const
{
	return *d.begin();
}

template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::push
	(typename cookie_priority_queue<_T, _Cw, _CA>::type * elt)
{
	cookie & elt_cookie(ca(elt));

	assert(cookie_pos(elt_cookie) == base_type::bad_pos());

	d.push_back(elt);
	cookie_pos(elt_cookie) = d.size() - 1;

	decrease_key(elt);
}

template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::pop
	(typename cookie_priority_queue<_T, _Cw, _CA>::type * elt)
{
	cookie & elt_cookie(ca(elt));

	assert(cookie_pos(elt_cookie) < d.size());

	while (cookie_pos(elt_cookie) > 0) {
		cookie & parent_cookie = ca(*(d.begin() + parent_pos(cookie_pos(elt_cookie))));

		assert(cookie_pos(parent_cookie) == parent_pos(cookie_pos(elt_cookie)));

		swap(elt_cookie, parent_cookie);
	}

	swap(elt_cookie, ca(d.back()));
	d.pop_back();
	cookie_pos(elt_cookie) = base_type::bad_pos();

	if (!d.empty())
		increase_key(*d.begin());
}

template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::decrease_key
	(typename cookie_priority_queue<_T, _Cw, _CA>::type * elt)
{
	cookie & elt_cookie(ca(elt));

	assert(cookie_pos(elt_cookie) < d.size());

	while (cookie_pos(elt_cookie) != 0) {
		size_type parent = parent_pos(cookie_pos(elt_cookie));

		if (!c(*elt, *d[parent]))
			break;

		cookie & parent_cookie(ca(d[parent]));

		assert(cookie_pos(parent_cookie) == parent);

		swap(elt_cookie, parent_cookie);
	}

	//selftest();
}

template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::increase_key
	(typename cookie_priority_queue<_T, _Cw, _CA>::type * elt)
{
	cookie & elt_cookie(ca(elt));

	assert(cookie_pos(elt_cookie) < d.size());

	for (;;) {
		size_type left_child = left_child_pos(cookie_pos(elt_cookie));
		size_type right_child = right_child_pos(cookie_pos(elt_cookie));

		if (left_child >= d.size())
			break;

		cookie & left_cookie(ca(d[left_child]));

		assert(cookie_pos(left_cookie) == left_child);

		if (c(**(d.begin() + left_child), *elt)) {
			if (right_child >= d.size() || c(*d[left_child], *d[right_child])) {
				swap(elt_cookie, left_cookie);
				continue;
			}
		}

		if (right_child >= d.size())
			break;

		cookie & right_cookie(ca(d[right_child]));

		assert(cookie_pos(right_cookie) == right_child);

		if (c(*d[right_child], *elt)) {
			swap(elt_cookie, right_cookie);
			continue;
		}

		break;
	}

	//selftest();
}

template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::swap
	(typename cookie_priority_queue<_T, _Cw, _CA>::cookie & a,
	 typename cookie_priority_queue<_T, _Cw, _CA>::cookie & b)
{
	std::swap(d[cookie_pos(a)], d[cookie_pos(b)]);
	std::swap(cookie_pos(a), cookie_pos(b));
}

// Disable in release builds.
template<typename _T, typename _Cw, typename _CA>
void cookie_priority_queue<_T, _Cw, _CA>::selftest()
{
	for (size_type pos = 0; pos < d.size(); ++pos) {
		cookie & elt_cookie(ca(d[pos]));

		assert(cookie_pos(elt_cookie) == pos);

		size_type left_child = left_child_pos(pos);
		size_type right_child = right_child_pos(pos);

		if (left_child < d.size()) {
			assert(!c(*d[left_child], *d[pos]));
		}

		if (right_child < d.size()) {
			assert(!c(*d[right_child], *d[pos]));
		}
	}
}

#endif // COOKIE_PRIORITY_QUEUE_H
