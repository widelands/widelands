/*
 * Copyright (C) 2004 by the Widelands Development Team
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

#ifndef included_ui_progressbar_h
#define included_ui_progressbar_h


/**
A very simple panel that displays progress or productivity in percent,
graphically enhanced with a coloured bar.

The actual state of progress
*/
class UIProgress_Bar : public UIPanel {
public:
	enum {
		Horizontal = 0,		///< from left to right
		Vertical = 1,			///< from bottom to top

		DefaultWidth = 100,
		DefaultHeight = 24,
	};

public:
	UIProgress_Bar(UIPanel* parent, int x, int y, int w, int h, uint orientation);

	uint get_state() { return m_state; }
	void set_state(uint state);
	uint get_total() { return m_total; }
	void set_total(uint total);

protected:
	virtual void draw(RenderTarget* dst);

private:
	uint	m_orientation;
	uint	m_state;				///< m_state is [0..m_total]
	uint	m_total;				///< maximum progress
};


#endif // included_ui_progressbar_h
