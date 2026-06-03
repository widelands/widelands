/*
 * Copyright (C) 2009-2026 by the Widelands Development Team
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

#include "ui/basic/spinbox.h"

#include <cassert>
#include <limits>

#include <SDL_keyboard.h>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "ui/basic/button.h"
#include "ui/basic/multilinetextarea.h"
#include "ui/basic/textarea.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"

namespace UI {

static inline UI::ButtonStyle get_button_style_for_value(const UI::PanelStyle style) {
	return style == PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
	                                      UI::ButtonStyle::kWuiSecondary;
};

// static
int SpinBox::default_unit_width_fit_text(const UI::PanelStyle style, const std::string& text) {
	return text_width(
	          text,
	          g_style_manager->button_style(get_button_style_for_value(style)).disabled().font()) +
	       2 * default_padding();
}
int SpinBox::default_unit_width_narrow(const UI::PanelStyle style) {
	return default_unit_width_fit_text(style, "100%");
}
int SpinBox::default_unit_width_wide(const UI::PanelStyle style) {
	return default_unit_width_fit_text(style, std::string(12, 'H'));
}

struct SpinBoxImpl {
	/// Value held by the spinbox
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
	UI::Button* text = nullptr;
	UI::MultilineTextarea* label = nullptr;
	UI::Panel* label_padding = nullptr;
	Button* button_plus = nullptr;
	Button* button_minus = nullptr;
	Button* button_ten_plus = nullptr;
	Button* button_ten_minus = nullptr;
};

/**
 * SpinBox constructor:
 *
 * initializes a new spinbox with either two (big = false) or four (big = true)
 * buttons.
 * The spinbox' height and button size is set automatically according to the height of its textarea.
 */
SpinBox::SpinBox(Panel* const parent,
                 const std::string& name,
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
   : Box(parent, style, name, 0, 0, UI::Box::Horizontal),
     type_(type),
     sbi_(new SpinBoxImpl),
     button_size_(default_button_size_small()),
     big_step_button_width_(default_button_size()) {
	if (type_ == SpinBox::Type::kValueList) {
		sbi_->min = 0;
		sbi_->max = 0;
	} else {
		assert(minval <= maxval);

		sbi_->min = minval;
		sbi_->max = maxval;
	}
	sbi_->value = startval;
	sbi_->unit = unit;
	sbi_->button_style = style == UI::PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuMenu :
	                                                        UI::ButtonStyle::kWuiSecondary;

	sbi_->label = new UI::MultilineTextarea(
	   this, "label", 0, 0,
	   text_width(label_text, g_style_manager->font_style(style == UI::PanelStyle::kFsMenu ?
	                                                         FontStyle::kFsMenuLabel :
	                                                         FontStyle::kWuiLabel)) +
	      2 * default_padding(),
	   0, style, label_text, UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling);
	add(sbi_->label, UI::Box::Resizing::kFillSpace, UI::Align::kBottom);

	sbi_->text = new UI::Button(
	   this, "value", 0, 0, unit_w, button_size_, get_button_style_for_value(style), "");
	sbi_->text->set_tooltip(label_text);
	sbi_->text->set_disable_style(UI::ButtonDisableStyle::kPermpressed);
	sbi_->text->set_enabled(false);

	assert(step_size > 0);
	sbi_->step_size = step_size;

	bool is_big = type_ == SpinBox::Type::kBig;
	if (is_big) {
		assert(big_step_size > 0);
		sbi_->big_step_size = big_step_size;

		// Prevent integer overflows
		assert(maxval < std::numeric_limits<int32_t>::max() - big_step_size);
		assert(minval > std::numeric_limits<int32_t>::min() + big_step_size);
	} else {
		// Step size for PgUp/PgDn
		calculate_big_step();
	}

	std::string button_tooltip;
	if (type_ == SpinBox::Type::kValueList) {
		button_tooltip = _("Decrease the value");
	} else {
		/** TRANSLATORS: You may want to treat this as "Decrease the value %s", you get
		                 the chance to translate the substituted text as "by <n units>" */
		button_tooltip = format(_("Decrease the value by %s"), unit_text(sbi_->step_size, true));
	}
	sbi_->button_minus =
	   new Button(this, "-", 0, 0, button_size_, button_size_, sbi_->button_style,
	              g_image_cache->get(is_big ? "images/ui_basic/scrollbar_left.png" :
	                                          "images/ui_basic/scrollbar_down.png"),
	              button_tooltip);

	if (type_ == SpinBox::Type::kValueList) {
		button_tooltip = _("Increase the value");
	} else {
		/** TRANSLATORS: You may want to treat this as "Increase the value %s", you get
		                 the chance to translate the substituted text as "by <n units>" */
		button_tooltip = format(_("Increase the value by %s"), unit_text(sbi_->step_size, true));
	}
	sbi_->button_plus =
	   new Button(this, "+", 0, 0, button_size_, button_size_, sbi_->button_style,
	              g_image_cache->get(is_big ? "images/ui_basic/scrollbar_right.png" :
	                                          "images/ui_basic/scrollbar_up.png"),
	              button_tooltip);

	sbi_->button_minus->set_can_focus(false);
	sbi_->button_plus->set_can_focus(false);

	if (is_big) {
		sbi_->button_ten_minus =
		   new Button(this, "--", 0, 0, big_step_button_width_, button_size_, sbi_->button_style,
		              g_image_cache->get("images/ui_basic/scrollbar_left_fast.png"),
		              format(_("Decrease the value by %s"), unit_text(sbi_->big_step_size, true)));
		sbi_->button_ten_plus =
		   new Button(this, "++", 0, 0, big_step_button_width_, button_size_, sbi_->button_style,
		              g_image_cache->get("images/ui_basic/scrollbar_right_fast.png"),
		              format(_("Increase the value by %s"), unit_text(sbi_->big_step_size, true)));
		sbi_->button_ten_minus->set_can_focus(false);
		sbi_->button_ten_plus->set_can_focus(false);

		sbi_->button_ten_plus->sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_CTRL) != 0) {
				set_value(sbi_->max);
			} else {
				change_value(sbi_->big_step_size);
			}
		});
		sbi_->button_ten_minus->sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_CTRL) != 0) {
				set_value(sbi_->min);
			} else {
				change_value(-sbi_->big_step_size);
			}
		});
		sbi_->button_ten_plus->set_repeating(true);
		sbi_->button_ten_minus->set_repeating(true);
		buttons_.push_back(sbi_->button_ten_minus);
		buttons_.push_back(sbi_->button_ten_plus);

		add(sbi_->button_ten_minus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
		add_space(default_spacing());
		add(sbi_->button_minus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
		add(sbi_->text, Box::Resizing::kAlign, UI::Align::kBottom);
		add(sbi_->button_plus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
		add_space(default_spacing());
		add(sbi_->button_ten_plus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
	} else {
		add(sbi_->button_minus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
		add(sbi_->text, Box::Resizing::kAlign, UI::Align::kBottom);
		add(sbi_->button_plus, UI::Box::Resizing::kAlign, UI::Align::kBottom);
	}

	sbi_->button_plus->sigclicked.connect([this]() {
		if ((SDL_GetModState() & KMOD_CTRL) != 0) {
			set_value(sbi_->max);
		} else {
			change_value(sbi_->step_size);
		}
	});
	sbi_->button_minus->sigclicked.connect([this]() {
		if ((SDL_GetModState() & KMOD_CTRL) != 0) {
			set_value(sbi_->min);
		} else {
			change_value(-sbi_->step_size);
		}
	});
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
		switch (get_keyboard_change(code)) {
		case ChangeType::kNone:
			break;
		case ChangeType::kPlus:
			change_value(sbi_->step_size);
			return true;
		case ChangeType::kMinus:
			change_value(-sbi_->step_size);
			return true;
		case ChangeType::kBigPlus:
			change_value(sbi_->big_step_size);
			return true;
		case ChangeType::kBigMinus:
			change_value(-sbi_->big_step_size);
			return true;
		case ChangeType::kSetMax:
			set_value(sbi_->max);
			return true;
		case ChangeType::kSetMin:
			set_value(sbi_->min);
			return true;
		default:
			NEVER_HERE();
		}
	}
	return Box::handle_key(down, code);
}

bool SpinBox::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValue, x, y, modstate);
	if (change != 0) {
		change_value(change * sbi_->step_size);
		return true;
	}
	// Try big step
	change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValueBig, x, y, modstate);
	if (change != 0) {
		change_value(change * sbi_->big_step_size);
		return true;
	}
	return false;
}

void SpinBox::set_min_height(uint32_t height) {
	set_min_desired_breadth(height);
}

/**
 * private function - takes care about all updates in the UI elements
 */
void SpinBox::update() {
	if (sbi_->value_replacements.count(sbi_->value) == 1) {
		sbi_->text->set_title(sbi_->value_replacements.at(sbi_->value));
	} else {
		if (type_ == SpinBox::Type::kValueList) {
			if ((sbi_->value >= 0) && (sbi_->values.size() > static_cast<size_t>(sbi_->value))) {
				sbi_->text->set_title(unit_text(sbi_->values.at(sbi_->value)));
			} else {
				sbi_->text->set_title(
				   "undefined");  // The user should never see this, so we're not localizing
			}
		} else {
			sbi_->text->set_title(unit_text(sbi_->value));
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
 */
void SpinBox::set_unit_width(uint32_t width) {
	sbi_->text->set_size(width, button_size_);
}

/**
 * private function called by spinbox buttons to in-/decrease the value
 */
void SpinBox::change_value(int32_t const value) {
	if (value != 0) {
		set_value(value + sbi_->value);
	}
}

/**
 * manually sets the used value to a given value
 */
void SpinBox::set_value(int32_t const value, const bool trigger_signal) {
	if (sbi_->value == value) {
		return;
	}
	if (value > sbi_->max) {
		if (sbi_->value == sbi_->max) {
			return;
		}
		sbi_->value = sbi_->max;
	} else if (value < sbi_->min) {
		if (sbi_->value == sbi_->min) {
			return;
		}
		sbi_->value = sbi_->min;
	} else {
		sbi_->value = value;
	}
	update();
	if (trigger_signal) {
		changed();
	}
}

void SpinBox::set_value_list(const std::vector<int32_t>& values) {
	sbi_->values = values;
	sbi_->min = 0;
	sbi_->max = values.size() - 1;
	calculate_big_step();
	update();
}

/**
 * sets the interval the value may lay in and fixes the value, if outside.
 */
void SpinBox::set_interval(int32_t const min,
                           int32_t const max,
                           const bool trigger_signal_if_changed) {
	assert(min <= max);
	sbi_->max = max;
	sbi_->min = min;
	bool changed_val = false;
	if (sbi_->value > max) {
		sbi_->value = max;
		changed_val = true;
	} else if (sbi_->value < min) {
		sbi_->value = min;
		changed_val = true;
	}
	calculate_big_step();
	update();
	if (changed_val && trigger_signal_if_changed) {
		changed();
	}
}

void SpinBox::calculate_big_step() {
	if (type_ == SpinBox::Type::kBig) {
		// Should have been set when the spinbox was set up, don't mess with it
		return;
	}

	assert(sbi_->min <= sbi_->max);
	// Also prevent integer overflow
	assert(sbi_->max - sbi_->min >= 0);
	assert(sbi_->step_size > 0);

	// It's OK if it just jumps to min/max when interval is smaller than ChangeBigStep::kMediumRange,
	// but let's have several big steps in the interval otherwise
	const int32_t big_step_multiplier =
	   (((sbi_->max - sbi_->min) / sbi_->step_size) <= 2 * ChangeBigStep::kWideRange) ?
	      ChangeBigStep::kMediumRange :
	      ChangeBigStep::kWideRange;

	sbi_->big_step_size = sbi_->step_size * big_step_multiplier;

	// Prevent integer overflows
	assert(sbi_->max < std::numeric_limits<int32_t>::max() - sbi_->big_step_size);
	assert(sbi_->min > std::numeric_limits<int32_t>::min() + sbi_->big_step_size);
}

/**
 * \returns the value
 */
int32_t SpinBox::get_value() const {
	if (type_ == SpinBox::Type::kValueList) {
		if ((sbi_->value >= 0) && (sbi_->values.size() > static_cast<size_t>(sbi_->value))) {
			return sbi_->values.at(sbi_->value);
		}
		return -1;
	}
	return sbi_->value;
}

int32_t SpinBox::get_min() const {
	return sbi_->min;
}
int32_t SpinBox::get_max() const {
	return sbi_->max;
}

/**
 * Adds a replacement text for a specific value
 * overwrites an old replacement if one exists.
 */
void SpinBox::add_replacement(int32_t value, const std::string& text) {
	sbi_->value_replacements[value] = text;
	update();
}

const std::string SpinBox::unit_text(int32_t value, const bool change) const {
	switch (sbi_->unit) {
	case (Units::kMinutes): {
		if (value < 60) {
			return change ?
			          format(
			             /** TRANSLATORS: "Increase/Decrease the value by <n> minutes"
			                    You may want to treat this as "by <n> minutes", depending on how you
			                    translated "Increase/Decrease the value by %s". */
			             npgettext("spinbox_change", "%d minute", "%d minutes", value), value) :
			          format(
			             /** TRANSLATORS: The current value of a spinbox */
			             ngettext("%d minute", "%d minutes", value), value);
		}

		if (value % 60 == 0) {
			value /= 60;
			return change ?
			          format(
			             /** TRANSLATORS: "Increase/Decrease the value by <n> hours"
			                    You may want to treat this as "by <n> hours", depending on how you
			                    translated "Increase/Decrease the value by %s". */
			             npgettext("spinbox_change", "%d hour", "%d hours", value), value) :
			          format(
			             /** TRANSLATORS: The current value of a spinbox */
			             ngettext("%d hour", "%d hours", value), value);
		}

		const int32_t hours = value / 60;
		value %= 60;

		const std::string hours_mins_format_string =
		   change ?
		      /** TRANSLATORS: "Increase/Decrease the value by X hours and Y minutes"
		             You may want to treat this as "by X hours and Y minutes", depending on how you
		             translated "Increase/Decrease the value by %s". You can also defer translation
		             of "by X hours" and "by Y minutes". */
		      pgettext("spinbox_change_hours_mins", "%1$s and %2$s") :
		      /** TRANSLATORS: The current value of a spinbox, "X hours and Y minutes" */
		      _("%1$s and %2$s");

		const std::string hours_string =
		   change ?
		      format(
		         /** TRANSLATORS: ..
		                The hours part of "Increase/Decrease the value by X hours and Y minutes".
		                You may want to treat this as "by <n> hours", depending on how you translated
		                "Increase/Decrease the value by %s" and "%1$s and %2$s". */
		         npgettext("spinbox_change_hours_mins", "%d hour", "%d hours", value), value) :
		      format(
		         /** TRANSLATORS: The current value of a spinbox */
		         ngettext("%d hour", "%d hours", hours), hours);

		const std::string minutes_string =
		   change ?
		      format(
		         /** TRANSLATORS: ..
		                The minutes part of "Increase/Decrease the value by X hours and Y minutes"
		                You may want to treat this as "by <n> minutes", depending on how you
		                translated "Increase/Decrease the value by %s" and "%1$s and %2$s". */
		         npgettext("spinbox_change_hours_mins", "%d minute", "%d minutes", value), value) :
		      format(
		         /** TRANSLATORS: The current value of a spinbox */
		         ngettext("%d minute", "%d minutes", value), value);

		return format(hours_mins_format_string, hours_string, minutes_string);
	}

	case (Units::kWeeks):
		return change ? format(
		                   /** TRANSLATORS: "Increase/Decrease the value by <n> weeks"
		                          You may want to treat this as "by <n> weeks", depending on how you
		                          translated "Increase/Decrease the value by %s". */
		                   npgettext("spinbox_change", "%d week", "%d weeks", value), value) :
		                format(
		                   /** TRANSLATORS: The current value of a spinbox */
		                   ngettext("%d week", "%d weeks", value), value);
	case (Units::kPixels):
		return change ? format(
		                   /** TRANSLATORS: "Increase/Decrease the value by <n> pixels"
		                          You may want to treat this as "by <n> pixels", depending on how you
		                          translated "Increase/Decrease the value by %s". */
		                   npgettext("spinbox_change", "%d pixel", "%d pixels", value), value) :
		                format(
		                   /** TRANSLATORS: The current value of a spinbox */
		                   ngettext("%d pixel", "%d pixels", value), value);
	case (Units::kFields):
		return change ? format(
		                   /** TRANSLATORS: "Increase/Decrease the value by <n> fields"
		                          You may want to treat this as "by <n> fields", depending on how you
		                          translated "Increase/Decrease the value by %s". */
		                   npgettext("spinbox_change", "%d field", "%d fields", value), value) :
		                format(
		                   /** TRANSLATORS: The current value of a spinbox */
		                   ngettext("%d field", "%d fields", value), value);
	case (Units::kPercent):
		return change ? format(
		                   /** TRANSLATORS: "Increase/Decrease the value by <n>%"
		                          You may want to treat this as "by <n>%", depending on how you
		                          translated "Increase/Decrease the value by %s". */
		                   pgettext("spinbox_change", "%i %%"), value) :
		                format(
		                   /** TRANSLATORS: The current value of a spinbox */
		                   _("%i %%"), value);
	case (Units::kNone):
		return change ? format(
		                   /** TRANSLATORS: "Increase/Decrease the value by <n>"
		                          You may want to treat this as "by <n>", depending on how you
		                          translated "Increase/Decrease the value by %s". */
		                   pgettext("spinbox_change", "%d"), value) :
		                format(
		                   /** TRANSLATORS: The current value of a spinbox */
		                   "%d", value);

	default:
		NEVER_HERE();
	}

	NEVER_HERE();
}

}  // namespace UI
