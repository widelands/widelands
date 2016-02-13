/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "wlapplication.h"

namespace UI {
/**
 * Initialize a list select panel
 *
 * Args: parent  parent panel
 *       x       coordinates of the Listselect
 *       y
 *       w       dimensions, in pixels, of the Listselect
 *       h
 *       align   alignment of text inside the Listselect
*/
BaseListselect::BaseListselect
	(Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 Align const align, bool const show_check)
	:
	Panel(parent, x, y, w, h),
	lineheight_(g_fh->get_fontheight(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_SMALL)),
	scrollbar_      (this, get_w() - 24, 0, 24, h, false),
	scrollpos_     (0),
	selection_     (no_selection_index()),
	last_click_time_(-10000),
	last_selection_(no_selection_index()),
	show_check_(show_check),
	fontname_(UI::g_fh1->fontset().serif()),
	fontsize_(UI_FONT_SIZE_SMALL)
{
	set_thinks(false);

	//  do not allow vertical alignment as it does not make sense
	align_ = align & UI::Align::kHorizontal;

	scrollbar_.moved.connect(boost::bind(&BaseListselect::set_scrollpos, this, _1));
	scrollbar_.set_singlestepsize(g_fh->get_fontheight(fontname_, fontsize_));
	scrollbar_.set_pagesize
		(h - 2 * g_fh->get_fontheight(fontname_, fontsize_));
	scrollbar_.set_steps(1);

	if (show_check) {
		uint32_t pic_h;
		check_pic_ = g_gr->images().get("images/ui_basic/list_selected.png");
		max_pic_width_ = check_pic_->width();
		pic_h = check_pic_->height();
		if (pic_h > lineheight_)
			lineheight_ = pic_h;
	}
	else {
		max_pic_width_ = 0;
	}
	set_can_focus(true);
}


/**
 * Free allocated resources
*/
BaseListselect::~BaseListselect()
{
	clear();
}


/**
 * Remove all entries from the listselect
*/
void BaseListselect::clear() {
	for (EntryRecord * entry : entry_records_) {
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
void BaseListselect::add
	(const std::string& name,
	 uint32_t             entry,
	 const Image*   pic,
	 bool         const   sel,
	 const std::string  & tooltip_text,
	 const std::string  & fontname)
{
	EntryRecord * er = new EntryRecord();

	er->entry_ = entry;
	er->pic   = pic;
	er->use_clr = false;
	er->name    = name;
	er->tooltip = tooltip_text;
	er->font_face = fontname;
	uint32_t entry_height = g_fh->get_fontheight(fontname.empty() ? fontname_ : fontname, fontsize_);
	if (pic) {
		uint16_t w = pic->width();
		uint16_t h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (max_pic_width_ < w)
			max_pic_width_ = w;
	}

	if (entry_height > lineheight_)
		lineheight_ = entry_height;

	entry_records_.push_back(er);

	scrollbar_.set_steps(entry_records_.size() * get_lineheight() - get_h());

	if (sel)
		select(entry_records_.size() - 1);
}

void BaseListselect::add_front
	(const std::string& name,
	 const Image*   pic,
	 bool         const   sel,
	 const std::string  & tooltip_text,
	 const std::string& fontname)
{
	EntryRecord * er = new EntryRecord();

	er->entry_ = 0;
	for (EntryRecord * temp_entry : entry_records_) {
		++(temp_entry)->entry_;
	}

	er->pic   = pic;
	er->use_clr = false;
	er->name    = name;
	er->tooltip = tooltip_text;
	er->font_face = fontname;

	uint32_t entry_height = g_fh->get_fontheight(fontname.empty() ? fontname_ : fontname, fontsize_);
	if (pic) {
		uint16_t w = pic->width();
		uint16_t h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (max_pic_width_ < w)
			max_pic_width_ = w;
	}

	if (entry_height > lineheight_)
		lineheight_ = entry_height;

	entry_records_.push_front(er);

	scrollbar_.set_steps(entry_records_.size() * get_lineheight() - get_h());

	if (sel)
		select(0);
}

/**
 * Switch two entries
 */
void BaseListselect::switch_entries(const uint32_t m, const uint32_t n)
{
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
void BaseListselect::sort(const uint32_t Begin, uint32_t End)
{
	if (End > size())
		End = size();
	for (uint32_t i = Begin; i < End; ++i)
		for (uint32_t j = i + 1; j < End; ++j) {
			EntryRecord * const eri = entry_records_[i];
			EntryRecord * const erj = entry_records_[j];
			if (strcmp(eri->name.c_str(), erj->name.c_str()) > 0) {
				if      (selection_ == i)
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
void BaseListselect::set_scrollpos(const int32_t i)
{
	if (scrollpos_ == uint32_t(i))
		return;

	scrollpos_ = i;
}


/**
 * Define a special color that will be used to display the item at the given
 * index.
 */
void BaseListselect::set_entry_color
	(const uint32_t n, const RGBColor col)
{
	assert(n < entry_records_.size());

	entry_records_[n]->use_clr = true;
	entry_records_[n]->clr = col;
}


/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void BaseListselect::select(const uint32_t i)
{
	if (selection_ == i)
		return;

	if (show_check_) {
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
bool BaseListselect::has_selection() const
{
	return selection_ != no_selection_index();
}


/**
 * \return the ID/entry value of the currently selected item.
 * The entry value is given as a parameter to \ref add
 *
 * Throws an exception when no item is selected.
 */
uint32_t BaseListselect::get_selected() const
{
	if (selection_ == no_selection_index())
		throw NoSelection();

	return entry_records_[selection_]->entry_;
}


/**
 * Remove the currently selected item. Throws an exception when no
 * item is selected.
 */
void BaseListselect::remove_selected()
{
	if (selection_ == no_selection_index())
		throw NoSelection();

	remove(selection_);
}


uint32_t BaseListselect::get_lineheight() const
{
	return lineheight_ + 2;
}

uint32_t BaseListselect::get_eff_w() const
{
	return scrollbar_.is_enabled() ? get_w() - scrollbar_.get_w() : get_w();
}

/**
Redraw the listselect box
*/
void BaseListselect::draw(RenderTarget & dst)
{
	// draw text lines
	const uint32_t lineheight = get_lineheight();
	uint32_t idx = scrollpos_ / lineheight;
	int32_t y = 1 + idx * lineheight - scrollpos_;

	dst.brighten_rect(Rect(Point(0, 0), get_w(), get_h()), ms_darken_value);

	while (idx < entry_records_.size()) {
		assert(get_h() < std::numeric_limits<int32_t>::max());
		if (y >= get_h()) {
			break;
		}

		const EntryRecord & er = *entry_records_[idx];

		// Highlight the current selected entry
		if (idx == selection_) {
			Rect r = Rect(Point(1, y), get_eff_w() - 2, lineheight_);
			if (r.x < 0) {
				r.w += r.x; r.x = 0;
			}
			if (r.y < 0) {
				r.h += r.y; r.y = 0;
			}
			assert(2 <= get_eff_w());
			// Make the area a bit more white and more transparent
			if (r.w > 0 && r.h > 0)
			{
				dst.brighten_rect(r, - ms_darken_value * 2);
			}
		}

		Align draw_alignment = mirror_alignment(align_);

		int32_t const x =
			(static_cast<int>(draw_alignment & UI::Align::kRight))   ? get_eff_w() -      1 :
			(static_cast<int>(draw_alignment & UI::Align::kHCenter)) ? get_eff_w() >>     1 :

			// Pictures are always left aligned, leave some space here
			max_pic_width_         ? max_pic_width_ + 10 :
			1;

		std::string font_face = er.font_face.empty() ? fontname_ : er.font_face;
		RGBColor color = er.use_clr ? er.clr : UI_FONT_CLR_FG;

		// Horizontal center the string
		UI::g_fh->draw_text
			(dst,
			 TextStyle::makebold(Font::get(font_face, fontsize_), color),
			 Point
			 	(x,
			 	 y +
				 (get_lineheight() - g_fh->get_fontheight(font_face, fontsize_))
			 	 /
			 	 2),
			 er.name,
			 draw_alignment);

		// Now draw pictures
		if (er.pic) {
			uint16_t h = er.pic->height();
			dst.blit(Point(1, y + (get_lineheight() - h) / 2), er.pic);
		}

		y += lineheight;
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
bool BaseListselect::handle_mousepress(const uint8_t btn, int32_t, int32_t y)
{
	switch (btn) {

	case SDL_BUTTON_LEFT: {
		uint32_t const time = SDL_GetTicks();

		//  This hick hack is needed if any of the callback functions calls clear
		//  to forget the last clicked time.
		uint32_t const real_last_click_time = last_click_time_;

		last_selection_  = selection_;
		last_click_time_ = time;

		y = (y + scrollpos_) / get_lineheight();
		if (y < 0 || static_cast<int32_t>(entry_records_.size()) <= y)
			return false;
		play_click();
		select(y);
		clicked(selection_);

		if //  check if doubleclicked
			(time - real_last_click_time < DOUBLE_CLICK_INTERVAL
			 &&
			 last_selection_ == selection_
			 &&
			 selection_ != no_selection_index())
			double_clicked(selection_);

		return true;
	}
	default:
		return false;
	}
}

bool BaseListselect::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}

bool BaseListselect::handle_mousemove(uint8_t, int32_t, int32_t y, int32_t, int32_t) {
	y = (y + scrollpos_) / get_lineheight();
	if (y < 0 || static_cast<int32_t>(entry_records_.size()) <= y) {
		set_tooltip("");
		return false;
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
			break; // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

/**
 * Remove entry
 */
void BaseListselect::remove(const uint32_t i)
{
	assert(i < entry_records_.size());

	delete (entry_records_[i]);
	entry_records_.erase(entry_records_.begin() + i);
	if (selection_ == i)
		selected(selection_ = no_selection_index());
	else if (i <  selection_)
		--selection_;
}

/**
 * Remove an entry by name. This only removes
 * the first entry with this name. If none is found, nothing
 * is done
 */
void BaseListselect::remove(const char * const str)
{
	for (uint32_t i = 0; i < entry_records_.size(); ++i) {
		if (!strcmp(entry_records_[i]->name.c_str(), str)) {
			remove(i);
			return;
		}
	}
}

}
