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

#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "tribe.h"
#include "warelist.h"
#include "wui_plot_area.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_textarea.h"


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

/*
 * This class is only needed here, that's
 * why it is defined here.
 *
 * This class is the same as an ordinary
 * checkbox, the only difference is, it has
 * a small rectangle on it with the color
 * of the graph and it needs a picture
 */
struct WSM_Checkbox : public UI::Checkbox {
	WSM_Checkbox
		(UI::Panel *,
		 int32_t x, int32_t y,
		 int32_t id, uint32_t picid,
		 RGBColor);

	virtual void draw(RenderTarget *t);

private:
	int32_t  m_pic;
	RGBColor m_color;
};


WSM_Checkbox::WSM_Checkbox
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y,
	 int32_t const id, uint32_t const picid,
	 RGBColor const color)
: UI::Checkbox(parent, x, y, g_gr->get_picture(PicMod_Game,  WARES_DISPLAY_BG))
{

	m_pic = picid;
	set_id(id);
	m_color = color;
}

/*
 * draw
 */
void WSM_Checkbox::draw(RenderTarget* dst) {
	//  First, draw normal.
	UI::Checkbox::draw(dst);

	//  Now, draw a small box with the color.
	assert(1 <= get_inner_w());
	compile_assert(2 <= COLOR_BOX_HEIGHT);
	dst->fill_rect
		(Rect(Point(1, 1), get_inner_w() - 1, COLOR_BOX_HEIGHT - 2), m_color);

	//  and the item
	dst->blit
		(Point((get_inner_w() - WARE_MENU_PIC_WIDTH) / 2, COLOR_BOX_HEIGHT),
		 m_pic);
}

/*
===============
Ware_Statistics_Menu::Ware_Statistics_Menu

Create all the buttons etc...
===============
*/
Ware_Statistics_Menu::Ware_Statistics_Menu
	(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent, &registry, 400, 270, _("Ware Statistics")),
m_parent(&parent)
{
	//  First, we must decide about the size.
	const int32_t nr_wares = parent.get_player()->tribe().get_nrwares();
	uint32_t wares_per_row = MIN_WARES_PER_LINE;
	while (nr_wares % wares_per_row && wares_per_row <= MAX_WARES_PER_LINE)
		++wares_per_row;
	const uint32_t nr_rows =
		nr_wares / wares_per_row + (nr_wares % wares_per_row ? 1 : 0);

	int32_t const spacing = 5;
	int32_t       offsx   = spacing;
	int32_t       offsy   = 30;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;


	set_inner_size
		(10,
		 (offsy + spacing + PLOT_HEIGHT + spacing +
		  nr_rows * (WARE_MENU_PIC_HEIGHT + spacing) + 100));


	m_plot = new WUIPlot_Area(this, spacing, offsy+spacing, get_inner_w()-2*spacing, PLOT_HEIGHT);
	m_plot->set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot->set_plotmode(WUIPlot_Area::PLOTMODE_RELATIVE);

	//  all wares
	int32_t cur_ware = 0;
	int32_t dposy    = 0;
	posy += PLOT_HEIGHT+ 2 * spacing;
	Widelands::Tribe_Descr const & tribe = parent.get_player()->tribe();
	for (uint32_t y = 0; y < nr_rows; ++y) {
		posx = spacing;
		for
			(uint32_t x = 0;
			 x < wares_per_row and cur_ware < nr_wares;
			 ++x, ++cur_ware)
		{
			Widelands::Item_Ware_Descr const & ware =
				*tribe.get_ware_descr(cur_ware);
			WSM_Checkbox & cb = *new WSM_Checkbox
				(this, posx, posy, cur_ware, ware.get_icon(), colors[cur_ware]);
			cb.set_tooltip(ware.descname().c_str());
			cb.changedtoid.set(this, &Ware_Statistics_Menu::cb_changed_to);
			posx += cb.get_w() + spacing;
			dposy = cb.get_h() + spacing;
			set_inner_size
				(spacing + (cb.get_w() + spacing) * wares_per_row, get_inner_h());
			m_plot->register_plot_data
				(cur_ware,
				 parent.get_player()->get_ware_production_statistics(cur_ware),
				 colors[cur_ware]);
		}
		posy += dposy;
	}

	m_plot->set_size(get_inner_w()-2*spacing, PLOT_HEIGHT);


	int32_t button_size = (get_inner_w() - spacing * 5) / 4;
	posx = spacing;
	posy +=spacing+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_15_MINS,
		 _("15 m"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_30_MINS,
		 _("30 m"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_ONE_HOUR,
		 _("1 h"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_TWO_HOURS,
		 _("2 h"));

	posy += 25 + spacing;
	posx = spacing;

	new UI::Button<Ware_Statistics_Menu>
		(this,
		 posx, posy, 32, 32,
		 4,
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &Ware_Statistics_Menu::clicked_help, this,
		 _("Help"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_FOUR_HOURS,
		 _("4 h"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_EIGHT_HOURS,
		 _("8 h"));

	posx += button_size+spacing;

	new UI::IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 posx, posy, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_16_HOURS,
		 _("16 h"));

	posx += button_size + spacing;
	posy += 32 + spacing;

	set_inner_size(get_inner_w(), posy);
}

/*
===============

Unregister from the registry pointer
===============
*/
Ware_Statistics_Menu::~Ware_Statistics_Menu() {}

/**
 * Called when the ok button has been clicked
 * \todo Implement help
*/
void Ware_Statistics_Menu::clicked_help() {}

/*
 * Cb has been changed to this state
 */
void Ware_Statistics_Menu::cb_changed_to(int32_t id, bool what) {
	m_plot->show_plot(id, what);
}
