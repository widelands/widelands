/*
 * Copyright (C) 2002-2007 by the Widelands Development Team
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
 */

#ifndef __S__SLIDER_H
#define __S__SLIDER_H


#include <string>
#include "rgbcolor.h"
#include "ui_panel.h"
#include "ui_signal.h"
#include "rendertarget.h"
#include "graphic.h"

namespace UI {

/**
 * \brief This class defines a generic slide bar.
 *
 * The callbacks for the slider value are done by two signal instances.
 */
class Slider : public Panel {

protected:
	Slider
		(Panel * const parent,
		 const int x, const int y, const uint w, const uint h,
		 const int min_value, const int max_value, const int value,
		 const uint background_picture_id,
		 const std::string & tooltip_text,
		 const uint cursor_size,
		 const bool enabled,
		 const int x_gap, const int y_gap, const int bar_size);

public:
	bool is_snap_target() const {return true;}

	int get_value() const {return m_value;}

	void set_enabled(const bool enabled);


protected:
	//  drawing
	int get_x_gap()    const {return m_x_gap;}
	int get_y_gap()    const {return m_y_gap;}
	int get_bar_size() const {return m_bar_size;}
	void draw_cursor(RenderTarget * dst, int x, int y, int w, int h);

	//  mouse events
	bool handle_mouserelease(const Uint8 btn, int, int);
	void handle_mousein(bool inside);
	void cursor_moved(int pointer, int x, int y);
	void cursor_pressed(int pointer);
	void bar_pressed(int pointer, int ofs);

private :
	void send_value_changed();

public:
	Signal        changed;
	Signal1<int>  changedto;

private:
	int m_min_value;          //  cursor values
	int m_max_value;
	int m_value;
	int m_relative_move;

	bool m_highlighted;       //  mouse over
	bool m_pressed;           //  the cursor is pressed
	bool m_enabled;           //  enabled widget

	uint m_pic_background;    //  background texture (picture ID)

	int m_x_gap;              //  draw positions
	int m_y_gap;
	int m_bar_size;

protected:
	int m_cursor_pos;         //  cursor position
	int m_cursor_size;        //  cursor width
};


/**
 * \brief This class defines an horizontal slide bar.
 */
struct HorizontalSlider : public Slider {
	HorizontalSlider
		(Panel * const parent,
		 const int x, const int y, const uint w, const uint h,
		 const int min_value, const int max_value, const int value,
		 const uint background_picture_id,
		 const std::string & tooltip_text = std::string(),
		 const uint cursor_size = 20,
		 const bool enabled = true)
		:
		Slider
		(parent,
		 x, y, w, h,
		 min_value, max_value, value,
		 background_picture_id,
		 tooltip_text,
		 cursor_size,
		 enabled,
		 cursor_size / 2,
		 h / 2 - 2,
		 w - cursor_size)
	{}

protected:
	void draw(RenderTarget * dst);
	bool handle_mousemove(const Uint8 btn, int x, int y, int, int);
	bool handle_mousepress(const Uint8 btn, int x, int y);
};



/**
 * \brief This class defines a verical slide bar.
 */
struct VerticalSlider : public Slider {
	VerticalSlider
		(Panel * const parent,
		 const int x, const int y, const uint w, const uint h,
		 const int min_value, const int max_value, const int value,
		 const uint background_picture_id,
		 const uint cursor_size = 20,
		 const std::string & tooltip_text = std::string(),
		 const bool enabled = true)
		:
		Slider
		(parent,
		 x, y, w, h,
		 min_value, max_value, value,
		 background_picture_id,
		 tooltip_text,
		 cursor_size,
		 enabled,
		 w / 2 - 2,
		 cursor_size / 2,
		 h - cursor_size)
	{}

protected:
	void draw(RenderTarget * dst);
	bool handle_mousemove(const Uint8 btn, int x, int y, int, int);
	bool handle_mousepress(const Uint8 btn, int x, int y);
};

};

#endif
