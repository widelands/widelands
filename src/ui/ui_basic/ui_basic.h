/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __S__UI_H
#define __S__UI_H

#include "graphic.h"
#include "font.h"


/** 
 * Serves as a base class for UI related objects. The only purpose is
 * to provide the base class for signal function pointers.
 */
class UIObject {
public:
	// Yeah right... force a VMT so that MSVC++ gets the pointers-to-members
	// right *sigh*
	// OTOH, looking at the gcc assembly, gcc seems to use a less efficient
	// pointer representation. Can anyone clear this up? -- Nicolai
	virtual ~UIObject() { }
};

/**
 * Provides a hook for callback function.
 * This is exactly what register_func used to provide but for UIPanel
 * member functions and with better type checking.
 *
 * Use as:
 *		UISignal signal;
 *		UISignal1<int> signal1;
 *
 *		foo->signal.set(this, &MyClass::Handler);
 *		signal.call();
 *		signal1.call(some_int);
 */
class UISignal : public UIObject {
	typedef void (UIObject::*fnT)();
	UIObject *_obj;
	fnT _fn;
public:
	UISignal() { _obj = 0; _fn = 0; }
	template<class T>
	void set(UIObject *p, void (T::*f)()) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	inline void call() { if (_fn) (_obj->*_fn)(); }
};

/**
 * See UISignal
 */
template<class T1>
class UISignal1 : public UIObject {
	typedef void (UIObject::*fnT)(T1);
	UIObject *_obj;
	fnT _fn;
public:
	UISignal1() { _obj = 0; _fn = 0; }
	template<class T>
	void set(UIObject *p, void (T::*f)(T1)) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	inline void call(T1 t1) { if (_fn) (_obj->*_fn)(t1); }
};

/*
 * See UISignal
 */
template<class T1, class T2>
class UISignal2 : public UIObject {
	typedef void (UIObject::*fnT)(T1, T2);
	UIObject *_obj;
	fnT _fn;
public:
	UISignal2() { _obj = 0; _fn = 0; }
	template<class T>
	void set(UIObject *p, void (T::*f)(T1, T2)) {
		_obj = p;
		_fn = static_cast<fnT>(f);
	}
	inline void call(T1 t1, T2 t2) { if (_fn) (_obj->*_fn)(t1, t2); }
};

#include "ui_panel.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_radiobutton.h"
#include "ui_textarea.h"
#include "ui_scrollbar.h"
#include "ui_multilinetextarea.h"
#include "ui_listselect.h"
#include "ui_window.h"
#include "ui_unique_window.h"
#include "ui_icongrid.h"
#include "ui_editbox.h"
#include "ui_box.h"
#include "ui_tabpanel.h"
#include "ui_progressbar.h"

#endif /* __S__UI_H */
