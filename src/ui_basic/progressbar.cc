/*
 * Copyright (C) 2004, 2006-2007, 2009 by the Widelands Development Team
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

#include "ui_basic/progressbar.h"

#include <cstdio>

#include <boost/format.hpp>

#include "constants.h"
#include "graphic/font.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "text_layout.h"

namespace UI {
/**
 * Initialize the progress bar.
*/
Progress_Bar::Progress_Bar
	(Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 uint32_t const orientation)
	:
	Panel        (parent, x, y, w, h),
	m_orientation(orientation),
	m_state      (0),
	m_total      (100)
{}


/**
 * Set the current state of progress.
*/
void Progress_Bar::set_state(uint32_t state)
{
	m_state = state;

	update();
}


/**
 * Set the maximum state
*/
void Progress_Bar::set_total(uint32_t total)
{
	assert(total);
	m_total = total;

	update();
}


/**
 * Draw the progressbar.
*/
void Progress_Bar::draw(RenderTarget & dst)
{
	assert(0 < get_w());
	assert(0 < get_h());
	assert(m_total);
	const float fraction =
		m_state < m_total ? static_cast<float>(m_state) / m_total : 1.0;
	assert(0 <= fraction);
	assert     (fraction <= 1);

	const RGBColor color = fraction <= 0.33 ?
		RGBColor(255, 0, 0)
		:
		fraction <= 0.67 ? RGBColor(255, 255, 0) : RGBColor(0, 255, 0);

	// Draw the actual bar
	if (m_orientation == Horizontal)
	{
		const uint32_t w = static_cast<uint32_t>(get_w() * fraction);
		assert(w <= static_cast<uint32_t>(get_w()));

		dst.fill_rect(Rect(Point(0, 0), w, get_h()), color);
		dst.fill_rect
			(Rect(Point(w, 0), get_w() - w, get_h()), RGBColor(0, 0, 0));
	}
	else
	{
		const uint32_t h = static_cast<uint32_t>(get_h() * (1.0 - fraction));

		dst.fill_rect(Rect(Point(0, 0), get_w(), h), RGBColor(0, 0, 0));
		dst.fill_rect(Rect(Point(0, h), get_w(), get_h() - h), color);
	}

	// Print the state in percent
	// TODO use UI_FNT_COLOR_BRIGHT when merged
	uint32_t percent = static_cast<uint32_t>(fraction * 100);
	const std::string progress_text =
		(boost::format("<font color=%1$s>%2$i%%</font>") % "ffffff" % percent).str();
	const Point pos(get_w() / 2, get_h() / 2);
	dst.blit(pos, UI::g_fh1->render(as_uifont(progress_text)), CM_Normal, Align_Center);
}
}
