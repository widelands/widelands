/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"

namespace UI {

// int instead of uint because of overflow situations
static constexpr int32_t kRichtextMargin = 2;

MultilineTextarea::MultilineTextarea(Panel* const parent,
                                     const int32_t x,
                                     const int32_t y,
                                     const uint32_t w,
                                     const uint32_t h,
                                     UI::PanelStyle style,
                                     const std::string& text,
                                     const Align align,
                                     MultilineTextarea::ScrollMode scroll_mode)
   : Panel(parent, style, x, y, w, h),
     text_(text),
     font_style_(&g_style_manager->font_style(
        style == UI::PanelStyle::kFsMenu ? FontStyle::kFsMenuLabel : FontStyle::kWuiLabel)),
     font_scale_(1.0f),
     align_(align),
     scrollbar_(this, get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, h, style, false) {
	set_thinks(false);

	scrollbar_.moved.connect([this](int32_t a) { scrollpos_changed(a); });

	scrollbar_.set_singlestepsize(text_height(*font_style_, font_scale_));
	scrollbar_.set_steps(1);
	set_scrollmode(scroll_mode);
}

void MultilineTextarea::set_style(const UI::FontStyleInfo& style) {
	font_style_ = &style;
	recompute();
}
void MultilineTextarea::set_font_scale(float scale) {
	font_scale_ = scale;
	scrollbar_.set_singlestepsize(text_height(*font_style_, font_scale_));
	recompute();
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
	// We wrap the text twice. We need to do this to account for the presence/absence of the
	// scrollbar. We first try without the scrollbar (unless it's forced) so it's only enabled
	// when necessary.
	scrollbar_.set_steps(1);
	bool scrollbar_was_enabled = scrollbar_.is_enabled();
	for (int i = 0; i < 2; ++i) {
		int height = 0;
		if (!text_.empty()) {
			// Ensure we have a text width. Simply overflow if there is no width available.
			const int text_width = std::max(10, get_eff_w() - 2 * kRichtextMargin);
			assert(text_width > 0);

			if (!is_richtext(text_)) {
				text_ = make_richtext();
			}
			try {
				rendered_text_ = UI::g_fh->render(text_, text_width);
			} catch (const std::exception& e) {
				log_warn("Error rendering richtext: %s. Text is:\n%s\n", e.what(), text_.c_str());
				text_ = make_richtext();
				rendered_text_ = UI::g_fh->render(text_, text_width);
			}
			height = rendered_text_->height();
		}

		if (scrollmode_ == ScrollMode::kNoScrolling) {
			scrollbar_.set_scrollpos(0);
			scrollbar_.set_steps(1);
			set_desired_size(get_w(), height);
			set_size(get_w(), height);
		}

		scrollbar_.set_steps(height - get_h());

		if (scrollmode_ == ScrollMode::kScrollLog || scrollmode_ == ScrollMode::kScrollLogForced) {
			if (scrollbar_.get_scrollpos() < scrollbar_.get_steps() - 1) {
				scrollbar_.set_scrollpos(height - get_h());
			}
		}

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
	scrollbar_.set_pagesize(get_h() - 2 * font_style_->size() * font_scale_);
}

/**
 * Redraw the textarea
 */
void MultilineTextarea::draw(RenderTarget& dst) {
	if (text_.empty()) {
		return;
	}
	int anchor = 0;
	Align alignment = mirror_alignment(align_, i18n::has_rtl_character(text_.c_str(), 20));
	switch (alignment) {
	// TODO(Arty): We might want to revisit this after the font renderer can handle long strings
	// without whitespaces differently.
	// Currently, such long unbreakable strings are silently assumed to fit the line exactly,
	// which means that rendered_text_->width() might actually be larger than the effective width
	// of the textarea. If we'd allow the anchor here to become negative in this case, it would
	// properly position the longest line (just truncated), BUT the positioning of shorter lines
	// would be off (possibly even outside the textarea, thus invisible) because their positioning
	// is calculated without regard for overlong lines.
	case UI::Align::kCenter:
		anchor = std::max(0, (get_eff_w() - rendered_text_->width()) / 2);
		break;
	case UI::Align::kRight:
		anchor = std::max(0, get_eff_w() - rendered_text_->width() - kRichtextMargin);
		break;
	case UI::Align::kLeft:
		anchor = kRichtextMargin;
	}
	rendered_text_->draw(dst, Vector2i(anchor, 0),
	                     Recti(0, scrollbar_.get_scrollpos(), rendered_text_->width(),
	                           rendered_text_->height() - scrollbar_.get_scrollpos()));
}

bool MultilineTextarea::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return scrollbar_.is_enabled() && scrollbar_.handle_mousewheel(which, x, y);
}
bool MultilineTextarea::handle_key(bool down, SDL_Keysym code) {
	return scrollbar_.handle_key(down, code);
}

void MultilineTextarea::scroll_to_top() {
	scrollbar_.set_scrollpos(0);
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

	FontStyleInfo scaled_style(*font_style_);
	scaled_style.set_size(std::max(g_style_manager->minimum_font_size(),
	                               static_cast<int>(std::ceil(scaled_style.size() * font_scale_))));
	return as_richtext_paragraph(temp, scaled_style, align_);
}

}  // namespace UI
