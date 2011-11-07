/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WARE_STATISTICS_MENU_H
#define WARE_STATISTICS_MENU_H

#include "ui_basic/unique_window.h"
#include "plot_area.h"
#include "differential_plot_area.h"

struct Interactive_Player;
struct WUIPlot_Area;

struct Ware_Statistics_Menu : public UI::UniqueWindow {
public:
	Ware_Statistics_Menu(Interactive_Player &, UI::UniqueWindow::Registry &);
	void set_time(int32_t);

private:
	Interactive_Player * m_parent;
	WUIPlot_Area       * m_plot_production;
	WUIPlot_Area       * m_plot_consumption;
	DifferentialPlot_Area       * m_plot_economy;

	void clicked_help();
	void cb_changed_to(Widelands::Ware_Index, bool);
};



/**
 * A discrete slider with plot time steps preconfigured, automatic signal
 * setup and the set_time callback function from Ware_Statistics_Menu.
 *
 */
struct WUIPlot_Generic_Area_Slider : public UI::DiscreteSlider {
	WUIPlot_Generic_Area_Slider
		(Panel * const parent,
		 WUIPlot_Area & plot_area,
		 Ware_Statistics_Menu * signal_listener,
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
		 enabled)
	{
		changedto->set(signal_listener, &Ware_Statistics_Menu::set_time);
	}
};

#endif
