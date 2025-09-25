/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "ui_basic/pagination.h"

#include "base/math.h"

namespace UI {

constexpr int kButtonSize = 32;
constexpr int kSpacing = 4;

constexpr int32_t kShowAll = std::numeric_limits<int32_t>::max();

Pagination::Pagination(Panel* parent,
                       const std::string& name,
                       const PanelStyle style,
                       const int32_t nr_items,
                       const int32_t nr_adjacent_buttons_per_side)
   : Box(parent, style, name, 0, 0, Box::Horizontal),
     button_style_(style == PanelStyle::kWui ? ButtonStyle::kWuiMenu : ButtonStyle::kFsMenuMenu),
     button_cur_(this, "current", 0, 0, kButtonSize, kButtonSize, button_style_, std::string()),
     button_first_(
        this, "first", 0, 0, kButtonSize, kButtonSize, button_style_, "1", _("Go to first page")),
     button_last_(this,
                  "last",
                  0,
                  0,
                  kButtonSize,
                  kButtonSize,
                  button_style_,
                  std::string(),
                  _("Go to last page")),
     dd_pagesize_(this,
                  "last",
                  0,
                  0,
                  250,
                  4,
                  kButtonSize,
                  _("Items per page"),
                  DropdownType::kTextual,
                  style,
                  button_style_),
     nr_items_(nr_items),
     pagesize_(0),
     current_page_(1) {

	button_cur_.set_disable_style(ButtonDisableStyle::kPermpressed);
	button_cur_.set_enabled(false);

	for (int i = 0; i < nr_adjacent_buttons_per_side; ++i) {
		buttons_left_.push_back(new Button(this, format("prev_%d", i + 1), 0, 0, kButtonSize,
		                                   kButtonSize, button_style_, std::string()));
		buttons_right_.push_back(new Button(this, format("next_%d", i + 1), 0, 0, kButtonSize,
		                                    kButtonSize, button_style_, std::string()));
		buttons_left_.at(i)->set_disable_style(ButtonDisableStyle::kFlat |
		                                       ButtonDisableStyle::kMonochrome);
		buttons_right_.at(i)->set_disable_style(ButtonDisableStyle::kFlat |
		                                        ButtonDisableStyle::kMonochrome);
	}

	dd_pagesize_.selected.connect([this]() {
		if (!selecting_) {
			set_pagesize(dd_pagesize_.get_selected());
		}
	});
	button_first_.sigclicked.connect([this]() { set_page(1); });
	button_last_.sigclicked.connect([this]() { set_page(get_nr_pages()); });
	for (int i = 0; i < nr_adjacent_buttons_per_side; ++i) {
		buttons_left_.at(i)->sigclicked.connect([this, i]() { set_page(current_page_ - i - 1); });
		buttons_right_.at(i)->sigclicked.connect([this, i]() { set_page(current_page_ + i + 1); });
	}

	add(&button_first_, Box::Resizing::kAlign, UI::Align::kCenter);
	add_space(kButtonSize);
	for (auto it = buttons_left_.rbegin(); it != buttons_left_.rend(); ++it) {
		add(*it, Box::Resizing::kAlign, UI::Align::kCenter);
		add_space(kSpacing);
	}
	add(&button_cur_, Box::Resizing::kAlign, UI::Align::kCenter);
	for (Button* b : buttons_right_) {
		add_space(kSpacing);
		add(b, Box::Resizing::kAlign, UI::Align::kCenter);
	}
	add_space(kButtonSize);
	add(&button_last_, Box::Resizing::kAlign, UI::Align::kCenter);
	add_space(kButtonSize);
	add(&dd_pagesize_, Box::Resizing::kAlign, UI::Align::kCenter);

	update_pagesizes();
}

int32_t Pagination::get_nr_pages() const {
	int32_t pages = nr_items_ / pagesize_;
	if (pages == 0 || pages * pagesize_ < nr_items_) {
		++pages;
	}
	assert(pages * pagesize_ >= nr_items_);
	return pages;
}

void Pagination::set_page(const int32_t page, const bool trigger_signal) {
	current_page_ = math::clamp(page, 1, get_nr_pages());
	update_buttons();
	if (trigger_signal) {
		changed(ChangeType::kPage);
	}
}

void Pagination::set_pagesize(const int32_t size,
                              const bool trigger_signal,
                              const bool store_config) {
	selecting_ = true;
	pagesize_ = size;

	dd_pagesize_.select(pagesize_);
	if (dd_pagesize_.get_selected() != pagesize_) {
		dd_pagesize_.add(format_l("%d", pagesize_), pagesize_, nullptr, true);
	}

	if (store_config) {
		set_config_int("pagination_pagesize", pagesize_);
	}

	selecting_ = false;

	set_page(current_page_, trigger_signal);
}

void Pagination::load_last_pagesize() {
	if (const int32_t desired = get_config_int("pagination_pagesize", 0); desired > 0) {
		pagesize_ = desired;

		if (desired != kShowAll && available_default_pagesizes_.count(desired) == 0) {
			assert(!available_default_pagesizes_.empty());
			for (int32_t candidate : available_default_pagesizes_) {
				pagesize_ = candidate;
				if (candidate > desired) {
					break;
				}
			}
			assert(available_default_pagesizes_.count(pagesize_) == 1);
		}

		dd_pagesize_.select(pagesize_);
		assert(dd_pagesize_.get_selected() == pagesize_);
		update_buttons();
	}
}

void Pagination::set_nr_items(const int32_t items, const bool trigger_signal) {
	nr_items_ = items;
	update_pagesizes();
	update_buttons();
	if (trigger_signal) {
		changed(ChangeType::kSize);
	}
}

void Pagination::update_pagesizes() {
	selecting_ = true;
	dd_pagesize_.clear();
	available_default_pagesizes_.clear();

	bool added_all = false;
	constexpr int32_t kValues[] = {10, 25, 50};
	for (int32_t multiplier = 1; !added_all; multiplier *= 10) {
		for (int32_t val : kValues) {
			val *= multiplier;
			available_default_pagesizes_.insert(val);
			dd_pagesize_.add(format_l("%d", val), val, nullptr, val == pagesize_);
			if (val >= nr_items_) {
				added_all = true;
				break;
			}
		}
	}

	dd_pagesize_.add(_("All"), kShowAll, nullptr, pagesize_ == kShowAll);
	if (!dd_pagesize_.has_selection()) {
		dd_pagesize_.select(kValues[0]);
		assert(dd_pagesize_.has_selection());
	}

	selecting_ = false;
	set_pagesize(dd_pagesize_.get_selected(), false, false);
	load_last_pagesize();
}

void Pagination::update_buttons() {
	const int32_t pages = get_nr_pages();

	button_first_.set_enabled(current_page_ > 1);
	button_last_.set_enabled(current_page_ < pages);

	button_last_.set_title(format_l("%d", pages));
	button_cur_.set_title(format_l("%d", current_page_));

	for (size_t i = 0; i < buttons_left_.size(); ++i) {
		const int32_t button_page = current_page_ - i - 1;
		buttons_left_.at(i)->set_enabled(button_page > 1);
		buttons_left_.at(i)->set_title(button_page > 1 ? format_l("%d", button_page) : std::string());
		buttons_left_.at(i)->set_tooltip(button_page > 1 ? format_l(_("Go to page %d"), button_page) :
		                                                   std::string());
	}
	for (size_t i = 0; i < buttons_right_.size(); ++i) {
		const int32_t button_page = current_page_ + i + 1;
		buttons_right_.at(i)->set_enabled(button_page < pages);
		buttons_right_.at(i)->set_title(button_page < pages ? format_l("%d", button_page) :
		                                                      std::string());
		buttons_right_.at(i)->set_tooltip(
		   button_page < pages ? format_l(_("Go to page %d"), button_page) : std::string());
	}
}

}  // namespace UI
