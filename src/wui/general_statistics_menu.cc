/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/general_statistics_menu.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/warelist.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_player.h"

using namespace Widelands;

#define PLOT_HEIGHT 130
#define NR_BASE_DATASETS 11

General_Statistics_Menu::General_Statistics_Menu
	(Interactive_GameBase & parent, General_Statistics_Menu::Registry & registry)
:
UI::UniqueWindow
	(&parent, "statistics_menu", &registry,
	 440, 400, _("General Statistics")),
m_my_registry      (&registry),
m_box           (this, 0, 0, UI::Box::Vertical, 0, 0, 5),
m_plot          (&m_box, 0, 0, 430, PLOT_HEIGHT),
m_selected_information(0)
{
	assert (m_my_registry);

	m_selected_information = m_my_registry->selected_information;

	set_center_panel(&m_box);
	m_box.set_border(5, 5, 5, 5);

	set_cache(false);

	// Setup plot data
	m_plot.set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot.set_plotmode(WUIPlot_Area::PLOTMODE_ABSOLUTE);
	Game & game = *parent.get_game();
	const Game::General_Stats_vector & genstats =
		game.get_general_statistics();
	const Game::General_Stats_vector::size_type
		general_statistics_size = genstats.size();

	// Is there a hook dataset?
	m_ndatasets = NR_BASE_DATASETS;
	std::unique_ptr<LuaTable> hook = game.lua().get_hook("custom_statistic");
	std::string cs_name, cs_pic;
	if (hook) {
		cs_name = hook->get_string("name");
		cs_pic = hook->get_string("pic");
		m_ndatasets++;
	}

	for
		(Game::General_Stats_vector::size_type i = 0;
		 i < general_statistics_size;
		 ++i)
	{
		const RGBColor & color = Player::Colors[i];
		m_plot.register_plot_data
			(i * m_ndatasets +  0, &genstats[i].land_size,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  1, &genstats[i].nr_workers,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  2, &genstats[i].nr_buildings,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  3, &genstats[i].nr_wares,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  4, &genstats[i].productivity,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  5, &genstats[i].nr_casualties,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  6, &genstats[i].nr_kills,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  7, &genstats[i].nr_msites_lost,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  8, &genstats[i].nr_msites_defeated,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets +  9, &genstats[i].nr_civil_blds_lost,
			 color);
		m_plot.register_plot_data
			(i * m_ndatasets + 10, &genstats[i].miltary_strength,
			 color);
		if (hook) {
			m_plot.register_plot_data
				(i * m_ndatasets + 11, &genstats[i].custom_statistic,
				 color);
		}
		if (game.get_player(i + 1)) // Show area plot
			m_plot.show_plot
				(i * m_ndatasets + m_selected_information,
				 m_my_registry->selected_players[i]);
	}

	m_plot.set_time(m_my_registry->time);

	// Setup Widgets
	m_box.add(&m_plot, UI::Box::AlignTop);

	UI::Box * hbox1 = new UI::Box(&m_box, 0, 0, UI::Box::Horizontal, 0, 0, 1);

	uint32_t plr_in_game = 0;
	Player_Number const nr_players = game.map().get_nrplayers();
	iterate_players_existing_novar(p, nr_players, game) ++plr_in_game;

	iterate_players_existing_const(p, nr_players, game, player) {
		char buffer[36];
		snprintf(buffer, sizeof(buffer), "pics/genstats_enable_plr_%02u.png", p);
		UI::Button & cb =
			*new UI::Button
				(hbox1, "playerbutton",
				 0, 0, 25, 25,
				 g_gr->images().get("pics/but4.png"),
				 g_gr->images().get(buffer),
				 player->get_name().c_str());
		cb.sigclicked.connect
			(boost::bind(&General_Statistics_Menu::cb_changed_to, this, p));
		cb.set_perm_pressed(m_my_registry->selected_players[p - 1]);

		m_cbs[p - 1] = &cb;

		hbox1->add(&cb, UI::Box::AlignLeft, false, true);
	} else //  player nr p does not exist
		m_cbs[p - 1] = nullptr;

	m_box.add(hbox1, UI::Box::AlignTop, true);

	UI::Box * hbox2 = new UI::Box(&m_box, 0, 0, UI::Box::Horizontal, 0, 0, 1);

	UI::Radiobutton * btn;

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_landsize.png"),
		 _("Land"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_nrworkers.png"),
		 _("Workers"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_nrbuildings.png"),
		 _("Buildings"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_nrwares.png"),
		 _("Wares"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_productivity.png"),
		 _("Productivity"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_casualties.png"),
		 _("Casualties"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_kills.png"),
		 _("Kills"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_msites_lost.png"),
		 _("Military buildings lost"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_msites_defeated.png"),
		 _("Military buildings defeated"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_civil_blds_lost.png"),
		 _("Civilian buildings lost"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	m_radiogroup.add_button
		(hbox2,
		 Point(0, 0),
		 g_gr->images().get("pics/genstats_militarystrength.png"),
		 _("Military"),
		 &btn);
	hbox2->add(btn, UI::Box::AlignLeft, false, true);

	if (hook) {
		m_radiogroup.add_button
			(hbox2,
			 Point(0, 0),
			 g_gr->images().get(cs_pic),
			 cs_name.c_str(),
			 &btn);
		hbox2->add(btn, UI::Box::AlignLeft, false, true);
	}

	m_radiogroup.set_state(m_selected_information);
	m_radiogroup.changedto.connect
		(boost::bind(&General_Statistics_Menu::radiogroup_changed, this, _1));

	m_box.add(hbox2, UI::Box::AlignTop, true);

	m_box.add
		(new WUIPlot_Area_Slider
			(&m_box, m_plot, 0, 0, 100, 45,
			 g_gr->images().get("pics/but1.png"))
		, UI::Box::AlignTop
		, true);

}

General_Statistics_Menu::~General_Statistics_Menu() {
	Game & game = ref_cast<Interactive_GameBase, UI::Panel>(*get_parent()).game();
	if (game.is_loaded()) {
		// Save informations for recreation, if window is reopened
		m_my_registry->selected_information = m_selected_information;
		m_my_registry->time = m_plot.get_time();
		Player_Number const nr_players = game.map().get_nrplayers();
		iterate_players_existing_novar(p, nr_players, game) {
			m_my_registry->selected_players[p - 1] = m_cbs[p - 1]->get_perm_pressed();
		}
	}
}

/**
 * called when the help button was clicked
 * \todo Implement help
*/
void General_Statistics_Menu::clicked_help() {}


/*
 * Cb has been changed to this state
 */
void General_Statistics_Menu::cb_changed_to(int32_t const id)
{
	// This represents our player number
	m_cbs[id - 1]->set_perm_pressed(not m_cbs[id - 1]->get_perm_pressed());

	m_plot.show_plot
		((id - 1) * m_ndatasets + m_selected_information,
		 m_cbs[id - 1]->get_perm_pressed());
}

/*
 * The radiogroup has changed
 */
void General_Statistics_Menu::radiogroup_changed(int32_t const id) {
	size_t const statistics_size =
		ref_cast<Interactive_GameBase, UI::Panel>(*get_parent()).game()
		.get_general_statistics().size();
	for (uint32_t i = 0; i < statistics_size; ++i)
		if (m_cbs[i]) {
			m_plot.show_plot
				(i * m_ndatasets + id, m_cbs[i]->get_perm_pressed());
			m_plot.show_plot
				(i * m_ndatasets + m_selected_information, false);
		}
	m_selected_information = id;
};
