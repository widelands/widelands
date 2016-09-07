/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#include "ui_basic/dropdown.h"

#include <algorithm>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/align.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/image.h"

namespace UI {

BaseDropdown::BaseDropdown(UI::Panel* parent,
                           int32_t x,
                           int32_t y,
                           uint32_t w,
                           uint32_t h,
                           const std::string& label,
                           bool show_tick)
   : UI::Panel(
        parent,
        x,
        y,
        w,
        std::max(24,
                 UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))
                       ->height() +
                    2)),  // Height only to fit the button, so we can use this in Box layout.
     button_box_(this, 0, 0, UI::Box::Horizontal, w, h),
     push_button_(&button_box_,
                  "dropdown_select",
                  0,
                  0,
                  24,
                  get_h(),
                  g_gr->images().get("images/ui_basic/but3.png"),
                  g_gr->images().get("images/ui_basic/scrollbar_down.png"),
                  _("Select")),
     display_button_(&button_box_,
                     "dropdown_label",
                     0,
                     0,
                     w - 24,
                     get_h(),
                     g_gr->images().get("images/ui_basic/but1.png"),
                     label,
                     "",
                     true,
                     false),
     list_(parent,
           0,
           get_h(),
           w,
           h - 2 * get_h(),
           show_tick),  // Hook into parent so we can drop down outside the panel
     label_(label) {
	// Make sure that the list covers and deactivates the elements below it
	list_.set_layout_toplevel(true);
	list_.set_visible(false);
	list_.set_background(g_gr->images().get("images/ui_basic/but1.png"));
	display_button_.set_perm_pressed(true);
	button_box_.add(&display_button_, UI::Align::kLeft);
	button_box_.add(&push_button_, UI::Align::kLeft);
	button_box_.set_size(w, get_h());

	display_button_.sigclicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
	push_button_.sigclicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
	list_.selected.connect(boost::bind(&BaseDropdown::set_value, this));
	list_.clicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
}

void BaseDropdown::add(const std::string& name,
                       const uint32_t value,
                       const Image* pic,
                       const bool select_this,
                       const std::string& tooltip_text) {
	list_.add(name, value, pic, select_this, tooltip_text);
}

bool BaseDropdown::has_selection() const {
	return list_.has_selection();
}

uint32_t BaseDropdown::get_selected() const {
	return list_.get_selected();
}

void BaseDropdown::set_value() {
	if (label_.empty()) {
		display_button_.set_title(list_.has_selection() ?
		                             list_.get_selected_name() :
		                             /** TRANSLATORS: Selection in Dropdown menus. */
		                             pgettext("dropdown", "Not Selected"));
	} else {
		display_button_.set_title(
		   /** TRANSLATORS: Title: Selection in Dropdown menus. */
		   (boost::format("%1%: %2%") % label_ % (list_.has_selection() ?
		                                             list_.get_selected_name() :
		                                             pgettext("dropdown", "Not Selected")))
		      .str());
	}
}

void BaseDropdown::toggle_list() {
	list_.set_visible(!list_.is_visible());
	if (list_.is_visible()) {
		list_.move_to_top();
		list_.focus();
	}
}

}  // namespace UI
