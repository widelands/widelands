/*
 * Copyright (C) 2002-2004, 2007 by the Widelands Development Team
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

#ifndef __S__WUI_PLOT_AREA__H
#define __S__WUI_PLOT_AREA__H

#include "rgbcolor.h"
#include "ui_panel.h"

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
         PLOTMODE_RELATIVE, // Always take the samples of some times together,
                            // so that the graph is not completly zigg-zagged
         PLOTMODE_ABSOLUTE
		};

      WUIPlot_Area(UI::Panel* parent, int x, int y, int w, int h);
      ~WUIPlot_Area();

      // Functions from the User Interface
      virtual void draw(RenderTarget* dst);

      // Set the time
	void set_time(TIME);
      void set_sample_rate(uint id); // in milliseconds

      // set what to plot
      void register_plot_data(uint id, const std::vector<uint>* data, RGBColor clr);
      void show_plot(uint id, bool t);

      // set plotmode
      void set_plotmode(int id) {m_plotmode = id;}

private:
      struct __plotdata {
         const std::vector<uint>* dataset;
         bool  showplot;
         RGBColor plotcolor;
		};
      std::vector<__plotdata> m_plotdata;
      int m_time;  // How much do you want to list
      int m_sample_rate;
      int m_plotmode;
};

#endif
