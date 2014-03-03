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

#include "container_iterate.h"
#include "i18n.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "wexception.h"

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

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;

	/// Special names for specific Values
	std::vector<IntValueTextReplacement> valrep;

	/// The UI parts
	Textarea * text;
	Button * butPlus;
	Button * butMinus;
	Button * butTenPlus;
	Button * butTenMinus;
};

/**
 * SpinBox constructor:
 *
 * initializes a new spinbox with either two (big = false) or four (big = true)
 * buttons. w must be >= 20 else the spinbox would become useless and so
 * throws an exception.
 */
SpinBox::SpinBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 int32_t const startval, int32_t const minval, int32_t const maxval,
	 const std::string &       unit,
	 const Image* background,
	 bool                const big,
	 Align               const alignm)
	:
	Panel(parent, x, y, w, h),
	m_big(big),
	sbi(new SpinBoxImpl)
{
	sbi->value = startval;
	sbi->min   = minval;
	sbi->max   = maxval;
	sbi->unit  = unit;

	sbi->background = background;
	sbi->align      = alignm;

	if (w < 20)
		throw wexception("Not enough space to draw spinbox");
	int32_t butw = h;
	int32_t textw = w - butw * 32 / 5;
	while (textw <= 0) {
		butw = butw * 3 / 4;
		textw = w - butw * 32 / 5;
	}

	char buf[64];
	snprintf(buf, sizeof(buf), "%i %s", sbi->value, sbi->unit.c_str());

	sbi->text = new UI::Textarea
		(this, butw * 16 / 5, 0, textw, h, buf, Align_Center);
	sbi->butPlus =
		new Button
			(this, "+",
			 w - butw * 31 / 10, 0, butw, butw,
			 sbi->background,
			 "+", _("Increase the value"),
			 true, false);
	sbi->butPlus->sigclicked.connect(boost::bind(&SpinBox::changeValue, boost::ref(*this), 1));
	sbi->butMinus =
		new Button
			(this, "-",
			 butw * 21 / 10, 0, butw, butw,
			 sbi->background,
			 "-", _("Decrease the value"),
			 true, false);
	sbi->butMinus->sigclicked.connect(boost::bind(&SpinBox::changeValue, boost::ref(*this), -1));
	sbi->butPlus->set_repeating(true);
	sbi->butMinus->set_repeating(true);
	if (m_big) {
		sbi->butTenPlus =
			new Button
				(this, "++",
				 w - 2 * butw, 0, butw * 2, butw,
				 sbi->background,
				 "++", _("Increase the value by 10"),
				 true, false);
		sbi->butTenPlus->sigclicked.connect(boost::bind(&SpinBox::changeValue, boost::ref(*this), 10));
		sbi->butTenMinus =
			new Button
				(this, "--",
				 0, 0, butw * 2, butw,
				 sbi->background,
				 "--", _("Decrease the value by 10"),
				 true, false);
		sbi->butTenMinus->sigclicked.connect(boost::bind(&SpinBox::changeValue, boost::ref(*this), -10));
		sbi->butTenPlus->set_repeating(true);
		sbi->butTenMinus->set_repeating(true);
	}

	set_font(UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG);
}

SpinBox::~SpinBox() {
	delete sbi;
	sbi = nullptr;
}


/**
 * private function - takes care about all updates in the UI elements
 */
void SpinBox::update()
{

	for
		(wl_const_range<std::vector<IntValueTextReplacement> > i(sbi->valrep);;
		 ++i)
		if (i.empty()) {
			char buf[64];
			snprintf(buf, sizeof(buf), "%i %s", sbi->value, sbi->unit.c_str());
			sbi->text->set_text(buf);
			break;
		} else if (i.current->value == sbi->value) {
			sbi->text->set_text(i.current->text);
			break;
		}

	sbi->butMinus->set_enabled(sbi->min < sbi->value);
	sbi->butPlus ->set_enabled           (sbi->value < sbi->max);
	if (m_big) {
		sbi->butTenMinus->set_enabled(sbi->min < sbi->value);
		sbi->butTenPlus ->set_enabled           (sbi->value < sbi->max);
	}
}


/**
 * private function called by spinbox buttons to in-/decrease the value
 */
void SpinBox::changeValue(int32_t const value)
{
	setValue(value + sbi->value);
}


/**
 * manually sets the used value to a given value
 */
void SpinBox::setValue(int32_t const value)
{
	sbi->value = value;
	if (sbi->value > sbi->max)
		sbi->value = sbi->max;
	else if (sbi->value < sbi->min)
		sbi->value = sbi->min;
	update();
}


/**
 * sets the interval the value may lay in and fixes the value, if outside.
 */
void SpinBox::setInterval(int32_t const min, int32_t const max)
{
	sbi->max = max;
	sbi->min = min;
	if (sbi->value > max)
		sbi->value = max;
	else if (sbi->value < min)
		sbi->value = min;
	update();
}


/**
 * manually sets the used unit to a given string
 */
void SpinBox::setUnit(const std::string & unit)
{
	sbi->unit = unit;
	update();
}


/**
 * \returns the value
 */
int32_t SpinBox::getValue()
{
	return sbi->value;
}

/**
 * \returns the unit
 */
std::string SpinBox::getUnit()
{
	return sbi->unit;
}


/**
 * \returns the text alignment
 */
Align SpinBox::align() const
{
	return sbi->align;
}


/**
 * Set a new alignment.
 */
void SpinBox::setAlign(Align alignm)
{
	if (alignm != sbi->align) {
		sbi->align = alignm;
		update();
	}
}


/**
 * Sets the font of all UI elements
 *
 * @deprecated, see set_textstyle
 */
void SpinBox::set_font(const std::string & name, int32_t size, RGBColor color)
{
	set_textstyle(TextStyle::makebold(Font::get(name, size), color));
}

/**
 * Sets the font and textstyle of all UI elements
 */
void SpinBox::set_textstyle(const TextStyle & textstyle)
{
	sbi->text->set_textstyle(textstyle);
	sbi->butPlus->set_font(textstyle.font);
	sbi->butMinus->set_font(textstyle.font);
	if (m_big) {
		sbi->butTenPlus->set_font(textstyle.font);
		sbi->butTenMinus->set_font(textstyle.font);
	}
	update();
}


/**
 * Searches for value in sbi->valrep
 * \returns the place where value was found or -1 if the value wasn't found.
 */
int32_t SpinBox::findReplacement(int32_t value)
{
	for (uint32_t i = 0; i < sbi->valrep.size(); ++i)
		if (sbi->valrep[i].value == value)
			return i;
	return -1;
}


/**
 * Adds a replacement text for a specific value
 * overwrites an old replacement if one exists.
 */
void SpinBox::add_replacement(int32_t value, std::string text)
{
	if (int32_t i = findReplacement(value) >= 0)
		sbi->valrep[i].text = text;
	else {
		IntValueTextReplacement newtr;
		newtr.value = value;
		newtr.text  = text;
		sbi->valrep.push_back(newtr);
	}
	update();
}


/**
 * Removes a replacement text for a specific value
 */
void SpinBox::remove_replacement(int32_t value)
{
	if (int32_t i = findReplacement(value) >= 0) {
		char buf[64];
		snprintf(buf, sizeof(buf), "%i %s", value, sbi->unit.c_str());
		sbi->valrep[i].text = buf;
	}
}

/**
 * \returns true, if findReplacement returns an int >= 0
 */
bool SpinBox::has_replacement(int32_t value)
{
	return findReplacement(value) >= 0;
}

}
