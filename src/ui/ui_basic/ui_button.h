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

#include "ui_panel.h"
#include "ui_signal.h"

#include "rgbcolor.h"

namespace UI {
/**
 * This defines a button.
 */
struct Basic_Button : public Panel {
	Basic_Button
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const bool enabled, const bool flat,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 const std::string & title_text,
		 const std::string & tooltip_text);
	~Basic_Button();

	void set_pic(uint32_t picid);
	void set_title(const std::string &);
	const std::string & get_title() const throw () {return m_title;}
	void set_enabled(bool on);
	void set_draw_caret(bool draw_caret) {m_draw_caret = draw_caret;}
	bool is_snap_target() const {return true;}

	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

protected:
	virtual void send_signal_clicked() const = 0;

private:
	bool        m_highlighted;    //  mouse is over the button
	bool        m_pressed;
	bool        m_enabled;
	bool        m_flat;

	std::string m_title;          //  title string used when _mypic == 0

	uint32_t        m_pic_background; //  background texture (picture ID)
	uint32_t        m_pic_custom;     //  custom icon on the button
	uint32_t        m_pic_custom_disabled;

	RGBColor    m_clr_down; //  color of border while a flat button is "down"
	bool        m_draw_caret;
};

/**
 * A button that calls a callback function with 1 argument when pressed.
 *
 * T is the type of the target of the 'this' argument of the callback function.
 */
template <typename T> struct Button : public Basic_Button {
	Button ///  for textual buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 void (T:: * callback_function)(),
		 T * const callback_argument_this,
		 const std::string & title_text,
		 const std::string & tooltip_text = std::string(),
		 const bool enabled = true,
		 const bool flat    = false)
		:
		Basic_Button
		(parent,
		 x, y, w, h,
		 enabled, flat,
		 background_pictute_id,
		 0,
		 title_text,
		 tooltip_text),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this)
	{}
	Button ///  for pictorial buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 void (T:: * callback_function)(),
		 T * const callback_argument_this,
		 const std::string & tooltip_text = std::string(),
		 const bool enabled = true,
		 const bool flat    = false)
		:
		Basic_Button
		(parent,
		 x, y, w, h,
		 enabled, flat,
		 background_pictute_id,
		 foreground_picture_id,
		 std::string(),
		 tooltip_text),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this)
	{}

protected:
	void (T:: * _callback_function)();
	T * const _callback_argument_this;
	void send_signal_clicked() const
	{(_callback_argument_this->*_callback_function)();}
};

/**
 * A button that calls a callback function with 2 argument when pressed. The
 * second argument is the so called 'id'.
 *
 * T is the type of the target of the 'this' argument of the callback function.
 * ID is the type of the 'id' argument of the callback function.
 */
template <typename T, typename ID> struct IDButton : public Basic_Button {
	IDButton ///  for textual buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 void (T:: * callback_function)(ID),
		 T * const callback_argument_this,
		 const ID callback_argument_id,
		 const std::string & title_text,
		 const std::string & tooltip_text = std::string(),
		 const bool enabled = true,
		 const bool flat    = false)
		:
		Basic_Button
		(parent,
		 x, y, w, h,
		 enabled, flat,
		 background_pictute_id,
		 0,
		 title_text,
		 tooltip_text),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this),
		_callback_argument_id  (callback_argument_id)
	{}
	IDButton ///  for pictorial buttons
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const uint32_t background_pictute_id,
		 const uint32_t foreground_picture_id,
		 void (T:: * callback_function)(ID),
		 T * const callback_argument_this,
		 const ID callback_argument_id,
		 const std::string & tooltip_text = std::string(),
		 const bool enabled = true,
		 const bool flat    = false)
		:
		Basic_Button
		(parent,
		 x, y, w, h,
		 enabled, flat,
		 background_pictute_id,
		 foreground_picture_id,
		 std::string(),
		 tooltip_text),
		_callback_function     (callback_function),
		_callback_argument_this(callback_argument_this),
		_callback_argument_id  (callback_argument_id)
	{}

protected:
	void (T:: * _callback_function)(ID);
	T  * const _callback_argument_this;
	const ID _callback_argument_id;
	void send_signal_clicked() const
	{(_callback_argument_this->*_callback_function)(_callback_argument_id);}
};
};

#endif
