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

#ifndef UI_SIGNAL_H
#define UI_SIGNAL_H

#include "ui_object.h"

namespace UI {
/**
 * Provides a hook for callback function.
 * This is exactly what register_func used to provide but for Panel
 * member functions and with better type checking.
 *
 * Use as:
 *      Signal signal;
 *      Signal1<int32_t> signal1;
 *
 *      foo->signal.set(this, &MyClass::Handler);
 *      signal.call();
 *      signal1.call(some_int);
 */
class Signal : public Object {
	typedef void (Object::*fnT)();
	Object *_obj;
	fnT _fn;
public:
	Signal() {_obj = 0; _fn = 0;}
	template<class T>
		void set(Object *p, void (T::*f)()) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	void unset() {
      _fn = 0;
	}
   inline void call() {if (_fn) (_obj->*_fn)();}
};

/**
 * See Signal
 */
template<class T1>
class Signal1 : public Object {
	typedef void (Object::*fnT)(T1);
	Object *_obj;
	fnT _fn;
public:
	Signal1() {_obj = 0; _fn = 0;}
	template<class T>
	void set(Object *p, void (T::*f)(T1)) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	void unset() {
      _fn = 0;
	}
	inline void call(T1 t1) {if (_fn) (_obj->*_fn)(t1);}
};

/**
 * See Signal
 */
template<class T1, class T2>
class Signal2 : public Object {
	typedef void (Object::*fnT)(T1, T2);
	Object *_obj;
	fnT _fn;
public:
	Signal2() {_obj = 0; _fn = 0;}
	template<class T>
	void set(Object * p, void (T::*f)(T1, T2)) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	void unset() {
      _fn = 0;
	}
   inline void call(T1 t1, T2 t2) {if (_fn) (_obj->*_fn)(t1, t2);}
};
};

#endif /* UI_SIGNAL_H */
