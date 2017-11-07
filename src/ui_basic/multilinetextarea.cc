/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_basic/multilinetextarea.h"

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"

namespace UI {

static const uint32_t RICHTEXT_MARGIN = 2;

MultilineTextarea::MultilineTextarea(Panel* const parent,
                                     const int32_t x,
                                     const int32_t y,
                                     const uint32_t w,
                                     const uint32_t h,
                                     const std::string& text,
                                     const Align align,
                                     const Image* button_background,
                                     MultilineTextarea::ScrollMode scroll_mode)
   : Panel(parent, x, y, w, h),
     text_(text),
     color_(UI_FONT_CLR_FG),
     align_(align),
     force_new_renderer_(false),
     use_old_renderer_(false),
     scrollbar_(this, get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, h, button_background, false),
     pic_background_(nullptr) {
	set_thinks(false);

	scrollbar_.moved.connect(boost::bind(&MultilineTextarea::scrollpos_changed, this, _1));

	scrollbar_.set_singlestepsize(text_height());
	scrollbar_.set_steps(1);
	set_scrollmode(scroll_mode);
	assert(scrollmode_ == MultilineTextarea::ScrollMode::kNoScrolling || Scrollbar::kSize <= w);
}

/**
 * Replace the current text with a new one.
 * Fix up scrolling state if necessary.
 */
void MultilineTextarea::set_text(const std::string& text) {
	text_ = text;
	recompute();
}

/**
 * Recompute the text rendering or rich-text layouting,
 * and adjust scrollbar settings accordingly.
 */
void MultilineTextarea::recompute() {
	int height = 0;

	// We wrap the text twice. We need to do this to account for the presence/absence of the
	// scollbar.
	bool scrollbar_was_enabled = scrollbar_.is_enabled();
	for (int i = 0; i < 2; ++i) {
		if (!is_richtext(text_)) {
			use_old_renderer_ = false;
			height = UI::g_fh1->render(make_richtext(), get_eff_w() - 2 * RICHTEXT_MARGIN)->height();
		} else if (force_new_renderer_) {
			use_old_renderer_ = false;
			height = UI::g_fh1->render(text_, get_eff_w() - 2 * RICHTEXT_MARGIN)->height();
		} else {
			use_old_renderer_ = true;
			rt.set_width(get_eff_w() - 2 * RICHTEXT_MARGIN);
			rt.parse(text_);
			height = rt.height() + 2 * RICHTEXT_MARGIN;
		}

		bool setbottom = false;

		if (scrollmode_ == ScrollMode::kScrollLog || scrollmode_ == ScrollMode::kScrollLogForced) {
			if (scrollbar_.get_scrollpos() >= scrollbar_.get_steps() - 1)
				setbottom = true;
		} else if (scrollmode_ == ScrollMode::kNoScrolling) {
			scrollbar_.set_scrollpos(0);
			scrollbar_.set_steps(1);
			set_desired_size(get_w(), height);
			set_size(get_w(), height);
		}

		scrollbar_.set_steps(height - get_h());
		if (setbottom)
			scrollbar_.set_scrollpos(height - get_h());

		if (scrollbar_.is_enabled() == scrollbar_was_enabled) {
			break;  // No need to wrap twice.
		}
	}
}

/**
 * Callback from the scrollbar.
 */
void MultilineTextarea::scrollpos_changed(int32_t const /* pixels */) {
}

/// Take care of the scrollbar on resize
void MultilineTextarea::layout() {
	recompute();

	// Take care of the scrollbar
	scrollbar_.set_pos(Vector2i(get_w() - Scrollbar::kSize, 0));
	scrollbar_.set_size(Scrollbar::kSize, get_h());
	scrollbar_.set_pagesize(get_h() - 2 * UI_FONT_SIZE_BIG);
}

/**
 * Redraw the textarea
 */
void MultilineTextarea::draw(RenderTarget& dst) {
	if (pic_background_) {
		dst.tile(Recti(0, 0, get_inner_w(), get_inner_h()), pic_background_, Vector2i::zero());
	}
	if (use_old_renderer_) {
		rt.draw(dst, Vector2i(RICHTEXT_MARGIN, RICHTEXT_MARGIN - scrollbar_.get_scrollpos()));
	} else {
		std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh1->render(
		   is_richtext(text_) ? text_ : make_richtext(), get_eff_w() - 2 * RICHTEXT_MARGIN);
		uint32_t blit_width = std::min(rendered_text->width(), static_cast<int>(get_eff_w()));
		uint32_t blit_height = std::min(rendered_text->height(), get_inner_h());

		if (blit_width > 0 && blit_height > 0) {
			int anchor = 0;
			Align alignment = mirror_alignment(align_, text_);
			switch (alignment) {
			case UI::Align::kCenter:
				anchor = (get_eff_w() - blit_width) / 2;
				break;
			case UI::Align::kRight:
				anchor = get_eff_w() - blit_width - RICHTEXT_MARGIN;
				break;
			case UI::Align::kLeft:
				anchor = RICHTEXT_MARGIN;
			}
			rendered_text->draw(dst, Vector2i(anchor, 0),
			                    Recti(0, scrollbar_.get_scrollpos(), blit_width, blit_height));
		}
	}
}

bool MultilineTextarea::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return scrollbar_.handle_mousewheel(which, x, y);
}

void MultilineTextarea::scroll_to_top() {
	scrollbar_.set_scrollpos(0);
}

void MultilineTextarea::set_background(const Image* background) {
	pic_background_ = background;
}
void MultilineTextarea::set_scrollmode(MultilineTextarea::ScrollMode scroll_mode) {
	scrollmode_ = scroll_mode;
	scrollbar_.set_force_draw(scrollmode_ == ScrollMode::kScrollNormalForced ||
	                          scrollmode_ == ScrollMode::kScrollLogForced);
	layout();
}

std::string MultilineTextarea::make_richtext() {
	std::string temp = richtext_escape(text_);
	// Double paragraphs should generate an empty line.
	// We do this here rather than in the font renderer, because a single \n
	// should only create a new line without any added space.
	// \n\n or \n\n\n will give us 1 blank line,
	// \n\n\n or \n\n\n\” will give us 2 blank lines etc.
	// TODO(GunChleoc): Revisit this once the old font renderer is completely gone.
	boost::replace_all(temp, "\n\n", "<br>&nbsp;<br>");
	boost::replace_all(temp, "\n", "<br>");
	return as_aligned(temp, align_, UI_FONT_SIZE_SMALL, color_);
}

}  // namespace UI
