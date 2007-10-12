/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "wui_plot_area.h"

#include "constants.h"
#include "font_loader.h"
#include "font_handler.h"
#include "graphic.h"
#include "rendertarget.h"
#include "rgbcolor.h"

#include "ui_panel.h"

#include <stdio.h>


/*
 * Where to draw tics
 */
static const int32_t how_many_ticks[] = {
   5,  // 15 Mins
   3,  // 30 Mins
   6,  // 1  H
   4,  // 2  H
   4,  // 4  H
   4,  // 8  H
   4,  // 16 H
};

static const int32_t max_x[] = {
   15,
   30,
   60,
   120,
   4,
   8,
   16
};

static const uint32_t time_in_ms[] = {
   15*60*1000,
   30*60*1000,
   1*60*60*1000,
   2*60*60*1000,
   4*60*60*1000,
   8*60*60*1000,
  16*60*60*1000,
};

#define NR_SAMPLES 30   // How many samples per diagramm when relative plotting

#define BG_PIC "pics/plot_area_bg.png"
#define LINE_COLOR RGBColor(0, 0, 0)


WUIPlot_Area::WUIPlot_Area(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h)
:
UI::Panel (parent, x, y, w, h),
m_time    (TIME_ONE_HOUR),
m_plotmode(PLOTMODE_ABSOLUTE)
{}


WUIPlot_Area::~WUIPlot_Area() {}


/*
 * Draw this. This is the main function
 */
void WUIPlot_Area::draw(RenderTarget* dst) {

   // first, tile the background
	dst->tile
		(Rect(Point(0, 0), get_inner_w(), get_inner_h()),
		 g_gr->get_picture(PicMod_Game, BG_PIC), Point(0, 0));

   int32_t spacing = 5;
   int32_t space_at_bottom=15;
   int32_t space_at_right=5;

   float xline_length = get_inner_w()-space_at_right-spacing;
   float yline_length = get_inner_h()-space_at_bottom-spacing;

   // Draw coordinate system
   // X Axis
   dst->draw_line(spacing, get_inner_h()-space_at_bottom, get_inner_w()-space_at_right, get_inner_h()-space_at_bottom, LINE_COLOR);
   // Arrow
   dst->draw_line(spacing, get_inner_h()-space_at_bottom, spacing + 5, get_inner_h()-space_at_bottom-3, LINE_COLOR);
   dst->draw_line(spacing, get_inner_h()-space_at_bottom, spacing + 5, get_inner_h()-space_at_bottom+3, LINE_COLOR);
   // Y Axis
   dst->draw_line(get_inner_w()-space_at_right, spacing, get_inner_w()-space_at_right, get_inner_h()-space_at_bottom, LINE_COLOR);
   // No Arrow here, since this doesn't continue

   // Draw xticks
   float sub = xline_length / how_many_ticks[m_time];
   float posx = get_inner_w()-space_at_right;
   char buffer[200];
	for (size_t i = 0; static_cast<int32_t>(i) <= how_many_ticks[m_time]; ++i) {
		dst->draw_line
			(static_cast<int32_t>(posx), get_inner_h() - space_at_bottom,
			 static_cast<int32_t>(posx), get_inner_h() - space_at_bottom + 3,
			 LINE_COLOR);

		snprintf
			(buffer, sizeof(buffer),
			 "%u", max_x[m_time] / how_many_ticks[m_time] * i);

      int32_t w, h;
      g_fh->get_size(UI_FONT_SMALL, buffer, &w, &h, 0);
		g_fh->draw_string
			(*dst,
			 UI_FONT_SMALL,
			 RGBColor(255, 0, 0), RGBColor(255, 255, 255),
			 Point
			 (static_cast<int32_t>(posx - w / 2),
			  get_inner_h() - space_at_bottom + 4),
			 buffer, Align_CenterLeft, -1,
			 Widget_Cache_None, 0, -1, false);
      posx -= sub;
	}

   // draw yticks, one at full, one at half
   dst->draw_line(get_inner_w()-space_at_right, spacing, get_inner_w()-space_at_right-3, spacing, LINE_COLOR);
   dst->draw_line(get_inner_w()-space_at_right, spacing + ((get_inner_h()-space_at_bottom)-spacing)/2, get_inner_w()-space_at_right-3,
         spacing + ((get_inner_h()-space_at_bottom)-spacing)/2, LINE_COLOR);

   uint32_t max = 0;
   // Find the maximum value
	if (m_plotmode == PLOTMODE_ABSOLUTE)  {
		for (uint32_t i = 0; i < m_plotdata.size(); ++i) {
         if (!m_plotdata[i].showplot) continue;
			for (uint32_t l = 0; l < m_plotdata[i].dataset->size(); ++l)
            if (max < (*m_plotdata[i].dataset)[l])
               max = (*m_plotdata[i].dataset)[l];
		}
	} else {
		for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot) {
         if (!m_plotdata[plot].showplot) continue;

         const std::vector<uint32_t>* dataset = m_plotdata[plot].dataset;

         // How many do we take together
			const int32_t how_many = static_cast<int32_t>
				((static_cast<float>(time_in_ms[m_time])
				  /
				  static_cast<float>(NR_SAMPLES))
				 /
				 static_cast<float>(m_sample_rate));

         uint32_t add = 0;
         // Relative data, first entry is always zero
			for (uint32_t i = 0; i < dataset->size(); ++i) {
            add += (*dataset)[i];
            if (! ((i+1) % how_many)) {
               if (add > max) max = add;
               add = 0;
				}
			}
		}
	}

   // Print the maximal value
   sprintf(buffer, "%i", max);
   int32_t w, h;
   g_fh->get_size(UI_FONT_SMALL, buffer, &w, &h, 0);
   g_fh->draw_string
		(*dst,
		 UI_FONT_SMALL,
		 RGBColor(120, 255, 0), RGBColor(255, 255, 255),
		 Point(get_inner_w() - space_at_right - w - 2, spacing),
		 buffer, Align_CenterLeft, -1,
		 Widget_Cache_None, 0, -1, false);

   // Now, plot the pixels
	sub =
		xline_length
		/
		(static_cast<float>(time_in_ms[m_time])
		 /
		 static_cast<float>(m_sample_rate));
	for (uint32_t plot = 0; plot < m_plotdata.size(); ++plot) {
      if (!m_plotdata[plot].showplot) continue;

      RGBColor color = m_plotdata[plot].plotcolor;
      const std::vector<uint32_t>* dataset = m_plotdata[plot].dataset;

      std::vector<uint32_t> m_data;
		if (m_plotmode == PLOTMODE_RELATIVE) {
         // How many do we take together
			const int32_t how_many = static_cast<int32_t>
				((static_cast<float>(time_in_ms[m_time])
				  /
				  static_cast<float>(NR_SAMPLES))
				 /
				 static_cast<float>(m_sample_rate));

         uint32_t add = 0;
         // Relative data, first entry is always zero
         m_data.push_back(0);
			for (uint32_t i = 0; i < dataset->size(); ++i) {
            add += (*dataset)[i];
            if (! ((i+1) % how_many)) {
               m_data.push_back(add);
               add = 0;
				}
			}

         dataset = &m_data;
			sub = xline_length / static_cast<float>(NR_SAMPLES);
		}

      posx = get_inner_w()-space_at_right;

      int32_t lx = get_inner_w()-space_at_right;
      int32_t ly = get_inner_h()-space_at_bottom;
		for (int32_t i = dataset->size() - 1; i > 0 and posx > spacing; --i) {
         int32_t value = (*dataset)[i];

			int32_t curx = static_cast<int32_t>(posx);
         int32_t cury = get_inner_h()-space_at_bottom;
			if (value) {
				const float length_y =
					yline_length
					/
					(static_cast<float>(max) / static_cast<float>(value));
				cury -= static_cast<int32_t>(length_y);
			}
         dst->draw_line(lx, ly, curx, cury, color);

         posx -= sub;

         lx = curx;
         ly = cury;
		}
	}
}

/*
 * Register a new plot data stream
 */
void WUIPlot_Area::register_plot_data(uint32_t id, const std::vector<uint32_t>* data, RGBColor color) {
   if (id >= m_plotdata.size()) {
      m_plotdata.resize(id+1);
	}

   m_plotdata[id].dataset = data;
   m_plotdata[id].showplot = false;
   m_plotdata[id].plotcolor = color;
}

/*
 * Show this plot data?
 */
void WUIPlot_Area::show_plot(uint32_t id, bool t) {
   assert(id < m_plotdata.size());
   m_plotdata[id].showplot = t;
};

/*
 * set time
 */
void WUIPlot_Area::set_time(TIME id) {m_time = id;}

/*
 * Set sample rate the data uses
 */
void WUIPlot_Area::set_sample_rate(uint32_t id) {
   m_sample_rate = id;
}
