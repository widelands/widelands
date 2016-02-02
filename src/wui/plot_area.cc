/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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


#include "wui/plot_area.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "ui_basic/panel.h"

using namespace std;

namespace {

const uint32_t minutes = 60 * 1000;
const uint32_t hours = 60 * 60 * 1000;
const uint32_t days = 24 * 60 * 60 * 1000;

const int32_t spacing = 5;
const int32_t space_at_bottom = 15;
const int32_t space_at_right = 10;
const int32_t space_left_of_label = 15;
const uint32_t nr_samples = 30;   // How many samples per diagramm when relative plotting

const uint32_t time_in_ms[] = {
	15 * minutes,
	30 * minutes,
	1  * hours,
	2  * hours,
	5  * hours,
	10 * hours,
	30 * hours
};

const char BG_PIC[] = "pics/plot_area_bg.png";
const RGBColor kAxisLineColor(0, 0, 0);
constexpr float kAxisLinesWidth = 1.5f;
constexpr float kPlotLinesWidth = 2.f;
const RGBColor kZeroLineColor(255, 255, 255);

enum UNIT {
	UNIT_MIN,
	UNIT_HOUR,
	UNIT_DAY,
};

string ytick_text_style(const string& text, const RGBColor& clr) {
	static boost::format f("<rt><p><font face=condensed size=13 color=%02x%02x%02x>%s</font></p></rt>");
	f % int(clr.r) % int(clr.g) % int(clr.b);
	f % text;
	return f.str();
}

string xtick_text_style(const string& text) {
	return ytick_text_style(text, RGBColor(255, 0, 0));
}

/**
 * scale value down to the available space, which is specifiey by
 * the length of the y axis and the highest scale.
 */
float scale_value
	(float const yline_length, uint32_t const highest_scale,
	 int32_t const value)
{
	return yline_length / (static_cast<float>(highest_scale) / static_cast<float>(value));
}

UNIT get_suggested_unit(uint32_t game_time) {
	// Find a nice unit for max_x
	if (game_time > 4 * days) {
		return UNIT_DAY;
	} else if (game_time > 4 * hours) {
		return UNIT_HOUR;
	} else {
		return UNIT_MIN;
	}
}

std::string get_unit_name(UNIT unit) {
	switch (unit) {
	/** TRANSLATOR: day(s). Used in statistics. */
	case UNIT_DAY:  return _("%1% d");
	/** TRANSLATOR: hour(s). Used in statistics. */
	case UNIT_HOUR: return _("%1% h");
	/** TRANSLATOR: minute(s). Used in statistics. */
	case UNIT_MIN:  return _("%1% min");
	}
	NEVER_HERE();
}

uint32_t ms_to_unit(UNIT unit, uint32_t ms) {
	switch (unit) {
	case UNIT_DAY: return ms / days;
	case UNIT_HOUR: return ms / hours;
	case UNIT_MIN: return ms / minutes;
	}
	NEVER_HERE();
}

/**
 * calculate how many values are taken together when plot mode is relative
 */
int32_t calc_how_many(uint32_t time_ms, int32_t sample_rate) {
	int32_t how_many = static_cast<int32_t>
			((static_cast<float>(time_ms)
				/
				static_cast<float>(nr_samples))
				/
				static_cast<float>(sample_rate));

	return how_many;
}



/**
 * print the string into the RenderTarget.
 */
void draw_value(const string& value, const RGBColor& color, const Point& pos, RenderTarget & dst) {
	const Image* pic = UI::g_fh1->render(ytick_text_style(value, color));
	dst.blit(pos, pic, BlendMode::UseAlpha, UI::Align::kCenterRight);
}

/**
 * draw the background and the axis of the diagram
 */
void draw_diagram
	(uint32_t time_ms, const uint32_t inner_w, const uint32_t inner_h,
	 const float xline_length, RenderTarget & dst)
{
	uint32_t how_many_ticks, max_x;

	UNIT unit = get_suggested_unit(time_ms);
	max_x = ms_to_unit(unit, time_ms);

	// Try to find a nice division of max_x (some number between 3 and 7 so data on graph are
	// readable) - to get correct data on graph, how_many_ticks has to be a divisor of max_x. Now
	// dividing by 5 is first and we get more readable intervals.
	how_many_ticks = max_x;

	while (how_many_ticks > 10 && how_many_ticks % 5 == 0) {
		how_many_ticks /= 5;
	}
	while (how_many_ticks > 7 && how_many_ticks % 2 == 0) {
		how_many_ticks /= 2;
	}
	while (how_many_ticks > 7 && how_many_ticks % 3 == 0) {
		how_many_ticks /= 3;
	}
	while (how_many_ticks > 7 && how_many_ticks % 7 == 0) {
		how_many_ticks /= 7;
	}

	// first, tile the background
	dst.tile
		(Rect(Point(0, 0), inner_w, inner_h),
		 g_gr->images().get(BG_PIC), Point(0, 0));

	// Draw coordinate system
	// X Axis
	dst.draw_line_strip({
		Point(spacing, inner_h - space_at_bottom),
		Point(inner_w - space_at_right, inner_h - space_at_bottom)},
		kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);
	// Arrow
	dst.draw_line_strip(
	   {
	    Point(spacing + 5, inner_h - space_at_bottom - 3),
	    Point(spacing, inner_h - space_at_bottom),
	    Point(spacing + 5, inner_h - space_at_bottom + 3),
	   },
	   kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);

	//  Y Axis
	dst.draw_line_strip({Point(inner_w - space_at_right, spacing),
	                     Point(inner_w - space_at_right, inner_h - space_at_bottom)},
	                    kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);
	//  No Arrow here, since this doesn't continue.

	float sub = (xline_length - space_left_of_label) / how_many_ticks;
	float posx = inner_w - space_at_right;

	for (uint32_t i = 0; i <= how_many_ticks; ++i) {
		dst.draw_line_strip({Point(static_cast<int32_t>(posx), inner_h - space_at_bottom),
		                     Point(static_cast<int32_t>(posx), inner_h - space_at_bottom + 3)},
		                    kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);

		// The space at the end is intentional to have the tick centered
		// over the number, not to the left
		const Image* xtick = UI::g_fh1->render
			(xtick_text_style((boost::format("-%u ") % (max_x / how_many_ticks * i)).str()));
		dst.blit
			(Point(static_cast<int32_t>(posx), inner_h - space_at_bottom + 10),
			 xtick, BlendMode::UseAlpha, UI::Align::kCenter);

		posx -= sub;
	}

	//  draw yticks, one at full, one at half
	dst.draw_line_strip(
	   {Point(inner_w - space_at_right, spacing), Point(inner_w - space_at_right - 3, spacing)},
	   kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);
	dst.draw_line_strip(
	   {Point(inner_w - space_at_right, spacing + ((inner_h - space_at_bottom) - spacing) / 2),
	    Point(inner_w - space_at_right - 3, spacing + ((inner_h - space_at_bottom) - spacing) / 2)},
	   kAxisLineColor, kAxisLinesWidth, LineDrawMode::kAntialiased);

	//  print the used unit
	const Image* xtick = UI::g_fh1->render(xtick_text_style((boost::format(get_unit_name(unit)) % "").str()));
	dst.blit(Point(2, spacing + 2), xtick, BlendMode::UseAlpha, UI::Align::kCenterLeft);
}

}  // namespace

WuiPlotArea::WuiPlotArea
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
:
UI::Panel (parent, x, y, w, h),
plotmode_(PLOTMODE_ABSOLUTE),
sample_rate_(0),
time_    (TIME_GAME),
game_time_id_(0)
{}


uint32_t WuiPlotArea::get_game_time() {
	uint32_t game_time = 0;

	// Find running time of the game, based on the plot data
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
		if (game_time < plotdata_[plot].dataset->size() * sample_rate_)
			game_time = plotdata_[plot].dataset->size() * sample_rate_;
	return game_time;
}

std::vector<std::string> WuiPlotArea::get_labels() {
	std::vector<std::string> labels;
	for (int32_t i = 0; i < game_time_id_; i++) {
		UNIT unit = get_suggested_unit(time_in_ms[i]);
		uint32_t val = ms_to_unit(unit, time_in_ms[i]);
		labels.push_back((boost::format(get_unit_name(unit)) % boost::lexical_cast<std::string>(val)).str());
	}
	labels.push_back(_("game"));
	return labels;
}

uint32_t WuiPlotArea::get_plot_time() {
	if (time_ == TIME_GAME) {
		// Start with the game time
		uint32_t time_ms = get_game_time();

		// Round up to a nice nearest multiple.
		// Either a multiple of 4 min
		// Either a multiple of 20 min
		// or a multiple of 2h
		// or a multiple of 20h
		// or a multiple of 4 days
		if (time_ms > 8 * days) {
			time_ms += - (time_ms % (4 * days)) + 4 * days;
		} else if (time_ms > 40 * hours) {
			time_ms += - (time_ms % (20 * hours)) + 20 * hours;
		} else if (time_ms > 4 * hours) {
			time_ms += - (time_ms % (2 * hours)) + 2 * hours;
		} else {
			time_ms += - (time_ms % (15 * minutes)) + 15 * minutes;
		}
		return time_ms;
	} else {
		return time_in_ms[time_];
	}
}



/**
 * Find the last predefined time span that is less than the game time. If this
 * is called from the outside, e.g. from a slider, then from that moment on
 * this class assumes that values passed to set_time_id adhere to the new
 * choice of time spans.
 * We start to search with i=1 to ensure that at least one option besides
 * "game" will be offered to the user.
 */
int32_t WuiPlotArea::get_game_time_id() {
	uint32_t game_time = get_game_time();
	uint32_t i;
	for (i = 1; i < 7 && time_in_ms[i] <= game_time; i++) {
	}
	game_time_id_ = i;
	return game_time_id_;
}

/*
 * Draw this. This is the main function
 */
void WuiPlotArea::draw(RenderTarget & dst) {
	float const xline_length = get_inner_w() - space_at_right  - spacing;
	float const yline_length = get_inner_h() - space_at_bottom - spacing;

	const uint32_t time_ms = get_plot_time();
	draw_diagram(time_ms, get_inner_w(), get_inner_h(), xline_length, dst);

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_ms, sample_rate_);

	uint32_t max = 0;
	//  Find the maximum value.
	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < plotdata_.size(); ++i)
			if (plotdata_[i].showplot) {
				for (uint32_t l = 0; l < plotdata_[i].dataset->size(); ++l)
					if (max < (*plotdata_[i].dataset)[l])
						max = (*plotdata_[i].dataset)[l];
			}
	} else {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
			if (plotdata_[plot].showplot) {

				const std::vector<uint32_t> & dataset = *plotdata_[plot].dataset;

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
	draw_value
		(std::to_string(max), RGBColor(60, 125, 0),
		 Point(get_inner_w() - space_at_right - 2, spacing + 2), dst);

	//  plot the pixels
	float sub =
		(xline_length - space_left_of_label)
		/
		(static_cast<float>(time_ms)
		 /
		 static_cast<float>(sample_rate_));
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
		if (plotdata_[plot].showplot) {

			RGBColor color = plotdata_[plot].plotcolor;
			std::vector<uint32_t> const * dataset = plotdata_[plot].dataset;

			std::vector<uint32_t> data_;
			if (plotmode_ == PLOTMODE_RELATIVE) {
				uint32_t add = 0;
				// Relative data, first entry is always zero
				data_.push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i];
					if (0 == ((i + 1) % how_many)) {
						data_.push_back(add);
						add = 0;
					}
				}

				dataset = &data_;
				sub = (xline_length - space_left_of_label) / static_cast<float>(nr_samples);
			}

			draw_plot_line
				(dst, dataset, yline_length, max, sub, color, get_inner_h() - space_at_bottom);
		}
}

/**
 * scale the values from dataset down to the available space and draw a single plot line
 * \param dataset the y values of the line
 * \param sub horizontal difference between 2 y values
 */
void WuiPlotArea::draw_plot_line
		(RenderTarget & dst, std::vector<uint32_t> const * dataset, float const yline_length,
		 uint32_t const highest_scale, float const sub, RGBColor const color, int32_t const yoffset)
{
	float posx = get_inner_w() - space_at_right;
	const int lx = get_inner_w() - space_at_right;
	int ly = yoffset;
	// init start point of the plot line with the first data value.
	// this prevent that the plot start always at zero
	if (int value = (*dataset)[dataset->size() - 1]) {
		ly -= static_cast<int32_t>(scale_value(yline_length, highest_scale, value));
	}

	std::vector<Point> points;
	points.emplace_back(lx, ly);

	for (int32_t i = dataset->size() - 1; i > 0 && posx > spacing; --i) {
		int32_t const curx = static_cast<int32_t>(posx);
		int32_t       cury = yoffset;

		//scale the line to the available space
		if (int32_t value = (*dataset)[i]) {
			const float length_y = scale_value(yline_length, highest_scale, value);
			cury -= static_cast<int32_t>(length_y);
		}
		points.emplace_back(curx, cury);
		posx -= sub;
	}
	dst.draw_line_strip(points, color, kPlotLinesWidth, LineDrawMode::kAntialiased);
}

/*
 * Register a new plot data stream
 */
void WuiPlotArea::register_plot_data
	(uint32_t const id,
	 std::vector<uint32_t> const * const data,
	 RGBColor const color)
{
	if (id >= plotdata_.size())
		plotdata_.resize(id + 1);

	plotdata_[id].dataset   = data;
	plotdata_[id].showplot  = false;
	plotdata_[id].plotcolor = color;

	get_game_time_id();
}

/**
 * Change the plot color of a registed data stream
 */
void WuiPlotArea::set_plotcolor(uint32_t id, RGBColor color) {
	if (id > plotdata_.size()) return;

	plotdata_[id].plotcolor = color;
}

/*
 * Show this plot data?
 */
void WuiPlotArea::show_plot(uint32_t const id, bool const t) {
	assert(id < plotdata_.size());
	plotdata_[id].showplot = t;
}

/*
 * Set sample rate the data uses
 */
void WuiPlotArea::set_sample_rate(uint32_t const id) {
	sample_rate_ = id;
}


void WuiPlotAreaSlider::draw(RenderTarget & dst) {
	int32_t new_game_time_id = plot_area_.get_game_time_id();
	if (new_game_time_id != last_game_time_id_) {
		last_game_time_id_ = new_game_time_id;
		set_labels(plot_area_.get_labels());
		slider.set_value(plot_area_.get_time_id());
	}
	UI::DiscreteSlider::draw(dst);
}

DifferentialPlotArea::DifferentialPlotArea
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
:
WuiPlotArea (parent, x, y, w, h)
{}

void DifferentialPlotArea::draw(RenderTarget & dst) {
	float const xline_length = get_inner_w() - space_at_right  - spacing;
	float const yline_length = get_inner_h() - space_at_bottom - spacing;
	//yoffset of the zero line
	float const yoffset = spacing + ((get_inner_h() - space_at_bottom) - spacing) / 2;

	const uint32_t time_ms = get_plot_time();
	draw_diagram(time_ms, get_inner_w(), get_inner_h(), xline_length, dst);

	// draw zero line
	dst.draw_line_strip({Point(get_inner_w() - space_at_right, yoffset),
	                     Point(get_inner_w() - space_at_right - xline_length, yoffset)},
	                    kZeroLineColor, kPlotLinesWidth, LineDrawMode::kAntialiased);

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_ms, sample_rate_);

	//find max and min value
	int32_t max = 0;
	int32_t min = 0;

	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < plotdata_.size(); ++i)
			if (plotdata_[i].showplot) {
				for (uint32_t l = 0; l < plotdata_[i].dataset->size(); ++l) {
					int32_t temp = (*plotdata_[i].dataset)[l] -
								   (*negative_plotdata_[i].dataset)[l];
					if (max < temp) max = temp;
					if (min > temp) min = temp;
				}
			}
	} else {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
			if (plotdata_[plot].showplot) {

				const std::vector<uint32_t> & dataset = *plotdata_[plot].dataset;
				const std::vector<uint32_t> & ndataset = *negative_plotdata_[plot].dataset;

				int32_t add = 0;
				//  Relative data, first entry is always zero.
				for (uint32_t i = 0; i < dataset.size(); ++i) {
					add += dataset[i] - ndataset[i];
					if (0 == ((i + 1) % how_many)) {
						if (max < add) max = add;
						if (min > add) min = add;

						add = 0;
					}
				}
			}
	}

	//use equal positive and negative range
	min = abs(min);
	uint32_t highest_scale = 0;
	if (min > max) {
		highest_scale = min;
	} else {
		highest_scale = max;
	}
	//print the min and max values
	draw_value
		(std::to_string(highest_scale), RGBColor(60, 125, 0),
		 Point(get_inner_w() - space_at_right - 2, spacing + 2), dst);

	draw_value
		((boost::format("-%u") % highest_scale).str(), RGBColor(125, 0, 0),
		 Point(get_inner_w() - space_at_right - 2, get_inner_h() - spacing - 15), dst);

	//  plot the pixels
	float sub =
		xline_length
		/
		(static_cast<float>(time_ms)
		 /
		 static_cast<float>(sample_rate_));
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
		if (plotdata_[plot].showplot) {

			RGBColor color = plotdata_[plot].plotcolor;
			std::vector<uint32_t> const * dataset = plotdata_[plot].dataset;
			std::vector<uint32_t> const * ndataset = negative_plotdata_[plot].dataset;

			std::vector<uint32_t> data_;
			if (plotmode_ == PLOTMODE_RELATIVE) {
				int32_t add = 0;
				// Relative data, first entry is always zero
				data_.push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i] - (*ndataset)[i];
					if (0 == ((i + 1) % how_many)) {
						data_.push_back(add);
						add = 0;
					}
				}

				dataset = &data_;
				sub = xline_length / static_cast<float>(nr_samples);
			}

			//highest_scale represent the space between zero line and top.
			//-> half of the whole differential plot area
			draw_plot_line(dst, dataset, yline_length, highest_scale * 2, sub, color, yoffset);
		}
}

/**
 * Register a new negative plot data stream. This stream is
 * used as subtrahend for calculating the plot data.
 */
void DifferentialPlotArea::register_negative_plot_data
	(uint32_t const id, std::vector<uint32_t> const * const data) {

	if (id >= negative_plotdata_.size())
		negative_plotdata_.resize(id + 1);

	negative_plotdata_[id].dataset   = data;
}
