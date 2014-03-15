/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include <boost/signals2.hpp>

#include "graphic/font.h"
#include "ui_basic/panel.h"

namespace UI {

struct DiscreteSlider;

/**
 * \brief This class defines a generic slide bar.
 *
 * The callbacks for the slider value are done by two signal instances.
 */
class Slider : public Panel {

friend struct DiscreteSlider;

protected:
	Slider
		(Panel * parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 int32_t min_value, int32_t max_value, int32_t value,
		 const Image* background_picture_id,
		 const std::string & tooltip_text,
		 uint32_t cursor_size,
		 bool enabled,
		 int32_t x_gap, int32_t y_gap, int32_t bar_size);

public:
	bool is_snap_target() const override {return true;}

	int32_t get_value() const {return m_value;}
	int32_t get_max_value() const {return m_max_value;}
	int32_t get_min_value() const {return m_min_value;}

	void set_value(int32_t);
	void set_max_value(int32_t);
	void set_min_value(int32_t);

	void set_enabled(bool enabled);


protected:
	void layout() override;
	void calc_cursor_pos();

	//  drawing
	int32_t get_x_gap()    const {return m_x_gap;}
	int32_t get_y_gap()    const {return m_y_gap;}
	int32_t get_bar_size() const {return m_bar_size;}
	void draw_cursor
		(RenderTarget &, int32_t x, int32_t y, int32_t w, int32_t h);

	//  mouse events
	bool handle_mouserelease(Uint8 btn, int32_t, int32_t) override;
	void handle_mousein(bool inside) override;
	void cursor_moved(int32_t pointer, int32_t x, int32_t y);
	void cursor_pressed(int32_t pointer);
	void bar_pressed(int32_t pointer, int32_t ofs);

private :
	void send_value_changed();
	void set_highlighted(bool highlighted);

public:
	boost::signals2::signal<void ()> changed;
	boost::signals2::signal<void (int32_t)> changedto;

private:
	int32_t m_min_value;          //  cursor values
	int32_t m_max_value;
	int32_t m_value;
	int32_t m_relative_move;

	bool m_highlighted;       //  mouse over
	bool m_pressed;           //  the cursor is pressed
	bool m_enabled;           //  enabled widget

	const Image* m_pic_background;    //  background texture (picture ID)

protected:
	int32_t m_x_gap;              //  draw positions
	int32_t m_y_gap;
	int32_t m_bar_size;

	int32_t m_cursor_pos;         //  cursor position
	int32_t m_cursor_size;        //  cursor width
};


/**
 * \brief This class defines an horizontal slide bar.
 */
struct HorizontalSlider : public Slider {
	HorizontalSlider
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const int32_t min_value, const int32_t max_value, const int32_t value,
		 const Image* background_picture_id,
		 const std::string & tooltip_text = std::string(),
		 const uint32_t cursor_size = 20,
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
	void draw(RenderTarget & dst) override;
	bool handle_mousemove (Uint8 btn, int32_t x, int32_t y, int32_t, int32_t) override;
	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y) override;
	void layout() override;
};



/**
 * \brief This class defines a verical slide bar.
 */
struct VerticalSlider : public Slider {
	VerticalSlider
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const int32_t min_value, const int32_t max_value, const int32_t value,
		 const Image* background_picture_id,
		 const uint32_t cursor_size = 20,
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
	void draw(RenderTarget & dst) override;
	bool handle_mousemove (Uint8 btn, int32_t x, int32_t y, int32_t, int32_t) override;
	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y) override;
	void layout() override;
};

/**
 * \brief This class defines an discrete slide bar. We do not derive from
 * Slider, but rather embed it, as we need to re-size it and add the labels.
 */
struct DiscreteSlider : public Panel {
	DiscreteSlider
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const std::vector<std::string> labels_in,
		 uint32_t m_value,
		 const Image* background_picture_id,
		 const std::string & tooltip_text = std::string(),
		 const uint32_t cursor_size = 20,
		 const bool enabled = true);

	void set_labels(std::vector<std::string>);

	boost::signals2::signal<void ()> changed;
	boost::signals2::signal<void (int32_t)> changedto;

protected:
	virtual void draw(RenderTarget & dst) override;
	virtual void layout() override;

	HorizontalSlider slider;

private:
	std::vector<std::string> labels;
};


}

#endif
