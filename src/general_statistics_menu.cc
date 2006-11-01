/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "general_statistics_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_radiobutton.h"
#include "ui_textarea.h"
#include "ware.h"
#include "wui_plot_area.h"

#define PLOT_HEIGHT 100
#define NR_DIFFERENT_DATASETS 7

enum {
   ID_LANDSIZE =  0,
   ID_NR_WORKERS,
   ID_NR_BUILDINGS,
   ID_NR_WARES,
   ID_PRODUCTIVITY,
   ID_KILLS,
   ID_MILITARY_STRENGTH
};

/*
===============
General_Statistics_Menu::General_Statistics_Menu

Create all the buttons etc...
===============
*/
General_Statistics_Menu::General_Statistics_Menu
(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent,&registry, 400, 400, _("General Statistics")),
m_parent(&parent)
{

   uint offsy = 35;
   uint spacing = 5;
   uint posx = spacing;
   uint posy = offsy;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("General Statistics"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // Plotter
   m_plot = new WUIPlot_Area(this, spacing, offsy+spacing, get_inner_w()-2*spacing, PLOT_HEIGHT);
   m_plot->set_sample_rate(STATISTICS_SAMPLE_TIME);
   m_plot->set_plotmode(WUIPlot_Area::PLOTMODE_ABSOLUTE);
   posy+=PLOT_HEIGHT+spacing+spacing;

	const Game & game = *parent.get_game();
	const Interactive_Player::General_Stats_vector & genstats =
		parent.get_general_statistics();
	const Interactive_Player::General_Stats_vector::size_type
		general_statistics_size = genstats.size();
	for
		(Interactive_Player::General_Stats_vector::size_type i = 0;
		 i < general_statistics_size;
		 ++i)
	{
		const uchar * colors = g_playercolors[i];
		const RGBColor color(colors[9], colors[10], colors[11]);
		m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 0, &genstats[i].land_size,        color);
      m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 1, &genstats[i].nr_workers,       color);
		m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 2, &genstats[i].nr_buildings,     color);
      m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 3, &genstats[i].nr_wares,         color);
		m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 4, &genstats[i].productivity,     color);
		m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 5, &genstats[i].nr_kills,         color);
		m_plot->register_plot_data
			(i * NR_DIFFERENT_DATASETS + 6, &genstats[i].miltary_strength, color);
		if (game.get_player(i + 1)) // Show area plot
         m_plot->show_plot(i*NR_DIFFERENT_DATASETS, 1);
   }


   // Buttons
   uint plr_in_game = 0;
	const uint nr_players = game.get_map().get_nrplayers();
	for (uint i = 1; i <= nr_players; ++i)
		if (game.get_player(i)) ++plr_in_game;

   posx = spacing;
   int button_size = ( get_inner_w()- ( spacing* (plr_in_game+1) ) ) / plr_in_game;
	for (uint i = 1; i <= nr_players; ++i) {
      m_cbs[i] = 0;
		if (not game.get_player(i)) continue;

      char buffer[1024];
      sprintf(buffer, "pics/genstats_enable_plr_%02i.png", i);
      UI::Checkbox* cb = new UI::Checkbox(this, posx, posy, g_gr->get_picture( PicMod_Game,  buffer ));
      cb->set_size(button_size, 25);
      cb->set_id(i);
      cb->set_state(1);
      cb->changedtoid.set(this, &General_Statistics_Menu::cb_changed_to);
      m_cbs[i-1] = cb;
      posx+= button_size + spacing;
   }

   posx = spacing;
   posy += 25+spacing+spacing;

   // Below, Radiobuttons for what to display
   m_radiogroup = new UI::Radiogroup();
   button_size = ( get_inner_w()-(spacing*8) ) / 7;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_landsize.png"),
		 _("Land").c_str());
   posx+=button_size + spacing;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrworkers.png"),
		 _("Workers").c_str());
   posx+=button_size + spacing;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrbuildings.png"),
		 _("Buildings").c_str());
   posx+=button_size + spacing;
	m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrwares.png"),
		 _("Wares").c_str());
   posx+=button_size + spacing;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_productivity.png"),
		 _("Productivity").c_str());
   posx+=button_size + spacing;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_kills.png"),
		 _("Kills").c_str());
   posx+=button_size + spacing;
   m_radiogroup->add_button
		(this,
		 posx, posy,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_militarystrength.png"),
		 _("Military").c_str());
   m_radiogroup->set_state(0);
   m_selected_information = 0;
   m_radiogroup->changedto.set(this, &General_Statistics_Menu::radiogroup_changed);
   posy += 25;


   // Below, time buttons
   button_size = ( get_inner_w()-(spacing*5) ) / 4;
   posx = spacing;
   posy +=spacing+spacing;
   UI::Button *b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_15_MINS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("15 m").c_str());
   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_30_MINS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("30 m").c_str());
   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_ONE_HOUR);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("1 h").c_str());
   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_TWO_HOURS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("2 h").c_str());

   posy += 25 + spacing;
   posx = spacing;
	b = new UI::Button(this, posx, posy, 32, 32, 4, 100);
	b->clickedid.set(this, &General_Statistics_Menu::clicked);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_help.png"));
	b->set_tooltip(_("Help").c_str());

   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_FOUR_HOURS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("4 h").c_str());
   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_EIGHT_HOURS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("8 h").c_str());
   posx += button_size+spacing;
   b = new UI::Button(this, posx, posy, button_size, 25, 4, WUIPlot_Area::TIME_16_HOURS);
   b->clickedid.set(this, &General_Statistics_Menu::clicked);
   b->set_title(_("16 h").c_str());
   posx += button_size+spacing;
   posy += 32+spacing;

   set_inner_size(get_inner_w(), posy);
}

/*
===============
General_Statistics_Menu::~General_Statistics_Menu

Unregister from the registry pointer
===============
*/
General_Statistics_Menu::~General_Statistics_Menu()
{
   delete m_radiogroup;
}

/*
===========
called when the help button was clicked
===========
*/
void General_Statistics_Menu::clicked(int id) {
   if(id == 100) {
      log("TODO: help not implemented\n");
   } else
      m_plot->set_time(id);

}

/*
 * Cb has been changed to this state
 */
void General_Statistics_Menu::cb_changed_to(int id, bool what) {
   // This represents our player number
   m_plot->show_plot( (id-1)* NR_DIFFERENT_DATASETS + m_selected_information, what);
}

/*
 * The radiogroup has changed
 */
void General_Statistics_Menu::radiogroup_changed(int id) {
   for(uint i = 0; i < m_parent->get_general_statistics().size(); i++) {
      if(!m_cbs[i]) continue;

      m_plot->show_plot( i* NR_DIFFERENT_DATASETS + id, m_cbs[i]->get_state());
      m_plot->show_plot( i* NR_DIFFERENT_DATASETS + m_selected_information, false);
   }
   m_selected_information = id;
};
