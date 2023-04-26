/*
 * Copyright (C) 2003-2023 by the Widelands Development Team
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

#include "ui_basic/tabpanel.h"

#include <SDL_mouse.h>

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication_mousewheel_options.h"

namespace UI {

// Margin around image. The image will be scaled down to fit into this rectangle with preserving
// size.
constexpr int kTabPanelImageMargin = 2;

// Left and right margin around text.
constexpr int kTabPanelTextMargin = 4;

//  height of the bar separating buttons and tab contents
constexpr int kTabPanelSeparatorHeight = 4;

// Constant to flag up when we're not at a tab.
constexpr uint32_t kNotFound = std::numeric_limits<uint32_t>::max();

/*
 * =================
 * class Tab
 * =================
 */
Tab::Tab(TabPanel* const tab_parent,
         PanelStyle s,
         size_t const tab_id,
         int32_t x,
         FontStyle style,
         const std::string& name,
         const std::string& init_title,
         const Image* init_pic,
         const std::string& tooltip_text,
         Panel* const contents)
   : NamedPanel(
        tab_parent, s, name, x, 0, kTabPanelButtonHeight, kTabPanelButtonHeight, tooltip_text),
     parent(tab_parent),
     id(tab_id),
     pic(init_pic),
     font_style_(style),
     rendered_title(nullptr),
     tooltip(tooltip_text),
     panel(contents) {
	set_title(init_title);
}

void Tab::update_template() {
	set_title(title_);  // update rendered_text_
}

void Tab::set_title(const std::string& init_title) {
	title_ = init_title;
	if (init_title.empty()) {
		rendered_title = nullptr;
	} else {
		rendered_title = UI::g_fh->render(as_richtext_paragraph(init_title, font_style_));
		const int16_t old_w = get_w();
		set_size(std::max(kTabPanelButtonHeight, rendered_title->width() + 2 * kTabPanelTextMargin),
		         kTabPanelButtonHeight);
		const int16_t new_w = get_w();
		if (old_w == new_w) {
			return;
		}
		TabPanel& t = dynamic_cast<TabPanel&>(*get_parent());
		bool found_self = false;
		for (Tab* tab : t.tabs()) {
			if (tab == this) {
				assert(!found_self);
				found_self = true;
			} else if (found_self) {
				tab->set_pos(Vector2i(tab->get_x() + new_w - old_w, tab->get_y()));
			}
		}
	}
}

/**
 * Currently active tab
 */
bool Tab::active() {
	return parent->active_ == id;
}
void Tab::activate() {
	return parent->activate(id);
}

bool Tab::handle_mousepress(uint8_t /*btn*/, int32_t /*x*/, int32_t /*y*/) {
	play_click();
	return false;
}

/*
 * =================
 * class TabPanel
 * =================
 */
/**
 * Initialize an empty TabPanel. We use width == 0 as an indicator that the size hasn't been set
 * yet.
 */
TabPanel::TabPanel(Panel* const parent, UI::TabPanelStyle style)
   : Panel(parent,
           style == TabPanelStyle::kFsMenu ? PanelStyle::kFsMenu : PanelStyle::kWui,
           0,
           0,
           0,
           0),
     tab_style_(style),

     highlight_(kNotFound),
     background_style_(style) {
	set_can_focus(true);
}

inline const UI::PanelStyleInfo& TabPanel::background_style() const {
	return *g_style_manager->tabpanel_style(background_style_);
}

std::vector<Recti> TabPanel::focus_overlay_rects() {
	const int f = g_style_manager->focus_border_thickness();
	const Tab* tab = active_ < tabs_.size() ? tabs_[active_] : nullptr;
	const int16_t w = tab != nullptr ? tab->get_w() : get_w();
	const int16_t h = tab != nullptr ? tab->get_h() : kTabPanelButtonHeight;
	if (w < 2 * f || h < 2 * f) {
		return {Recti(0, 0, get_w(), kTabPanelButtonHeight)};
	}

	const int16_t x = tab != nullptr ? tab->get_x() : 0;
	const int16_t y = tab != nullptr ? tab->get_y() : 0;
	return {Recti(x, y, w, f), Recti(x, y + f, f, h - f), Recti(x + w - f, y + f, f, h - f)};
}

bool TabPanel::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	Vector2i mousepos = get_mouse_position();
	size_t id = find_tab(mousepos.x, mousepos.y);
	if (id != kNotFound) {
		int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kTabBar, x, y, modstate);
		if (change != 0) {
			activate(std::max<int>(0, std::min<int>(active() + change, tabs_.size() - 1)));
			return true;
		}
	}
	return Panel::handle_mousewheel(x, y, modstate);
}

bool TabPanel::handle_key(bool down, SDL_Keysym code) {
	if (down && tabs_.size() > 1) {
		bool handle = true;
		uint32_t selected_idx = active();
		const uint32_t max = tabs_.size() - 1;

		if (((code.mod & KMOD_CTRL) != 0) && (code.sym >= SDLK_1 && code.sym <= SDLK_9)) {
			// Keys CTRL + 1-9 directly address the 1st through 9th item in tabpanels with less than 10
			// tabs
			if (max < 9) {
				if (code.sym >= SDLK_1 && code.sym <= static_cast<int>(SDLK_1 + max)) {
					selected_idx = code.sym - SDLK_1;
				} else {
					// don't handle the '9' when there are less than 9 tabs
					handle = false;
				}
			} else {
				// 10 or more tabs – ignore number keys
				handle = false;
			}
		} else {
			switch (code.sym) {
			case SDLK_TAB:
				if ((code.mod & KMOD_CTRL) != 0) {
					if ((code.mod & KMOD_SHIFT) != 0) {
						if (selected_idx > 0) {
							--selected_idx;
						} else {
							selected_idx = 0;
						}
					} else {
						if (selected_idx < max) {
							++selected_idx;
						} else {
							selected_idx = max;
						}
					}
				} else {
					handle = false;
				}
				break;

			case SDLK_HOME:
				selected_idx = 0;
				break;
			case SDLK_END:
				selected_idx = max;
				break;
			default:
				handle = false;
				break;  // not handled
			}
		}
		if (handle) {
			activate(selected_idx);
			return true;
		}
	}
	return Panel::handle_key(down, code);
}

/**
 * Resize the visible tab based on our actual size.
 */
void TabPanel::layout() {
	if (get_w() == 0) {
		// The size hasn't been set yet
		return;
	}
	if (active_ < tabs_.size()) {
		Panel* const panel = tabs_[active_]->panel;
		uint32_t h = get_h();

		// avoid excessive craziness in case there is a wraparound
		h = std::min(h, h - (kTabPanelButtonHeight + kTabPanelSeparatorHeight));
		// If we have a border, we will also want some margin to the bottom
		if (tab_style_ == UI::TabPanelStyle::kFsMenu) {
			h -= kTabPanelSeparatorHeight;
		}
		panel->set_size(get_w(), h);
	}
}

/**
 * Compute our desired size based on the currently selected tab.
 */
void TabPanel::update_desired_size() {
	// size of button row
	int w = kTabPanelButtonHeight * tabs_.size();
	int h = kTabPanelButtonHeight + kTabPanelSeparatorHeight;

	// size of contents
	if (active_ < tabs_.size()) {
		Panel* const panel = tabs_[active_]->panel;
		int panelw;
		int panelh = 0;

		panel->get_desired_size(&panelw, &panelh);
		// TODO(unknown):  the panel might be bigger -> add a scrollbar in that case

		w = std::max(w, panelw);
		h += panelh;
	}

	set_desired_size(w, h);

	// This is not redundant, because even if all this doesn't change our
	// desired size, we were typically called because of a child window that
	// changed, and we need to relayout that.
	layout();
}

/**
 * Add a new textual tab
 */
uint32_t TabPanel::add(const std::string& name,
                       const std::string& title,
                       Panel* const panel,
                       const std::string& tooltip_text,
                       int index) {
	return add_tab(name, title, nullptr, tooltip_text, panel, index);
}

/**
 * Add a new pictorial tab
 */
uint32_t TabPanel::add(const std::string& name,
                       const Image* pic,
                       Panel* const panel,
                       const std::string& tooltip_text,
                       int index) {
	return add_tab(name, "", pic, tooltip_text, panel, index);
}

/** Common adding function for textual and pictorial tabs. */
uint32_t TabPanel::add_tab(const std::string& name,
                           const std::string& title,
                           const Image* pic,
                           const std::string& tooltip_text,
                           Panel* panel,
                           int index) {
	assert(panel);
	assert(panel->get_parent() == this);

	const size_t id = index < 0 ? tabs_.size() : index;
	if (active_ >= id && !tabs_.empty()) {
		++active_;
	}
	int32_t x = id > 0 ? tabs_[id - 1]->get_x() + tabs_[id - 1]->get_w() : 0;
	tabs_.insert(
	   tabs_.begin() + id,
	   new Tab(this, panel_style_, id, x,
	           tab_style_ == TabPanelStyle::kFsMenu ? FontStyle::kFsMenuLabel : FontStyle::kWuiLabel,
	           name, title, pic, tooltip_text, panel));
	const int tabw = tabs_.at(id)->get_w();
	for (size_t i = id + 1; i < tabs_.size(); ++i) {
		Tab* t = tabs_.at(i);
		t->set_pos(Vector2i(t->get_x() + tabw, t->get_y()));
	}

	// Add a margin if there is a border
	if (tab_style_ == UI::TabPanelStyle::kFsMenu) {
		panel->set_border(kTabPanelSeparatorHeight + 1, kTabPanelSeparatorHeight + 1,
		                  kTabPanelSeparatorHeight, kTabPanelSeparatorHeight);
		panel->set_pos(Vector2i(0, kTabPanelButtonHeight));
	} else {
		panel->set_pos(Vector2i(0, kTabPanelButtonHeight + kTabPanelSeparatorHeight));
	}

	panel->set_visible(id == active_);
	update_desired_size();

	return id;
}

/**
 * Make a different tab the currently active tab.
 */
void TabPanel::activate(uint32_t idx) {
	if (active_ < tabs_.size()) {
		tabs_[active_]->panel->set_visible(false);
	}
	if (idx < tabs_.size()) {
		tabs_[idx]->panel->set_visible(true);
	}

	active_ = idx;

	update_desired_size();
	sigclicked();
}

void TabPanel::activate(const std::string& name) {
	for (uint32_t t = 0; t < tabs_.size(); ++t) {
		if (tabs_[t]->get_name() == name) {
			activate(t);
		}
	}
}

/**
 * Return the tabs in order
 */
const TabPanel::TabList& TabPanel::tabs() const {
	return tabs_;
}

bool TabPanel::remove_last_tab(const std::string& tabname) {
	if (tabs_.empty()) {
		return false;
	}
	if (tabs_.back()->get_name() == tabname) {
		tabs_.pop_back();
		if (active_ > tabs_.size() - 1) {
			active_ = 0ul;
		}
		update_desired_size();
		return true;
	}
	return false;
}

/**
 * Draw the buttons and the tab
 */
void TabPanel::draw(RenderTarget& dst) {
	if (get_w() == 0) {
		// The size hasn't been set yet
		return;
	}

	// Draw the background
	static_assert(2 < kTabPanelButtonHeight, "assert(2 < kTabPanelButtonSize) failed.");
	static_assert(4 < kTabPanelButtonHeight, "assert(4 < kTabPanelButtonSize) failed.");
	assert(kTabPanelButtonHeight - 2 <= get_h());

	draw_background(
	   dst, Recti(0, 0, tabs_.back()->get_x() + tabs_.back()->get_w(), kTabPanelButtonHeight - 2),
	   background_style());
	draw_background(
	   dst, Recti(0, kTabPanelButtonHeight - 2, get_w(), get_h() - kTabPanelButtonHeight + 2),
	   background_style());

	// Draw the buttons
	RGBColor black(0, 0, 0);

	// draw the buttons
	int x = 0;
	int tab_width = 0;
	for (size_t idx = 0; idx < tabs_.size(); ++idx) {
		x = tabs_[idx]->get_x();
		tab_width = tabs_[idx]->get_w();

		if (highlight_ == idx) {
			dst.brighten_rect(Recti(x, 0, tab_width, kTabPanelButtonHeight), MOUSE_OVER_BRIGHT_FACTOR);
		}

		// If pic is there, we will assume a pictorial tab
		if (tabs_[idx]->pic != nullptr) {
			// Scale the image down if needed, but keep the ratio.
			constexpr int kMaxImageSize = kTabPanelButtonHeight - 2 * kTabPanelImageMargin;
			double image_scale =
			   std::min(1., std::min(static_cast<double>(kMaxImageSize) / tabs_[idx]->pic->width(),
			                         static_cast<double>(kMaxImageSize) / tabs_[idx]->pic->height()));

			uint16_t picture_width = image_scale * tabs_[idx]->pic->width();
			uint16_t picture_height = image_scale * tabs_[idx]->pic->height();
			dst.blitrect_scale(
			   Rectf(x + (kTabPanelButtonHeight - picture_width) / 2.f,
			         (kTabPanelButtonHeight - picture_height) / 2.f, picture_width, picture_height),
			   tabs_[idx]->pic, Recti(0, 0, tabs_[idx]->pic->width(), tabs_[idx]->pic->height()), 1,
			   BlendMode::UseAlpha);
		} else if (tabs_[idx]->rendered_title != nullptr) {
			tabs_[idx]->rendered_title->draw(
			   dst, Vector2i(x + kTabPanelTextMargin,
			                 (kTabPanelButtonHeight - tabs_[idx]->rendered_title->height()) / 2));
		}

		// Draw top part of border
		dst.brighten_rect(Recti(x, 0, tab_width, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect(Recti(x, 2, 2, kTabPanelButtonHeight - 4), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.fill_rect(Recti(x + tab_width - 2, 2, 1, kTabPanelButtonHeight - 4), black);
		dst.fill_rect(Recti(x + tab_width - 1, 1, 1, kTabPanelButtonHeight - 3), black);

		// Draw bottom part
		if (active_ != idx) {
			dst.brighten_rect(
			   Recti(x, kTabPanelButtonHeight - 2, tab_width, 2), 2 * BUTTON_EDGE_BRIGHT_FACTOR);
		} else {
			dst.brighten_rect(Recti(x, kTabPanelButtonHeight - 2, 2, 2), BUTTON_EDGE_BRIGHT_FACTOR);

			dst.brighten_rect(Recti(x + tab_width - 2, kTabPanelButtonHeight - 2, 2, 2),
			                  2 * BUTTON_EDGE_BRIGHT_FACTOR);
			dst.fill_rect(Recti(x + tab_width - 2, kTabPanelButtonHeight - 1, 1, 1), black);
			dst.fill_rect(Recti(x + tab_width - 2, kTabPanelButtonHeight - 2, 2, 1), black);
		}
	}

	// draw the remaining separator
	assert(x <= get_w());
	dst.brighten_rect(Recti(x + tab_width, kTabPanelButtonHeight - 2, get_w() - x, 2),
	                  2 * BUTTON_EDGE_BRIGHT_FACTOR);

	// Draw border around the main panel
	if (tab_style_ == UI::TabPanelStyle::kFsMenu) {
		//  left edge
		dst.brighten_rect(Recti(0, kTabPanelButtonHeight, 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		//  bottom edge
		dst.fill_rect(Recti(2, get_h() - 2, get_w() - 2, 1), black);
		dst.fill_rect(Recti(1, get_h() - 1, get_w() - 1, 1), black);
		//  right edge
		dst.fill_rect(Recti(get_w() - 2, kTabPanelButtonHeight - 1, 1, get_h() - 2), black);
		dst.fill_rect(Recti(get_w() - 1, kTabPanelButtonHeight - 2, 1, get_h() - 1), black);
	}
}

/**
 * Cancel all highlights and the tooltip when the mouse leaves the panel
 */
void TabPanel::handle_mousein(bool inside) {
	if (!inside && highlight_ != kNotFound) {
		highlight_ = kNotFound;
		set_tooltip("");
	}
}

/**
 * Update highlighting
 */
bool TabPanel::handle_mousemove(
   uint8_t /*state*/, int32_t const x, int32_t const y, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	size_t hl = find_tab(x, y);

	if (hl != highlight_) {
		highlight_ = hl;
		set_tooltip(highlight_ != kNotFound ? tabs_[highlight_]->tooltip : "");
	}
	return true;
}

/**
 * Change the active tab if a tab button has been clicked
 */
bool TabPanel::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		size_t id = find_tab(x, y);
		if (id != kNotFound) {
			activate(id);
			return true;
		}
	}
	return false;
}

/**
 * Find the tab at the coordinates x, y
 * Returns kNotFound if no tab was found
 */
size_t TabPanel::find_tab(int32_t x, int32_t y) const {
	if (y < 0 || y >= kTabPanelButtonHeight) {
		return kNotFound;
	}

	int32_t width = 0;
	for (size_t id = 0; id < tabs_.size(); ++id) {
		width += tabs_[id]->get_w();
		if (width > x) {
			return id;
		}
	}
	return kNotFound;
}
}  // namespace UI
