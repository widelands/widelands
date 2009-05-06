/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "constants.h"
#include "panel.h"
#include "m_signal.h"

#include "rgbcolor.h"

namespace UI {

/// This is simply a button. Override void clicked() to react to the click.
/// This is all that is needed in most cases, but if there is a need to give a
/// callback function to the button, there are some templates for that below.
struct Button : public Panel {
	Button /// for textual buttons
		(Panel * const parent,
		 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
		 uint32_t const background_pictute_id,
		 std::string const & title_text,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat    = false,
		 std::string const & fontname = UI_FONT_NAME,
		 uint32_t const      fontsize = UI_FONT_SIZE_SMALL);
	Button /// for pictorial buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat     = false,
		 const std::string & fontname = UI_FONT_NAME,
		 const uint32_t      fontsize = UI_FONT_SIZE_SMALL);
	~Button();

	void set_pic(uint32_t picid);
	void set_title(const std::string &);
	const std::string & get_title() const throw () {return m_title;}
	bool enabled() const {return m_enabled;}
	void set_enabled(bool on);
	void set_repeating(bool const on) {m_repeating = on;}
	void set_draw_caret(bool draw_caret) {m_draw_caret = draw_caret;}
	void set_font(std::string const & name, int32_t size) {
		m_fontname = name;
		m_fontsize = size;
	}
	bool is_snap_target() const {return true;}

	// Drawing and event handlers
	void draw(RenderTarget &);
	void think();

	void handle_mousein(bool inside);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	virtual void clicked() const = 0; /// Override this to react on the click.

	bool        m_highlighted;    //  mouse is over the button
	bool        m_pressed;
	bool        m_enabled;
	bool        m_repeating;
	bool        m_flat;

	int32_t     m_time_nextact;

	std::string m_title;          //  title string used when _mypic == 0

	uint32_t        m_pic_background; //  background texture (picture ID)
	uint32_t        m_pic_custom;     //  custom icon on the button
	uint32_t        m_pic_custom_disabled;
	std::string     m_fontname;
	uint32_t        m_fontsize;

	RGBColor    m_clr_down; //  color of border while a flat button is "down"
	bool        m_draw_caret;
};


/// A compatibility/convenience version of Button. Overrides void clicked()
/// with a function that calls a given callback function (nonstatic member of
/// T), with the given instance of T as its only parameter.
template <typename T> struct Callback_Button : public Button {
	Callback_Button /// for textual buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 void (T::*callback_function)(),
		 T & callback_argument_this,
		 const std::string & title_text,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat     = false,
		 const std::string & fontname = UI_FONT_NAME,
		 const uint32_t      fontsize = UI_FONT_SIZE_SMALL)
		:
		Button
			(parent,
			 x, y, w, h,
			 background_pictute_id,
			 title_text,
			 tooltip_text,
			 _enabled, flat,
			 fontname,
			 fontsize),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this)
	{}
	Callback_Button /// for pictorial buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 void (T::*callback_function)(),
		 T & callback_argument_this,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat     = false,
		 const std::string & fontname = UI_FONT_NAME,
		 const uint32_t      fontsize = UI_FONT_SIZE_SMALL)
		:
		Button
			(parent,
			 x, y, w, h,
			 background_pictute_id,
			 foreground_picture_id,
			 tooltip_text,
			 _enabled, flat,
			 fontname,
			 fontsize),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this)
	{}

protected:
	void (T::*_callback_function)();
	T & _callback_argument_this;
	void clicked() const {
		(_callback_argument_this.*_callback_function)();
	}
};

/**
 * A button that calls a callback function with 2 argument when pressed. The
 * second argument is the so called 'id'.
 *
 * T is the type of the target of the 'this' argument of the callback function.
 * ID is the type of the 'id' argument of the callback function.
 */
template <typename T, typename ID> struct Callback_IDButton : public Button {
	Callback_IDButton /// for textual buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 void (T::*callback_function)(ID),
		 T & callback_argument_this,
		 const ID callback_argument_id,
		 const std::string & title_text,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat     = false,
		 const std::string & fontname = UI_FONT_NAME,
		 const uint32_t      fontsize = UI_FONT_SIZE_SMALL)
		:
		Button
			(parent,
			 x, y, w, h,
			 background_pictute_id,
			 title_text,
			 tooltip_text,
			 _enabled, flat,
			 fontname,
			 fontsize),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this),
		_callback_argument_id  (callback_argument_id)
	{}
	Callback_IDButton /// for pictorial buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 void (T::*callback_function)(ID),
		 T & callback_argument_this,
		 const ID callback_argument_id,
		 std::string const & tooltip_text = std::string(),
		 bool const _enabled = true,
		 bool const flat     = false,
		 const std::string & fontname = UI_FONT_NAME,
		 const uint32_t      fontsize = UI_FONT_SIZE_SMALL)
		:
		Button
			(parent,
			 x, y, w, h,
			 background_pictute_id,
			 foreground_picture_id,
			 tooltip_text,
			 _enabled, flat,
			 fontname,
			 fontsize),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this),
		_callback_argument_id  (callback_argument_id)
	{}

protected:
	void (T::*_callback_function)(ID);
	T & _callback_argument_this;
	const ID _callback_argument_id;
	void clicked() const {
		(_callback_argument_this.*_callback_function)(_callback_argument_id);
	}
};
};

#endif
