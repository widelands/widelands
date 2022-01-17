/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#include <memory>

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"

namespace UI {
/**
 * Initialize the progress bar.
 */
ProgressBar::ProgressBar(Panel* const parent,
                         PanelStyle style,
                         int32_t const x,
                         int32_t const y,
                         int32_t const w,
                         int32_t const h,
                         uint32_t const orientation)
   : Panel(parent, style, x, y, w, h),
     orientation_(orientation),
     state_(0),
     total_(100),
     progress_style_(style),
     show_percent_(true) {
}

inline const UI::ProgressbarStyleInfo& ProgressBar::progress_style() const {
	return g_style_manager->progressbar_style(progress_style_);
}

/**
 * Set the current state of progress.
 */
void ProgressBar::set_state(uint32_t state) {
	state_ = state;
}

/**
 * Set the maximum state
 */
void ProgressBar::set_total(uint32_t total) {
	assert(total);
	total_ = total;
}

/**
 * Draw the progressbar.
 */
void ProgressBar::draw(RenderTarget& dst) {
	assert(0 < get_w());
	assert(0 < get_h());
	assert(total_);
	const float fraction = state_ < total_ ? static_cast<float>(state_) / total_ : 1.0f;
	assert(0 <= fraction);
	assert(fraction <= 1);

	const RGBColor& color = fraction <= 0.33f ? progress_style().low_color() :
	                        fraction <= 0.67f ? progress_style().medium_color() :
                                               progress_style().high_color();

	// Draw the actual bar
	if (orientation_ == Horizontal) {
		const float w = get_w() * fraction;
		assert(w <= get_w());

		dst.fill_rect(Recti(0, 0, w, get_h()), color);
		dst.fill_rect(Recti(w, 0, get_w() - w, get_h()), RGBColor(0, 0, 0));
	} else {
		const uint32_t h = static_cast<uint32_t>(get_h() * (1.0f - fraction));

		dst.fill_rect(Recti(0, 0, get_w(), h), RGBColor(0, 0, 0));
		dst.fill_rect(Recti(0, h, get_w(), get_h() - h), color);
	}

	// Print the state in percent without decimal points.
	std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh->render(as_richtext_paragraph(
	   show_percent_ ? format("%u%%", floorf(fraction * 100.f)) : std::to_string(state_),
	   progress_style().font()));
	Vector2i pos(get_w() / 2, get_h() / 2);
	UI::center_vertically(rendered_text->height(), &pos);
	rendered_text->draw(dst, pos, UI::Align::kCenter);
}
}  // namespace UI
