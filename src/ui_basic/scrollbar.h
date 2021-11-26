/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_SCROLLBAR_H
#define WL_UI_BASIC_SCROLLBAR_H

#include "base/rect.h"
#include "ui_basic/panel.h"

namespace UI {
/**
 * This class provides a scrollbar
 */
struct Scrollbar : public Panel {
	enum class Area { None, Minus, Plus, Knob, MinusPage, PlusPage };

	/// default width for vertical scrollbars,
	/// or height for horizontal scrollbars
	static constexpr int kSize = 24;

public:
	Scrollbar(Panel* parent,
	          int32_t x,
	          int32_t y,
	          uint32_t w,
	          uint32_t h,
	          UI::PanelStyle style,
	          bool horiz = false);

	Notifications::Signal<int32_t> moved;

	void set_steps(int32_t steps);
	void set_singlestepsize(uint32_t singlestepsize);
	void set_pagesize(int32_t pagesize);
	void set_scrollpos(int32_t pos);

	bool is_enabled() const;
	uint32_t get_steps() const;
	uint32_t get_singlestepsize() const {
		return singlestepsize_;
	}
	uint32_t get_pagesize() const {
		return pagesize_;
	}
	uint32_t get_scrollpos() const {
		return pos_;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousewheel(int32_t, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym code) override;

	void set_force_draw(bool const t) {
		force_draw_ = t;
	}

	void layout() override;

private:
	Area get_area_for_point(int32_t x, int32_t y);
	uint32_t get_knob_pos();
	uint32_t get_knob_size();
	void set_knob_pos(int32_t p);

	void action(Area area);

	void draw_button(RenderTarget&, Area, const Recti&);
	void draw_area(RenderTarget& dst, Area area, const Recti& r);
	void draw(RenderTarget&) override;
	void think() override;

	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t mx, int32_t my, int32_t xdiff, int32_t ydiff) override;

	bool horizontal_;
	bool force_draw_;  // draw this scrollbar, even if it can't do anything

	uint32_t pos_;  ///< from 0 to range_ - 1
	uint32_t singlestepsize_;
	uint32_t pagesize_;
	uint32_t buttonsize_;
	uint32_t steps_;

	Area pressed_;  ///< area that the user clicked on (None if mouse is up)
	uint32_t time_nextact_;
	int32_t knob_grabdelta_;  ///< only while pressed_ == Knob

	const Image* pic_minus_;  ///< left/up
	const Image* pic_plus_;   ///< right/down

	const UI::PanelStyleInfo& button_style() const;  // Background color and texture.
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_SCROLLBAR_H
