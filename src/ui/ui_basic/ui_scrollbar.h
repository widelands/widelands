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

#ifndef __S__SCROLLBAR_H
#define __S__SCROLLBAR_H

#include "ui_panel.h"
#include "ui_signal.h"

/** class UIScrollbar
 *
 * This class provides a scrollbar
 */
class UIScrollbar : public UIPanel {
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
		Size = 24,	///< default width for vertical scrollbars, height for horizontal scrollbars
	};

public:
	UIScrollbar(UIPanel *parent, int x, int y, uint w, uint h, bool horiz);

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

	uint		m_pos;		///< from 0 to m_range - 1
	uint		m_pagesize;
	uint		m_steps;

	Area		m_pressed;			///< area that the user clicked on (None if mouse is up)
	int		m_time_nextact;
	int		m_knob_grabdelta;	///< only while m_pressed == Knob

	uint		m_pic_minus;	///< left/up
	uint		m_pic_plus;		///< right/down
	uint		m_pic_background;
	uint		m_pic_buttons;
};

#endif // __S__SCROLLBAR_H
