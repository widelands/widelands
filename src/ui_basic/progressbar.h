/*
 * Copyright (C) 2004, 2006, 2009 by the Widelands Development Team
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

#ifndef UI_PROGRESSBAR_H
#define UI_PROGRESSBAR_H

#include "ui_basic/panel.h"

namespace UI {

/**
 * A very simple panel that displays progress or productivity in percent,
 * graphically enhanced with a coloured bar.
 *
 * The actual state of progress
*/
struct Progress_Bar : public Panel {
	enum {
		Horizontal = 0, ///< from left to right
		Vertical   = 1, ///< from bottom to top

		DefaultWidth = 100,
		DefaultHeight = 24,
	};

public:
	Progress_Bar
		(Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 uint32_t orientation);

	uint32_t get_state() const {return m_state;}
	void set_state(uint32_t);
	uint32_t get_total() const {return m_total;}
	void set_total(uint32_t);

protected:
	virtual void draw(RenderTarget &) override;

private:
	uint32_t m_orientation;
	uint32_t m_state; ///< m_state is [0..m_total]
	uint32_t m_total; ///< maximum progress
};

}

#endif
