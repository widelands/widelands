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
#include "growablearray.h"
#include "auto_pic.h"

/** class UIObject
 *
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

/** class UISignal
 *
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

/** class Panel
 *
 * Panel is a basic rectangular UI element.
 * The outer rectangle is defined by (_x,_y,_w,_h) and encloses the entire panel,
 * including both border and inner area/rectangle.
 * The inner rectangle is the outer rectangle minus the border sizes.
 * Child panel coordinates are always relative to the inner rectangle.
 */
class Panel : public UIObject {
public:
	enum {
		pf_handle_mouse = 1, // receive mouse events
		pf_think = 2, // call think() function during run
		pf_top_on_click = 4, // bring panel on top when clicked inside it
		pf_die = 8, // this panel needs to die
		pf_child_die = 16, // a child needs to die
		pf_visible = 32, // render the panel
		pf_can_focus = 64, // can receive the keyboard focus
	};

	Panel(Panel *nparent, const int nx, const int ny, const uint nw, const uint nh);
	virtual ~Panel();

	inline Panel *get_parent() const { return _parent; }

	// Modal
	int run();
	void end_modal(int code);

	virtual void start();
	virtual void end();

	// Geometry
	void set_size(const uint nw, const uint nh);
	void set_pos(const int nx, const int ny);

	inline int get_x() const { return _x; }
	inline int get_y() const { return _y; }
	inline int get_w() const { return _w; }
	inline int get_h() const { return _h; }

	void set_inner_size(uint nw, uint nh);
	void set_border(uint l, uint r, uint t, uint b);

	inline uint get_lborder() const { return _lborder; }
	inline uint get_rborder() const { return _rborder; }
	inline uint get_tborder() const { return _tborder; }
	inline uint get_bborder() const { return _bborder; }

	inline int get_inner_w() const { return _w-(_lborder+_rborder); }
	inline int get_inner_h() const { return _h-(_tborder+_bborder); }

	void move_to_top();

	// Drawing, visibility
	inline bool get_visible() const { return (_flags & pf_visible) ? true : false; }
	void set_visible(bool on);
	
	virtual void draw(Bitmap *dst, int ofsx, int ofsy);
	virtual void draw_border(Bitmap *dst, int ofsx, int ofsy);
	void update(int x, int y, int w, int h);
	void update_inner(int x, int y, int w, int h);
	void set_cache(bool enable);

	// Events
	virtual void think();

	int get_mouse_x();
	int get_mouse_y();
	void set_mouse_pos(int x, int y);
	
	virtual void handle_mousein(bool inside);
	virtual bool handle_mouseclick(uint btn, bool down, int x, int y);
	virtual void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	virtual bool handle_key(bool down, int code, char c);

	void set_handle_mouse(bool yes);
	inline bool get_handle_mouse() const { return (_flags & pf_handle_mouse) ? true : false; }
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	inline bool get_can_focus() const { return (_flags & pf_can_focus) ? true : false; }
	void focus();
	
	void set_think(bool yes);
	inline bool get_think() const { return (_flags & pf_think) ? true : false; }

	inline void set_top_on_click(bool on) {
		if (on) _flags |= pf_top_on_click;
		else _flags &= ~pf_top_on_click;
	}
	inline bool get_top_on_click() const { return (_flags & pf_top_on_click) ? true : false; }

protected:
	void die();

private:
	void check_child_death();

	void do_draw(Bitmap *dst, int ofsx, int ofsy);

	Panel *get_mousein(int x, int y);
	void do_mousein(bool inside);
	bool do_mouseclick(uint btn, bool down, int x, int y);
	void do_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool do_key(bool down, int code, char c);

	Panel *_parent;
	Panel *_next, *_prev;
	Panel *_fchild, *_lchild; // first, last child
	Panel *_mousein; // child panel the mouse is in
	Panel *_focus; // keyboard focus

	uint _flags;
	Pic *_cache;
	bool _needdraw;

	int _x, _y;
	uint _w, _h;
	uint _lborder, _rborder, _tborder, _bborder;

	bool _running;
	int _retcode;

private:
	static Panel *ui_trackmouse(int *x, int *y);
	static void ui_mouseclick(bool down, int button, uint btns, int x, int y);
	static void ui_mousemove(uint btns, int x, int y, int xdiff, int ydiff);
	static void ui_key(bool down, int code, char c);

	static Panel *_modal;
	static Panel *_g_mousegrab;
	static Panel *_g_mousein;
};

/** class Button : public Panel
 *
 * This defines a button.
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */
#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR  15

class Button : public Panel {
	friend void setup_ui(void);
	static void setup_ui();

public:
	Button(Panel *parent, int x, int y, uint w, uint h, uint background, int id = 0);
	~Button();

	UISignal clicked;
	UISignal1<int> clickedid;

	void set_pic(Pic *pic);
	void set_pic(AutoPic *pic);
	void set_enabled(bool on);

	// Drawing and event handlers
	void draw(Bitmap *dst, int ofsx, int ofsy);

	void handle_mousein(bool inside);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	static AutoPic bg0, bg1, bg2; // background pictures
	static Pic bg0e, bg1e, bg2e;

	Pic *_mybg, *_mybge; // one of the static bgX
	Pic *_mypic; // the text etc.. on the button

	int _id;
   bool _must_delete_mypic;
	bool _highlighted; // mouse is over the button
	bool _pressed;
	bool _enabled;
};

/** class Statebox [virtual]
 *
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
#define STATEBOX_WIDTH 20
#define STATEBOX_HEIGHT 20

class Statebox : public Panel {
	friend void setup_ui(void);
	static ushort dflt_highlightcolor;
	static void setup_ui();

public:
	Statebox(Panel *parent, int x, int y);
	~Statebox();

	UISignal changed;
	UISignal1<bool> changedto;

	void set_enabled(bool enabled);

	inline bool get_state() const { return _state; }
	void set_state(bool on);

	// Drawing and event handlers
	void draw(Bitmap *dst, int ofsx, int ofsy);

	void handle_mousein(bool inside);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	virtual void clicked() = 0;

	static AutoPic _gr;

	bool _highlighted;
	bool _enabled; // true if the checkbox can be clicked
	bool _state; // true if the box is checked
};

/** class Checkbox
 *
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 */
class Checkbox : public Statebox {
public:
	Checkbox(Panel *parent, int x, int y) : Statebox(parent, x, y) { }

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

	int add_button(Panel *parent, int x, int y);

	inline int get_state() const { return _state; }
	void set_state(int state);

private:
	Radiobutton *_buttons; // linked list of buttons (not sorted)
	int _highestid;
	int _state; // -1: none
};

/** class Textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */
class Textarea : public Panel {
public:
	enum Align {
		H_LEFT = 0,
		H_RIGHT = 1,
		H_CENTER = 2,

		V_TOP = 0,
		V_BOTTOM = 4,
		V_CENTER = 8,

		CENTERRIGHT = H_RIGHT|V_CENTER,
		BOTTOMRIGHT = H_RIGHT|V_BOTTOM,
		CENTER = H_CENTER|V_CENTER,
		BOTTOMCENTER = H_CENTER|V_BOTTOM,
	};

	Textarea(Panel *parent, int x, int y, const char *text, Align align = H_LEFT, uint font = 0);
	~Textarea();

	void set_text(const char *text);
	void set_align(Align align);

	inline int get_fh() const { return g_fh.get_fh(_font); }

	// Drawing and event handlers
	void draw(Bitmap *dst, int ofsx, int ofsy);

private:
	void collapse();
	void expand();

	uint _font;
	Pic *_textpic; // picture with prerendered text
	Align _align;
};

/** class Multiline_textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */
class Multiline_Textarea : public Panel {
public:
	enum Align {
		H_LEFT = 0,
		H_RIGHT = 1,
		H_CENTER = 2
	};

	Multiline_Textarea(Panel *parent, int x, int y, uint w, uint h, const char *text,
	                   Align align = H_LEFT, uint font = 0);
	~Multiline_Textarea();

	void clear();
	void set_text(const char *text);

	void move_up(int i);
	void move_down(int i);

	inline uint get_eff_w() { return get_w(); }

	// Drawing and event handlers
	void draw(Bitmap *bmp, int ofsx, int ofsy);

private:
	uint _font;
	Align _align;
	Growable_Array _lines;
	uint _firstvis;
};

/** class Listselect
 *
 * This class defines a list-select box.
 *
 * Depends: class Graph::Pic
 * 			g_fh
 * 			class Button
 */
class Listselect : public Panel {
	friend void setup_ui(void);
	static ushort dflt_bgcolor, dflt_framecolor, dflt_selcolor;
	static void setup_ui();

public:
	enum Align {
		H_LEFT = 0,
		H_RIGHT = 1,
		H_CENTER = 2
	};

	Listselect(Panel *parent, int x, int y, uint w, uint h, Align align = H_LEFT, uint font = 0);
	~Listselect();
	static Listselect *create_scrolling(Panel *parent, int x, int y, uint w, uint h,
	                                    Align align = H_LEFT, uint font = 0);

	UISignal1<int> selected;

	void clear();
	void add_entry(const char *name, const char *value = 0);

	void move_up(int i);
	void move_down(int i);

	void set_colors(ushort bg, ushort frame, ushort sel);

	void select(int i);
	inline const char *get_selection() {
		if (_selection < 0) return 0;
		return ((Entry *)_entries.element_at(_selection))->str;
	}

	inline uint get_eff_w() { return get_w(); }

	// Drawing and event handling
	void draw(Bitmap *dst, int ofsx, int ofsy);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	struct Entry {
		Pic *pic;
		char str[1]; // variable size
	};

	ushort _bgcolor, _framecolor, _selcolor;
	uint _font;
	Align _align;
	Growable_Array _entries;
	uint _firstvis;
	int _selection;
};

/** class Scrollbar
 *
 * This class provides a scrollbar (single-step only atm)
 */
class Scrollbar : public Panel {
public:
	Scrollbar(Panel *parent, int x, int y, uint w, uint h, bool horiz);

	UISignal1<int> up; // left for horizontal scrollbars
	UISignal1<int> down; // right for vertical scrollbars

private:
   static AutoPic pic_up;
   static AutoPic pic_down;
	void btn_up() { up.call(1); }
	void btn_down() { down.call(1); }
};

/** class Window
 *
 * Windows are cached by default.
 *
 * The graphics (see static Pics) are used in the following manner: (Example)
 *
 *  <--20leftmostpixel_of_top--><60Pixels as often as possible to reach window with from top><20rightmost pixel of top>
 *  ^
 *  20 topmost pixels of l_border                                      <--- > same for r_border
 *  as often as needed: 60 pixels of l_border								  <--- > same for r_border
 *  20 bottom pixels of l_border													  <--- > same for r_borde
 *  <--20leftmostpixel_of_bot--><60Pixels as often as possible to reach window with from bot><20rightmost pixel of bot>
 *
 * So: the l_border and the r_border pics MUST have a height of 100, while the width must be  20
 * 	 and the top and bot pics MUST have a width of 100, while the height must be 20
 *
 * DEPENDS: Graph::Pic
 * 			Graph::draw_pic
 * 			Initalized g_gr object
 * 			User_Interface
 */

// widht/height the graphs above must have
#define MUST_HAVE_NPIX	100
// width/height to use as the corner
#define CORNER			20
#define MIDDLE			(MUST_HAVE_NPIX-(CORNER*2))

#define WINDOW_BORDER	20

class Window : public Panel {
public:
	Window(Panel *parent, int x, int y, uint w, uint h, const char *title);
	~Window();

	void set_title(const char *text);
	void set_new_bg(Pic* p);

	void move_to_mouse();

	// Drawing and event handlers
	void draw_border(Bitmap *dst, int ofsx, int ofsy);

	bool handle_mouseclick(uint btn, bool down, int mx, int my);
	void handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns);

private:
	Pic *_title;
	Pic *_custom_bg; // custom background, set through set_new_bg()
	bool _dragging;

	static AutoPic l_border;
	static AutoPic r_border;
	static AutoPic top;
	static AutoPic bot;
	static AutoPic bg;
};

#endif /* __S__UI_H */
