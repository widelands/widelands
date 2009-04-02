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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ui_spinbox.h"

#include "i18n.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "wexception.h"

#include <vector>

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
	uint32_t background;

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;

	/// Special names for specific Values
	std::vector<IntValueTextReplacement> valrep;

	/// Font variables
	std::string fontname;
	uint32_t    fontsize;
	RGBColor    fontcolor;

	/// The UI parts
	Textarea * text;
	IDButton<SpinBox, int32_t> * butPlus;
	IDButton<SpinBox, int32_t> * butMinus;
	IDButton<SpinBox, int32_t> * butTenPlus;
	IDButton<SpinBox, int32_t> * butTenMinus;
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
	 int32_t startval, int32_t minval, int32_t maxval, std::string unit,
	 const uint32_t background, bool big,
	 Align alignm)
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
	sbi->fontname   = UI_FONT_NAME;
	sbi->fontsize   = UI_FONT_SIZE_SMALL;
	sbi->fontcolor  = UI_FONT_CLR_FG;

	if (w < 20)
		throw wexception("Not enough space to draw spinbox");
	int32_t butw = h;
	int32_t textw = w - butw * 32 / 5;
	while (textw <= 0) {
		butw = butw * 3 / 4;
		textw = w - butw * 32 / 5;
	}

	char buf[64];
	snprintf(buf, sizeof(buf), "%i%s", sbi->value, sbi->unit.c_str());

	sbi->text = new UI::Textarea
		(this, butw * 16 / 5, 0, textw, h, buf, Align_Center);
	sbi->text->set_font(sbi->fontname, sbi->fontsize, sbi->fontcolor);
	sbi->butPlus = new IDButton<SpinBox, int32_t>
		(this,
		 butw * 21 / 10, 0, butw, butw,
		 sbi->background,
		 &SpinBox::changeValue, this, 1,
		 "+", _("Increase the value"),
		 true, false, sbi->fontname, sbi->fontsize);
	sbi->butMinus = new IDButton<SpinBox, int32_t>
		(this,
		 w - butw * 31 / 10, 0, butw, butw,
		 sbi->background,
		 &SpinBox::changeValue, this, -1,
		 "-", _("Decrease the value"),
		 true, false, sbi->fontname, sbi->fontsize);
	sbi->butPlus->set_repeating(true);
	sbi->butMinus->set_repeating(true);
	if (m_big) {
		sbi->butTenPlus = new IDButton<SpinBox, int32_t>
			(this,
			 0, 0, butw * 2, butw,
			 sbi->background,
			 &SpinBox::changeValue, this, 10,
			 "++", _("Increase the value by 10"),
			 true, false, sbi->fontname, sbi->fontsize);
		sbi->butTenMinus = new IDButton<SpinBox, int32_t>
			(this,
			 w - 2 * butw, 0, butw * 2, butw,
			 sbi->background,
			 &SpinBox::changeValue, this, -10,
			 "--", _("Decrease the value by 10"),
			 true, false, sbi->fontname, sbi->fontsize);
		sbi->butTenPlus->set_repeating(true);
		sbi->butTenMinus->set_repeating(true);
	}
}

SpinBox::~SpinBox() {}


/**
 * private function - takes care about all updates in the UI elements
 */
void SpinBox::update()
{
	char buf[64];
	bool replacement = false;
	for (uint32_t i = 0; i < sbi->valrep.size(); ++i) {
		if (sbi->valrep[i].value == sbi->value) {
			replacement = true;
			snprintf(buf, sizeof(buf), sbi->valrep[i].text.c_str());
			break;
		}
	}
	if (!replacement)
		snprintf(buf, sizeof(buf), "%i%s", sbi->value, sbi->unit.c_str());

	sbi->text->set_text(buf);

	if (sbi->value <= sbi->min)
		sbi->butMinus->set_enabled(false);
	else
		sbi->butMinus->set_enabled(true);
	if (sbi->value >= sbi->max)
		sbi->butPlus->set_enabled(false);
	else
		sbi->butPlus->set_enabled(true);
	if (m_big) {
		if (sbi->value <= sbi->min + 10)
			sbi->butTenMinus->set_enabled(false);
		else
			sbi->butTenMinus->set_enabled(true);
		if (sbi->value >= sbi->max - 10)
			sbi->butTenPlus->set_enabled(false);
		else
			sbi->butTenPlus->set_enabled(true);
	}
}


/**
 * private function called by spinbox buttons to in-/decrease the value
 */
void SpinBox::changeValue(int32_t value)
{
	setValue(value + sbi->value);
}


/**
 * manually sets the used value to a given value
 */
void SpinBox::setValue(int32_t value)
{
	sbi->value = value;
	update();
}

/**
 * manually sets the used unit to a given string
 */
void SpinBox::setUnit(std::string unit)
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
 */
void SpinBox::set_font(std::string const & name, int32_t size, RGBColor color)
{
	sbi->fontname = name;
	sbi->fontsize = size;
	sbi->fontcolor = color;

	sbi->text->set_font(name, size, color);
	sbi->butPlus->set_font(name, size);
	sbi->butMinus->set_font(name, size);
	if (m_big) {
		sbi->butTenPlus->set_font(name, size);
		sbi->butTenMinus->set_font(name, size);
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
		snprintf(buf, sizeof(buf), "%i%s", value, sbi->unit.c_str());
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

};
