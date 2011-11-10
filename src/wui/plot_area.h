/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#ifndef WUI_PLOT_AREA_H
#define WUI_PLOT_AREA_H

#include "ui_basic/panel.h"
#include "ui_basic/slider.h"

#include "rgbcolor.h"

#include <vector>

/*
 * A Plot Area is a simple 2D Plot, with the
 * X Axis as time (actually Minus Time)
 * and the Y Axis some Data
 */
struct WUIPlot_Area : public UI::Panel {
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
	enum UNIT {
		UNIT_MIN,
		UNIT_HOUR,
		UNIT_DAY,
	};
	enum PLOTMODE {
		//  Always take the samples of some times together, so that the graph is
		//  not completely zigg-zagged.
		PLOTMODE_RELATIVE,

		PLOTMODE_ABSOLUTE
	};

	WUIPlot_Area
		(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);

	virtual void draw(RenderTarget &);

	void set_time(TIME id) {
		m_time = id;
	}

	void set_time_id(int32_t time) {
		if (time == m_game_time_id)
			set_time(TIME_GAME);
		else
			set_time(static_cast<TIME>(time));
	};
	TIME get_time() {return static_cast<TIME>(m_time); };
	int32_t get_time_id() {
		if (m_time == TIME_GAME)
			return m_game_time_id;
		else
			return m_time;
	};
	void set_sample_rate(uint32_t id); // in milliseconds
	
	int32_t get_game_time_id();

	void register_plot_data
		(uint32_t id, const std::vector<uint32_t> * data, RGBColor);
	void show_plot(uint32_t id, bool t);

	void set_plotmode(int32_t id) {m_plotmode = id;}

	std::vector<std::string> get_labels();

protected:
	uint32_t draw_diagram
		(RenderTarget & dst, float const xline_length, float const yline_length);
	void draw_value
		(RenderTarget & dst, const char * value, RGBColor color, Point pos);
	void draw_plot_line
		(RenderTarget & dst, std::vector<uint32_t> const * dataset, float const yline_length,
		 uint32_t const highest_scale, float const sub, RGBColor const color, int32_t yoffset);

	int32_t calc_how_many(uint32_t time_in_ms_);

	int32_t const spacing;
	int32_t const space_at_bottom;
	int32_t const space_at_right;

	static const uint32_t time_in_ms[];
	static const uint32_t nr_samples = 30;   // How many samples per diagramm when relative plotting

	struct __plotdata {
		const std::vector<uint32_t> * dataset;
		bool                          showplot;
		RGBColor                      plotcolor;
	};
	std::vector<__plotdata> m_plotdata;

	TIME                    m_time;  // How much do you want to list
	int32_t                 m_sample_rate;
	int32_t                 m_plotmode;

private:
	uint32_t get_game_time();
	uint32_t get_plot_time();
	void calc_game_time_id();
	UNIT get_suggested_unit(uint32_t game_time);
	std::string get_unit_name(UNIT unit);
	uint32_t ms_to_unit(UNIT unit, uint32_t ms);
	int32_t                 m_game_time_id; // what label is used for TIME_GAME
};

/**
 * A discrete slider with plot time steps preconfigured and automatic signal
 * setup.
 */
struct WUIPlot_Area_Slider : public UI::DiscreteSlider {
	WUIPlot_Area_Slider
		(Panel * const parent,
		 WUIPlot_Area & plot_area,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const PictureID background_picture_id,
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
	  m_plot_area(plot_area),
	  m_last_game_time_id(plot_area.get_game_time_id())
	{
		changedto->set(&plot_area, &WUIPlot_Area::set_time_id);
	}

protected:
	void draw(RenderTarget & dst);

private:
	WUIPlot_Area & m_plot_area;
	int32_t m_last_game_time_id;
};

#endif
