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

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "ui_basic/panel.h"

using namespace std;

namespace {

constexpr int32_t kUpdateTimeInGametimeMs = 1000;  //  1 second, gametime

constexpr uint32_t kMinutes = 60 * 1000;
constexpr uint32_t kHours = 60 * 60 * 1000;
constexpr uint32_t kDays = 24 * 60 * 60 * 1000;

constexpr int32_t kSpacing = 5;
constexpr int32_t kSpaceBottom = 20;
constexpr int32_t kSpaceRight = 10;
constexpr int32_t kSpaceLeftOfLabel = 15;
constexpr uint32_t KNoSamples = 30;   // How many samples per diagramm when relative plotting

const uint32_t time_in_ms[] = {
	15 * kMinutes,
	30 * kMinutes,
	1  * kHours,
	2  * kHours,
	5  * kHours,
	10 * kHours,
	30 * kHours
};

const char BG_PIC[] = "images/wui/plot_area_bg.png";
const RGBColor kAxisLineColor(0, 0, 0);
constexpr float kAxisLinesWidth = 2.0f;
constexpr float kPlotLinesWidth = 3.f;
const RGBColor kZeroLineColor(255, 255, 255);

enum class Units {
	kMinutesNarrow,
	kHourNarrow,
	kDayNarrow,
	kMinutesGeneric,
	kHourGeneric,
	kDayGeneric
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

Units get_suggested_unit(uint32_t game_time, bool is_generic = false) {
	// Find a nice unit for max_x
	if (is_generic) {
		if (game_time > 4 * kDays) {
			return Units::kDayGeneric;
		} else if (game_time > 4 * kHours) {
			return Units::kHourGeneric;
		} else {
			return Units::kMinutesGeneric;
		}
	}
	else {
		if (game_time > 4 * kDays) {
			return Units::kDayNarrow;
		} else if (game_time > 4 * kHours) {
			return Units::kHourNarrow;
		} else {
			return Units::kMinutesNarrow;
		}
	}
}

std::string get_value_with_unit(Units unit, int value) {
	switch (unit) {
	case Units::kDayNarrow:
		/** TRANSLATORS: day(s). Keep this as short as possible. Used in statistics. */
		return (boost::format(npgettext("unit_narrow", "%1%d", "%1%d", value)) % value).str();
	case Units::kHourNarrow:
		/** TRANSLATORS: hour(s). Keep this as short as possible. Used in statistics. */
		return (boost::format(npgettext("unit_narrow", "%1%h", "%1%h", value)) % value).str();
	case Units::kMinutesNarrow:
		/** TRANSLATORS: minute(s). Keep this as short as possible. Used in statistics. */
		return (boost::format(npgettext("unit_narrow", "%1%m", "%1%m", value)) % value).str();
	default: NEVER_HERE();
	}
}

std::string get_generic_unit_name(Units unit) {
	switch (unit) {
	case Units::kDayGeneric:
		/** TRANSLATORS: Generic unit label. Used in statistics. */
		return pgettext("unit_generic", "days");
	case Units::kHourGeneric:
		/** TRANSLATORS: Generic unit label. Used in statistics. */
		return pgettext("unit_generic", "hours");
	case Units::kMinutesGeneric:
		/** TRANSLATORS: Generic unit label. Used in statistics. */
		return pgettext("unit_generic", "minutes");
	default: NEVER_HERE();
	}
}


uint32_t ms_to_unit(Units unit, uint32_t ms) {
	switch (unit) {
	case Units::kDayGeneric:
	case Units::kDayNarrow:
		return ms / kDays;
	case Units::kHourGeneric:
	case Units::kHourNarrow:
		return ms / kHours;
	case Units::kMinutesGeneric:
	case Units::kMinutesNarrow:
		return ms / kMinutes;
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
				static_cast<float>(KNoSamples))
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

	Units unit = get_suggested_unit(time_ms, true);
	max_x = ms_to_unit(unit, time_ms);
	// Make sure that we always have a tick
	max_x = std::max(max_x, 1u);

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
	// Make sure that we always have a tick
	how_many_ticks = std::max(how_many_ticks, 1u);

	// first, tile the background
	dst.tile
		(Rect(Point(0, 0), inner_w, inner_h),
		 g_gr->images().get(BG_PIC), Point(0, 0));

	// Draw coordinate system
	// X Axis
	dst.draw_line_strip({
		FloatPoint(kSpacing, inner_h - kSpaceBottom),
		FloatPoint(inner_w - kSpaceRight, inner_h - kSpaceBottom)},
		kAxisLineColor, kAxisLinesWidth);
	// Arrow
	dst.draw_line_strip({
		FloatPoint(kSpacing + 5, inner_h - kSpaceBottom - 3),
		FloatPoint(kSpacing, inner_h - kSpaceBottom),
		FloatPoint(kSpacing + 5, inner_h - kSpaceBottom + 3),
		}, kAxisLineColor, kAxisLinesWidth);

	//  Y Axis
	dst.draw_line_strip({FloatPoint(inner_w - kSpaceRight, kSpacing),
	                     FloatPoint(inner_w - kSpaceRight, inner_h - kSpaceBottom)},
							  kAxisLineColor, kAxisLinesWidth);
	//  No Arrow here, since this doesn't continue.

	float sub = (xline_length - kSpaceLeftOfLabel) / how_many_ticks;
	float posx = inner_w - kSpaceRight;

	for (uint32_t i = 0; i <= how_many_ticks; ++i) {
		dst.draw_line_strip({FloatPoint(static_cast<int32_t>(posx), inner_h - kSpaceBottom),
		                     FloatPoint(static_cast<int32_t>(posx), inner_h - kSpaceBottom + 3)},
								  kAxisLineColor, kAxisLinesWidth);

		// The space at the end is intentional to have the tick centered
		// over the number, not to the left
		const Image* xtick = UI::g_fh1->render
			(xtick_text_style((boost::format("-%u ") % (max_x / how_many_ticks * i)).str()));
		dst.blit
			(Point(static_cast<int32_t>(posx), inner_h - kSpaceBottom + 10),
			 xtick, BlendMode::UseAlpha, UI::Align::kCenter);

		posx -= sub;
	}

	//  draw yticks, one at full, one at half
	dst.draw_line_strip({
		FloatPoint(inner_w - kSpaceRight, kSpacing), FloatPoint(inner_w - kSpaceRight - 3, kSpacing)},
		kAxisLineColor, kAxisLinesWidth);
	dst.draw_line_strip({
		FloatPoint(inner_w - kSpaceRight, kSpacing + ((inner_h - kSpaceBottom) - kSpacing) / 2),
		FloatPoint(inner_w - kSpaceRight - 3, kSpacing + ((inner_h - kSpaceBottom) - kSpacing) / 2)},
		kAxisLineColor, kAxisLinesWidth);

	//  print the used unit
	const Image* xtick = UI::g_fh1->render(xtick_text_style(get_generic_unit_name(unit)));
	dst.blit(Point(2, kSpacing + 2), xtick, BlendMode::UseAlpha, UI::Align::kCenterLeft);
}

}  // namespace

WuiPlotArea::WuiPlotArea
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
:
UI::Panel (parent, x, y, w, h),
plotmode_(PLOTMODE_ABSOLUTE),
sample_rate_(0),
needs_update_(true),
lastupdate_(0),
xline_length_(get_inner_w() - kSpaceRight  - kSpacing),
yline_length_(get_inner_h() - kSpaceBottom - kSpacing),
time_ms_(0),
highest_scale_(0),
sub_(0.0f),
time_(TIME_GAME),
game_time_id_(0)
{
	update();
}


uint32_t WuiPlotArea::get_game_time() {
	uint32_t game_time = 0;

	// Find running time of the game, based on the plot data
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
		if (game_time < plotdata_[plot].absolute_data->size() * sample_rate_)
			game_time = plotdata_[plot].absolute_data->size() * sample_rate_;
	return game_time;
}

std::vector<std::string> WuiPlotArea::get_labels() {
	std::vector<std::string> labels;
	for (int32_t i = 0; i < game_time_id_; i++) {
		Units unit = get_suggested_unit(time_in_ms[i], false);
		labels.push_back(get_value_with_unit(unit, ms_to_unit(unit, time_in_ms[i])));
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
		if (time_ms > 8 * kDays) {
			time_ms += - (time_ms % (4 * kDays)) + 4 * kDays;
		} else if (time_ms > 40 * kHours) {
			time_ms += - (time_ms % (20 * kHours)) + 20 * kHours;
		} else if (time_ms > 4 * kHours) {
			time_ms += - (time_ms % (2 * kHours)) + 2 * kHours;
		} else {
			time_ms += - (time_ms % (15 * kMinutes)) + 15 * kMinutes;
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


void WuiPlotArea::think() {
	// Don't update that often
	const int32_t gametime = get_game_time();
	if (needs_update_ || (gametime - lastupdate_) > kUpdateTimeInGametimeMs) {
		update();
		lastupdate_ = gametime;
	}
	needs_update_ = false;
}


//  Find the maximum value.
void WuiPlotArea::update() {
	time_ms_ = get_plot_time();
	for (uint32_t i = 0; i < plotdata_.size(); ++i) {
		plotdata_[i].relative_data->clear();
	}

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_ms_, sample_rate_);
	highest_scale_ = 0;
	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < plotdata_.size(); ++i)
			if (plotdata_[i].showplot) {
				for (uint32_t l = 0; l < plotdata_[i].absolute_data->size(); ++l) {
					if (highest_scale_ < (*plotdata_[i].absolute_data)[l]) {
						highest_scale_ = (*plotdata_[i].absolute_data)[l];
					}
				}
			}
	} else {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot) {
			if (plotdata_[plot].showplot) {
				const std::vector<uint32_t> & dataset = *plotdata_[plot].absolute_data;
				uint32_t add = 0;
				//  Relative data, first entry is always zero.
				for (uint32_t i = 0; i < dataset.size(); ++i) {
					add += dataset[i];
					if (0 == ((i + 1) % how_many)) {
						if (highest_scale_ < add)
							highest_scale_ = add;
						add = 0;
					}
				}
			}
		}
	}

	//  Update the datasets
	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		sub_ =
			(xline_length_ - kSpaceLeftOfLabel)
			/
			(static_cast<float>(time_ms_)
			 /
			 static_cast<float>(sample_rate_));
	} else {
		sub_ = (xline_length_ - kSpaceLeftOfLabel) / static_cast<float>(KNoSamples);
	}

	if (plotmode_ == PLOTMODE_RELATIVE) {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot) {
			if (plotdata_[plot].showplot) {
				std::vector<uint32_t> const * dataset = plotdata_[plot].absolute_data;
				uint32_t add = 0;
				// Relative data, first entry is always zero
				plotdata_[plot].relative_data->push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i];
					if (0 == ((i + 1) % how_many)) {
						plotdata_[plot].relative_data->push_back(add);
						add = 0;
					}
				}
			}
		}
	}
}



/*
 * Draw this. This is the main function
 */
void WuiPlotArea::draw(RenderTarget & dst) {
	draw_diagram(time_ms_, get_inner_w(), get_inner_h(), xline_length_, dst);

	//  print the maximal value into the top right corner
	draw_value
		(std::to_string(highest_scale_), RGBColor(60, 125, 0),
		 Point(get_inner_w() - kSpaceRight - 2, kSpacing + 2), dst);

	//  plot the pixels
	float const yoffset = get_inner_h() - kSpaceBottom;
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot) {
		if (plotdata_[plot].showplot) {
			draw_plot_line
				(dst, (plotmode_ == PLOTMODE_RELATIVE) ? plotdata_[plot].relative_data : plotdata_[plot].absolute_data, highest_scale_, sub_, plotdata_[plot].plotcolor, yoffset);
		}
	}
}

/**
 * scale the values from dataset down to the available space and draw a single plot line
 * \param dataset the y values of the line
 * \param sub horizontal difference between 2 y values
 */
void WuiPlotArea::draw_plot_line
		(RenderTarget & dst, std::vector<uint32_t> const * dataset,
		 uint32_t const highest_scale, float const sub, RGBColor const color, int32_t const yoffset)
{
	if (!dataset->empty()) {
		float posx = get_inner_w() - kSpaceRight;
		const int lx = get_inner_w() - kSpaceRight;
		int ly = yoffset;
		// Init start point of the plot line with the first data value.
		// This prevents that the plot starts always at zero

		if (int value = (*dataset)[dataset->size() - 1]) {
			ly -= static_cast<int32_t>(scale_value(yline_length_, highest_scale, value));
		}

		std::vector<FloatPoint> points;
		points.emplace_back(lx, ly);

		for (int32_t i = dataset->size() - 1; i > 0 && posx > kSpacing; --i) {
			int32_t const curx = static_cast<int32_t>(posx);
			int32_t       cury = yoffset;

			// Scale the line to the available space
			if (int32_t value = (*dataset)[i]) {
				const float length_y = scale_value(yline_length_, highest_scale, value);
				cury -= static_cast<int32_t>(length_y);
			}
			points.emplace_back(curx, cury);
			posx -= sub;
		}
		dst.draw_line_strip(points, color, kPlotLinesWidth);
	}
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

	plotdata_[id].absolute_data = data;
	plotdata_[id].relative_data = new std::vector<uint32_t>(); // Will be filled in the update() function.
	plotdata_[id].showplot  = false;
	plotdata_[id].plotcolor = color;

	get_game_time_id();
	needs_update_ = true;
}

/**
 * Change the plot color of a registed data stream
 */
void WuiPlotArea::set_plotcolor(uint32_t id, RGBColor color) {
	if (id > plotdata_.size()) return;

	plotdata_[id].plotcolor = color;
	needs_update_ = true;
}

/*
 * Show this plot data?
 */
void WuiPlotArea::show_plot(uint32_t const id, bool const t) {
	assert(id < plotdata_.size());
	plotdata_[id].showplot = t;
	needs_update_ = true;
}

/*
 * Set sample rate the data uses
 */
void WuiPlotArea::set_sample_rate(uint32_t const id) {
	sample_rate_ = id;
	needs_update_ = true;
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
{
	update();
}

void DifferentialPlotArea::update() {
	time_ms_ = get_plot_time();
	for (uint32_t i = 0; i < plotdata_.size(); ++i) {
		plotdata_[i].relative_data->clear();
	}

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_ms_, sample_rate_);

	// Find max and min value
	int32_t max = 0;
	int32_t min = 0;

	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < plotdata_.size(); ++i)
			if (plotdata_[i].showplot) {
				for (uint32_t l = 0; l < plotdata_[i].absolute_data->size(); ++l) {
					int32_t temp = (*plotdata_[i].absolute_data)[l] -
									(*negative_plotdata_[i].absolute_data)[l];
					if (max < temp) max = temp;
					if (min > temp) min = temp;
				}
			}
	} else {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot)
			if (plotdata_[plot].showplot) {

				const std::vector<uint32_t> & dataset = *plotdata_[plot].absolute_data;
				const std::vector<uint32_t> & ndataset = *negative_plotdata_[plot].absolute_data;

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

	// Use equal positive and negative range
	min = abs(min);
	highest_scale_ = 0;
	if (min > max) {
		highest_scale_ = min;
	} else {
		highest_scale_ = max;
	}

	//  plot the pixels
	if (plotmode_ == PLOTMODE_ABSOLUTE)  {
		sub_ =
			xline_length_
			/
			(static_cast<float>(time_ms_)
			 /
			 static_cast<float>(sample_rate_));
	} else {
		sub_ = xline_length_ / static_cast<float>(KNoSamples);
	}

	if (plotmode_ == PLOTMODE_RELATIVE) {
		for (uint32_t plot = 0; plot < plotdata_.size(); ++plot) {
			if (plotdata_[plot].showplot) {
				std::vector<uint32_t> const * dataset = plotdata_[plot].absolute_data;
				std::vector<uint32_t> const * ndataset = negative_plotdata_[plot].absolute_data;
				uint32_t add = 0;
				// Relative data, first entry is always zero
				plotdata_[plot].relative_data->push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i] - (*ndataset)[i];
					if (0 == ((i + 1) % how_many)) {
						plotdata_[plot].relative_data->push_back(add);
						add = 0;
					}
				}
			}
		}
	}
}

void DifferentialPlotArea::draw(RenderTarget & dst) {
	draw_diagram(time_ms_, get_inner_w(), get_inner_h(), xline_length_, dst);

	// Print the min and max values
	draw_value
		(std::to_string(highest_scale_), RGBColor(60, 125, 0),
		 Point(get_inner_w() - kSpaceRight - 2, kSpacing + 2), dst);

	draw_value
		((boost::format("-%u") % highest_scale_).str(), RGBColor(125, 0, 0),
		 Point(get_inner_w() - kSpaceRight - 2, get_inner_h() - kSpacing - 15), dst);

	// yoffset of the zero line
	float const yoffset = kSpacing + ((get_inner_h() - kSpaceBottom) - kSpacing) / 2;

	// draw zero line
	dst.draw_line_strip({FloatPoint(get_inner_w() - kSpaceRight, yoffset),
	                     FloatPoint(get_inner_w() - kSpaceRight - xline_length_, yoffset)},
							  kZeroLineColor, kPlotLinesWidth);

	//  plot the pixels
	for (uint32_t plot = 0; plot < plotdata_.size(); ++plot) {
		if (plotdata_[plot].showplot) {
			draw_plot_line
				(dst, (plotmode_ == PLOTMODE_RELATIVE) ? plotdata_[plot].relative_data : plotdata_[plot].absolute_data, highest_scale_ * 2, sub_, plotdata_[plot].plotcolor, yoffset);
		}
	}
}

/**
 * Register a new negative plot data stream. This stream is
 * used as subtrahend for calculating the plot data.
 */
void DifferentialPlotArea::register_negative_plot_data
	(uint32_t const id, std::vector<uint32_t> const * const data) {

	if (id >= negative_plotdata_.size()) {
		negative_plotdata_.resize(id + 1);
	}

	negative_plotdata_[id].absolute_data = data;
	needs_update_ = true;
}
