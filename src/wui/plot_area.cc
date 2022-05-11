/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "wui/plot_area.h"

#include <cstdlib>
#include <memory>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/panel.h"

namespace {

constexpr int32_t kUpdateTimeInGametimeMs = 1000;  //  1 second, gametime

constexpr uint32_t kMinutes = 60 * 1000;
constexpr uint32_t kHours = 60 * 60 * 1000;
constexpr uint32_t kDays = 24 * 60 * 60 * 1000;

constexpr int32_t kSpacing = 5;
constexpr int32_t kSpaceBottom = 20;
constexpr int32_t kSpaceRight = 10;
constexpr int32_t kSpaceLeftOfLabel = 15;
constexpr uint32_t KNoSamples = 30;  // How many samples per diagramm when relative plotting

const uint32_t time_in_ms[] = {
   15 * kMinutes, 30 * kMinutes, 1 * kHours, 2 * kHours, 5 * kHours, 10 * kHours, 30 * kHours};

const char BG_PIC[] = "images/wui/plot_area_bg.png";
constexpr int kAxisLinesWidth = 2;
constexpr int kPlotLinesWidth = 3;

enum class Units {
	kMinutesNarrow,
	kHourNarrow,
	kDayNarrow,
	kMinutesGeneric,
	kHourGeneric,
	kDayGeneric
};

std::string ytick_text_style(const std::string& text, const UI::FontStyleInfo& style) {
	return format("<rt keep_spaces=1><p>%s</p></rt>", style.as_font_tag(text));
}

std::string xtick_text_style(const std::string& text) {
	return ytick_text_style(text, g_style_manager->statistics_plot_style().x_tick_font());
}

/**
 * scale value down to the available space, which is specified by
 * the length of the y axis and the highest scale.
 */
float scale_value(float const yline_length, uint32_t const highest_scale, int32_t const value) {
	return yline_length / (static_cast<float>(highest_scale) / static_cast<float>(value));
}

Units get_suggested_unit(uint32_t game_time, bool is_generic = false) {
	// Find a nice unit for max_x
	if (is_generic) {
		if (game_time > 4 * kDays) {
			return Units::kDayGeneric;
		}
		if (game_time > 4 * kHours) {
			return Units::kHourGeneric;
		}
		return Units::kMinutesGeneric;
	}
	if (game_time > 4 * kDays) {
		return Units::kDayNarrow;
	}
	if (game_time > 4 * kHours) {
		return Units::kHourNarrow;
	}
	return Units::kMinutesNarrow;
}

std::string get_value_with_unit(Units unit, int value) {
	switch (unit) {
	case Units::kDayNarrow:
		/** TRANSLATORS: day(s). Keep this as short as possible. Used in statistics. */
		return format(npgettext("unit_narrow", "%1%d", "%1%d", value), value);
	case Units::kHourNarrow:
		/** TRANSLATORS: hour(s). Keep this as short as possible. Used in statistics. */
		return format(npgettext("unit_narrow", "%1%h", "%1%h", value), value);
	case Units::kMinutesNarrow:
		/** TRANSLATORS: minute(s). Keep this as short as possible. Used in statistics. */
		return format(npgettext("unit_narrow", "%1%m", "%1%m", value), value);
	case Units::kMinutesGeneric:
	case Units::kHourGeneric:
	case Units::kDayGeneric:
		NEVER_HERE();
	}
	NEVER_HERE();
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
	case Units::kMinutesNarrow:
	case Units::kHourNarrow:
	case Units::kDayNarrow:
		NEVER_HERE();
	}
	NEVER_HERE();
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
int32_t calc_how_many(uint32_t time_ms, uint32_t sample_rate) {
	int32_t how_many =
	   static_cast<int32_t>((static_cast<float>(time_ms) / static_cast<float>(KNoSamples)) /
	                        static_cast<float>(sample_rate));

	return how_many;
}

/**
 * print the string into the RenderTarget.
 */
void draw_value(const std::string& value,
                const UI::FontStyleInfo& style,
                const Vector2i& pos,
                RenderTarget& dst) {
	std::shared_ptr<const UI::RenderedText> tick = UI::g_fh->render(ytick_text_style(value, style));
	Vector2i point(pos);  // Un-const this
	UI::center_vertically(tick->height(), &point);
	tick->draw(dst, point, UI::Align::kRight);
}

uint32_t calc_plot_x_max_ticks(int32_t plot_width) {
	// Render a number with 3 digits (maximal length which should appear)
	return plot_width / UI::g_fh->render(xtick_text_style(" -888 "))->width();
}

int calc_slider_label_width(const std::string& label) {
	// Font size and style as used by DiscreteSlider
	return UI::g_fh
	   ->render(as_richtext_paragraph(
	      label, g_style_manager->slider_style(UI::SliderStyle::kWuiLight).font()))
	   ->width();
}

/**
 * draw the background and the axis of the diagram
 */
void draw_diagram(uint32_t time_ms,
                  const uint32_t inner_w,
                  const uint32_t inner_h,
                  const float xline_length,
                  RenderTarget& dst) {
	const RGBColor& axis_line_color = g_style_manager->statistics_plot_style().axis_line_color();

	uint32_t how_many_ticks;
	uint32_t max_x;

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

	// Make sure we haven't more ticks than we have space for -> avoid overlap
	how_many_ticks = std::min(how_many_ticks, calc_plot_x_max_ticks(inner_w));

	// Make sure how_many_ticks is a divisor of max_x
	while (how_many_ticks > 0 && max_x % how_many_ticks != 0) {
		how_many_ticks--;
	}

	// Draw coordinate system
	// X Axis
	dst.draw_line_strip({Vector2f(kSpacing, inner_h - kSpaceBottom),
	                     Vector2f(inner_w - kSpaceRight, inner_h - kSpaceBottom)},
	                    axis_line_color, kAxisLinesWidth);
	// Arrow
	dst.draw_line_strip(
	   {
	      Vector2f(kSpacing + 5, inner_h - kSpaceBottom - 3),
	      Vector2f(kSpacing, inner_h - kSpaceBottom),
	      Vector2f(kSpacing + 5, inner_h - kSpaceBottom + 3),
	   },
	   axis_line_color, kAxisLinesWidth);

	//  Y Axis
	dst.draw_line_strip({Vector2f(inner_w - kSpaceRight, kSpacing * 3),
	                     Vector2f(inner_w - kSpaceRight, inner_h - kSpaceBottom)},
	                    axis_line_color, kAxisLinesWidth);
	//  No Arrow here, since this doesn't continue.

	float sub = (xline_length - kSpaceLeftOfLabel) / how_many_ticks;
	float posx = inner_w - kSpaceRight;

	for (uint32_t i = 0; i <= how_many_ticks; ++i) {
		dst.draw_line_strip({Vector2f(static_cast<int32_t>(posx), inner_h - kSpaceBottom),
		                     Vector2f(static_cast<int32_t>(posx), inner_h - kSpaceBottom + 3)},
		                    axis_line_color, kAxisLinesWidth);

		// The space at the end is intentional to have the tick centered
		// over the number, not to the left
		if (how_many_ticks != 0 && i != 0) {
			std::shared_ptr<const UI::RenderedText> xtick =
			   UI::g_fh->render(xtick_text_style(format("-%u ", (max_x / how_many_ticks * i))));
			Vector2i pos(posx, inner_h - kSpaceBottom + 10);
			UI::center_vertically(xtick->height(), &pos);
			xtick->draw(dst, pos, UI::Align::kCenter);
		}

		posx -= sub;
	}

	//  draw yticks, one at full, one at three-quarter, one at half, one at quarter & 0
	dst.draw_line_strip({Vector2f(inner_w - kSpaceRight + 3, kSpacing * 3),
	                     Vector2f(inner_w - kSpaceRight - 3, kSpacing * 3)},
	                    axis_line_color, kAxisLinesWidth);

	dst.draw_line_strip(
	   {Vector2f(
	       inner_w - kSpaceRight + 2,
	       kSpacing * 3 + ((((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f) - kSpacing * 3) / 2.f),
	    Vector2f(
	       inner_w - kSpaceRight,
	       kSpacing * 3 + ((((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f) - kSpacing * 3) / 2.f)},
	   axis_line_color, kAxisLinesWidth);

	dst.draw_line_strip(
	   {Vector2f(inner_w - kSpaceRight + 3, ((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f),
	    Vector2f(inner_w - kSpaceRight, ((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f)},
	   axis_line_color, kAxisLinesWidth);

	dst.draw_line_strip(
	   {Vector2f(
	       inner_w - kSpaceRight + 2,
	       inner_h - kSpaceBottom -
	          (inner_h - kSpaceBottom - ((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f) / 2.f),
	    Vector2f(
	       inner_w - kSpaceRight,
	       inner_h - kSpaceBottom -
	          (inner_h - kSpaceBottom - ((inner_h - kSpaceBottom) + kSpacing * 3) / 2.f) / 2.f)},
	   axis_line_color, kAxisLinesWidth);

	dst.draw_line_strip({Vector2f(inner_w - kSpaceRight + 3, inner_h - kSpaceBottom),
	                     Vector2f(inner_w - kSpaceRight, inner_h - kSpaceBottom)},
	                    axis_line_color, kAxisLinesWidth);

	//  print the used unit
	std::shared_ptr<const UI::RenderedText> xtick =
	   UI::g_fh->render(xtick_text_style(get_generic_unit_name(unit)));
	Vector2i pos(2, kSpacing + 2);
	UI::center_vertically(xtick->height(), &pos);
	xtick->draw(dst, pos);
}

}  // namespace

WuiPlotArea::WuiPlotArea(UI::Panel* const parent,
                         int32_t const x,
                         int32_t const y,
                         int32_t const w,
                         int32_t const h,
                         uint32_t sample_rate,
                         Plotmode plotmode)
   : UI::Panel(parent, UI::PanelStyle::kWui, x, y, w, h),
     plotmode_(plotmode),
     sample_rate_(sample_rate),
     needs_update_(true),
     lastupdate_(0),
     xline_length_(get_inner_w() - kSpaceRight - kSpacing),
     yline_length_(get_inner_h() - kSpaceBottom - kSpacing * 3),
     time_ms_(0),
     highest_scale_(0),
     sub_(0),
     time_(TIME_GAME),
     game_time_id_(0) {
	update();
}

uint32_t WuiPlotArea::get_game_time() const {
	uint32_t game_time = 0;

	// Find running time of the game, based on the plot data
	for (const auto& plot : plotdata_) {
		if (game_time < plot.second.absolute_data->size() * sample_rate_) {
			game_time = plot.second.absolute_data->size() * sample_rate_;
		}
	}
	return game_time;
}

std::vector<std::string> WuiPlotArea::get_labels() const {
	std::vector<std::string> labels;
	for (uint32_t i = 0; i < game_time_id_; i++) {
		Units unit = get_suggested_unit(time_in_ms[i], false);
		labels.push_back(get_value_with_unit(unit, ms_to_unit(unit, time_in_ms[i])));
	}
	labels.emplace_back(_("game"));
	return labels;
}

uint32_t WuiPlotArea::get_plot_time() const {
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
			time_ms += 4 * kDays - (time_ms % (4 * kDays));
		} else if (time_ms > 40 * kHours) {
			time_ms += 20 * kHours - (time_ms % (20 * kHours));
		} else if (time_ms > 4 * kHours) {
			time_ms += 2 * kHours - (time_ms % (2 * kHours));
		} else {
			time_ms += 15 * kMinutes - (time_ms % (15 * kMinutes));
		}
		return time_ms;
	}
	return time_in_ms[time_];
}

/**
 * Find the last predefined time span that is less than the game time. If this
 * is called from the outside, e.g. from a slider, then from that moment on
 * this class assumes that values passed to set_time_id adhere to the new
 * choice of time spans.
 * We start to search with i=1 to ensure that at least one option besides
 * "game" will be offered to the user.
 */
uint32_t WuiPlotArea::get_game_time_id() {
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

int32_t WuiPlotArea::initialize_update() {
	// Initialize
	for (const auto& plot : plotdata_) {
		plot.second.relative_data->clear();
	}
	// Get range
	time_ms_ = get_plot_time();
	if (plotmode_ == Plotmode::kAbsolute) {
		sub_ = (xline_length_ - kSpaceLeftOfLabel) /
		       (static_cast<float>(time_ms_) / static_cast<float>(sample_rate_));
	} else {
		sub_ = (xline_length_ - kSpaceLeftOfLabel) / static_cast<float>(KNoSamples);
	}

	// How many do we aggregate when relative plotting
	return calc_how_many(time_ms_, sample_rate_);
}

//  Find the maximum value.
void WuiPlotArea::update() {
	const int32_t how_many = initialize_update();

	// Calculate highest scale
	highest_scale_ = 0;
	if (plotmode_ == Plotmode::kAbsolute) {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {
				for (const auto& absdata : *plot.second.absolute_data) {
					if (highest_scale_ < absdata) {
						highest_scale_ = absdata;
					}
				}
			}
		}
	} else {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {
				const std::vector<uint32_t>& dataset = *plot.second.absolute_data;
				uint32_t add = 0;
				//  Relative data, first entry is always zero.
				for (uint32_t i = 0; i < dataset.size(); ++i) {
					add += dataset[i];
					if (0 == ((i + 1) % how_many)) {
						if (highest_scale_ < add) {
							highest_scale_ = add;
						}
						add = 0;
					}
				}
			}
		}
	}

	//  Calculate plot data
	if (plotmode_ == Plotmode::kRelative) {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {
				std::vector<uint32_t> const* dataset = plot.second.absolute_data;
				uint32_t add = 0;
				// Relative data, first entry is always zero
				plot.second.relative_data->push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i];
					if (0 == ((i + 1) % how_many)) {
						plot.second.relative_data->push_back(add);
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
void WuiPlotArea::draw(RenderTarget& dst) {
	dst.tile(Recti(Vector2i::zero(), get_inner_w(), get_inner_h()), g_image_cache->get(BG_PIC),
	         Vector2i::zero());
	if (needs_update_) {
		update();
		needs_update_ = false;
	}
	if (highest_scale_ != 0u) {
		draw_plot(dst, get_inner_h() - kSpaceBottom, std::to_string(highest_scale_), highest_scale_);
	}
	// Print the 0
	draw_value("0", g_style_manager->statistics_plot_style().x_tick_font(),
	           Vector2i(get_inner_w() - kSpaceRight + 3, get_inner_h() - kSpaceBottom + 10), dst);
}

void WuiPlotArea::draw_plot(RenderTarget& dst,
                            float yoffset,
                            const std::string& yscale_label,
                            uint32_t highest_scale) {

	//  plot the pixels
	for (const auto& plot : plotdata_) {
		if (plot.second.showplot) {
			draw_plot_line(dst,
			               (plotmode_ == Plotmode::kRelative) ? plot.second.relative_data.get() :
                                                             plot.second.absolute_data,
			               highest_scale, sub_, plot.second.plotcolor, yoffset);
		}
	}

	draw_diagram(time_ms_, get_inner_w(), get_inner_h(), xline_length_, dst);

	//  print the maximal value into the top right corner
	draw_value(yscale_label, g_style_manager->statistics_plot_style().y_max_value_font(),
	           Vector2i(get_inner_w() - kSpaceRight + 3, kSpacing + 2), dst);
}

/**
 * scale the values from dataset down to the available space and draw a single plot line
 * \param dataset the y values of the line
 * \param sub horizontal difference between 2 y values
 */
void WuiPlotArea::draw_plot_line(RenderTarget& dst,
                                 std::vector<uint32_t> const* dataset,
                                 uint32_t const highest_scale,
                                 float const sub,
                                 const RGBColor& color,
                                 int32_t const yoffset) {
	if (!dataset->empty()) {
		float posx = get_inner_w() - kSpaceRight;
		const int lx = get_inner_w() - kSpaceRight;
		int ly = yoffset;
		// Init start point of the plot line with the first data value.
		// This prevents that the plot starts always at zero

		if (int value = (*dataset)[dataset->size() - 1]) {
			ly -= static_cast<int32_t>(scale_value(yline_length_, highest_scale, value));
		}

		std::vector<Vector2f> points;
		points.emplace_back(lx, ly);

		for (int32_t i = dataset->size() - 1; i > 0 && posx > kSpacing; --i) {
			int32_t const curx = static_cast<int32_t>(posx);
			int32_t cury = yoffset;

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
void WuiPlotArea::register_plot_data(unsigned const id,
                                     std::vector<uint32_t> const* const data,
                                     RGBColor const color) {

	// Let the map create the ID for us if it doesn't exist yet
	plotdata_[id].absolute_data = data;
	plotdata_[id].relative_data.reset(
	   new std::vector<uint32_t>());  // Will be filled in the update() function.
	plotdata_[id].showplot = false;
	plotdata_[id].plotcolor = color;

	get_game_time_id();
	needs_update_ = true;
}

/**
 * Change the plot color of a registed data stream
 */
void WuiPlotArea::set_plotcolor(unsigned id, RGBColor color) {
	if (plotdata_.count(id) == 0) {
		return;
	}

	plotdata_[id].plotcolor = color;
	needs_update_ = true;
}

/*
 * Show this plot data?
 */
void WuiPlotArea::show_plot(unsigned const id, bool const t) {
	assert(plotdata_.count(id) == 1);
	plotdata_[id].showplot = t;
	needs_update_ = true;
}

void WuiPlotAreaSlider::draw(RenderTarget& dst) {
	uint32_t new_game_time_id = plot_area_.get_game_time_id();
	if (new_game_time_id != last_game_time_id_) {
		last_game_time_id_ = new_game_time_id;
		std::vector<std::string> new_labels = plot_area_.get_labels();
		// There should be always at least "15m" and "game"
		assert(new_labels.size() >= 2);
		// The slider places the level with equal distances, so find the
		// longest label and use it to calculate how many labels are possible
		int max_width = std::max(
		   calc_slider_label_width(new_labels[0]), calc_slider_label_width(new_labels.back()));
		for (size_t i = 1; i < new_labels.size() - 1; ++i) {
			max_width = std::max(max_width, calc_slider_label_width(new_labels[i]));
			if (max_width * (static_cast<int>(i) + 2) > get_w()) {
				// We have too many labels. Drop all further ones
				new_labels[i] = new_labels.back();
				new_labels.resize(i + 1);
				break;
			}
		}
		set_labels(new_labels);
		slider.set_value(plot_area_.get_time_id());
	}
	UI::DiscreteSlider::draw(dst);
}

DifferentialPlotArea::DifferentialPlotArea(UI::Panel* const parent,
                                           int32_t const x,
                                           int32_t const y,
                                           int32_t const w,
                                           int32_t const h,
                                           uint32_t sample_rate,
                                           Plotmode plotmode)
   : WuiPlotArea(parent, x, y, w, h, sample_rate, plotmode) {
	update();
}

void DifferentialPlotArea::update() {
	const int32_t how_many = initialize_update();

	// Calculate highest scale
	int32_t max = 0;
	int32_t min = 0;

	if (plotmode_ == Plotmode::kAbsolute) {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {
				for (uint32_t l = 0; l < plot.second.absolute_data->size(); ++l) {
					int32_t temp = (*plot.second.absolute_data)[l] -
					               (*negative_plotdata_[plot.first].absolute_data)[l];
					if (max < temp) {
						max = temp;
					}
					if (min > temp) {
						min = temp;
					}
				}
			}
		}
	} else {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {

				const std::vector<uint32_t>& dataset = *plot.second.absolute_data;
				const std::vector<uint32_t>& ndataset = *negative_plotdata_[plot.first].absolute_data;

				int32_t add = 0;
				//  Relative data, first entry is always zero.
				for (uint32_t i = 0; i < dataset.size(); ++i) {
					add += dataset[i] - ndataset[i];
					if (0 == ((i + 1) % how_many)) {
						if (max < add) {
							max = add;
						}
						if (min > add) {
							min = add;
						}
						add = 0;
					}
				}
			}
		}
	}

	// Use equal positive and negative range
	min = abs(min);
	highest_scale_ = (min > max) ? min : max;

	//  Calculate plot data
	if (plotmode_ == Plotmode::kRelative) {
		for (const auto& plot : plotdata_) {
			if (plot.second.showplot) {
				std::vector<uint32_t> const* dataset = plot.second.absolute_data;
				std::vector<uint32_t> const* ndataset = negative_plotdata_[plot.first].absolute_data;
				uint32_t add = 0;
				// Relative data, first entry is always zero
				plot.second.relative_data->push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i] - (*ndataset)[i];
					if (0 == ((i + 1) % how_many)) {
						plot.second.relative_data->push_back(add);
						add = 0;
					}
				}
			}
		}
	}
}

void DifferentialPlotArea::draw(RenderTarget& dst) {

	// first, tile the background
	dst.tile(Recti(Vector2i::zero(), get_inner_w(), get_inner_h()), g_image_cache->get(BG_PIC),
	         Vector2i::zero());

	// yoffset of the zero line
	float const yoffset = ((get_inner_h() - kSpaceBottom) + kSpacing * 3) / 2.f;

	// draw zero line
	dst.draw_line_strip({Vector2f(get_inner_w() - kSpaceRight, yoffset),
	                     Vector2f(get_inner_w() - kSpaceRight - xline_length_, yoffset)},
	                    g_style_manager->statistics_plot_style().zero_line_color(), kPlotLinesWidth);

	// Draw data and diagram
	draw_plot(dst, yoffset, std::to_string(highest_scale_), 2 * highest_scale_);
	// Print the min value
	draw_value(format("-%u", (highest_scale_)),
	           g_style_manager->statistics_plot_style().y_min_value_font(),
	           Vector2i(get_inner_w() - kSpaceRight + 3, get_inner_h() - kSpaceBottom + 10), dst);
}

/**
 * Register a new negative plot data stream. This stream is
 * used as subtrahend for calculating the plot data.
 */
void DifferentialPlotArea::register_negative_plot_data(unsigned const id,
                                                       std::vector<uint32_t> const* const data) {

	// Let the map create the ID for us if it doesn't exist yet
	negative_plotdata_[id].absolute_data = data;
	needs_update_ = true;
}
