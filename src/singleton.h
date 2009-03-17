/*
 * Copyright (C) 2002, 2008 by the Widelands Development Team
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

#ifndef SINGLETON_H
#define SINGLETON_H


#include <cassert>

/* The original code seems to have come from Game Programming Gems I, P. 38
 * but the current version probably doesn't really resemble it any more.
 */

/// Singleton class, to be used in
///
/// class SomeClass : public Singleton<SomeClass> {
///   etc.
/// };
///
/// The singleton must be explicitly instantiated using
///  new SomeClass;
/// and explicitly deleted using
///  delete SomeClass::get_ptsingleton();
///
/// \note There is a weakness here because the implementation doesn't
/// enforce that T equals the derived class. Unfortunately, using dynamic_cast
/// (or any other kind of RTTI) in the constructor doesn't work.
template <typename T> class Singleton {
	static T * ms;

protected:
	Singleton() {
		assert(!ms);
		ms = static_cast<T *>(this);
	}

	~Singleton() {
		assert(ms);
		ms = 0;
	}

public:
	static T & get_singleton  () {assert(ms); return *ms;}
	static T * get_ptsingleton() {assert(ms); return  ms;}
};

template <typename T> T * Singleton <T>::ms = 0;


#endif
