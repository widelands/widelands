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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WUI_PLOT_AREA_H
#define WL_WUI_PLOT_AREA_H

#include <map>
#include <memory>

#include "graphic/color.h"
#include "ui_basic/panel.h"
#include "ui_basic/slider.h"

/*
 * A Plot Area is a simple 2D Plot, with the
 * X Axis as time (actually Minus Time)
 * and the Y Axis some Data
 */
struct WuiPlotArea : public UI::Panel {
	enum TIME {
		TIME_15_MINS = 0,
		TIME_30_MINS,
		TIME_ONE_HOUR,
		TIME_TWO_HOURS,
		TIME_FOUR_HOURS,
		TIME_EIGHT_HOURS,
		TIME_16_HOURS,
		TIME_GAME,
	};
	enum class Plotmode {
		//  Always aggregate the samples of some time periods, so that the graph is
		//  not completely zig-zagged.
		kRelative,
		kAbsolute
	};

	// sample_rate is in in milliseconds
	WuiPlotArea(UI::Panel* parent,
	            int32_t x,
	            int32_t y,
	            int32_t w,
	            int32_t h,
	            uint32_t sample_rate,
	            Plotmode plotmode);

	/// Calls update() if needed
	void think() override;

	void draw(RenderTarget&) override;

	void set_time(TIME id) {
		time_ = id;
		needs_update_ = true;
	}

	void set_time_id(uint32_t time) {
		if (time == game_time_id_)
			set_time(TIME_GAME);
		else
			set_time(static_cast<TIME>(time));
		needs_update_ = true;
	}
	TIME get_time() const {
		return static_cast<TIME>(time_);
	}
	int32_t get_time_id() const {
		if (time_ == TIME_GAME)
			return game_time_id_;
		else
			return time_;
	}

	uint32_t get_game_time_id();

	void register_plot_data(unsigned id, const std::vector<uint32_t>* data, RGBColor);
	void show_plot(unsigned id, bool t);

	void set_plotcolor(unsigned id, RGBColor color);

	std::vector<std::string> get_labels() const;

protected:
	void draw_plot(RenderTarget& dst,
	               float yoffset,
	               const std::string& yscale_label,
	               uint32_t highest_scale);
	void draw_plot_line(RenderTarget& dst,
	                    std::vector<uint32_t> const* dataset,
	                    uint32_t const highest_scale,
	                    float const sub,
	                    const RGBColor& color,
	                    int32_t yoffset);
	uint32_t get_plot_time() const;
	/// Recalculates the data
	virtual void update();
	// Initializes relative_dataset and time scaling.
	// Returns how many values will be aggregated when relative plotting
	int32_t initialize_update();

	struct PlotData {
		const std::vector<uint32_t>* absolute_data;            // The absolute dataset
		std::unique_ptr<std::vector<uint32_t>> relative_data;  // The relative dataset
		bool showplot;
		RGBColor plotcolor;
	};
	std::map<unsigned, PlotData> plotdata_;

	Plotmode plotmode_;
	uint32_t sample_rate_;

	/// Whether there has ben a data update since the last time that think() was executed
	bool needs_update_;
	/// The last time the information in this Panel got updated
	uint32_t lastupdate_;

	/// For first updating and then plotting the data
	float const xline_length_;
	float const yline_length_;
	uint32_t time_ms_;
	uint32_t highest_scale_;
	float sub_;

private:
	uint32_t get_game_time() const;

	TIME time_;              // How much do you want to list
	uint32_t game_time_id_;  // what label is used for TIME_GAME
};

/**
 * A discrete slider with plot time steps.
 * Enclosing element will need to connect the changedto signal.
 */
struct WuiPlotAreaSlider : public UI::DiscreteSlider {
	WuiPlotAreaSlider(Panel* const parent,
	                  WuiPlotArea& plot_area,
	                  const int32_t x,
	                  const int32_t y,
	                  const uint32_t w,
	                  const uint32_t h,
	                  const std::string& tooltip_text = std::string(),
	                  const uint32_t cursor_size = 20,
	                  const bool enabled = true)
	   : DiscreteSlider(parent,
	                    x,
	                    y,
	                    w,
	                    h,
	                    plot_area.get_labels(),
	                    plot_area.get_time_id(),
	                    UI::SliderStyle::kWuiLight,
	                    tooltip_text,
	                    cursor_size,
	                    enabled),
	     plot_area_(plot_area),
	     last_game_time_id_(0) {
	}

protected:
	void draw(RenderTarget& dst) override;

private:
	WuiPlotArea& plot_area_;
	uint32_t last_game_time_id_;
};

/**
 * A Plot Area is a simple 2D Plot, with the
 * X Axis as time (actually Minus Time)
 * and the Y Axis as the difference between two data vectors
 */
struct DifferentialPlotArea : public WuiPlotArea {
public:
	DifferentialPlotArea(UI::Panel* parent,
	                     int32_t x,
	                     int32_t y,
	                     int32_t w,
	                     int32_t h,
	                     uint32_t sample_rate,
	                     Plotmode plotmode);

	void draw(RenderTarget&) override;

	void register_negative_plot_data(unsigned id, const std::vector<uint32_t>* data);

protected:
	/// Recalculates the data
	void update() override;

private:
	/**
	 * For the negative plotdata, only the values matter. The color and
	 * visibility is determined by the normal plotdata.
	 * We don't need relative data to fill - this is also done in the
	 * normal plotdata
	 */
	struct ReducedPlotData {
		const std::vector<uint32_t>* absolute_data;
	};
	std::map<unsigned, ReducedPlotData> negative_plotdata_;
};

#endif  // end of include guard: WL_WUI_PLOT_AREA_H
