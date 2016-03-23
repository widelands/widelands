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

#ifndef WL_WUI_PLOT_AREA_H
#define WL_WUI_PLOT_AREA_H

#include <vector>

#include <boost/bind.hpp>

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
	enum PLOTMODE {
		//  Always take the samples of some times together, so that the graph is
		//  not completely zigg-zagged.
		PLOTMODE_RELATIVE,

		PLOTMODE_ABSOLUTE
	};

	WuiPlotArea
		(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);

	/// Calls update() if needed
	void think() override;

	void draw(RenderTarget &) override;

	void set_time(TIME id) {
		time_ = id;
		needs_update_ = true;
	}

	void set_time_id(int32_t time) {
		if (time == game_time_id_)
			set_time(TIME_GAME);
		else
			set_time(static_cast<TIME>(time));
		needs_update_ = true;
	}
	TIME get_time() {return static_cast<TIME>(time_); }
	int32_t get_time_id() {
		if (time_ == TIME_GAME)
			return game_time_id_;
		else
			return time_;
	}
	void set_sample_rate(uint32_t id); // in milliseconds

	int32_t get_game_time_id();

	void register_plot_data
		(uint32_t id, const std::vector<uint32_t> * data, RGBColor);
	void show_plot(uint32_t id, bool t);

	void set_plotmode(int32_t id) {plotmode_ = id;}

	void set_plotcolor(uint32_t id, RGBColor color);

	std::vector<std::string> get_labels();

protected:
	void draw_plot_line
		(RenderTarget & dst, std::vector<uint32_t> const * dataset, float const yline_length,
		 uint32_t const highest_scale, float const sub, RGBColor const color, int32_t yoffset);
	uint32_t get_plot_time();
	/// Recalculates the data
	virtual void update();

	struct PlotData {
		const std::vector<uint32_t> * dataset;
		bool                          showplot;
		RGBColor                      plotcolor;
	};
	std::vector<PlotData> plotdata_;

	int32_t                 plotmode_;
	int32_t                 sample_rate_;

	/// Whether there has ben a data update since the last time that think() was executed
	bool needs_update_;
	/// The last time the information in this Panel got updated
	uint32_t lastupdate_;

	/// For first updating and then plotting the data
	uint32_t time_ms_;
	uint32_t highest_scale_;
	float sub_;

private:
	uint32_t get_game_time();

	TIME                    time_;  // How much do you want to list
	int32_t                 game_time_id_; // what label is used for TIME_GAME
};

/**
 * A discrete slider with plot time steps preconfigured and automatic signal
 * setup.
 */
struct WuiPlotAreaSlider : public UI::DiscreteSlider {
	WuiPlotAreaSlider
		(Panel * const parent,
		 WuiPlotArea & plot_area,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const Image* background_picture_id,
		 const std::string & tooltip_text = std::string(),
		 const uint32_t cursor_size = 20,
		 const bool enabled = true)
	: DiscreteSlider
		(parent,
		 x, y, w, h,
		 plot_area.get_labels(),
		 plot_area.get_time_id(),
		 background_picture_id,
		 tooltip_text,
		 cursor_size,
		 enabled),
	  plot_area_(plot_area),
	  last_game_time_id_(plot_area.get_game_time_id())
	{
		changedto.connect(boost::bind(&WuiPlotArea::set_time_id, &plot_area, _1));
	}

protected:
	void draw(RenderTarget & dst) override;

private:
	WuiPlotArea & plot_area_;
	int32_t last_game_time_id_;
};

/**
 * A Plot Area is a simple 2D Plot, with the
 * X Axis as time (actually Minus Time)
 * and the Y Axis as the difference between two data vectors
 */
struct DifferentialPlotArea : public WuiPlotArea {
public:
	DifferentialPlotArea
		(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);

	void draw(RenderTarget &) override;

	void register_negative_plot_data
		(uint32_t id, const std::vector<uint32_t> * data);

protected:
	/// Recalculates the data
	void update() override;

private:

	/**
	 * for the negative plotdata only the values matter. The color and
	 * visibility is determined by the normal plotdata.
	 */
	struct ReducedPlotData {
		const std::vector<uint32_t> * dataset;
	};
	std::vector<ReducedPlotData>  negative_plotdata_;
};


#endif  // end of include guard: WL_WUI_PLOT_AREA_H
