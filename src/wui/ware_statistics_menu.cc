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

#include "ware_statistics_menu.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "logic/player.h"
#include "graphic/rendertarget.h"
#include "logic/tribe.h"
#include "logic/warelist.h"
#include "plot_area.h"
#include "waresdisplay.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/slider.h"

#define WARES_DISPLAY_BG "pics/ware_list_bg.png"

#define MIN_WARES_PER_LINE 7
#define MAX_WARES_PER_LINE 11


#define PLOT_HEIGHT 100

#define COLOR_BOX_HEIGHT 7

static const RGBColor colors[] = {
	RGBColor  (0, 210, 254),
	RGBColor(255, 157,  74),
	RGBColor(222,  97,  98),
	RGBColor(115,  82, 157),
	RGBColor(172, 113, 205),
	RGBColor(156, 117, 189),
	RGBColor(156, 165, 172),
	RGBColor (16,  56, 164),
	RGBColor(115,  68,  32),
	RGBColor(172,   0,   0),
	RGBColor  (0,  85,  16),
	RGBColor (98,  36,  90),
	RGBColor (90,  64, 115),
	RGBColor (32,  36,  32),
	RGBColor(153, 153, 153),
	RGBColor(102, 102, 102),
	RGBColor (51,  51,  51),
	RGBColor(255, 204,   0),
	RGBColor(255, 153,   0),
	RGBColor(255, 102,   0),
	RGBColor(255,  51,   0),
	RGBColor (51,  51,  51),
	RGBColor(102, 102, 102),
	RGBColor(153, 153, 153),
	RGBColor(204, 204, 204),
	RGBColor(255, 255, 255),
	RGBColor(153, 204,   0),
	RGBColor(204, 153,   0),
	RGBColor(255, 204,  51),
	RGBColor(255, 204, 102),
	RGBColor(255, 153, 102),
	RGBColor(255, 102,  51),
	RGBColor(204,  51,   0),
	RGBColor(204,   0,  51),
	RGBColor(204, 255,   0),
	RGBColor(204, 255,  51),
	RGBColor (51,  51,   0),
	RGBColor(102, 102,   0),
	RGBColor(153, 153,   0),
	RGBColor(204, 204,   0),
	RGBColor(255, 255,   0),
	RGBColor(204, 153,  51),
	RGBColor(204, 102,  51),
	RGBColor (51,   0,   0),
	RGBColor(102,   0,   0),
	RGBColor(153,   0,   0),
	RGBColor(204,   0,   0),
	RGBColor(255,   0,   0),
	RGBColor(255,  51, 102),
	RGBColor(255,   0,  51),
	RGBColor(153, 255,   0),
	RGBColor(204, 255, 102),
	RGBColor(153, 204,  51),
	RGBColor(102, 102,  51),
	RGBColor(153, 153,  51),
	RGBColor(204, 204,  51),
	RGBColor(255, 255,  51),
	RGBColor(153, 102,   0),
	RGBColor(153,  51,   0),
	RGBColor(102,  51,  51),
	RGBColor(153,  51,  51),
	RGBColor(204,  51,  51),
	RGBColor(255,  51,  51),
	RGBColor(204,  51, 102),
	RGBColor(255, 102, 153),
	RGBColor(255,   0, 102),
	RGBColor(102, 255,   0),
	RGBColor(153, 255, 102),
	RGBColor(102, 204,  51),
	RGBColor(102, 153,   0),
	RGBColor(153, 153, 102),
	RGBColor(204, 204, 102),
	RGBColor(255, 255, 102),
	RGBColor(153, 102,  51),
	RGBColor(102,  51,   0),
	RGBColor(153, 102, 102),
	RGBColor(204, 102, 102),
	RGBColor(255, 102, 102),
	RGBColor(153,   0,  51),
	RGBColor(204,  51, 153),
	RGBColor(255, 102, 204),
	RGBColor(255,   0, 153),
	RGBColor (51, 255,   0),
	RGBColor(102, 255,  51),
	RGBColor (51, 153,   0),
	RGBColor(102, 204,   0),
	RGBColor(153, 255,  51),
	RGBColor(204, 204, 153),
	RGBColor(255, 255, 153),
	RGBColor(204, 153, 102),
	RGBColor(204, 102,   0),
	RGBColor(204, 153, 153),
	RGBColor(255, 153, 153),
	RGBColor(255,  51, 153),
	RGBColor(204,   0, 102),
	RGBColor(153,   0, 102),
	RGBColor(255,  51, 204),
	RGBColor(255,   0, 204),
	RGBColor  (0, 204,   0),
	RGBColor (51, 204,   0),
	RGBColor (51, 102,   0),
	RGBColor(102, 153,  51),
	RGBColor(153, 204, 102),
	RGBColor(204, 255, 153),
	RGBColor(255, 255, 204),
	RGBColor(255, 204, 153),
	RGBColor(255, 153,  51),
	RGBColor(255, 204, 204),
	RGBColor(255, 153, 204),
	RGBColor(204, 102, 153),
	RGBColor(153,  51, 102),
	RGBColor(102,   0,  51),
	RGBColor(204,   0, 153),
	RGBColor (51,   0,  51),
	RGBColor (51, 204,  51),
	RGBColor(102, 204, 102),
	RGBColor  (0, 255,   0),
	RGBColor (51, 255,  51),
	RGBColor(102, 255, 102),
	RGBColor(153, 255, 153),
	RGBColor(204, 255, 204),
	RGBColor(204, 153, 204),
	RGBColor(153, 102, 153),
	RGBColor(153,  51, 153),
	RGBColor(153,  0,  153),
	RGBColor(102,  51, 102),
	RGBColor(102,   0, 102),
	RGBColor  (0, 102,   0),
	RGBColor (51, 102,  51),
	RGBColor  (0, 153,   0),
	RGBColor (51, 153,  51),
	RGBColor(102, 153, 102),
	RGBColor(153, 204, 153),
	RGBColor(255, 204, 255),
	RGBColor(255, 153, 255),
	RGBColor(255, 102, 255),
	RGBColor(255,  51, 255),
	RGBColor(255,   0, 255),
	RGBColor(204, 102, 204),
	RGBColor(204,  51, 204),
	RGBColor  (0,  51,   0),
	RGBColor  (0, 204,  51),
	RGBColor  (0, 102,  51),
	RGBColor (51, 153, 102),
	RGBColor(102, 204, 153),
	RGBColor(153, 255, 204),
	RGBColor(204, 255, 255),
	RGBColor (51, 153, 255),
	RGBColor(153, 204, 255),
	RGBColor(204, 204, 255),
	RGBColor(204, 153, 255),
	RGBColor(153, 102, 204),
	RGBColor(102,  51, 153),
	RGBColor (51,   0, 102),
	RGBColor(153,   0, 204),
	RGBColor(204,   0, 204),
	RGBColor  (0, 255,  51),
	RGBColor (51, 255, 102),
	RGBColor  (0, 153,  51),
	RGBColor  (0, 204, 102),
	RGBColor (51, 255, 153),
	RGBColor(153, 255, 255),
	RGBColor(153, 204, 204),
	RGBColor  (0, 102, 204),
	RGBColor(102, 153, 204),
	RGBColor(153, 153, 255),
	RGBColor(153, 153, 204),
	RGBColor(153,  51, 255),
	RGBColor(102,   0, 204),
	RGBColor(102,   0, 153),
	RGBColor(204,  51, 255),
	RGBColor(204,   0, 255),
	RGBColor  (0, 255, 102),
	RGBColor(102, 255, 153),
	RGBColor (51, 204, 102),
	RGBColor  (0, 153, 102),
	RGBColor(102, 255, 255),
	RGBColor(102, 204, 204),
	RGBColor(102, 153, 153),
	RGBColor  (0,  51, 102),
	RGBColor (51, 102, 153),
	RGBColor(102, 102, 255),
	RGBColor(102, 102, 204),
	RGBColor(102, 102, 153),
	RGBColor (51,   0, 153),
	RGBColor(153,  51, 204),
	RGBColor(204, 102, 255),
	RGBColor(153,   0, 255),
	RGBColor  (0, 255, 153),
	RGBColor(102, 255, 204),
	RGBColor (51, 204, 153),
	RGBColor (51, 255, 255),
	RGBColor (51, 204, 204),
	RGBColor (51, 153, 153),
	RGBColor (51, 102, 102),
	RGBColor  (0, 102, 153),
	RGBColor  (0,  51, 153),
	RGBColor (51,  51, 255),
	RGBColor (51,  51, 204),
	RGBColor (51,  51, 153),
	RGBColor (51,  51, 102),
	RGBColor(102,  51, 204),
	RGBColor(153, 102, 255),
	RGBColor(102,   0, 255),
	RGBColor  (0, 255, 204),
	RGBColor (51, 255, 204),
	RGBColor  (0, 255, 255),
	RGBColor  (0, 204, 204),
	RGBColor  (0, 153, 153),
	RGBColor  (0, 102, 102),
	RGBColor  (0,  51,  51),
	RGBColor (51, 153, 204),
	RGBColor (51, 102, 204),
	RGBColor  (0,   0, 255),
	RGBColor  (0,   0, 204),
	RGBColor  (0,   0, 153),
	RGBColor  (0,   0, 102),
	RGBColor  (0,   0,  51),
	RGBColor(102,  51, 255),
	RGBColor (51,   0, 255),
	RGBColor  (0, 204, 153),
	RGBColor  (0, 153, 202),
	RGBColor (51, 204, 255),
	RGBColor(102, 204, 255),
	RGBColor(102, 153, 255),
	RGBColor (51, 102, 255),
	RGBColor  (0,  51, 204),
	RGBColor (51,   0, 204),
	RGBColor(255, 255, 255),
	RGBColor(204, 204, 204),
	RGBColor(153, 153, 153),
	RGBColor(102, 102, 102),
	RGBColor (51,  51,  51),
	RGBColor  (0, 204, 255),
	RGBColor  (0, 153, 255),
	RGBColor  (0, 102, 255),
	RGBColor  (0,  51, 255),
	RGBColor (51,  51,  51),
	RGBColor(102, 102, 102),
	RGBColor(153, 153, 153),
	RGBColor(204, 204, 204),
	RGBColor(255, 255, 255),
};


struct StatisticWaresDisplay : public AbstractWaresDisplay {
	typedef AbstractWaresDisplay::wdType wdType;

	StatisticWaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y,
		 Widelands::Tribe_Descr const & tribe,
		 boost::function<void(Widelands::Ware_Index, bool)> callback_function)
	:
		 AbstractWaresDisplay(parent, x, y, tribe, WaresDisplay::WARE, true, callback_function)
	{
		uint32_t w, h;
		get_desired_size(w, h);
		set_size(w, h);
	}
protected:
	std::string info_for_ware(Widelands::Ware_Index const ware) {
		return "";
	}

	RGBColor info_color_for_ware(Widelands::Ware_Index const ware)
	{
		return colors[static_cast<size_t>(ware)];
	}
};


Ware_Statistics_Menu::Ware_Statistics_Menu
	(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&parent, "ware_statistics", &registry, 400, 270, _("Ware Statistics")),
m_parent(&parent)
{
	set_cache(false);

	UI::Box * box = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, 5);
	box->set_border(5, 5, 5, 5);
	set_center_panel(box);

	m_plot = new WUIPlot_Area
			(box, 0, 0, 100, PLOT_HEIGHT);
	m_plot->set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot->set_plotmode(WUIPlot_Area::PLOTMODE_RELATIVE);

	box->add(m_plot, UI::Box::AlignLeft, true);

	uint8_t const nr_wares = parent.get_player()->tribe().get_nrwares().value();
	for (Widelands::Ware_Index::value_t cur_ware = 0; cur_ware < nr_wares; ++cur_ware) {
		m_plot->register_plot_data
			(cur_ware,
			 parent.get_player()->get_ware_production_statistics
				(Widelands::Ware_Index(cur_ware)),
			 colors[cur_ware]);
	}

	box->add
		(new StatisticWaresDisplay
			(box, 0, 0, parent.get_player()->tribe(),
			 boost::bind(&Ware_Statistics_Menu::cb_changed_to, boost::ref(*this), _1, _2)),
		 UI::Box::AlignLeft, true);


	box->add
		(new WUIPlot_Area_Slider
			(box, *m_plot, 0, 0, 100, 45,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png")),
		UI::Box::AlignLeft, true);
}


/**
 * Called when the ok button has been clicked
 * \todo Implement help
*/
void Ware_Statistics_Menu::clicked_help() {}

/*
 * Cb has been changed to this state
 */
void Ware_Statistics_Menu::cb_changed_to(Widelands::Ware_Index id, bool what) {
	m_plot->show_plot(static_cast<size_t>(id), what);
}

