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

#include "differential_plot_area.h"
#include "graphic/rendertarget.h"

#define ZERO_LINE_COLOR RGBColor(255, 255, 255)

DifferentialPlot_Area::DifferentialPlot_Area
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
:
WUIPlot_Area (parent, x, y, w, h)
{}

void DifferentialPlot_Area::draw(RenderTarget & dst) {
	float const xline_length = get_inner_w() - space_at_right  - spacing;
	float const yline_length = get_inner_h() - space_at_bottom - spacing;
	//yoffset of the zero line
	float const yoffset = spacing + ((get_inner_h() - space_at_bottom) - spacing) / 2;

	uint32_t time_in_ms_ = draw_diagram(dst, xline_length, yline_length);

	//draw zero line
	dst.draw_line
		(get_inner_w() - space_at_right,
		 yoffset,
		 get_inner_w() - space_at_right - xline_length,
		 yoffset,
		 ZERO_LINE_COLOR);

	// How many do we take together when relative ploting
	const int32_t how_many = calc_how_many(time_in_ms_);

	//find max and min value
	int32_t max = 0;
	int32_t min = 0;

	if (m_plotmode == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < m_plotdata.size(); ++i)
			if (m_plotdata[i].showplot) {
				for (uint32_t l = 0; l < m_plotdata[i].dataset->size(); ++l) {
					int32_t temp = (*m_plotdata[i].dataset)[l] -
								   (*m_negative_plotdata[i].dataset)[l];
					if (max < temp) max = temp;
					if (min > temp) min = temp;
				}
			}
	} else {
		for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot)
			if (m_plotdata[plot].showplot) {

				std::vector<uint32_t> const & dataset = *m_plotdata[plot].dataset;
				std::vector<uint32_t> const & ndataset = *m_negative_plotdata[plot].dataset;

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
	char buffer[200];

	sprintf(buffer, "%u", highest_scale);
	draw_value
		(dst, buffer, RGBColor(60, 125, 0),
		 Point(get_inner_w() - space_at_right - 2, spacing + 2));

	sprintf(buffer, "-%u", highest_scale);
	draw_value
		(dst, buffer, RGBColor(125, 0, 0),
		 Point(get_inner_w() - space_at_right - 2, get_inner_h() - spacing - 15));

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
			std::vector<uint32_t> const * ndataset = m_negative_plotdata[plot].dataset;

			std::vector<uint32_t> m_data;
			if (m_plotmode == PLOTMODE_RELATIVE) {
				int32_t add = 0;
				// Relative data, first entry is always zero
				m_data.push_back(0);
				for (uint32_t i = 0; i < dataset->size(); ++i) {
					add += (*dataset)[i] - (*ndataset)[i];
					if (0 == ((i + 1) % how_many)) {
						m_data.push_back(add);
						add = 0;
					}
				}

				dataset = &m_data;
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
void DifferentialPlot_Area::register_negative_plot_data
	(uint32_t const id, std::vector<uint32_t> const * const data) {

	if (id >= m_negative_plotdata.size())
		m_negative_plotdata.resize(id + 1);

	m_negative_plotdata[id].dataset   = data;
}
