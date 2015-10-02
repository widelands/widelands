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
	Button * button_ten_plus_;
	Button * button_ten_minus;
};

/**
 * SpinBox constructor:
 *
 * initializes a new spinbox with either two (big = false) or four (big = true)
 * buttons. w must be >= the normal textarea height, else the spinbox would become useless and so
 * throws an exception.
 * The spinbox' height is set automatically according to the height of its textarea.
 */
SpinBox::SpinBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w,
	 int32_t const startval, int32_t const minval, int32_t const maxval,
	 const std::string& unit,
	 const Image* background,
	 bool const big)
	:
	Panel(parent, x, y, w, 20), // Height needs to be > 0, otherwise the panel won't resize.
	big_(big),
	sbi_(new SpinBoxImpl)
{
	sbi_->value = startval;
	sbi_->min   = minval;
	sbi_->max   = maxval;
	sbi_->unit  = unit;
	sbi_->background = background;

	uint32_t texth = UI::g_fh1->render(as_uifont("."))->height();

	if (w < texth) {
		throw wexception("Not enough space to draw spinbox");
	}

	set_desired_size(w, texth);
	int32_t butw = texth;
	int32_t textw = w - butw * 32 / 5;

	int32_t but_plus_x;
	int32_t but_minus_x;
	int32_t text_x;

	if (big_) {
		but_plus_x = w - butw * 31 / 10;
		but_minus_x = butw * 21 / 10;

	} else {
		but_plus_x = w - butw;
		but_minus_x = 0;
		textw = textw + 4 * butw;
	}
	while (textw <= 0) {
		butw = butw * 3 / 4;
		textw = w - butw * 32 / 5;
	}
	text_x = (w - textw) / 2;

	sbi_->text = new UI::Textarea
		(this, text_x, 0, textw, texth, "", Align_Center);
	if (big_) {
		sbi_->button_plus =
			new Button
				(this, "+",
				 but_plus_x, 0, butw, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_right.png"),
				 _("Increase the value"));
		sbi_->button_minus =
			new Button
				(this, "-",
				 but_minus_x, 0, butw, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_left.png"),
				 _("Decrease the value"));
		sbi_->button_ten_plus_ =
			new Button
				(this, "++",
				 w - 2 * butw, 0, butw * 2, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_right_fast.png"),
				 _("Increase the value by 10"));
		sbi_->button_ten_plus_->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), 10));
		sbi_->button_ten_minus =
			new Button
				(this, "--",
				 0, 0, butw * 2, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_left_fast.png"),
				 _("Decrease the value by 10"));
		sbi_->button_ten_minus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), -10));
		sbi_->button_ten_plus_->set_repeating(true);
		sbi_->button_ten_minus->set_repeating(true);
		buttons_.push_back(sbi_->button_ten_minus);
		buttons_.push_back(sbi_->button_ten_plus_);
	} else {
		sbi_->button_plus =
			new Button
				(this, "+",
				 but_plus_x, 0, butw, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_up.png"),
				 _("Increase the value"));
		sbi_->button_minus =
			new Button
				(this, "-",
				 but_minus_x, 0, butw, butw,
				 sbi_->background,
				 g_gr->images().get("pics/scrollbar_down.png"),
				 _("Decrease the value"));
	}

	sbi_->button_plus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), 1));
	sbi_->button_minus->sigclicked.connect(boost::bind(&SpinBox::change_value, boost::ref(*this), -1));
	sbi_->button_plus->set_repeating(true);
	sbi_->button_minus->set_repeating(true);
	buttons_.push_back(sbi_->button_minus);
	buttons_.push_back(sbi_->button_plus);
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
		/** TRANSLATORS: %i = number, %s = unit, e.g. "5 pixels" in the advanced options */
		sbi_->text->set_text((boost::format(_("%1$i %2$s")) % sbi_->value % sbi_->unit.c_str()).str());
	}

	sbi_->button_minus->set_enabled(sbi_->min < sbi_->value);
	sbi_->button_plus ->set_enabled(sbi_->value < sbi_->max);
	if (big_) {
		sbi_->button_ten_minus->set_enabled(sbi_->min < sbi_->value);
		sbi_->button_ten_plus_ ->set_enabled(sbi_->value < sbi_->max);
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
	return sbi_->value;
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
