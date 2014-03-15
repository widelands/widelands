/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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
 *
 */

#ifndef UI_SCROLLBAR_H
#define UI_SCROLLBAR_H

#include <boost/signals2.hpp>

#include "ui_basic/panel.h"
#include "rect.h"

namespace UI {
/**
 * This class provides a scrollbar
 */
struct Scrollbar : public Panel {
	enum Area {
		None,
		Minus,
		Plus,
		Knob,
		MinusPage,
		PlusPage
	};

	enum {
		///< default width for vertical scrollbars,
		// height for horizontal scrollbar
		Size = 24,
	};

public:
	Scrollbar
		(Panel * parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h, bool horiz);

	boost::signals2::signal<void (int32_t)> moved;

	void set_steps(int32_t steps);
	void set_singlestepsize(uint32_t singlestepsize);
	void set_pagesize(int32_t pagesize);
	void set_scrollpos(int32_t pos);

	uint32_t get_steps() const;
	uint32_t get_singlestepsize() const {return m_singlestepsize;}
	uint32_t get_pagesize() const {return m_pagesize;}
	uint32_t get_scrollpos() const {return m_pos;}

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;

	void set_force_draw(bool const t) {m_force_draw = t;}

private:
	Area get_area_for_point(int32_t x, int32_t y);
	uint32_t get_knob_pos();
	uint32_t get_knob_size();
	void set_knob_pos(int32_t p);

	void action(Area area);

	void draw_button(RenderTarget &, Area, Rect);
	void draw_area  (RenderTarget &, Area, Rect);
	void draw(RenderTarget &) override;
	void think() override;

	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mousemove
		(Uint8 state, int32_t mx, int32_t my, int32_t xdiff, int32_t ydiff) override;

private:
	bool m_horizontal;
	bool      m_force_draw; // draw this scrollbar, even if it can't do anything

	uint32_t  m_pos;            ///< from 0 to m_range - 1
	uint32_t  m_singlestepsize;
	uint32_t  m_pagesize;
	uint32_t  m_steps;

	Area m_pressed; ///< area that the user clicked on (None if mouse is up)
	int32_t   m_time_nextact;
	int32_t   m_knob_grabdelta; ///< only while m_pressed == Knob

	const Image* m_pic_minus;      ///< left/up
	const Image* m_pic_plus;       ///< right/down
	const Image* m_pic_background;
	const Image* m_pic_buttons;
};

}

#endif
