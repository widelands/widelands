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
 * This is exactly what register_func used to provide but for Panel
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

/** class Statebox [virtual]
 *
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
#define STATEBOX_WIDTH 20
#define STATEBOX_HEIGHT 20

class Statebox : public Panel {
public:
	Statebox(Panel *parent, int x, int y, uint picid = 0);
	~Statebox();

	UISignal changed;
	UISignal1<bool> changedto;
   UISignal2<int,bool> changedtoid;

	void set_enabled(bool enabled);

	inline bool get_state() const { return m_state; }
	void set_state(bool on);

   inline void set_id(int n) { m_id=n; }

	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	virtual void clicked() = 0;

   int   m_id;

	bool	m_custom_picture;		// the statebox displays a custom picture
	uint	m_pic_graphics;

	bool	m_highlighted;
	bool	m_enabled; // true if the checkbox can be clicked
	bool	m_state; // true if the box is checked

	RGBColor	m_clr_state;		// color of border when checked (custom picture only)
	RGBColor	m_clr_highlight;	// color of border when highlighted
};


/** class Checkbox
 *
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 */
class Checkbox : public Statebox {
public:
	Checkbox(Panel *parent, int x, int y, int picid=0) : Statebox(parent, x, y, picid) { }

private:
	void clicked();
};


/** class Radiogroup
 *
 * A group of radiobuttons. At most one of them is checked at any time.
 * State is -1 if none is checked, otherwise it's the index of the checked button.
 */
class Radiobutton;

class Radiogroup {
	friend class Radiobutton;

public:
	Radiogroup();
	~Radiogroup();

	UISignal changed;
	UISignal1<int> changedto;
   UISignal clicked; // clicked without things changed

	int add_button(Panel* parent, int x, int y, uint picid = 0);

	inline int get_state() const { return m_state; }
	void set_state(int state);

private:
	Radiobutton*	m_buttons; // linked list of buttons (not sorted)
	int				m_highestid;
	int				m_state; // -1: none
};


/** class Textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 */
class Textarea : public Panel {
public:
	Textarea(Panel *parent, int x, int y, std::string text, Align align = Align_Left);
	Textarea(Panel *parent, int x, int y, int w, int h, std::string text,
			   Align align = Align_Left, bool multiline = false);
	~Textarea();

	void set_text(std::string text);
	void set_align(Align align);

	// Drawing and event handlers
	void draw(RenderTarget* dst);

private:
	void collapse();
	void expand();

	std::string		m_text;
	Align				m_align;
	bool				m_multiline;
};


/** class Scrollbar
 *
 * This class provides a scrollbar
 */
class Scrollbar : public Panel {
public:
	enum Area {
		None,
		Minus,
		Plus,
		Knob,
		MinusPage,
		PlusPage
	};

	enum {
		Size = 24,	// default width for vertical scrollbars, height for horizontal scrollbars
	};

public:
	Scrollbar(Panel *parent, int x, int y, uint w, uint h, bool horiz);

	UISignal1<int> moved;

	void set_steps(int steps);
	void set_pagesize(int pagesize);
	void set_pos(int pos);

	uint get_steps() const { return m_steps; }
	uint get_pagesize() const { return m_pagesize; }
	uint get_pos() const { return m_pos; }

private:
	Area get_area_for_point(int x, int y);
	int get_knob_pos();
	void set_knob_pos(int p);

	void action(Area area);

	void draw_button(RenderTarget* dst, Area area, int x, int y, int w, int h);
	void draw_area(RenderTarget* dst, Area area, int x, int y, int w, int h);
	void draw(RenderTarget* dst);
	void think();

	bool handle_mouseclick(uint btn, bool down, int x, int y);
	void handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns);

private:
	bool		m_horizontal;

	uint		m_pos;		// from 0 to m_range - 1
	uint		m_pagesize;
	uint		m_steps;

	Area		m_pressed;			// area that the user clicked on (None if mouse is up)
	int		m_time_nextact;
	int		m_knob_grabdelta;	// only while m_pressed == Knob

	uint		m_pic_minus;	// left/up
	uint		m_pic_plus;		// right/down
	uint		m_pic_background;
	uint		m_pic_buttons;
};


/** class Multiline_textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 */
class Multiline_Textarea : public Panel {
   public:
      enum ScrollMode {
         ScrollNormal = 0,    // (default) only explicit or forced scrolling
         ScrollLog = 1,       // follow the bottom of the text
      };

   public:
      Multiline_Textarea(Panel *parent, int x, int y, uint w, uint h, const char *text,
            Align align = Align_Left);
      ~Multiline_Textarea();

      std::string get_text() const { return m_text; }
      ScrollMode get_scrollmode() const { return m_scrollmode; }

      void set_text(const char *text);
      void set_align(Align align);
      void set_scrollpos(int pixels);
      void set_scrollmode(ScrollMode mode);

      inline uint get_eff_w() { return get_w(); }

      // Drawing and event handlers
      void draw(RenderTarget* dst);

   private:
	Align				m_align;
	std::string		m_text;
	Scrollbar*		m_scrollbar;
	ScrollMode     m_scrollmode;
   int				m_textheight;	// total height of wrapped text, in pixels
	int				m_textpos;		// current scrolling position in pixels (0 is top)
};

#include "ui_listselect.h"
#include "ui_window.h"
#include "ui_unique_window.h"

#endif /* __S__UI_H */
