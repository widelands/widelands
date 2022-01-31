/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_basic/listselect.h"

#include <memory>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "graphic/align.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/bidi.h"
#include "graphic/text_layout.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication_options.h"

constexpr int kMargin = 2;
constexpr int kHotkeyGap = 16;
constexpr int kIndentStrength = 20;

namespace UI {

BaseListselect::EntryRecord::EntryRecord(const std::string& init_name,
                                         uint32_t init_entry,
                                         const Image* init_pic,
                                         const std::string& tooltip_text,
                                         const std::string& hotkey_text,
                                         const unsigned i,
                                         const TableStyleInfo& style)
   : name(init_name),
     entry_(init_entry),
     pic(init_pic),
     tooltip(tooltip_text),
     name_alignment(i18n::has_rtl_character(init_name.c_str(), 20) ? Align::kRight : Align::kLeft),
     hotkey_alignment(i18n::has_rtl_character(hotkey_text.c_str(), 20) ? Align::kRight :
                                                                         Align::kLeft),
     indent(i) {
	rendered_name = UI::g_fh->render(as_richtext_paragraph(richtext_escape(name), style.enabled()));
	rendered_hotkey =
	   UI::g_fh->render(as_richtext_paragraph(richtext_escape(hotkey_text), style.hotkey()));
}

inline BaseListselect::EntryRecord::~EntryRecord() {
	// This ensures that the last instance of this smart pointer does
	// not go out of scope in a thread that is not the main thread.
	std::shared_ptr<const UI::RenderedText> r1 = rendered_name;
	std::shared_ptr<const UI::RenderedText> r2 = rendered_hotkey;
	NoteThreadSafeFunction::instantiate([r1, r2]() {}, false);
}

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
                               UI::PanelStyle style,
                               const ListselectLayout selection_mode)
   : Panel(parent, style, x, y, w, h),
     widest_text_(0),
     widest_hotkey_(0),
     scrollbar_(this, get_w() - Scrollbar::kSize, 0, 0, h, style),
     scrollpos_(0),
     selection_(no_selection_index()),
     last_click_time_(-10000),
     last_selection_(no_selection_index()),
     selection_mode_(selection_mode),
     lineheight_(text_height(table_style().enabled()) + kMargin),
     linked_dropdown(nullptr) {
	set_thinks(false);

	scrollbar_.moved.connect([this](int32_t a) { set_scrollpos(a); });

	if (selection_mode_ == ListselectLayout::kShowCheck) {
		check_pic_ = g_image_cache->get("images/ui_basic/list_selected.png");
		max_pic_width_ = check_pic_->width();
		int pic_h = check_pic_->height();
		if (pic_h > lineheight_) {
			lineheight_ = pic_h;
		}
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
	if (linked_dropdown) {
		linked_dropdown->notify_list_deleted();
	}
}

inline const UI::TableStyleInfo& BaseListselect::table_style() const {
	return g_style_manager->table_style(panel_style_);
}
inline const UI::PanelStyleInfo* BaseListselect::background_style() const {
	return selection_mode_ == ListselectLayout::kDropdown ?
             g_style_manager->dropdown_style(panel_style_) :
             nullptr;
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
                         const std::string& tooltip_text,
                         const std::string& hotkey,
                         const unsigned indent) {
	EntryRecord* er = new EntryRecord(name, entry, pic, tooltip_text, hotkey, indent, table_style());

	int entry_height = lineheight_;
	if (pic) {
		int w = pic->width();
		int h = pic->height();
		entry_height = (h >= entry_height) ? h : entry_height;
		if (max_pic_width_ < w) {
			max_pic_width_ = w;
		}
	}

	if (entry_height > lineheight_) {
		lineheight_ = entry_height;
	}

	entry_records_.push_back(er);

	layout();

	if (sel) {
		select(entry_records_.size() - 1);
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
	if (End > size()) {
		End = size();
	}
	for (uint32_t i = Begin; i < End; ++i) {
		for (uint32_t j = i + 1; j < End; ++j) {
			EntryRecord* const eri = entry_records_[i];
			EntryRecord* const erj = entry_records_[j];
			if (strcmp(eri->name.c_str(), erj->name.c_str()) > 0) {
				if (selection_ == i) {
					selection_ = j;
				} else if (selection_ == j) {
					selection_ = i;
				}
				entry_records_[i] = erj;
				entry_records_[j] = eri;
			}
		}
	}
}

/**
 * Scroll to the given position, in pixels.
 */
void BaseListselect::set_scrollpos(const int32_t i) {
	if (scrollpos_ == uint32_t(i)) {
		return;
	}

	scrollpos_ = i;
}

/**
 * Change the currently selected entry
 *
 * Args: i  the entry to select
 */
void BaseListselect::select(const uint32_t i) {
	if (selection_ == i) {
		return;
	}

	if (selection_mode_ == ListselectLayout::kShowCheck) {
		if (selection_ != no_selection_index()) {
			entry_records_[selection_]->pic = nullptr;
		}
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
	return get_selected() != no_selection_index();
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
	return lineheight_ + (selection_mode_ == ListselectLayout::kDropdown ? 2 * kMargin : kMargin);
}

uint32_t BaseListselect::get_eff_w() const {
	return scrollbar_.is_enabled() ? get_w() - scrollbar_.get_w() : get_w();
}

// Make enough room for all texts + hotkeys in tabular format
int BaseListselect::calculate_desired_width() {
	if (entry_records_.empty()) {
		return 0;
	}

	// Find the widest entries
	widest_text_ = 0;
	widest_hotkey_ = 0;
	for (const EntryRecord* er : entry_records_) {
		const int current_text_width = er->rendered_name->width() + (er->indent * kIndentStrength);
		if (current_text_width > widest_text_) {
			widest_text_ = current_text_width;
		}
		const int current_hotkey_width = er->rendered_hotkey->width();
		if (current_hotkey_width > widest_hotkey_) {
			widest_hotkey_ = current_hotkey_width;
		}
	}

	// Add up the width
	int txt_width = widest_text_;
	if (widest_hotkey_ > 0) {
		txt_width += kHotkeyGap;
		txt_width += widest_hotkey_;
	}

	const int picw = max_pic_width_ ? max_pic_width_ + 10 : 0;
	const int old_width = get_w();
	return txt_width + picw + 8 + old_width - get_eff_w();
}

void BaseListselect::layout() {
	const int steps = entry_records_.size() * get_lineheight() - get_h();
	scrollbar_.set_steps(steps);
	if (scrollbar_.is_enabled()) {
		scrollbar_.set_size(Scrollbar::kSize, get_h());
		scrollbar_.set_pos(Vector2i(get_w() - Scrollbar::kSize, 0));
		scrollbar_.set_pagesize(get_h() - 2 * get_lineheight());
		scrollbar_.set_singlestepsize(get_lineheight());
		if (selection_mode_ == ListselectLayout::kDropdown) {
			scrollbar_.set_steps(steps + kMargin);
		}
	} else {
		// Prevent invisible child
		scrollbar_.set_size(0, get_h());
	}
	// For dropdowns, autoincrease width
	if (selection_mode_ == ListselectLayout::kDropdown) {
		const int new_width = calculate_desired_width();
		if (new_width > get_w()) {
			set_size(new_width, get_h());
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

	if (const UI::PanelStyleInfo* s = background_style()) {
		draw_background(dst, *s);
	}

	if (selection_mode_ == ListselectLayout::kDropdown) {
		RGBAColor black(0, 0, 0, 255);
		//  top edge
		dst.brighten_rect(Recti(0, 0, get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
		//  left edge
		dst.brighten_rect(Recti(0, 2, 2, get_h()), BUTTON_EDGE_BRIGHT_FACTOR);
		//  bottom edge
		dst.fill_rect(Recti(2, get_h() - 2, get_eff_w() - 2, 1), black);
		dst.fill_rect(Recti(1, get_h() - 1, get_eff_w() - 1, 1), black);
		//  right edge
		dst.fill_rect(Recti(get_w() - 2, 1, 1, get_h() - 1), black);
		dst.fill_rect(Recti(get_w() - 1, 0, 1, get_h()), black);
	} else {
		dst.brighten_rect(Recti(0, 0, get_eff_w(), get_h()), ms_darken_value);
	}

	while (idx < entry_records_.size()) {
		assert(eff_h < std::numeric_limits<int32_t>::max());

		const EntryRecord& er = *entry_records_[idx];
		const int txt_height = std::max(er.rendered_name->height(), er.rendered_hotkey->height());

		int lineheight = std::max(get_lineheight(), txt_height);

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
			Recti r(point, maxw, lineheight_);
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
			dst.blit(Vector2i(UI::g_fh->fontset()->is_rtl() ?
                              get_eff_w() - er.pic->width() - 1 - kIndentStrength * er.indent :
                              kIndentStrength * er.indent + 1,
			                  y + (lineheight_ - er.pic->height()) / 2),
			         er.pic);
		}

		// Fix vertical position for mixed font heights
		if (get_lineheight() > txt_height) {
			point.y += (lineheight_ - txt_height) / 2;
		} else {
			point.y -= (txt_height - lineheight_) / 2;
		}

		// Don't draw over the bottom edge
		lineheight = std::min(eff_h - static_cast<int>(point.y), lineheight);
		if (lineheight < 0) {
			break;
		}

		// Tabular layout for hotkeys + shift for image width
		Vector2i text_point(point);
		Vector2i hotkey_point(point);
		if (UI::g_fh->fontset()->is_rtl()) {
			if (er.name_alignment == UI::Align::kRight) {
				text_point.x = maxw - widest_text_ - picw;
			} else if (widest_hotkey_ > 0) {
				text_point.x += widest_hotkey_ + kHotkeyGap;
			}
			text_point.x -= kIndentStrength * er.indent;
		} else {
			hotkey_point.x = maxw - widest_hotkey_;
			text_point.x += picw;
			text_point.x += kIndentStrength * er.indent;
		}

		// Position the text and hotkey according to their alignment
		if (er.name_alignment == UI::Align::kRight) {
			text_point.x += widest_text_ - er.rendered_name->width();
		}
		if (er.hotkey_alignment == UI::Align::kRight) {
			hotkey_point.x += widest_hotkey_ - er.rendered_hotkey->width();
		}

		er.rendered_name->draw(dst, text_point, Recti(0, 0, maxw - widest_hotkey_, lineheight),
		                       UI::Align::kLeft, RenderedText::CropMode::kSelf);
		if (er.rendered_hotkey->width() > 0) {
			er.rendered_hotkey->draw(dst, hotkey_point, Recti(0, 0, maxw - widest_text_, lineheight),
			                         UI::Align::kLeft, RenderedText::CropMode::kSelf);
		}
		y += get_lineheight();
		++idx;
	}
}

/**
 * Handle mouse wheel events
 */
bool BaseListselect::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	const uint32_t selected_idx = selection_index();
	uint32_t max = size();
	if (max > 0) {
		--max;
	} else {
		return false;
	}
	if (y != 0 && matches_keymod(modstate, KMOD_NONE)) {
		if (selected_idx > max) {
			select(y < 0 ? 0 : max);
		} else if (y > 0 && selected_idx > 0) {
			select(selected_idx - 1);
		} else if (y < 0 && selected_idx < max) {
			select(selected_idx + 1);
		}
		return scrollbar_.handle_mousewheel(x, y, modstate);
	}
	return false;
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
		if (y < 0 || static_cast<int32_t>(entry_records_.size()) <= y) {
			if (selection_mode_ == ListselectLayout::kDropdown) {
				set_visible(false);
				linked_dropdown->disable_textinput();
				return true;
			}
			return false;
		}
		play_click();
		select(y);
		clicked();

		if  //  check if doubleclicked
		   (time - real_last_click_time < DOUBLE_CLICK_INTERVAL && last_selection_ == selection_ &&
		    selection_ != no_selection_index()) {
			double_clicked(selection_);
		}

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
		switch (code.sym) {
		case SDLK_BACKSPACE:
			if (linked_dropdown != nullptr) {
				linked_dropdown->delete_last_of_filter();
				return true;
			}
			return UI::Panel::handle_key(down, code);
		case SDLK_ESCAPE:
			if (linked_dropdown != nullptr) {
				if (linked_dropdown->is_filtered()) {
					linked_dropdown->clear_filter();
				} else {
					linked_dropdown->set_list_visibility(false);
				}
				return true;
			}
			return UI::Panel::handle_key(down, code);
		default:
			break;
		}

		bool handle = true;
		uint32_t selected_idx = selection_index();
		const uint32_t max = empty() ? 0 : size() - 1;
		const uint32_t pagesize = std::max(1, get_h() / get_lineheight());
		switch (code.sym) {
		case SDLK_DOWN:
			if (!has_selection()) {
				selected_idx = 0;
			} else if (selected_idx < max) {
				++selected_idx;
			}
			break;
		case SDLK_UP:
			if (!has_selection()) {
				selected_idx = max;
			} else if (selected_idx > 0) {
				--selected_idx;
			}
			break;
		case SDLK_HOME:
			selected_idx = 0;
			break;
		case SDLK_END:
			selected_idx = max;
			break;
		case SDLK_PAGEDOWN:
			selected_idx = has_selection() ? std::min(max, selected_idx + pagesize) : 0;
			break;
		case SDLK_PAGEUP:
			selected_idx =
			   has_selection() ? selected_idx > pagesize ? selected_idx - pagesize : 0 : max;
			break;
		default:
			handle = false;
			break;  // not handled
		}
		assert((selected_idx <= max) ^ (selected_idx == no_selection_index()));
		if (handle) {
			select(selected_idx);
			if (selection_index() * get_lineheight() < scrollpos_) {
				scrollpos_ = selection_index() * get_lineheight();
				scrollbar_.set_scrollpos(scrollpos_);
			} else if ((selected_idx + 1) * get_lineheight() - get_inner_h() > scrollpos_) {
				int32_t scrollpos = (selection_index() + 1) * get_lineheight() - get_inner_h();
				scrollpos_ = (scrollpos < 0) ? 0 : scrollpos;
				scrollbar_.set_scrollpos(scrollpos_);
			}
			return true;
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
	if (selection_ == i) {
		selected(selection_ = no_selection_index());
	} else if (i < selection_) {
		--selection_;
	}
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
}  // namespace UI
