/*
 * Copyright (C) 2003-2025 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_BOX_H
#define WL_UI_BASIC_BOX_H

#include <memory>

#include "graphic/align.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {

/**
 * A layouting panel that holds a number of child panels.
 *
 * The Panels you add to the Box must be children of the Box.
 * The Box automatically resizes itself and positions the added children.
 */
struct Box : public Panel {
	// Determines whether the box' contents are layed out horizontally or vertically.
	enum {
		Horizontal = 0,
		Vertical = 1,
	};

	Box(Panel* parent,
	    PanelStyle,
	    const std::string& name,
	    int32_t x,
	    int32_t y,
	    uint32_t orientation,
	    int32_t max_x = 0,
	    int32_t max_y = 0,
	    uint32_t inner_spacing = 0);

	void set_scrolling(bool scroll);
	void set_force_scrolling(bool);

	[[nodiscard]] int32_t get_nritems() const {
		return items_.size();
	}

	enum class Resizing { kAlign, kFullSize, kFillSpace, kExpandBoth };
	void add(Panel* panel, Resizing resizing = Resizing::kAlign, UI::Align align = UI::Align::kLeft);
	void add_space(uint32_t space);
	void add_inf_space();

	void set_min_desired_breadth(uint32_t min);
	void set_inner_spacing(uint32_t size);
	/// Sets the maximum dimensions and calls set_desired_size()
	virtual void set_max_size(int w, int h);

	[[nodiscard]] uint32_t get_orientation() const {
		return orientation_;
	}
	[[nodiscard]] bool is_scrolling() const {
		return scrolling_;
	}
	[[nodiscard]] bool is_force_scrolling() const {
		return force_scrolling_;
	}
	[[nodiscard]] uint32_t get_inner_spacing() const {
		return inner_spacing_;
	}
	[[nodiscard]] uint32_t get_min_desired_breadth() const {
		return mindesiredbreadth_;
	}
	[[nodiscard]] int get_max_x() const {
		return max_x_;
	}
	[[nodiscard]] int get_max_y() const {
		return max_y_;
	}

	// Forget all our entries. Does not delete or even remove them.
	void clear() {
		items_.clear();
	}
	void delete_all_children() {
		scrollbar_.reset();
		free_children();
		clear();
	}

	Scrollbar* get_scrollbar() {
		return scrollbar_.get();
	}

protected:
	void layout() override;
	void update_desired_size() override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	void get_item_desired_size(uint32_t idx, int* depth, int* breadth);
	void get_item_size(uint32_t idx, int* depth, int* breadth);
	void set_item_size(uint32_t idx, int depth, int breadth);
	void set_item_pos(uint32_t idx, int32_t pos);
	bool is_item_visible(uint32_t idx);
	void scrollbar_moved(int32_t);
	void on_death(Panel* p) override;
	void on_visibility_changed() override;

	// Don't resize beyond this size
	int max_x_, max_y_;

public:
	void update_positions();

	struct Item {
		enum Type {
			ItemPanel,
			ItemSpace,
		};

		Type type;

		union {
			struct {
				Panel* panel;
				UI::Align align;
				bool fullsize;
			} panel;
			int space;
		} u;

		bool fillspace;
		int assigned_var_depth;
	};

	[[nodiscard]] const Item& at(size_t i) const {
		return items_.at(i);
	}

private:
	bool scrolling_{false};
	bool force_scrolling_{false};
	std::unique_ptr<Scrollbar> scrollbar_;
	uint32_t orientation_;
	uint32_t mindesiredbreadth_{0U};
	uint32_t inner_spacing_;

	std::vector<Item> items_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_BOX_H
