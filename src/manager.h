/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#ifndef MANAGER_H
#define MANAGER_H

#include "container_iterate.h"

#include <cassert>
#include <vector>

/// Manages items of type T. Takes ownership of them. Assumes that they can be
/// deallocated with operator delete, so only register items that were
/// allocated with operator new.
template<typename T> struct Manager {
	~Manager() {remove_all();}

	struct Already_Exists {};
	struct Nonexistent    {};

	/// \Throws Already_Exists if an item with the same name is already
	/// registered. The item will be deallocated before throwing.
	void register_new(T & new_item) {
		if (operator[](new_item.name())) {
			delete &new_item;
			throw Already_Exists();
		}
		items.push_back(&new_item);
	}

	/// Removes all items.
	void remove_all() {
		container_iterate_const(typename container, items, i)
			delete *i.current;
		items.clear();
	}

	/// Removes the item.
	///
	/// \Throws Nonexistent if there is no such item.
	void remove(T & item) {
		container_iterate(typename container, items, i)
			if (*i.current == &item) {
				delete &item;
				*i.current = *(i.end - 1);
				items.pop_back();
				return;
			}
		throw Nonexistent();
	}

	///  Removes the item with the given name.
	///
	/// \Throws Nonexistent if there is no such item.
	void remove(char const * const name) {
		container_iterate(typename container, items, i)
			if (not strcmp((*i.current)->name().c_str(), name)) {
				delete *i.current;
				*i.current = *(i.end - 1);
				items.pop_back();
				return;
			}
		throw Nonexistent();
	}
	void remove(std::string const & name) {
		container_iterate(typename container, items, i)
			if (name == (*i.current)->name()) {
				delete *i.current;
				*i.current = *(i.end - 1);
				items.pop_back();
				return;
			}
		throw Nonexistent();
	}

	/// Removes all unreferenced items. Note that it does not remove groups of
	/// items that reference each other but are unreferenced from the outside.
	void remove_unreferenced() {
		Index items_size = size();
		for (Index i = items_size; i;)
			if (items[--i]->referencers().empty()) {
				delete items[i];
				items[i] = items.back();
				items.pop_back();
				--items_size;
				i = items_size;
			}
	}

	typedef std::vector<T *> container;
	typedef typename container::size_type Index;
	Index size() const {return items.size();}
	T const & operator[](Index const i) const {
		assert(i < size());
		return *items[i];
	}
	T       & operator[](Index const i)       {
		assert(i < size());
		return *items[i];
	}
	T const * operator[](char const * const name) const {
		container_iterate_const(typename container, items, i)
			if (not strcmp((*i.current)->name().c_str(), name))
				return *i.current;
		return 0;
	}
	T       * operator[](char const * const name)       {
		container_iterate_const(typename container, items, i)
			if (not strcmp((*i.current)->name().c_str(), name))
				return *i.current;
		return 0;
	}
	T const * operator[](std::string const & name) const {
		container_iterate_const(typename container, items, i)
			if (name == (*i.current)->name())
				return *i.current;
		return 0;
	}
	T       * operator[](std::string const & name)       {
		container_iterate_const(typename container, items, i)
			if (name == (*i.current)->name())
				return *i.current;
		return 0;
	}

private:
	container items;
};

#endif
