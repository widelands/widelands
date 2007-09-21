/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#ifndef __S__SCROLLBAR_H
#define __S__SCROLLBAR_H

#include <stdint.h>
#include "ui_panel.h"
#include "ui_signal.h"

namespace UI {
/** class Scrollbar
 *
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
		/**
		 * default width for vertical scrollbars, height for horizontal scrollbars
		 */
		Size = 24,
	};

public:
	Scrollbar(Panel *parent, int x, int y, uint32_t w, uint32_t h, bool horiz);

	Signal1<int> moved;

	void set_steps(int steps);
	void set_pagesize(int pagesize);
	void set_pos(int pos);

	uint32_t get_pagesize() const {return m_pagesize;}
	uint32_t get_pos() const {return m_pos;}

	bool handle_mousepress  (const Uint8 btn, int x, int y);

   void set_force_draw(bool t) {m_force_draw=t;}

private:
	Area get_area_for_point(int x, int y);
	int get_knob_pos();
	void set_knob_pos(int p);

	void action(Area area);

	void draw_button(RenderTarget &, const Area area, const Rect);
	void draw_area(RenderTarget &, const Area, const Rect);
	void draw(RenderTarget* dst);
	void think();

	bool handle_mouserelease(const Uint8 btn, int x, int y);
	bool handle_mousemove
		(const Uint8 state, int mx, int my, int xdiff, int ydiff);

private:
	bool m_horizontal;
   bool     m_force_draw; // draw this scrollbar, even if it can't do anything

	uint32_t m_pos;            ///< from 0 to m_range - 1
	uint32_t m_pagesize;
	uint32_t m_steps;

	Area m_pressed;        ///< area that the user clicked on (None if mouse is up)
	int  m_time_nextact;
	int  m_knob_grabdelta; ///< only while m_pressed == Knob

	uint32_t m_pic_minus;      ///< left/up
	uint32_t m_pic_plus;       ///< right/down
	uint32_t m_pic_background;
	uint32_t m_pic_buttons;
};
};

#endif // __S__SCROLLBAR_H
