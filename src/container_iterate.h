/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef CONTAINER_ITERATE_H
#define CONTAINER_ITERATE_H

#include "boost/range.hpp"

// helper for erasing element in range so that range stays valid.
// temporary variable ensures that end() is evaluated after erase().
template <class C> 
boost::sub_range<C>
wl_erase(C &c, typename C::iterator &w)
{
    C::iterator it = c.erase(w); 
    return boost::sub_range<C>(it , c.end());
}

template<class I> 
struct wl_index_range
{
    wl_index_range(I max) : current(I::First()), end(max) {}
    wl_index_range(const wl_index_range &r) : current(r.current), end(r.end) {}
    I current;
    I end;
    wl_index_range &operator++() {++current;return *this;}
    wl_index_range &operator++(int) {wl_index_range r(*this);++current;return r;}
    bool empty() const {return current==end;}
    operator bool() const
    {
        return empty() ? false: true;
    }
};
template<class T> 
struct wl_range
{
    wl_range(T beginIndex, T endIndex) : current(beginIndex), end(endIndex) {}
    wl_range(const wl_range &r) : current(r.current), end(r.end) {}
    wl_range(T &r) : current(r.begin()), end(r.end()) {}
    T current;
    T end;
    wl_range &operator++() {++current;return *this;}
    wl_range &operator++(int) {wl_range r(*this);++current;return r;}
    bool empty() const {return current==end;}
    operator bool() const
    {
        return empty() ? false: true;
    }
};

template<class T1, class T2=T1 const> struct wl_iterator_helper
{
    wl_iterator_helper(T1 b, T2 e) : current(b), end(e){}
      T1 current;
      T2 end;
};
/*
template<class C> struct wl_range
{
    wl_range(C::iterator b, C::const_iterator e) : current(b), end(e){}
    C::iterator current;
    C::iterator   T2 end;
};
*/

#define container_iterate_const_noinc(type, container, i)                           \
   for                                                                        \
   (wl_iterator_helper<type::const_iterator, type::const_iterator const> i((container).begin(), (container).end());\
       i.current != i.end;)                                                           \

#define container_iterate_const_init(type, container, i)                           \
   for                                                                        \
   (wl_iterator_helper<type::const_iterator, type::const_iterator const> \
    i((container).begin(), (container).end());;)\

#define container_iterate_const(type, container, i)                           \
   for                                                                        \
   (wl_iterator_helper<type::const_iterator, type::const_iterator const> i((container).begin(), (container).end());\
       i.current != i.end;                                                    \
       ++i.current)                                                           \

#define container_iterate(type, container, i)                                 \
   for                                                                        \
      (wl_iterator_helper<type::iterator, type::const_iterator const> i((container).begin(), (container).end());\
       i.current != i.end;                                                    \
       ++i.current)                                                           \

#endif
