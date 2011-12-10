/*
 * Copyright (C) 2002-2004, 2008-2009 by the Widelands Development Team
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

#include "plot_area.h"

#include "i18n.h"
#include "constants.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"


#include "ui_basic/panel.h"

#include <cstdio>
#include <boost/lexical_cast.hpp>

static const uint32_t minutes = 60 * 1000;
static const uint32_t hours = 60 * 60 * 1000;
static const uint32_t days = 24 * 60 * 60 * 1000;

const uint32_t WUIPlot_Area::time_in_ms[] = {
	15 * minutes,
	30 * minutes,
	1  * hours,
	2  * hours,
	5  * hours,
	10 * hours,
	30 * hours
};

#define BG_PIC "pics/plot_area_bg.png"
#define LINE_COLOR RGBColor(0, 0, 0)


WUIPlot_Area::WUIPlot_Area
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
:
UI::Panel (parent, x, y, w, h),
spacing(5),
space_at_bottom(15),
space_at_right(5),
m_time    (TIME_GAME),
m_plotmode(PLOTMODE_ABSOLUTE),
m_game_time_id(0)
{}


uint32_t WUIPlot_Area::get_game_time() {
	uint32_t game_time = 0;

	// Find running time of the game, based on the plot data
	for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot)
		if (game_time < m_plotdata[plot].dataset->size() * m_sample_rate)
			game_time = m_plotdata[plot].dataset->size() * m_sample_rate;
	return game_time;
}

uint32_t WUIPlot_Area::get_plot_time() {
	if (m_time == TIME_GAME) {
		// Start with the game time
		uint32_t time_in_ms_ = get_game_time();

		// Round up to a nice nearest multiple.
		// Either a multiple of 4 min
		// Either a multiple of 20 min
		// or a multiple of 2h
		// or a multiple of 20h
		// or a multiple of 4 days
		if (time_in_ms_ > 8 * days) {
			time_in_ms_ += - (time_in_ms_ % (4 * days)) + 4 * days;
		} else if (time_in_ms_ > 40 * hours) {
			time_in_ms_ += - (time_in_ms_ % (20 * hours)) + 20 * hours;
		} else if (time_in_ms_ > 4 * hours) {
			time_in_ms_ += - (time_in_ms_ % (2 * hours)) + 2 * hours;
		} else {
			time_in_ms_ += - (time_in_ms_ % (15 * minutes)) + 15 * minutes;
		}
		return time_in_ms_;
	} else {
		return time_in_ms[m_time];
	}
}

WUIPlot_Area::UNIT WUIPlot_Area::get_suggested_unit(uint32_t game_time) {
	// Find a nice unit for max_x
	if (game_time > 4 * days) {
		return UNIT_DAY;
	} else if (game_time > 4 * hours) {
		return UNIT_HOUR;
	} else {
		return UNIT_MIN;
	}
}

std::string WUIPlot_Area::get_unit_name(UNIT unit) {
	switch (unit) {
		case UNIT_DAY:  return _("d");
		case UNIT_HOUR: return _("h");
		case UNIT_MIN:  return _("min");
	}
	return "invalid";
}

uint32_t WUIPlot_Area::ms_to_unit(UNIT unit, uint32_t ms) {
	switch (unit) {
		case UNIT_DAY: return ms / days;
		case UNIT_HOUR: return ms / hours;
		case UNIT_MIN: return ms / minutes;
	}
	return -1;
}

std::vector<std::string> WUIPlot_Area::get_labels() {
	std::vector<std::string> labels;
	for (int32_t i = 0; i < m_game_time_id; i++) {
		UNIT unit = get_suggested_unit(time_in_ms[i]);
		uint32_t val = ms_to_unit(unit, time_in_ms[i]);
		labels.push_back(boost::lexical_cast<std::string>(val) + get_unit_name(unit));
	}
	labels.push_back(_("game"));
	return labels;
}

/**
 * Find the last predefined time span that is less than the game time. If this
 * is called from the outside, e.g. from a slider, then from that moment on
 * this class assumes that values passed to set_time_id adhere to the new
 * choice of time spans.
 * We start to search with i=1 to ensure that at least one option besides
 * "game" will be offered to the user.
 */
int32_t WUIPlot_Area::get_game_time_id() {
	uint32_t game_time = get_game_time();
	uint32_t i;
	for (i = 1; i < 7 && time_in_ms[i] <= game_time; i++) {
	}
	m_game_time_id = i;
	return m_game_time_id;
}

/*
 * Draw this. This is the main function
 */
void WUIPlot_Area::draw(RenderTarget & dst) {

	uint32_t time_in_ms_;

	float const xline_length = get_inner_w() - space_at_right  - spacing;
	float const yline_length = get_inner_h() - space_at_bottom - spacing;


	time_in_ms_ = draw_diagram(dst, xline_length, yline_length);

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_in_ms_);

	uint32_t max = 0;
	//  Find the maximum value.
	if (m_plotmode == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < m_plotdata.size(); ++i)
			if (m_plotdata[i].showplot) {
				for (uint32_t l = 0; l < m_plotdata[i].dataset->size(); ++l)
					if (max < (*m_plotdata[i].dataset)[l])
						max = (*m_plotdata[i].dataset)[l];
			}
	} else {
		for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot)
			if (m_plotdata[plot].showplot) {

				std::vector<uint32_t> const & dataset = *m_plotdata[plot].dataset;

				uint32_t add = 0;
				//  Relative data, first entry is always zero.
				for (uint32_t i = 0; i < dataset.size(); ++i) {
					add += dataset[i];
					if (0 == ((i + 1) % how_many)) {
						if (max < add)
							max = add;
						add = 0;
					}
				}
			}
	}

	//  print the maximal value into the top right corner
	char buffer[200];
	sprintf(buffer, "%u", max);

	draw_value
		(dst, buffer, RGBColor(60, 125, 0),
		 Point(get_inner_w() - space_at_right - 2, spacing + 2));

	//  plot the pixels
	float sub =
		xline_length
		/
		(static_cast<float>(time_in_ms_)
		 /
		 static_cast<float>(m_sample_rate));
	for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot)
		if (m_plotdata[plot].showplot) {

			RGBColor color = m_plotdata[plot].plotcolor;
			std::vector<uint32_t> const * dataset = m_plotdata[plot].dataset;

			std::vector<uint32_t> m_data;
			if (m_plotmode == PLOTMODE_RELATIVE) {
				uint32_t add = 0;
				// Relative data, first entry is always zero
				m_data.push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i];
					if (0 == ((i + 1) % how_many)) {
						m_data.push_back(add);
						add = 0;
					}
				}

				dataset = &m_data;
				sub = xline_length / static_cast<float>(nr_samples);
			}

			draw_plot_line
				(dst, dataset, yline_length, max, sub, color, get_inner_h() - space_at_bottom);
		}
}

/**
 * calculate how many values are taken together when plot mode is relative
 */
int32_t WUIPlot_Area::calc_how_many(uint32_t time_in_ms_) {
	int32_t how_many = static_cast<int32_t>
			((static_cast<float>(time_in_ms_)
				/
				static_cast<float>(nr_samples))
				/
				static_cast<float>(m_sample_rate));

	return how_many;
}


/**
 * scale value down to the available space, which is specifiey by
 * the length of the y axis and the highest scale.
 */
float WUIPlot_Area::scale_value
	(float const yline_length, uint32_t const highest_scale,
	 int32_t const value)
{
	return yline_length / (static_cast<float>(highest_scale) / static_cast<float>(value));
}


/**
 * scale the values from dataset down to the available space and draw a single plot line
 * \param dataset the y values of the line
 * \param sub horizontal difference between 2 y values
 */
void WUIPlot_Area::draw_plot_line
		(RenderTarget & dst, std::vector<uint32_t> const * dataset, float const yline_length,
		 uint32_t const highest_scale, float const sub, RGBColor const color, int32_t const yoffset)
{

	float posx = get_inner_w() - space_at_right;

	int32_t lx = get_inner_w() - space_at_right;
	int32_t ly = yoffset;
	//init start point of the plot line with the first data value.
	//this prevent that the plot start always at zero
	if (int32_t value = (*dataset)[dataset->size() - 1]) {
		ly -= static_cast<int32_t>(scale_value(yline_length, highest_scale, value));
	}

	for (int32_t i = dataset->size() - 1; i > 0 and posx > spacing; --i) {
		int32_t const curx = static_cast<int32_t>(posx);
		int32_t       cury = yoffset;

		//scale the line to the available space
		if (int32_t value = (*dataset)[i]) {
			const float length_y = scale_value(yline_length, highest_scale, value);

			cury -= static_cast<int32_t>(length_y);
		}

		dst.draw_line(lx, ly, curx, cury, color, 2);

		posx -= sub;

		lx = curx;
		ly = cury;
	}
}

/**
 * draw the background and the axis of the diagram
 */
uint32_t WUIPlot_Area::draw_diagram
	(RenderTarget & dst, float const xline_length, float const yline_length) {

	uint32_t time_in_ms_, how_many_ticks, max_x;
	char buffer[200];

	time_in_ms_ = get_plot_time();
	UNIT unit = get_suggested_unit(time_in_ms_);
	max_x = ms_to_unit(unit, time_in_ms_);

	// Find a nice division of max_x
	if (max_x % 5 == 0) {
		if (max_x <= 10) {
			how_many_ticks = 5;
		} else {
			how_many_ticks = max_x / 5;
			while (how_many_ticks > 7 && how_many_ticks % 2 == 0) {
				how_many_ticks /= 2;
			}
			while (how_many_ticks > 7 && how_many_ticks % 3 == 0) {
				how_many_ticks /= 3;
			}
			while (how_many_ticks > 7 && how_many_ticks % 5 == 0) {
				how_many_ticks /= 5;
			}
			while (how_many_ticks > 7 && how_many_ticks % 7 == 0) {
				how_many_ticks /= 7;
			}
		}
	} else {
		how_many_ticks = 4;
	}

	// first, tile the background
	dst.tile
		(Rect(Point(0, 0), get_inner_w(), get_inner_h()),
		 g_gr->get_picture(PicMod_Game, BG_PIC), Point(0, 0));

	// Draw coordinate system
	// X Axis
	dst.draw_line
		(spacing,                        get_inner_h() - space_at_bottom,
		 get_inner_w() - space_at_right, get_inner_h() - space_at_bottom,
		 LINE_COLOR, 2);
	// Arrow
	dst.draw_line
		(spacing,     get_inner_h() - space_at_bottom,
		 spacing + 5, get_inner_h() - space_at_bottom - 3,
		 LINE_COLOR, 2);
	dst.draw_line
		(spacing,     get_inner_h() - space_at_bottom,
		 spacing + 5, get_inner_h() - space_at_bottom + 3,
		 LINE_COLOR, 2);
	//  Y Axis
	dst.draw_line
		(get_inner_w() - space_at_right, spacing,
		 get_inner_w() - space_at_right,
		 get_inner_h() - space_at_bottom,
		 LINE_COLOR, 2);
	//  No Arrow here, since this doesn't continue.

	//  draw xticks
	UI::TextStyle xtickstyle(UI::TextStyle::ui_small());
	xtickstyle.fg = RGBColor(255, 0, 0);

	float sub = xline_length / how_many_ticks;
	float posx = get_inner_w() - space_at_right;

	for (uint32_t i = 0; i <= how_many_ticks; ++i) {
		dst.draw_line
			(static_cast<int32_t>(posx), get_inner_h() - space_at_bottom,
			 static_cast<int32_t>(posx), get_inner_h() - space_at_bottom + 3,
			 LINE_COLOR, 2);

		snprintf
			(buffer, sizeof(buffer),
			 "%u", max_x / how_many_ticks * i);

		UI::g_fh->draw_text
			(dst, xtickstyle,
			 Point
			 	(static_cast<int32_t>(posx),
			 	 get_inner_h() - space_at_bottom + 10),
			 buffer,
			 UI::Align_Center);
		posx -= sub;
	}

	//  draw yticks, one at full, one at half
	dst.draw_line
		(get_inner_w() - space_at_right,    spacing,
		 get_inner_w() - space_at_right -3, spacing,
		 LINE_COLOR, 2);
	dst.draw_line
		(get_inner_w() - space_at_right,
		 spacing + ((get_inner_h() - space_at_bottom) - spacing) / 2,
		 get_inner_w() - space_at_right - 3,
		 spacing + ((get_inner_h() - space_at_bottom) - spacing) / 2,
		 LINE_COLOR, 2);

	//  print the used unit
	UI::g_fh->draw_text
		(dst, xtickstyle,
		 Point(2, spacing + 2),
		 get_unit_name(unit), UI::Align_CenterLeft);

	return time_in_ms_;
}

/**
 * print the string into the RenderTarget.
 */
void WUIPlot_Area::draw_value
	(RenderTarget & dst, const char * value, RGBColor color, Point pos)
{

	UI::TextStyle ymarkstyle(UI::TextStyle::ui_small());
	ymarkstyle.fg = color;

	UI::g_fh->draw_text
		(dst, ymarkstyle,
		 pos,
		 value, UI::Align_CenterRight);
}

/*
 * Register a new plot data stream
 */
void WUIPlot_Area::register_plot_data
	(uint32_t const id,
	 std::vector<uint32_t> const * const data,
	 RGBColor const color)
{
	if (id >= m_plotdata.size())
		m_plotdata.resize(id + 1);

	m_plotdata[id].dataset   = data;
	m_plotdata[id].showplot  = false;
	m_plotdata[id].plotcolor = color;

	get_game_time_id();
}

/**
 * Change the plot color of a registed data stream
 */
void WUIPlot_Area::set_plotcolor(uint32_t id, RGBColor color) {
	if (id > m_plotdata.size()) return;

	m_plotdata[id].plotcolor = color;
}

/*
 * Show this plot data?
 */
void WUIPlot_Area::show_plot(uint32_t const id, bool const t) {
	assert(id < m_plotdata.size());
	m_plotdata[id].showplot = t;
};

/*
 * Set sample rate the data uses
 */
void WUIPlot_Area::set_sample_rate(uint32_t const id) {
	m_sample_rate = id;
}


void WUIPlot_Area_Slider::draw(RenderTarget & dst) {
	int32_t new_game_time_id = m_plot_area.get_game_time_id();
	if (new_game_time_id != m_last_game_time_id) {
		m_last_game_time_id = new_game_time_id;
		set_labels(m_plot_area.get_labels());
		slider.set_value(m_plot_area.get_time_id());
	}
	UI::DiscreteSlider::draw(dst);
}
