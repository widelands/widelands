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

#include "ui_panel.h"

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
	};
	enum PLOTMODE {
		//  Always take the samples of some times together, so that the graph is
		//  not completely zigg-zagged.
		PLOTMODE_RELATIVE,

		PLOTMODE_ABSOLUTE
	};

	WUIPlot_Area(UI::Panel* parent, int32_t x, int32_t y, int32_t w, int32_t h);
	~WUIPlot_Area();

	virtual void draw(RenderTarget &);

	void set_time(TIME);
	void set_sample_rate(uint32_t id); // in milliseconds

	void register_plot_data
		(uint32_t id, const std::vector<uint32_t> * data, RGBColor);
	void show_plot(uint32_t id, bool t);

	void set_plotmode(int32_t id) {m_plotmode = id;}

private:
	struct __plotdata {
		const std::vector<uint32_t> * dataset;
		bool                          showplot;
		RGBColor                      plotcolor;
	};
	std::vector<__plotdata> m_plotdata;
	int32_t                 m_time;  // How much do you want to list
	int32_t                 m_sample_rate;
	int32_t                 m_plotmode;
};

#endif
