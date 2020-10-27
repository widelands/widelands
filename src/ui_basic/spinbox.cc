/*
 * Copyright (C) 2009-2020 by the Widelands Development Team
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

#include "ui_basic/spinbox.h"

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "graphic/text/font_set.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

namespace UI {

struct SpinBoxImpl {
	/// Value hold by the spinbox
	int32_t value = 0;

	/// Minimum and maximum that \ref value may reach
	int32_t min = 0;
	int32_t max = 0;

	int32_t step_size = 0;
	int32_t big_step_size = 0;

	/// List of possible values for type kValueList
	std::vector<int32_t> values;

	/// The unit of the value
	UI::SpinBox::Units unit;

	/// Background tile style of buttons.
	UI::ButtonStyle button_style;

	/// Special names for specific values
	std::map<int32_t, std::string> value_replacements;

	/// The UI parts
	Textarea* text = nullptr;
	UI::MultilineTextarea* label = nullptr;
	Button* button_plus = nullptr;
	Button* button_minus = nullptr;
	Button* button_ten_plus = nullptr;
	Button* button_ten_minus = nullptr;
};

/**
 * SpinBox constructor:
 *
 * initializes a new spinbox with either two (big = false) or four (big = true)
 * buttons. w must be >= the space taken up by the buttons, else the spinbox would become useless
 * and so
 * throws an exception.
 * The spinbox' height and button size is set automatically according to the height of its textarea.
 */
SpinBox::SpinBox(Panel* const parent,
                 const int32_t x,
                 const int32_t y,
                 const uint32_t w,
                 const uint32_t unit_w,
                 int32_t const startval,
                 int32_t const minval,
                 int32_t const maxval,
                 UI::PanelStyle style,
                 const std::string& label_text,
                 const SpinBox::Units& unit,
                 SpinBox::Type type,
                 int32_t step_size,
                 int32_t big_step_size)
   : Panel(parent, style, x, y, std::max(w, unit_w), 0),
     type_(type),
     sbi_(new SpinBoxImpl),
     unit_width_(unit_w),
     button_height_(20),
     padding_(2),
     number_of_paddings_(type_ == SpinBox::Type::kBig ? 6 : 4) {
	if (type_ == SpinBox::Type::kValueList) {
		sbi_->min = 0;
		sbi_->max = 0;
	} else {
		sbi_->min = minval;
		sbi_->max = maxval;
	}
	sbi_->value = startval;
	sbi_->unit = unit;
	sbi_->button_style = style == UI::PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuMenu :
	                                                        UI::ButtonStyle::kWuiSecondary;

	box_ = new UI::Box(this, style, 0, 0, UI::Box::Horizontal, 0, 0, padding_);

	sbi_->label = new UI::MultilineTextarea(box_, 0, 0, 0, 0, style, label_text, UI::Align::kLeft,
	                                        UI::MultilineTextarea::ScrollMode::kNoScrolling);
	box_->add(sbi_->label);

	sbi_->text = new UI::Textarea(
	   box_, style,
	   style == PanelStyle::kFsMenu ? UI::FontStyle::kFsMenuLabel : UI::FontStyle::kWuiLabel, "",
	   UI::Align::kCenter);

	bool is_big = type_ == SpinBox::Type::kBig;

	sbi_->step_size = step_size;
	sbi_->big_step_size = big_step_size;

	sbi_->button_minus =
	   new Button(box_, "-", 0, 0, button_height_, button_height_, sbi_->button_style,
	              g_image_cache->get(is_big ? "images/ui_basic/scrollbar_left.png" :
	                                          "images/ui_basic/scrollbar_down.png"),
	              _("Decrease the value"));
	sbi_->button_plus =
	   new Button(box_, "+", 0, 0, button_height_, button_height_, sbi_->button_style,
	              g_image_cache->get(is_big ? "images/ui_basic/scrollbar_right.png" :
	                                          "images/ui_basic/scrollbar_up.png"),
	              _("Increase the value"));
	sbi_->button_minus->set_can_focus(false);
	sbi_->button_plus->set_can_focus(false);

	if (is_big) {
		sbi_->button_ten_minus =
		   new Button(box_, "--", 0, 0, 2 * button_height_, button_height_, sbi_->button_style,
		              g_image_cache->get("images/ui_basic/scrollbar_left_fast.png"),
		              _("Decrease the value by 10"));
		sbi_->button_ten_plus =
		   new Button(box_, "++", 0, 0, 2 * button_height_, button_height_, sbi_->button_style,
		              g_image_cache->get("images/ui_basic/scrollbar_right_fast.png"),
		              _("Increase the value by 10"));
		sbi_->button_ten_minus->set_can_focus(false);
		sbi_->button_ten_plus->set_can_focus(false);

		sbi_->button_ten_plus->sigclicked.connect([this]() { change_value(sbi_->big_step_size); });
		sbi_->button_ten_minus->sigclicked.connect([this]() { change_value(-sbi_->big_step_size); });
		sbi_->button_ten_plus->set_repeating(true);
		sbi_->button_ten_minus->set_repeating(true);
		buttons_.push_back(sbi_->button_ten_minus);
		buttons_.push_back(sbi_->button_ten_plus);

		box_->add(sbi_->button_ten_minus);
		box_->add(sbi_->button_minus);
		box_->add(sbi_->text);
		box_->add(sbi_->button_plus);
		box_->add(sbi_->button_ten_plus);
	} else {
		box_->add(sbi_->button_minus);
		box_->add(sbi_->text);
		box_->add(sbi_->button_plus);
	}

	sbi_->button_plus->sigclicked.connect([this]() { change_value(sbi_->step_size); });
	sbi_->button_minus->sigclicked.connect([this]() { change_value(-sbi_->step_size); });
	sbi_->button_plus->set_repeating(true);
	sbi_->button_minus->set_repeating(true);
	buttons_.push_back(sbi_->button_minus);
	buttons_.push_back(sbi_->button_plus);

	set_can_focus(true);
	layout();
	update();
}

SpinBox::~SpinBox() {
	delete sbi_;
	sbi_ = nullptr;
}

bool SpinBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {

		// Up and Right behave like clicking the Increase button
		case SDLK_KP_6:
		case SDLK_KP_8:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_UP:
		case SDLK_RIGHT:
			if (sbi_->button_plus) {
				change_value(sbi_->step_size);
				return true;
			}
			break;

		// Down and Left behave like clicking the Decrease button
		case SDLK_KP_2:
		case SDLK_KP_4:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DOWN:
		case SDLK_LEFT:
			if (sbi_->button_minus) {
				change_value(-sbi_->step_size);
				return true;
			}
			break;

		// PageUp behaves like clicking the IncreaseFast button (if any)
		case SDLK_KP_9:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_PAGEUP:
			if (sbi_->button_ten_plus) {
				change_value(sbi_->big_step_size);
				return true;
			}
			break;

		// PageDown behaves like clicking the DecreaseFast button (if any)
		case SDLK_KP_3:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_PAGEDOWN:
			if (sbi_->button_ten_minus) {
				change_value(-sbi_->big_step_size);
				return true;
			}
			break;

		default:
			break;
		}
	}
	return Panel::handle_key(down, code);
}

void SpinBox::layout() {
	// Do not layout if the size hasn't been set yet.
	if (get_w() == 0 && get_h() == 0) {
		return;
	}

	// 40 is an ad hoc width estimate for the MultilineTextarea scrollbar + a bit of text.
	if (!sbi_->label->get_text().empty() && (get_w() + padding_ + 40) <= unit_width_) {
		throw wexception("SpinBox: Overall width %d must be bigger than unit width %d + %d * %d + "
		                 "40 for padding",
		                 get_w(), unit_width_, number_of_paddings_, padding_);
	}

	if (unit_width_ < (type_ == SpinBox::Type::kBig ? 7 * button_height_ : 3 * button_height_)) {
		log_warn("Not enough space to draw spinbox \"%s\".\n"
		         "Width %d is smaller than required width %d."
		         "Please report as a bug.\n",
		         sbi_->label->get_text().c_str(), unit_width_,
		         (type_ == SpinBox::Type::kBig ? 7 * button_height_ : 3 * button_height_));
	}

	// 10 is arbitrary, the actual height will be set by the Multilinetextarea itself
	sbi_->label->set_size(get_w() - unit_width_ - number_of_paddings_ * padding_, 10);
	if (type_ == SpinBox::Type::kBig) {
		sbi_->text->set_fixed_width(unit_width_ - 2 * sbi_->button_ten_plus->get_w() -
		                            2 * sbi_->button_minus->get_w() - 2 * padding_);
	} else {
		sbi_->text->set_fixed_width(unit_width_ - 2 * sbi_->button_minus->get_w());
	}

	uint32_t box_height = std::max(sbi_->label->get_h(), static_cast<int32_t>(button_height_));
	box_->set_size(get_w(), box_height);
	set_desired_size(get_w(), box_height);
	set_size(get_w(), box_height);
}

/**
 * private function - takes care about all updates in the UI elements
 */
void SpinBox::update() {
	if (sbi_->value_replacements.count(sbi_->value) == 1) {
		sbi_->text->set_text(sbi_->value_replacements.at(sbi_->value));
	} else {
		if (type_ == SpinBox::Type::kValueList) {
			if ((sbi_->value >= 0) && (sbi_->values.size() > static_cast<size_t>(sbi_->value))) {
				sbi_->text->set_text(unit_text(sbi_->values.at(sbi_->value)));
			} else {
				sbi_->text->set_text(
				   "undefined");  // The user should never see this, so we're not localizing
			}
		} else {
			sbi_->text->set_text(unit_text(sbi_->value));
		}
	}

	sbi_->button_minus->set_enabled(sbi_->min < sbi_->value);
	sbi_->button_plus->set_enabled(sbi_->value < sbi_->max);
	if (type_ == SpinBox::Type::kBig) {
		sbi_->button_ten_minus->set_enabled(sbi_->min < sbi_->value);
		sbi_->button_ten_plus->set_enabled(sbi_->value < sbi_->max);
	}
}

/**
 * Display width for the spinbox unit
 * \note This does not relayout the spinbox.
 */
void SpinBox::set_unit_width(uint32_t width) {
	unit_width_ = width;
}

/**
 * private function called by spinbox buttons to in-/decrease the value
 */
void SpinBox::change_value(int32_t const value) {
	set_value(value + sbi_->value);
	changed();
}

/**
 * manually sets the used value to a given value
 */
void SpinBox::set_value(int32_t const value) {
	sbi_->value = value;
	if (sbi_->value > sbi_->max) {
		sbi_->value = sbi_->max;
	} else if (sbi_->value < sbi_->min) {
		sbi_->value = sbi_->min;
	}
	update();
}

void SpinBox::set_value_list(const std::vector<int32_t>& values) {
	sbi_->values = values;
	sbi_->min = 0;
	sbi_->max = values.size() - 1;
	update();
}

/**
 * sets the interval the value may lay in and fixes the value, if outside.
 */
void SpinBox::set_interval(int32_t const min, int32_t const max) {
	sbi_->max = max;
	sbi_->min = min;
	if (sbi_->value > max) {
		sbi_->value = max;
	} else if (sbi_->value < min) {
		sbi_->value = min;
	}
	update();
}

/**
 * \returns the value
 */
int32_t SpinBox::get_value() const {
	if (type_ == SpinBox::Type::kValueList) {
		if ((sbi_->value >= 0) && (sbi_->values.size() > static_cast<size_t>(sbi_->value))) {
			return sbi_->values.at(sbi_->value);
		} else {
			return -1;
		}
	} else {
		return sbi_->value;
	}
}

/**
 * Adds a replacement text for a specific value
 * overwrites an old replacement if one exists.
 */
void SpinBox::add_replacement(int32_t value, const std::string& text) {
	sbi_->value_replacements[value] = text;
	update();
}

const std::string SpinBox::unit_text(int32_t value) const {
	switch (sbi_->unit) {
	case (Units::kMinutes):
		/** TRANSLATORS: A spinbox unit */
		return (boost::format(ngettext("%d minute", "%d minutes", value)) % value).str();
	case (Units::kPixels):
		/** TRANSLATORS: A spinbox unit */
		return (boost::format(ngettext("%d pixel", "%d pixels", value)) % value).str();
	case (Units::kFields):
		/** TRANSLATORS: A spinbox unit */
		return (boost::format(ngettext("%d field", "%d fields", value)) % value).str();
	case (Units::kPercent):
		/** TRANSLATORS: A spinbox unit */
		return (boost::format(_("%i %%")) % value).str();
	case (Units::kNone):
		return (boost::format("%d") % value).str();
	}
	NEVER_HERE();
}

}  // namespace UI
