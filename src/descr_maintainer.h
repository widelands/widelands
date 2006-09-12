/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__DESCR_MAINTAINER_H
#define __S__DESCR_MAINTAINER_H

#include "building.h"
#include "ware.h"

/**
 * This template is used to have a typesafe mainting class for Bob_Descr,
 * Worker_Descr and so on
 */
template <class T> class Descr_Maintainer {
public:

	typedef Uint16 Index;

	Descr_Maintainer() : nitems(0), place_for(0), items(0) {}
	~Descr_Maintainer();

	T * exists(const char * const name) const;
	Index get_nitems() const {return nitems;}

	struct Nonexistent {};

	Index add(T * const item) {
		const Index result = nitems;
		++nitems;
		if (nitems >= place_for) reserve(nitems);
		items[result] = item;
		return result;
	}

	Index get_index(const char * const name) const {
		for (Index i = 0; i < nitems; ++i)
			if(not strcasecmp(name, items[i]->get_name())) return i;
		throw Nonexistent();
	}

	void reserve(const Index n) {
		items = static_cast<T * * const>(realloc(items, sizeof(T *) * n));
		place_for = n;
	}

	T * get(const Index i) const {return i < nitems ? items[i] : 0;}

private:
	Index nitems;
	Index place_for;
	T** items;

	Descr_Maintainer & operator=(const Descr_Maintainer &);
	Descr_Maintainer            (const Descr_Maintainer &);
};

//
//returns elemt if it exists, 0 if it doesnt
//
template <class T>
T* Descr_Maintainer<T>::exists(const char * const name) const {
   for (Index i = 0; i < nitems; ++i) {
      if(!strcasecmp(name, items[i]->get_name())) return items[i];
   }
   return 0;
}

template <class T>
Descr_Maintainer<T>::~Descr_Maintainer() {
	for (Index i = 0; i < nitems; ++i) delete items[i];
   free(items);
}

#endif
