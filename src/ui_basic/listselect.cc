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

#include "ui_basic/listselect.h"

#include <iostream>

#include <boost/bind.hpp>

#include "base/log.h"
#include "graphic/align.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication.h"

constexpr int kMargin = 2;

namespace UI {
/**
 * Initialize a list select panel
 *
 * Args: parent  parent panel
 *       x       coordinates of the Listselect
 *       y
 *       w       dimensions, in pixels, of the Listselect
 *       h
*/
BaseListselect::BaseListselect(Panel* const parent,
                               const int32_t x,
                               const int32_t y,
                               const uint32_t w,
                               const uint32_t h,
                               const Image* button_background,
                               const ListselectLayout selection_mode)
   : Panel(parent, x, y, w, h),
     lineheight_(
        UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height() +
        kMargin),
     scrollbar_(this, get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, h, button_background),
     scrollpos_(0),
     selection_(no_selection_index()),
     last_click_time_(-10000),
     last_selection_(no_selection_index()),
     selection_mode_(selection_mode),
     background_(nullptr) {
	set_thinks(false);

	scrollbar_.moved.connect(boost::bind(&BaseListselect::set_scrollpos, this, _1));

	if (selection_mode_ == ListselectLayout::kShowCheck) {
		check_pic_ = g_gr->images().get("images/ui_basic/list_selected.png");
		max_pic_width_ = check_pic_->width();
		int pic_h = check_pic_->height();
		if (pic_h > lineheight_)
			lineheight_ = pic_h;
	} else {
		max_pic_width_ = 0;
	}
	set_can_focus(true);
	layout();
}

/**
 * Free allocated resources
*/
BaseListselect::~BaseListselect() {
	clear();
}

/**
 * Remove all entries from the listselect
*/
void BaseListselect::clear() {
	for (EntryRecord* entry : entry_records_) {
		delete entry;
	}
	entry_records_.clear();

	scrollbar_.set_steps(1);
	scrollpos_ = 0;
	selection_ = no_selection_index();
	last_click_time_ = -10000;
	last_selection_ = no_selection_index();
}

/**
 * Add a new entry to the listselect.
 *
 * Args: name   name that will be displayed
 * entry  value returned by get_select()
 *       sel    if true, directly select the new entry
*/
void BaseListselect::add(const std::string& name,
                         uint32_t entry,
                         const Image* pic,
                         bool const sel,
                         const std::string& tooltip_text) {
	EntryRecord* er = new EntryRecord();

	er->entry_ = entry;
	er->pic = pic;
	er->use_clr = false;
	er->name = name;
	er->tooltip = tooltip_text;
	int entry_height = lineheight_;
	if (pic) {
		int w = pic->width();
		int h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (max_pic_width_ < w)
			max_pic_width_ = w;
	}

	if (entry_height > lineheight_)
		lineheight_ = entry_height;

	entry_records_.push_back(er);

	layout();

	if (sel)
		select(entry_records_.size() - 1);
}

void BaseListselect::add_front(const std::string& name,
                               const Image* pic,
                               bool const sel,
                               const std::string& tooltip_text) {
	EntryRecord* er = new EntryRecord();

	er->entry_ = 0;
	for (EntryRecord* temp_entry : entry_records_) {
		++(temp_entry)->entry_;
	}

	er->pic = pic;
	er->use_clr = false;
	er->name = name;
	er->tooltip = tooltip_text;

	int entry_height = lineheight_;
	if (pic) {
		int w = pic->width();
		int h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (max_pic_width_ < w)
			max_pic_width_ = w;
	}

	if (entry_height > lineheight_)
		lineheight_ = entry_height;

	entry_records_.push_front(er);

	layout();

	if (sel)
		select(0);
}

/**
 * Switch two entries
 */
void BaseListselect::switch_entries(const uint32_t m, const uint32_t n) {
	assert(m < size());
	assert(n < size());

	std::swap(entry_records_[m], entry_records_[n]);

	if (selection_ == m) {
		selection_ = n;
		selected(n);
	} else if (selection_ == n) {
		selection_ = m;
		selected(m);
	}
}

/**
 * Sort the listbox alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * start and end defines the beginning and the end of a subarea to
 * sort, for example you might want to sort directories for themselves at the
 * top of list and files at the bottom.
 */
void BaseListselect::sort(const uint32_t Begin, uint32_t End) {
	if (End > size())
		End = size();
	for (uint32_t i = Begin; i < End; ++i)
		for (uint32_t j = i + 1; j < End; ++j) {
			EntryRecord* const eri = entry_records_[i];
			EntryRecord* const erj = entry_records_[j];
			if (strcmp(eri->name.c_str(), erj->name.c_str()) > 0) {
				if (selection_ == i)
					selection_ = j;
				else if (selection_ == j)
					selection_ = i;
				entry_records_[i] = erj;
				entry_records_[j] = eri;
			}
		}
}

/**
 * Scroll to the given position, in pixels.
*/
void BaseListselect::set_scrollpos(const int32_t i) {
	if (scrollpos_ == uint32_t(i))
		return;

	scrollpos_ = i;
}

/**
 * Define a special color that will be used to display the item at the given
 * index.
 */
void BaseListselect::set_entry_color(const uint32_t n, const RGBColor& col) {
	assert(n < entry_records_.size());

	entry_records_[n]->use_clr = true;
	entry_records_[n]->clr = col;
}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void BaseListselect::select(const uint32_t i) {
	if (selection_ == i)
		return;

	if (selection_mode_ == ListselectLayout::kShowCheck) {
		if (selection_ != no_selection_index())
			entry_records_[selection_]->pic = nullptr;
		entry_records_[i]->pic = check_pic_;
	}
	selection_ = i;

	selected(selection_);
}

/**
 * \return \c true if an item is select, or \c false if there is no current
 * selection
 */
bool BaseListselect::has_selection() const {
	return selection_ != no_selection_index();
}

/**
 * \return the ID/entry value of the currently selected item.
 * The entry value is given as a parameter to \ref add
 *
 * Returns no_selection_index() if no item has been selected.
 */
uint32_t BaseListselect::get_selected() const {
	return selection_ < entry_records_.size() ? entry_records_[selection_]->entry_ :
	                                            no_selection_index();
}

/**
 * Remove the currently selected item. Requires an element to have been selected first.
 */
void BaseListselect::remove_selected() {
	assert(selection_ != no_selection_index());
	remove(selection_);
}

/**
 * \return The name of the currently selected entry. Requires an entry to have been selected.
 */
const std::string& BaseListselect::get_selected_name() const {
	assert(selection_ < entry_records_.size());
	return entry_records_[selection_]->name;
}

/**
 * \return The tooltip for the currently selected entry. Requires an entry to have been selected.
 */
const std::string& BaseListselect::get_selected_tooltip() const {
	assert(selection_ < entry_records_.size());
	return entry_records_[selection_]->tooltip;
}

/**
 * \return The image for the currently selected entry. Requires an entry to have been selected.
 */
const Image* BaseListselect::get_selected_image() const {
	assert(selection_ < entry_records_.size());
	return entry_records_[selection_]->pic;
}

int BaseListselect::get_lineheight() const {
	return lineheight_ + kMargin;
}

uint32_t BaseListselect::get_eff_w() const {
	return scrollbar_.is_enabled() ? get_w() - scrollbar_.get_w() : get_w();
}

void BaseListselect::layout() {
	scrollbar_.set_size(scrollbar_.get_w(), get_h());
	scrollbar_.set_pos(Vector2i(get_w() - Scrollbar::kSize, 0));
	scrollbar_.set_pagesize(get_h() - 2 * get_lineheight());
	const int steps = entry_records_.size() * get_lineheight() - get_h();
	scrollbar_.set_steps(steps);
	if (scrollbar_.is_enabled() && selection_mode_ == ListselectLayout::kDropdown) {
		scrollbar_.set_steps(steps + kMargin);
	}
	// For dropdowns, autoincrease width
	if (selection_mode_ == ListselectLayout::kDropdown) {
		for (size_t i = 0; i < entry_records_.size(); ++i) {
			const EntryRecord& er = *entry_records_[i];
			const Image* entry_text_im = UI::g_fh1->render(as_uifont(
			   richtext_escape(er.name), UI_FONT_SIZE_SMALL, er.use_clr ? er.clr : UI_FONT_CLR_FG));
			int picw = max_pic_width_ ? max_pic_width_ + 10 : 0;
			int difference = entry_text_im->width() + picw + 8 - get_eff_w();
			if (difference > 0) {
				set_size(get_w() + difference, get_h());
			}
		}
	}
}

/**
Redraw the listselect box
*/
void BaseListselect::draw(RenderTarget& dst) {
	// draw text lines
	const int eff_h =
	   selection_mode_ == ListselectLayout::kDropdown ? get_inner_h() - kMargin : get_inner_h();
	uint32_t idx = scrollpos_ / get_lineheight();
	int y = 1 + idx * get_lineheight() - scrollpos_;

	if (background_ != nullptr) {
		dst.tile(Recti(Vector2i(0, 0), get_w(), get_h()), background_, Vector2i(0, 0));
	}

	if (selection_mode_ == ListselectLayout::kDropdown) {
		RGBAColor black(0, 0, 0, 255);
		//  top edge
		dst.brighten_rect(Rectf(0.f, 0.f, get_w(), 2.f), BUTTON_EDGE_BRIGHT_FACTOR / 4);
		//  left edge
		dst.brighten_rect(Rectf(0.f, 0.f, 2.f, get_h()), BUTTON_EDGE_BRIGHT_FACTOR);
		//  bottom edge
		dst.fill_rect(Rectf(2.f, get_h() - 2.f, get_eff_w() - 2.f, 1.f), black);
		dst.fill_rect(Rectf(1.f, get_h() - 1.f, get_eff_w() - 1.f, 1.f), black);
		//  right edge
		dst.fill_rect(Rectf(get_w() - 2.f, 1.f, 1.f, get_h() - 1.f), black);
		dst.fill_rect(Rectf(get_w() - 1.f, 0.f, 1.f, get_h()), black);
	} else {
		dst.brighten_rect(Rectf(0.f, 0.f, get_eff_w(), get_h()), ms_darken_value);
	}

	while (idx < entry_records_.size()) {
		assert(eff_h < std::numeric_limits<int32_t>::max());

		const EntryRecord& er = *entry_records_[idx];
		const Image* entry_text_im = UI::g_fh1->render(as_uifont(
		   richtext_escape(er.name), UI_FONT_SIZE_SMALL, er.use_clr ? er.clr : UI_FONT_CLR_FG));

		int lineheight = std::max(get_lineheight(), entry_text_im->height());

		// Don't draw over the bottom edge
		lineheight = std::min(eff_h - y, lineheight);
		if (lineheight < 0) {
			break;
		}

		Vector2i point(selection_mode_ == ListselectLayout::kDropdown ? 3 : 1, y);
		uint32_t maxw =
		   get_eff_w() -
		   (selection_mode_ == ListselectLayout::kDropdown ? scrollbar_.is_enabled() ? 4 : 5 : 2);

		// Highlight the current selected entry
		if (idx == selection_) {
			Rectf r(point, maxw, lineheight_);
			if (r.x < 0) {
				r.w += r.x;
				r.x = 0;
			}
			if (r.y < 0) {
				r.h += r.y;
				r.y = 0;
			}
			assert(2 <= get_eff_w());
			// Make the area a bit more white and more transparent
			if (r.w > 0 && r.h > 0) {
				dst.brighten_rect(r, -ms_darken_value * 2);
			}
		}

		int picw = max_pic_width_ ? max_pic_width_ + 10 : 0;

		// Now draw pictures
		if (er.pic) {
			dst.blit(Vector2i(UI::g_fh1->fontset()->is_rtl() ? get_eff_w() - er.pic->width() - 1 : 1,
			                  y + (get_lineheight() - er.pic->height()) / 2),
			         er.pic);
		}

		Align alignment = i18n::has_rtl_character(er.name.c_str(), 20) ? Align::kRight : Align::kLeft;
		if (alignment == UI::Align::kRight) {
			point.x += maxw - picw;
		}

		UI::correct_for_align(alignment, entry_text_im->width(), &point);

		// Shift for image width
		if (!UI::g_fh1->fontset()->is_rtl()) {
			point.x += picw;
		}

		// Fix vertical position for mixed font heights
		if (get_lineheight() > entry_text_im->height()) {
			point.y += (lineheight_ - entry_text_im->height()) / 2;
		} else {
			point.y -= (entry_text_im->height() - lineheight_) / 2;
		}

		// Don't draw over the bottom edge
		lineheight = std::min(eff_h - static_cast<int>(point.y), lineheight);
		if (lineheight < 0) {
			break;
		}

		// Crop to column width while blitting
		if ((alignment == UI::Align::kRight) &&
		    (maxw + picw) < static_cast<uint32_t>(entry_text_im->width())) {
			// Fix positioning for BiDi languages.
			point.x = 0;

			// We want this always on, e.g. for mixed language savegame filenames, or the languages
			// list
			dst.blitrect(point, entry_text_im, Recti(entry_text_im->width() - maxw + picw, 0, maxw,
			                                         entry_text_im->height()));
		} else {
			dst.blitrect(point, entry_text_im, Recti(0, 0, maxw, lineheight));
		}

		y += get_lineheight();
		++idx;
	}
}

/**
 * Handle mouse wheel events
 */
bool BaseListselect::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return scrollbar_.handle_mousewheel(which, x, y);
}

/**
 * Handle mouse presses: select the appropriate entry
 */
bool BaseListselect::handle_mousepress(const uint8_t btn, int32_t, int32_t y) {
	switch (btn) {

	case SDL_BUTTON_LEFT: {
		uint32_t const time = SDL_GetTicks();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		uint32_t const real_last_click_time = last_click_time_;

		last_selection_ = selection_;
		last_click_time_ = time;

		y = (y + scrollpos_) / get_lineheight();
		if (y < 0 || static_cast<int32_t>(entry_records_.size()) <= y)
			return false;
		play_click();
		select(y);
		clicked(selection_);

		if  //  check if doubleclicked
		   (time - real_last_click_time < DOUBLE_CLICK_INTERVAL && last_selection_ == selection_ &&
		    selection_ != no_selection_index())
			double_clicked(selection_);

		return true;
	}
	default:
		return false;
	}
}

bool BaseListselect::handle_mousemove(uint8_t, int32_t, int32_t y, int32_t, int32_t) {
	y = (y + scrollpos_) / get_lineheight();
	if (y < 0 || static_cast<int32_t>(entry_records_.size()) <= y) {
		set_tooltip("");
		return false;
	}
	if (selection_mode_ == ListselectLayout::kDropdown) {
		select(y);
	}
	set_tooltip(entry_records_.at(y)->tooltip);
	return true;
}

bool BaseListselect::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		uint32_t selected_idx;
		switch (code.sym) {
		case SDLK_KP_2:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		case SDLK_DOWN:
			selected_idx = selection_index() + 1;
			if (selected_idx < size())
				select(selected_idx);
			if ((selection_index() + 1) * get_lineheight() - get_inner_h() > scrollpos_) {
				int32_t scrollpos = (selection_index() + 1) * get_lineheight() - get_inner_h();
				scrollpos_ = (scrollpos < 0) ? 0 : scrollpos;
				scrollbar_.set_scrollpos(scrollpos_);
			}
			return true;
		case SDLK_KP_8:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		case SDLK_UP:
			selected_idx = selection_index();
			if (selected_idx > 0)
				select(selected_idx - 1);
			if (selection_index() * get_lineheight() < scrollpos_) {
				scrollpos_ = selection_index() * get_lineheight();
				scrollbar_.set_scrollpos(scrollpos_);
			}
			return true;
		default:
			break;  // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

/**
 * Remove entry
 */
void BaseListselect::remove(const uint32_t i) {
	assert(i < entry_records_.size());

	delete (entry_records_[i]);
	entry_records_.erase(entry_records_.begin() + i);
	if (selection_ == i)
		selected(selection_ = no_selection_index());
	else if (i < selection_)
		--selection_;
}

/**
 * Remove an entry by name. This only removes
 * the first entry with this name. If none is found, nothing
 * is done
 */
void BaseListselect::remove(const char* const str) {
	for (uint32_t i = 0; i < entry_records_.size(); ++i) {
		if (!strcmp(entry_records_[i]->name.c_str(), str)) {
			remove(i);
			return;
		}
	}
}
}
