/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef WUI_DIFFERENTIAL_PLOT_AREA_H
#define WUI_DIFFERENTIAL_PLOT_AREA_H

#include "plot_area.h"

/**
 * A Plot Area is a simple 2D Plot, with the
 * X Axis as time (actually Minus Time)
 * and the Y Axis as the difference between two data vectors
 */
struct DifferentialPlot_Area : public WUIPlot_Area {
public:
	DifferentialPlot_Area
		(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);

	virtual void draw(RenderTarget &);

	void register_negative_plot_data
		(uint32_t id, const std::vector<uint32_t> * data);

private:
	/**
	 * for the negative plotdata only the values matter.
	 * The color and visibillity is determined by the normal
	 * plotdata
	 */
	struct __reduced_plotdata {
		const std::vector<uint32_t> * dataset;
	};
	std::vector<__reduced_plotdata>  m_negative_plotdata;
};

#endif
