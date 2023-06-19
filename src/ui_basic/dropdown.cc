/*
 * Copyright (C) 2016-2023 by the Widelands Development Team
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

#include "ui_basic/dropdown.h"

#include "base/i18n.h"
#include "base/utf8.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"

namespace {
int base_height(int button_dimension, UI::PanelStyle style) {
	int result =
	   std::max(button_dimension, text_height(g_style_manager->table_style(style).enabled()) + 2);
	return result;
}
}  // namespace

namespace UI {

int BaseDropdown::next_id_ = 0;

// Dropdowns hook into parent elements to be notified of layouting changes. We need to keep track of
// whether a dropdown actually still exists when notified to avoid heap-use-after-free's.
static std::map<int, BaseDropdown*> living_dropdowns_;
// static
void BaseDropdown::layout_if_alive(int id) {
	auto it = living_dropdowns_.find(id);
	if (it != living_dropdowns_.end()) {
		it->second->layout();
	}
}

BaseDropdown::BaseDropdown(UI::Panel* parent,
                           const std::string& name,
                           int32_t x,
                           int32_t y,
                           uint32_t w,
                           uint32_t max_list_items,
                           int button_dimension,
                           const std::string& label,
                           const DropdownType type,
                           UI::PanelStyle style,
                           ButtonStyle button_style)
   : UI::NamedPanel(parent,
                    style,
                    name,
                    x,
                    y,
                    (type == DropdownType::kPictorial || type == DropdownType::kPictorialMenu) ?
                       button_dimension :
                       w,
                    // Height only to fit the button, so we can use this in Box layout.
                    base_height(button_dimension, style)),
     id_(next_id_++),
     max_list_items_(max_list_items),

     base_height_(base_height(button_dimension, style)),

     button_box_(this, style, 0, 0, UI::Box::Horizontal, w, get_h()),
     push_button_(type == DropdownType::kTextual ?
                     new UI::Button(&button_box_,
                                    "dropdown_select",
                                    0,
                                    0,
                                    button_dimension,
                                    get_h(),
                                    button_style,
                                    g_image_cache->get("images/ui_basic/scrollbar_down.png")) :
                     nullptr),
     display_button_(&button_box_,
                     "dropdown_label",
                     0,
                     0,
                     type == DropdownType::kTextual       ? w - button_dimension :
                     type == DropdownType::kTextualNarrow ? w :
                                                            button_dimension,
                     get_h(),
                     type == DropdownType::kTextual ?
                        (style == UI::PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
                                                            UI::ButtonStyle::kWuiSecondary) :
                        button_style,
                     label),
     label_(label),
     type_(type),

     button_style_(button_style)

{
	if (label.empty()) {
		set_tooltip(pgettext("dropdown", "Select Item"));
	} else {
		set_tooltip(label);
	}

	// Close whenever another dropdown is opened
	dropdown_subscriber_ = Notifications::subscribe<NoteDropdown>([this](const NoteDropdown& note) {
		if (id_ != note.id) {
			close();
		}
	});
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& /* note */) { layout(); });

	assert(max_list_items_ > 0);
	// Hook into highest parent that we can get so that we can drop down outside the panel.
	UI::Panel* list_parent = &display_button_;
	while (list_parent->get_parent() != nullptr) {
		list_parent = list_parent->get_parent();
	}
	list_ =
	   new UI::Listselect<uintptr_t>(list_parent, 0, 0, w, 0, style, ListselectLayout::kDropdown);
	list_->set_linked_dropdown(this);

	list_->set_visible(false);
	list_->set_z(UI::Panel::ZOrder::kDropdown);
	button_box_.add(&display_button_, UI::Box::Resizing::kExpandBoth);
	display_button_.sigclicked.connect([this]() {
		toggle_list();
		clear_filter();
	});
	if (push_button_ != nullptr) {
		display_button_.set_perm_pressed(true);
		button_box_.add(push_button_, UI::Box::Resizing::kFullSize);
		push_button_->sigclicked.connect([this]() {
			toggle_list();
			clear_filter();
		});
	}
	button_box_.set_size(w, get_h());
	list_->clicked.connect([this]() {
		set_value();
		clear_filter();
	});

	if (push_button_ != nullptr) {
		push_button_->set_can_focus(false);
	}
	display_button_.set_can_focus(false);
	list_->set_can_focus(false);
	set_can_focus(true);

	set_value();

	const int serial = id_;  // Not a member variable, because when the lambda below is triggered we
	                         // might no longer exist
	living_dropdowns_.insert(std::make_pair(serial, this));
	// Find parent windows, boxes etc. so that we can move the list along with them
	UI::Panel* ancestor = this;
	while ((ancestor = ancestor->get_parent()) != nullptr) {
		ancestor->position_changed.connect([serial] { layout_if_alive(serial); });
	}
	layout();

	list_->initialization_complete();
}

BaseDropdown::~BaseDropdown() {
	// The list needs to be able to drop outside of windows, so it won't close with the window.
	// So, we tell it to die.
	if (list_ != nullptr) {
		list_->set_linked_dropdown(nullptr);
		list_->die();
	}

	// Unsubscribe from layouting hooks
	assert(living_dropdowns_.find(id_) != living_dropdowns_.end());
	living_dropdowns_.erase(living_dropdowns_.find(id_));
}

void BaseDropdown::set_height(int height) {
	max_list_height_ = height - base_height_;
	layout();
}

int BaseDropdown::lineheight() const {
	assert(list_ != nullptr);
	return list_->get_lineheight();
}

std::vector<Recti> BaseDropdown::focus_overlay_rects() {
	return Panel::focus_overlay_rects(2, 2, 0);
}

/*
 * This function is part of an ugly hack to handle dropdowns in modal
 * windows correctly. The problem is that our ListSelect's parent is the
 * topmost parent panel there is. If the currently modal panel is not
 * the topmost one, this would mean that input events are not passed to
 * our list. This is fixed by using this function in the `panel.cc` code
 * to pass events to an open dropdown list (if any) even if it is not a
 * child of the currently modal panel (provided that we ourselves are a
 * descendant of the modal panel).
 */
UI::Panel* BaseDropdown::get_open_dropdown() {
	return (list_ != nullptr) && list_->is_visible() ? list_ : nullptr;
}

void BaseDropdown::layout() {
	int list_width = list_->calculate_desired_width();

	const int new_list_height = std::min(max_list_height_ / list_->get_lineheight(),
	                                     std::min(list_->size(), max_list_items_)) *
	                            list_->get_lineheight();
	list_->set_size(std::max(list_width, button_box_.get_w()), new_list_height);

	// Update list position. The list is hooked into the highest parent that we can get so that we
	// can drop down outside the panel.
	UI::Panel* parent = &display_button_;
	int new_list_x = display_button_.get_x();
	int new_list_y = display_button_.get_y();
	while (parent->get_parent() != nullptr) {
		parent = parent->get_parent();
		new_list_x += parent->get_x() + parent->get_lborder();
		new_list_y += parent->get_y() + parent->get_tborder();
	}

	// Drop up instead of down if it doesn't fit
	if (new_list_y + display_button_.get_h() + list_->get_h() > g_gr->get_yres()) {
		list_offset_y_ = -list_->get_h();
	} else {
		list_offset_y_ = display_button_.get_h();
	}

	// Right align instead of left align if it doesn't fit
	if (new_list_x + list_->get_w() > g_gr->get_xres()) {
		list_offset_x_ = display_button_.get_w() - list_->get_w();
		if (push_button_ != nullptr) {
			list_offset_x_ += push_button_->get_w();
		}
	}

	// Do not drop outside the screen if it fits in neither direction
	if (new_list_x + list_offset_x_ < 0) {
		list_offset_x_ = -new_list_x;
	}
	if (new_list_y + list_offset_y_ < 0) {
		list_offset_y_ = -new_list_y;
	}

	list_->set_pos(Vector2i(new_list_x + list_offset_x_, new_list_y + list_offset_y_));

	// Keep open list on top while dragging
	// TODO(GunChleoc): It would be better to close the list if any other panel is clicked,
	// but we'd need a global "clicked" signal in the Panel class for that.
	// This will imply a complete overhaul of the signal names.
	if (list_->is_visible()) {
		list_->move_to_top();
	}
}

void BaseDropdown::set_size(int nw, int nh) {
	button_box_.set_size(nw, nh);
	Panel::set_size(nw, nh);
	layout();
}
void BaseDropdown::set_desired_size(int nw, int nh) {
	if (push_button_ == nullptr) {
		display_button_.set_desired_size(nw, nh);
	} else {
		display_button_.set_desired_size(nw - nh, nh);
		push_button_->set_desired_size(nh, nh);
	}
	Panel::set_desired_size(nw, nh);
	layout();
}

void BaseDropdown::set_autoexpand_display_button() {
	autoexpand_display_button_ = true;
}

void BaseDropdown::add(const std::string& name,
                       const uint32_t value,
                       const Image* pic,
                       const bool select_this,
                       const std::string& tooltip_text,
                       const std::string& hotkey) {
	assert(pic != nullptr || type_ != DropdownType::kPictorial);
	list_->add(name, value, pic, select_this, tooltip_text, hotkey);
	if (select_this) {
		set_value();
	}

	if (autoexpand_display_button_) {
		/// Fit width of display button to make enough room for the entry's text
		const std::string fitme = label_.empty() ? name : format(_("%1%: %2%"), label_, name);
		const int new_width =
		   text_width(fitme, g_style_manager->button_style(button_style_).enabled().font()) + 8;
		if (new_width > display_button_.get_w()) {
			set_desired_size(get_w() + new_width - display_button_.get_w(), get_h());
			set_size(get_w() + new_width - display_button_.get_w(), get_h());
		}
	}
	layout();
}

bool BaseDropdown::has_selection() const {
	return list_->has_selection();
}

void BaseDropdown::clear_selection() {
	list_->select(BaseListselect::no_selection_index());
	update();
}

uint32_t BaseDropdown::get_selected() const {
	assert(has_selection());
	return list_->get_selected();
}

void BaseDropdown::select(uint32_t entry) {
	assert(entry < list_->size());
	list_->select(entry);
	current_selection_ = list_->selection_index();
	update();
}

void BaseDropdown::set_label(const std::string& text) {
	label_ = text;
	if (type_ != DropdownType::kPictorial && type_ != DropdownType::kPictorialMenu) {
		display_button_.set_title(label_);
	}
}

void BaseDropdown::set_image(const Image* image) {
	display_button_.set_pic(image);
}

void BaseDropdown::set_tooltip(const std::string& text) {
	tooltip_ = text;
	display_button_.set_tooltip(tooltip_);
	if (push_button_ != nullptr) {
		push_button_->set_tooltip(push_button_->enabled() ? tooltip_ : "");
	}
}

void BaseDropdown::set_errored(const std::string& error_message) {
	set_tooltip(format(_("%1%: %2%"), _("Error"), error_message));
	if (type_ != DropdownType::kPictorial && type_ != DropdownType::kPictorialMenu) {
		set_label(_("Error"));
	} else {
		set_image(g_image_cache->get("images/ui_basic/different.png"));
	}
}

void BaseDropdown::set_enabled(bool on) {
	is_enabled_ = on;
	set_can_focus(on);
	if (push_button_ != nullptr) {
		push_button_->set_enabled(on);
		push_button_->set_tooltip(on ? tooltip_ : "");
	}
	display_button_.set_enabled(on);
	set_list_visibility(false);
}

void BaseDropdown::set_disable_style(UI::ButtonDisableStyle disable_style) {
	display_button_.set_disable_style(disable_style);
}

bool BaseDropdown::is_expanded() const {
	return list_->is_visible();
}

void BaseDropdown::set_pos(Vector2i point) {
	UI::Panel::set_pos(point);
	layout();
}

void BaseDropdown::clear() {
	list_->clear();
	current_selection_ = list_->selection_index();
	list_->set_size(list_->get_w(), 0);
}

void BaseDropdown::think() {
	if (list_->is_visible()) {
		// Autocollapse with a bit of tolerance for the mouse movement to make it less fiddly.
		if (!(has_focus() || list_->has_focus()) || is_mouse_away()) {
			toggle_list();
		}
	}
}

uint32_t BaseDropdown::size() const {
	return list_->size();
}

void BaseDropdown::update() {
	if (type_ == DropdownType::kPictorialMenu || type_ == DropdownType::kTextualMenu) {
		// Menus never change their main image and text
		return;
	}

	const std::string name = list_->has_selection() ?
                               list_->get_selected_name() :
                               /** TRANSLATORS: Selection in Dropdown menus. */
                               pgettext("dropdown", "Not Selected");

	if (type_ != DropdownType::kPictorial) {
		if (label_.empty()) {
			display_button_.set_title(name);
		} else {
			/** TRANSLATORS: Label: Value. */
			display_button_.set_title(format(_("%1%: %2%"), label_, name));
		}
		display_button_.set_tooltip(list_->has_selection() ? list_->get_selected_tooltip() :
                                                           tooltip_);
	} else {
		display_button_.set_pic(list_->has_selection() ?
                                 list_->get_selected_image() :
                                 g_image_cache->get("images/ui_basic/different.png"));
		display_button_.set_tooltip(format(_("%1%: %2%"), label_, name));
	}
}

void BaseDropdown::set_value() {
	if (list_->has_selection()) {
		current_selection_ = list_->selection_index();
		save_selected_entry(current_selection_);
		update();
		close();
		selected();
	}
}

void BaseDropdown::toggle() {
	set_list_visibility(!list_->is_visible(), is_mouse_away());
}

void BaseDropdown::set_list_visibility(bool open, bool move_mouse) {
	if (!open) {
		list_->select(current_selection_);
		clear_filter();
	}
	if (!is_enabled_) {
		list_->set_visible(false);
		return;
	}
	list_->set_visible(open);
	if (list_->is_visible()) {
		enable_textinput();
		list_->move_to_top();
		focus();

		Notifications::publish(NoteDropdown(id_));
		if (move_mouse) {
			set_mouse_pos(Vector2i(display_button_.get_x() + (display_button_.get_w() * 3 / 5),
			                       display_button_.get_y() + (display_button_.get_h() * 2 / 5)));
		}

		if ((type_ == DropdownType::kPictorialMenu || type_ == DropdownType::kTextualMenu) &&
		    !has_selection() && !list_->empty()) {
			select(0);
		}
	} else {
		disable_textinput();
	}
	if (type_ != DropdownType::kTextual) {
		display_button_.set_perm_pressed(list_->is_visible());
	}
	// Make sure that the list covers and deactivates the elements below it
	set_layout_toplevel(list_->is_visible());
}

void BaseDropdown::toggle_list() {
	set_list_visibility(!list_->is_visible(), false);
}

void BaseDropdown::close() {
	if (is_expanded()) {
		toggle_list();
	}
}

bool BaseDropdown::is_mouse_away() const {
	return (get_mouse_position().x + mouse_tolerance_) < list_offset_x_ ||
	       get_mouse_position().x > (list_offset_x_ + list_->get_w() + mouse_tolerance_) ||
	       (get_mouse_position().y + mouse_tolerance_) < list_offset_y_ ||
	       get_mouse_position().y > (list_offset_y_ + get_h() + list_->get_h() + mouse_tolerance_);
}

bool BaseDropdown::handle_key(bool down, SDL_Keysym code) {
	if (down && (code.mod & KMOD_CTRL) == 0) {
		switch (code.sym) {
		case SDLK_RETURN:
			if (list_->is_visible()) {
				set_value();
				// Check list visibility again, set_value() might have toggled it
				if (list_->is_visible()) {
					toggle_list();
				}
			} else {
				set_list_visibility(true);
			}
			return true;
		case SDLK_ESCAPE:
			if (is_expanded()) {
				if (is_filtered()) {
					clear_filter();
				} else {
					set_list_visibility(false);
				}
				return true;
			}
			break;
		default:
			break;  // not handled
		}
	}
	if (is_expanded()) {
		return list_->handle_key(down, code);
	}
	return NamedPanel::handle_key(down, code);
}
void BaseDropdown::delete_last_of_filter() {
	if (is_filtered()) {
		size_t pos = current_filter_.size() - 1;
		while (pos > 0 && Utf8::is_utf8_extended(current_filter_.at(pos))) {
			pos--;
		}

		current_filter_.erase(pos);
		apply_filter();
	}
}
bool BaseDropdown::is_filtered() {
	return !current_filter_.empty();
}
void BaseDropdown::disable_textinput() {
	set_handle_textinput(false);
	disable_sdl_textinput();
}

void BaseDropdown::enable_textinput() {
	set_handle_textinput(true);
}

std::string BaseDropdown::get_filter_text() {
	return current_filter_;
}

}  // namespace UI
