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

#include "general_statistics_menu.h"

#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "logic/player.h"
#include "graphic/rendertarget.h"
#include "rgbcolor.h"
#include "tribe.h"
#include "warelist.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"

using namespace Widelands;

#define PLOT_HEIGHT 130
#define NR_DIFFERENT_DATASETS 11

/*
===============
General_Statistics_Menu::General_Statistics_Menu

Create all the buttons etc...
===============
*/
General_Statistics_Menu::General_Statistics_Menu
	(Interactive_GameBase & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent, &registry, 440, 400, _("General Statistics")),
m_plot          (this, 5, 5, 430, PLOT_HEIGHT)
{
	uint32_t const spacing =  5;
	Point          pos       (spacing, spacing);

	//  plotter
	m_plot.set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot.set_plotmode(WUIPlot_Area::PLOTMODE_ABSOLUTE);
	pos.y += PLOT_HEIGHT + spacing + spacing;

	const Game & game = *parent.get_game();
	const Game::General_Stats_vector & genstats =
		game.get_general_statistics();
	const Game::General_Stats_vector::size_type
		general_statistics_size = genstats.size();
	for
		(Game::General_Stats_vector::size_type i = 0;
		 i < general_statistics_size;
		 ++i)
	{
		const uint8_t * colors = g_playercolors[i];
		const RGBColor color(colors[9], colors[10], colors[11]);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  0, &genstats[i].land_size,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  1, &genstats[i].nr_workers,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  2, &genstats[i].nr_buildings,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  3, &genstats[i].nr_wares,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  4, &genstats[i].productivity,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  5, &genstats[i].nr_casualties,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  6, &genstats[i].nr_kills,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  7, &genstats[i].nr_msites_lost,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  8, &genstats[i].nr_msites_defeated,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS +  9, &genstats[i].nr_civil_blds_lost,
			 color);
		m_plot.register_plot_data
			(i * NR_DIFFERENT_DATASETS + 10, &genstats[i].miltary_strength,
			 color);
		if (game.get_player(i + 1)) // Show area plot
			m_plot.show_plot(i * NR_DIFFERENT_DATASETS, 1);
	}


	uint32_t plr_in_game = 0;
	Player_Number const nr_players = game.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) ++plr_in_game;

	pos.x = spacing;
	int32_t button_size =
		(get_inner_w() - (spacing * (plr_in_game + 1))) / plr_in_game;
	iterate_players_existing_const(p, nr_players, game, player) {
		char buffer[36];
		snprintf(buffer, sizeof(buffer), "pics/genstats_enable_plr_%02u.png", p);
		UI::Checkbox & cb =
			*new UI::Checkbox
				(this, pos, g_gr->get_picture(PicMod_Game, buffer));
		cb.set_size(button_size, 25);
		cb.set_id(p);
		cb.set_state(1);
		cb.changedtoid.set(this, &General_Statistics_Menu::cb_changed_to);
		m_cbs[p - 1] = &cb;
		pos.x += button_size + spacing;
	} else //  player nr p does not exist
		m_cbs[p - 1] = 0;

	pos.x  = spacing;
	pos.y += 25 + spacing + spacing;

	button_size =
		(get_inner_w() - spacing * (NR_DIFFERENT_DATASETS + 1))
		/
		NR_DIFFERENT_DATASETS;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_landsize.png"),
		 _("Land"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrworkers.png"),
		 _("Workers"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrbuildings.png"),
		 _("Buildings"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_nrwares.png"),
		 _("Wares"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_productivity.png"),
		 _("Productivity"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_casualties.png"),
		 _("Casualties"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_kills.png"),
		 _("Kills"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_msites_lost.png"),
		 _("Military buildings lost"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_msites_defeated.png"),
		 _("Military buildings defeated"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_civil_blds_lost.png"),
		 _("Civilian buildings lost"));
	pos.x += button_size + spacing;
	m_radiogroup.add_button
		(this,
		 pos,
		 g_gr->get_picture(PicMod_Game, "pics/genstats_militarystrength.png"),
		 _("Military"));
	m_radiogroup.set_state(0);
	m_selected_information = 0;
	m_radiogroup.changedto.set
		(this, &General_Statistics_Menu::radiogroup_changed);
	pos.y += 25;


	//  time buttons
	button_size = (get_inner_w() - spacing * 5) / 4;
	pos.x = spacing;
	pos.y += spacing + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_15_MINS,
		 _("15 m"));

	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_30_MINS,
		 _("30 m"));

	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_ONE_HOUR,
		 _("1 h"));

	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_TWO_HOURS,
		 _("2 h"));

	pos.y += 25 + spacing;
	pos.x = spacing;

	new UI::Callback_Button<General_Statistics_Menu>
		(this,
		 pos.x, pos.y, 32, 32,
		 4,
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &General_Statistics_Menu::clicked_help, *this,
		 _("Help"));

	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_FOUR_HOURS,
		 _("4 h"));
	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_EIGHT_HOURS,
		 _("8 h"));

	pos.x += button_size + spacing;

	new UI::Callback_IDButton<WUIPlot_Area, WUIPlot_Area::TIME>
		(this,
		 pos.x, pos.y, button_size, 25,
		 4,
		 &WUIPlot_Area::set_time, m_plot, WUIPlot_Area::TIME_16_HOURS,
		 _("16 h"));

	pos.x += button_size + spacing;
	pos.y += 32 + spacing;

	set_inner_size(get_inner_w(), pos.y);
}


/**
 * called when the help button was clicked
 * \todo Implement help
*/
void General_Statistics_Menu::clicked_help() {}


/*
 * Cb has been changed to this state
 */
void General_Statistics_Menu::cb_changed_to(int32_t const id, bool const what)
{
	// This represents our player number
	m_plot.show_plot
		((id - 1) * NR_DIFFERENT_DATASETS + m_selected_information, what);
}

/*
 * The radiogroup has changed
 */
void General_Statistics_Menu::radiogroup_changed(int32_t const id) {
	size_t const statistics_size =
		dynamic_cast<Interactive_GameBase &>(*get_parent()).game()
		.get_general_statistics().size();
	for (uint32_t i = 0; i < statistics_size; ++i)
		if (m_cbs[i]) {
			m_plot.show_plot
				(i * NR_DIFFERENT_DATASETS + id, m_cbs[i]->get_state());
			m_plot.show_plot
				(i * NR_DIFFERENT_DATASETS + m_selected_information, false);
		}
	m_selected_information = id;
};
