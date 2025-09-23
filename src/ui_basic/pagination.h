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

#ifndef WL_UI_BASIC_PAGINATION_H
#define WL_UI_BASIC_PAGINATION_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"

namespace UI {

/**
 * A UI element that allows the user to select the page of a paginated UI element,
 * and also the page size.
 * Page numbers are 1-based.
 */
struct Pagination : public Box {
	Pagination(Panel* parent,
	           const std::string& name,
	           PanelStyle style,
	           int32_t nr_items,
	           int32_t nr_adjacent_buttons_per_side = 3);

	enum class ChangeType { kPage, kSize };
	Notifications::Signal<ChangeType> changed;

	[[nodiscard]] int32_t get_current_page() const {
		return current_page_;
	}
	[[nodiscard]] int32_t get_pagesize() const {
		return pagesize_;
	}
	[[nodiscard]] int32_t get_nr_items() const {
		return nr_items_;
	}
	[[nodiscard]] int32_t get_nr_pages() const;

	void set_nr_items(int32_t items, bool trigger_signal = true);
	void set_page(int32_t page, bool trigger_signal = true);
	void set_pagesize(int32_t size, bool trigger_signal = true, bool store_config = true);

private:
	void load_last_pagesize();
	void update_buttons();
	void update_pagesizes();

	ButtonStyle button_style_;

	std::vector<Button*> buttons_left_;
	std::vector<Button*> buttons_right_;
	Button button_cur_, button_first_, button_last_;
	Dropdown<int32_t> dd_pagesize_;
	std::set<int32_t> available_default_pagesizes_;

	int32_t nr_items_;
	int32_t pagesize_;
	int32_t current_page_;
	bool selecting_{false};
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_PAGINATION_H
