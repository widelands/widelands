/*
 * Copyright (C) 2002 by Holger Rapp
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
#include "singleton.h"
#include "font.h"
#include "growablearray.h"

#if 0
// predeclaration
class Window;

/** class Checkbox
 *
 * This defines a checkbox, which will be marked or unmarked, depending on their state
 *
 * Depends:	g_gr
 * 			class Graph::Pic
 */
#define CHECKBOX_WIDTH 20
#define CHECKBOX_HEIGHT 20
class Checkbox {
		  Checkbox( const Checkbox&);
		  Checkbox operator=(const Checkbox&);

		  friend class Window;

		  public:
					 // Returns the current state of the checkbox
					 bool get_state(void) const { return bstate; }

		  private:
					 Checkbox(const uint, const uint, const bool, Pic*, const uint, const uint);
					 ~Checkbox(void);
					 int draw(void);
					 void set_state(bool b) { bstate=b; }
					 // Information funcs
					 static inline uint get_w(void) { return CHECKBOX_WIDTH; }
					 static inline uint get_h(void) { return CHECKBOX_HEIGHT; }
					 inline uint get_xpos(void) { return x+xp; }
					 inline uint get_ypos(void) { return y+yp; }


					 bool bstate;
					 static AutoPic gr;
					 uint x, y;
					 uint xp, yp;
					 Pic* dp;
};

/** class Window
 *
 * This class offers a window. Should't be user directly
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

#define MAX_BUT 100  // these values don't get checked. you better don't ignore them
#define MAX_TA   40
#define MAX_CHECKBOX   10
#define MAX_LISTSELECT   2
#define MAX_MULTILINE_TEXTAREA  1

class Window {
		  // Copy is non trivial and shouldn't be needed
		  Window(const Window&);
		  Window& operator=(const Window&);

		  public:
					 enum Flags {
								DEFAULT,
								FLAT // No clicks, no moves, no borders.
					 };

					 // creation functions
					 Textarea* create_textarea(const uint, const uint, const uint, const Textarea::Align = Textarea::LEFTA);
					 Textarea* create_textarea(const uint, const uint, const char* ,  Textarea::Align = Textarea::LEFTA);
					 Button*   create_button(const uint, const uint, const uint, const uint, const uint);
					 Checkbox*   create_checkbox(const uint, const uint, const bool b);
					 Listselect*   create_listselect(const uint, const uint, const uint, const uint);
					 Multiline_Textarea*   create_multiline_textarea(const uint, const uint, const uint, const uint, const bool, const Multiline_Textarea::Align);
					 void set_new_bg(Pic* p);

					 // inline functions to get some informations
					 inline uint get_xpos(void) { return x; }
					 inline uint get_ypos(void) { return y; }
					 inline uint get_w(void) const { return w; }
					 inline uint get_h(void) const { return h; }
					 inline Flags get_flags(void) { return myf; }

		  private:
					  // The next two functions are there to get the different between asked window size and given window size
					 // Ex: you want a 100x100 window. Now, the User_Interface class makes sure that border widths and top,bottom heights
					 // are added to the size and still the window musn't leave the screen on any edges.
					 inline const static uint get_border(void) { return (CORNER<<1); }


					 int handle_click(const uint, const bool, const uint, const uint);
					 int handle_mm(const uint, const uint, const bool, const bool);
					 void draw(void);

										 //					 void set_closefunc(...)


		  friend class User_Interface;

		  private:
					 // Only friends can create and destroy and move us!
					 void set_pos(const uint, const uint);
					 Window(const uint, const uint, const uint, const uint, const Flags);
					 ~Window();
					 void redraw_win(void);

					 uint x, y, w, h;
					 Pic* winpic;
					 Pic* own_bg;
					 Flags myf;

					 // for textareas
					 uint nta;
					 Textarea** ta;

					 // for buttons
					 uint nbut;
					 Button** but;

					 // for checkboxes
					 uint ncheckbox;
					 Checkbox** checkbox;

					 // for listselects
					 uint nlistselect;
					 Listselect** listselect;

					 // for multiline_textareas
					 uint nmultiline_textarea;
					 Multiline_Textarea** multiline_textarea;

					 //closefunc dfkj;
					 static AutoPic l_border;
					 static AutoPic r_border;
					 static AutoPic top;
					 static AutoPic bot;
					 static AutoPic bg;
};
#endif

class Panel;

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
class UISignal {
	typedef void (Panel::*fnT)();
	Panel *_pnl;
	fnT _fn;
public:
	UISignal() { _pnl = 0; _fn = 0; }
	template<class T>
	void set(Panel *p, void (T::*f)()) {
		_pnl = p;
		_fn = static_cast<fnT>(f);
	}
	void call() { if (_fn) (_pnl->*_fn)(); }
};

template<class T1>
class UISignal1 {
	typedef void (Panel::*fnT)(T1);
	Panel *_pnl;
	fnT _fn;
public:
	UISignal1() { _pnl = 0; _fn = 0; }
	template<class T>
	void set(Panel *p, void (T::*f)(T1)) {
		_pnl = p;
		_fn = static_cast<fnT>(f);
	}
	void call(T1 t1) { if (_fn) (_pnl->*_fn)(t1); }
};

/** class Panel
 *
 * Panel is a basic rectangular UI element.
 */
class Panel {
public:
	enum {
		pf_handle_mouse = 1,
		pf_think
	};

	Panel(Panel *nparent, const int nx, const int ny, const uint nw, const uint nh);
	virtual ~Panel();

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
	inline uint get_w() const { return _w; }
	inline uint get_h() const { return _h; }

	// Drawing, visibility
	virtual void draw(Bitmap *dst, int ofsx, int ofsy);
	void update(int x, int y, int w, int h);

	// Events
	virtual void think();

	virtual void handle_mousein(bool inside);
	virtual void handle_mouseclick(uint btn, bool down, uint x, uint y);
	virtual void handle_mousemove(uint x, uint y, int xdiff, int ydiff, uint btns);

	void set_handle_mouse(bool yes);
	inline bool get_handle_mouse() const { return _flags & pf_handle_mouse; }

	void set_think(bool yes);
	inline bool get_think() const { return _flags & pf_think; }

private:
	void do_draw(Bitmap *dst, int ofsx, int ofsy);

	Panel *get_mousein(uint x, uint y);
	void do_mousein(bool inside);
	void do_mouseclick(uint btn, bool down, uint x, uint y);
	void do_mousemove(uint x, uint y, int xdiff, int ydiff, uint btns);

	static int ui_mouseclick(const bool down, const uint x, const uint y, void *a);
	static int ui_mousemove(const uint x, const uint y, const int xdiff, const int ydiff,
	                        const bool lbtn, const bool rbtn, void *a);

	static Panel *_modal;

	Panel *_parent;
	Panel *_next, *_prev;
	Panel *_fchild, *_lchild; // first, last child
	Panel *_mousein; // child panel the mouse is in

	uint _flags;

	int _x, _y;
	uint _w, _h;

	bool _running;
	int _retcode;
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
	void set_enabled(bool on);

	// Drawing and event handlers
	void draw(Bitmap *dst, int ofsx, int ofsy);

	void handle_mousein(bool inside);
	void handle_mouseclick(uint btn, bool down, uint x, uint y);

private:
	static AutoPic bg0, bg1, bg2; // background pictures
	static Pic bg0e, bg1e, bg2e;

	Pic *_mybg, *_mybge; // one of the static bgX
	Pic *_mypic; // the text etc.. on the button

	int _id;
	bool _highlighted;
	bool _pressed;
	bool _enabled;
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
typedef void (Panel::*LISTSELECT_FUNC)(int id, int arg);

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
	void handle_mouseclick(uint btn, bool down, uint x, uint y);

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
	void btn_up() { up.call(1); }
	void btn_down() { down.call(1); }
};

/** class User_Interface
 *
 * This class offers a user interface. This means windows, buttons and
 * so on.
 *
 * It's a Singleton.
 *
 * DEPENDS: on all the sub interface classes (Buttons, Textareas, windows a.s.on)
 * 			class Graph::Pic
 * 			func	Graph::draw_pic
 * 			global Object: g_gr
 */
class User_Interface : public Singleton<User_Interface> {
	User_Interface(const User_Interface&);
	User_Interface& operator=(const User_Interface&);

public:
	User_Interface(void);
	~User_Interface(void);
};

#define g_ui	User_Interface::get_singleton()

#endif /* __S__UI_H */
