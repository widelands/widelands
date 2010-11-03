/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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
 */

#include <cstdio>

#include "logic/editor_game_base.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "profile/profile.h"
#include "s2map.h"
#include "wexception.h"
#include "map_io/widelands_map_loader.h"
#include "log.h"

#include "mapselect.h"


using Widelands::WL_Map_Loader;

Fullscreen_Menu_MapSelect::Fullscreen_Menu_MapSelect
		(Map::ScenarioTypes allowed_scenario_types) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (m_xres / 4),
	m_buth (m_yres * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Text labels
	m_title
		(this,
		 m_xres / 2, m_yres * 9 / 50,
		 _("Choose your map!"),
		 UI::Align_HCenter),
	m_label_load_map_as_scenario
		(this,
		 m_xres * 23 / 25, m_yres * 11 / 40,
		 _("Load Map as scenario: "),
		 UI::Align_Right),
	m_label_name
		(this,
		 m_xres * 7 / 10, m_yres * 17 / 50,
		 _("Name:"),
		 UI::Align_Right),
	m_name (this, m_xres * 71 / 100, m_yres * 17 / 50),
	m_label_author
		(this,
		 m_xres * 7 / 10, m_yres * 3 / 8,
		 _("Author:"),
		 UI::Align_Right),
	m_author (this, m_xres * 71 / 100, m_yres * 3 / 8),
	m_label_size
		(this,
		 m_xres * 7 / 10, m_yres * 41 / 100,
		 _("Size:"),
		 UI::Align_Right),
	m_size (this, m_xres * 71 / 100, m_yres * 41 / 100),
	m_label_world
		(this,
		 m_xres * 7 / 10, m_yres * 89 / 200,
		 _("World:"),
		 UI::Align_Right),
	m_world (this, m_xres * 71 / 100, m_yres * 89 / 200),
	m_label_nr_players
		(this,
		 m_xres * 7 / 10, m_yres * 12 / 25,
		 _("Players:"),
		 UI::Align_Right),
	m_nr_players (this, m_xres * 71 / 100, m_yres * 12 / 25),
	m_label_descr
		(this,
		 m_xres * 7 / 10, m_yres * 103 / 200,
		 _("Descr:"),
		 UI::Align_Right),
	m_descr
		(this,
		 m_xres * 71 / 100, m_yres * 13 / 25, m_xres / 4, m_yres * 63 / 200),

// Buttons
	m_back
		(this, "back",
		 m_xres * 71 / 100, m_yres * 17 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind(&Fullscreen_Menu_MapSelect::end_modal, boost::ref(*this), 0),
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_ok
		(this, "ok",
		 m_xres * 71 / 100, m_yres * 9 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 boost::bind(&Fullscreen_Menu_MapSelect::ok, boost::ref(*this)),
		 _("OK"), std::string(), false, false,
		 m_fn, m_fs),

// Checkbox
	m_load_map_as_scenario (this, Point (m_xres * 187 / 200, m_yres * 7 / 25)),

// Map table
	m_table
		(this,
		 m_xres *  47 / 2500, m_yres * 3417 / 10000,
		 m_xres * 711 / 1250, m_yres * 6083 / 10000),
	m_curdir("maps"), m_basedir("maps")
{
	m_title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_load_map_as_scenario.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_name                .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_name                      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_author              .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_author                    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_size                .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_size                      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_world               .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_world                     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_nr_players          .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_nr_players                .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_descr               .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_descr                     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_table                     .set_font(m_fn, m_fs);

#define NR_PLAYERS_WIDTH 35
	m_table.add_column(NR_PLAYERS_WIDTH, _("#"), UI::Align_HCenter);
	m_table.add_column
		(m_table.get_w() - NR_PLAYERS_WIDTH, _("Map Name"), UI::Align_Left);
	m_table.set_column_compare
		(1,
		 boost::bind
		 (&Fullscreen_Menu_MapSelect::compare_maprows, this, _1, _2));
	m_table.set_sort_column(0);

	m_load_map_as_scenario.set_state(false);
	m_load_map_as_scenario.set_enabled(false);

	m_table.selected.set(this, &Fullscreen_Menu_MapSelect::map_selected);
	m_table.double_clicked.set(this, &Fullscreen_Menu_MapSelect::double_clicked);

	m_scenario_types = allowed_scenario_types;
	if (m_scenario_types) {
		m_load_map_as_scenario.set_visible(true);
		m_label_load_map_as_scenario.set_visible(true);
	} else {
		m_load_map_as_scenario.set_visible(false);
		m_label_load_map_as_scenario.set_visible(false);
	}

	fill_list();
}

bool Fullscreen_Menu_MapSelect::compare_maprows
	(uint32_t const rowa, uint32_t const rowb)
{
	MapData const & r1 = m_maps_data[m_table[rowa]];
	MapData const & r2 = m_maps_data[m_table[rowb]];

	if (!r1.width and !r2.width) {
		return r1.name < r2.name;
	} else if (!r1.width and r2.width) {
		return true;
	} else if(r1.width and !r2.width) {
		return false;
	}
	return r1.name < r2.name;
}

bool Fullscreen_Menu_MapSelect::is_scenario()
{
	return m_load_map_as_scenario.get_state();
}

MapData const * Fullscreen_Menu_MapSelect::get_map() const
{
	if (not m_table.has_selection())
		return 0;
	return &m_maps_data[m_table.get_selected()];
}


void Fullscreen_Menu_MapSelect::ok()
{
	MapData const & mapdata = m_maps_data[m_table.get_selected()];

	if (!mapdata.width) {
		m_curdir = mapdata.filename;
		fill_list();
	} else
		end_modal(1 + is_scenario());
}


/**
 * Called when a different entry in the listbox gets selected.
 * When this happens, the information display at the right needs to be
 * refreshed.
 */
void Fullscreen_Menu_MapSelect::map_selected(uint32_t)
{
	MapData const & map = m_maps_data[m_table.get_selected()];

	if (map.width) {
		char buf[256];

		// get translated worldsname
		std::string worldpath("worlds/" + map.world);
		Profile prof((worldpath + "/conf").c_str(), 0, "world_" + map.world);
		Section & global = prof.get_safe_section("world");
		std::string world(global.get_safe_string("name"));

		m_name      .set_text(map.name);
		m_author    .set_text(map.author);
		sprintf(buf, "%-4ux%4u", map.width, map.height);
		m_size      .set_text(buf);
		sprintf(buf, "%i", map.nrplayers);
		m_nr_players.set_text(buf);
		m_descr     .set_text(map.description);
		m_world     .set_text(world);
		m_load_map_as_scenario.set_enabled(map.scenario);
	} else {
		// Directory
		m_name      .set_text(_("(directory)"));
		m_author    .set_text(std::string());
		m_size      .set_text(std::string());
		m_nr_players.set_text(std::string());
		m_descr     .set_text(std::string());
		m_world     .set_text(std::string());
		m_load_map_as_scenario.set_enabled(false);
	}
	m_ok.set_enabled(true);
	m_load_map_as_scenario.set_state(false); // reset
}

/**
 * listbox got double clicked
 */
void Fullscreen_Menu_MapSelect::double_clicked(uint32_t) {
	ok();
}

/**
 * Fill the list with maps that can be opened.
 *
 *
 * At first, only the subdirectories are added to the list, then the normal
 * files follow. This is done to make navigation easier.
 *
 * To make things more difficult, we have to support compressed and uncompressed
 * map files here - the former are files, the latter are directories. Care must
 * be taken to sort uncompressed maps (which look like and really are
 * directories) with the files.
 *
 * The search starts in \ref m_curdir ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 */
void Fullscreen_Menu_MapSelect::fill_list()
{
	m_maps_data.clear();
	m_table.clear();

	//  Fill it with all files we find in all directories.
	filenameset_t files;
	g_fs->FindFiles(m_curdir, "*", &files);

	int32_t ndirs = 0;

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (m_curdir != m_basedir) {
		MapData map;
#ifndef WIN32
		map.filename = m_curdir.substr(0, m_curdir.rfind('/'));
#else
		map.filename = m_curdir.substr(0, m_curdir.rfind('\\'));
#endif
		m_maps_data.push_back(map);
		UI::Table<uintptr_t const>::Entry_Record & te =
			m_table.add(m_maps_data.size() - 1);

		te.set_string(0, "");
		te.set_picture
			(1,  g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"),
			 _("<parent>"));

		++ndirs;
	}

	//Add subdirectories to the list (except for uncompressed maps)
	for
		(filenameset_t::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		char const * const name = pname->c_str();
		if (!strcmp(FileSystem::FS_Filename(name), "."))
			continue;
		// Upsy, appeared again. ignore
		if (!strcmp(FileSystem::FS_Filename(name), ".."))
			continue;
		if (!g_fs->IsDirectory(name))
			continue;
		if (WL_Map_Loader::is_widelands_map(name))
			continue;

		MapData dir;
		dir.filename = name;

		m_maps_data.push_back(dir);
		UI::Table<uintptr_t const>::Entry_Record & te =
			m_table.add(m_maps_data.size() - 1);

		te.set_string(0, "");
		te.set_picture
			(1,  g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"),
			 FileSystem::FS_Filename(name));

		++ndirs;
	}

	//Add map files(compressed maps) and directories(uncompressed)
	{
		Widelands::Map map; //  Map_Loader needs a place to put it's preload data
		i18n::Textdomain td("maps");

		for
			(filenameset_t::iterator pname = files.begin();
			 pname != files.end();
			 ++pname)
		{
			char const * const name = pname->c_str();

			Widelands::Map_Loader * const ml = map.get_correct_loader(name);
			if (!ml)
				continue;

			try {
				map.set_filename(name);
				ml->preload_map(true);

				MapData mapdata;
				mapdata.filename = name;
				mapdata.name = map.get_name();
				mapdata.author = map.get_author();
				mapdata.description = map.get_description();
				mapdata.world = map.get_world_name();
				mapdata.nrplayers = map.get_nrplayers();
				mapdata.width = map.get_width();
				mapdata.height = map.get_height();
				mapdata.scenario = map.scenario_types() & m_scenario_types;

				if (!mapdata.width || !mapdata.height)
					continue;

				m_maps_data.push_back(mapdata);
				UI::Table<uintptr_t const>::Entry_Record & te =
					m_table.add(m_maps_data.size() - 1);

				char buf[256];
				sprintf(buf, "(%i)", mapdata.nrplayers);
				te.set_string(0, buf);
				te.set_picture
					(1,  g_gr->get_picture
					 (PicMod_Game,
					  dynamic_cast<WL_Map_Loader const *>(ml) ?
					  (mapdata.scenario ?
						"pics/ls_wlscenario.png" : "pics/ls_wlmap.png")
					  :
					  "pics/ls_s2map.png"), mapdata.name.c_str());
			} catch (const std::exception & e) {
				log
					("Mapselect: Skip %s due to preload error: %s\n",
					 name, e.what());
			} catch (...) {
				log("Mapselect: Skip %s due to unknown exception\n", name);
			}

			delete ml;
		}
	}

	m_table.sort();

	if (m_table.size())
		m_table.select(0);
}
