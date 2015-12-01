/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

namespace UI {

struct IntValueTextReplacement {
	/// Value to be replaced
	int32_t value;

	/// Text to be used
	std::string text;
};

struct SpinBoxImpl {
	/// Value hold by the spinbox
	int32_t value;

	/// Minimum and maximum that \ref value may reach
	int32_t min;
	int32_t max;

	/// List of possible values for type kValueList
	std::vector<int32_t> values;

	/// The unit of the value
	std::string unit;

	/// Background tile style of buttons.
	const Image* background;

	/// Special names for specific Values
	std::vector<IntValueTextReplacement> valrep;

	/// The UI parts
	Textarea * text;
	Button * button_plus;
	Button * button_minus;
	Button * button_ten_plus;
	Button * button_ten_minus;
};

/**
 * SpinBox constructor:
 *
 * initializes a new spinbox with either two (big = false) or four (big = true)
 * buttons. w must be >= the space taken up by the buttons, else the spinbox would become useless and so
 * throws an exception.
 * The spinbox' height and button size is set automatically according to the height of its textarea.
 */
SpinBox::SpinBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t unit_w,
	 int32_t const startval, int32_t const minval, int32_t const maxval,
	 const std::string& label_text,
	 const std::string& unit,
	 const Image* background,
	 SpinBox::Type type)
	:
	Panel(parent, x, y, std::max(w, unit_w), 0),
	type_(type),
	sbi_(new SpinBoxImpl)
{
	if (type_ == SpinBox::Type::kValueList) {
		sbi_->min   = 0;
		sbi_->max   = 0;
	} else {
		sbi_->min   = minval;
		sbi_->max   = maxval;
	}
	sbi_->value = startval;
	sbi_->unit  = unit;
	sbi_->background = background;

	bool is_big = type_ == SpinBox::Type::kBig;

	uint32_t padding = 2;
	uint32_t actual_w = std::max(w, unit_w);
	uint32_t no_padding = (is_big ? 6 : 4);
	uint32_t texth = UI::g_fh1->render(as_uifont("."))->height();

	// 40 is an ad hoc width estimate for the MultilineTextarea scrollbar + a bit of text.
	if (!label_text.empty() && (w + padding) <= unit_w - 40) {
		throw wexception(
					"SpinBox: Overall width %d must be bigger than unit width %d + %d * %d + 40 for padding",
					w, unit_w, no_padding, padding);
	}

#ifndef NDEBUG //  only in debug builds
	if (unit_w < (is_big ? 7 * texth : 3 * texth)) {
		throw wexception("Not enough space to draw spinbox. Width %d is smaller than required width %d",
							  unit_w, (is_big ? 7 * texth : 3 * texth));
	}
#endif

	box_ = new UI::Box(this, 0, 0, UI::Box::Horizontal, actual_w, texth, padding);

	// Find out how much height we need for the label. We give it 6 rows maximum.
	const Image* rendered_text = UI::g_fh1->render(as_uifont(label_text));
	uint32_t available_width = w - unit_w - no_padding * padding;
	uint32_t extra_rows =
			available_width > 0 ?
				std::min(static_cast<int>(rendered_text->width() / available_width), 6) : 0;

	UI::MultilineTextarea* label = new UI::MultilineTextarea(box_, 0, 0, available_width,
																				texth * (extra_rows + 1), label_text);

	box_->add(label, UI::Box::AlignTop);

	sbi_->text = new UI::Textarea(box_, "", Align_Center);

	sbi_->button_minus =
		new Button
			(box_, "-",
			 0, 0, texth, texth,
			 sbi_->background,
			 g_gr->images().get(is_big ? "pics/scrollbar_left.png" : "pics/scrollbar_down.png"),
			 _("Decrease the value"));
	sbi_->button_plus =
		new Button
			(box_, "+",
			 0, 0, texth, texth,
			 sbi_->background,
			 g_gr->images().get(is_big ? "pics/scrollbar_right.png" : "pics/scrollbar_up.png"),
			 _("Increase the value"));

	if (is_big) {
		sbi_->button_ten_minus =
			new Button
				(box_, "--",
				 0, 0, 2 * texth, texth,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_left_fast.png"),
				 _("Decrease the value by 10"));
		sbi_->button_ten_plus =
			new Button
				(box_, "++",
				 0, 0, 2 * texth, texth,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_right_fast.png"),
				 _("Increase the value by 10"));

		sbi_->button_ten_plus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), 10));
		sbi_->button_ten_minus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), -10));
		sbi_->button_ten_plus->set_repeating(true);
		sbi_->button_ten_minus->set_repeating(true);
		buttons_.push_back(sbi_->button_ten_minus);
		buttons_.push_back(sbi_->button_ten_plus);

		sbi_->text->set_fixed_width(unit_w
											 - 2 * sbi_->button_ten_plus->get_w()
											 - 2 * sbi_->button_minus->get_w()
											 - 4 * padding);

		box_->add(sbi_->button_ten_minus, UI::Box::AlignTop);
		box_->add(sbi_->button_minus, UI::Box::AlignTop);
		box_->add(sbi_->text, UI::Box::AlignTop);
		box_->add(sbi_->button_plus, UI::Box::AlignTop);
		box_->add(sbi_->button_ten_plus, UI::Box::AlignTop);
	} else {
		sbi_->text->set_fixed_width(unit_w - 2 * sbi_->button_minus->get_w() - 2 * padding);

		box_->add(sbi_->button_minus, UI::Box::AlignTop);
		box_->add(sbi_->text, UI::Box::AlignTop);
		box_->add(sbi_->button_plus, UI::Box::AlignTop);
	}

	sbi_->button_plus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), 1));
	sbi_->button_minus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), -1));
	sbi_->button_plus->set_repeating(true);
	sbi_->button_minus->set_repeating(true);
	buttons_.push_back(sbi_->button_minus);
	buttons_.push_back(sbi_->button_plus);
	box_->set_size(actual_w, texth * (extra_rows + 1));
	set_desired_size(actual_w, texth * (extra_rows + 1));
	set_size(actual_w, texth * (extra_rows + 1));
	update();
}

SpinBox::~SpinBox() {
	delete sbi_;
	sbi_ = nullptr;
}


/**
 * private function - takes care about all updates in the UI elements
 */
void SpinBox::update()
{
	bool was_in_list = false;
	for (const IntValueTextReplacement& value : sbi_->valrep) {
		if (value.value == sbi_->value) {
			sbi_->text->set_text(value.text);
			was_in_list = true;
			break;
		}
	}
	if (!was_in_list) {
		if (type_ == SpinBox::Type::kValueList) {
			if ((sbi_->value >= 0) && (sbi_->values.size() > static_cast<size_t>(sbi_->value))) {
				/** TRANSLATORS: %i = number, %s = unit, e.g. "5 pixels" in the advanced options */
				sbi_->text->set_text((boost::format(_("%1$i %2$s")) % sbi_->values.at(sbi_->value) % sbi_->unit.c_str()).str());
			} else {
				sbi_->text->set_text("undefined"); // The user should never see this, so we're not localizing
			}
		} else {
			/** TRANSLATORS: %i = number, %s = unit, e.g. "5 pixels" in the advanced options */
			sbi_->text->set_text((boost::format(_("%1$i %2$s")) % sbi_->value % sbi_->unit.c_str()).str());
		}
	}

	sbi_->button_minus->set_enabled(sbi_->min < sbi_->value);
	sbi_->button_plus ->set_enabled(sbi_->value < sbi_->max);
	if (type_ == SpinBox::Type::kBig) {
		sbi_->button_ten_minus->set_enabled(sbi_->min < sbi_->value);
		sbi_->button_ten_plus ->set_enabled(sbi_->value < sbi_->max);
	}
}


/**
 * private function called by spinbox buttons to in-/decrease the value
 */
void SpinBox::change_value(int32_t const value)
{
	set_value(value + sbi_->value);
}


/**
 * manually sets the used value to a given value
 */
void SpinBox::set_value(int32_t const value)
{
	sbi_->value = value;
	if (sbi_->value > sbi_->max)
		sbi_->value = sbi_->max;
	else if (sbi_->value < sbi_->min)
		sbi_->value = sbi_->min;
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
void SpinBox::set_interval(int32_t const min, int32_t const max)
{
	sbi_->max = max;
	sbi_->min = min;
	if (sbi_->value > max)
		sbi_->value = max;
	else if (sbi_->value < min)
		sbi_->value = min;
	update();
}


/**
 * manually sets the used unit to a given string
 */
void SpinBox::set_unit(const std::string & unit)
{
	sbi_->unit = unit;
	update();
}


/**
 * \returns the value
 */
int32_t SpinBox::get_value()
{
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
 * \returns the unit
 */
std::string SpinBox::get_unit()
{
	return sbi_->unit;
}


/**
 * Searches for value in sbi->valrep
 * \returns the place where value was found or -1 if the value wasn't found.
 */
int32_t SpinBox::find_replacement(int32_t value) const
{
	for (uint32_t i = 0; i < sbi_->valrep.size(); ++i)
		if (sbi_->valrep[i].value == value)
			return i;
	return -1;
}


/**
 * Adds a replacement text for a specific value
 * overwrites an old replacement if one exists.
 */
void SpinBox::add_replacement(int32_t value, const std::string& text)
{
	if (int32_t i = find_replacement(value) >= 0)
		sbi_->valrep[i].text = text;
	else {
		IntValueTextReplacement newtr;
		newtr.value = value;
		newtr.text  = text;
		sbi_->valrep.push_back(newtr);
	}
	update();
}


/**
 * Removes a replacement text for a specific value
 */
void SpinBox::remove_replacement(int32_t value)
{
	if (int32_t i = find_replacement(value) >= 0) {
		sbi_->valrep[i].text = (boost::format(_("%1$i %2$s")) % value % sbi_->unit.c_str()).str();
	}
}

/**
 * \returns true, if find_replacement returns an int >= 0
 */
bool SpinBox::has_replacement(int32_t value) const
{
	return find_replacement(value) >= 0;
}

}
