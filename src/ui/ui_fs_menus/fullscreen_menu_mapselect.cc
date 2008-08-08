/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "fullscreen_menu_mapselect.h"


#include "editor_game_base.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "map.h"
#include "profile.h"
#include "widelands_map_loader.h"
#include "s2map.h"
#include "wexception.h"
#include "widelands_map_loader.h"

#include "log.h"

#include <stdio.h>

using Widelands::WL_Map_Loader;


Fullscreen_Menu_MapSelect::Fullscreen_Menu_MapSelect()
:
Fullscreen_Menu_Base("choosemapmenu.jpg"),
m_title(this, MENU_XRES / 2, 110, _("Choose your map!"), Align_HCenter),
m_label_load_map_as_scenario
	(this, MENU_XRES - 300, 180, _("Load Map as scenario: "), Align_VCenter),
m_label_name      (this, 560, 205, _("Name:"),    Align_Right),
m_name            (this, 570, 205, ""),
m_label_author    (this, 560, 225, _("Author:"),  Align_Right),
m_author          (this, 570, 225, ""),
m_label_size      (this, 560, 245, _("Size:"),    Align_Right),
m_size            (this, 570, 245, ""),
m_label_world     (this, 560, 265, _("World:"),   Align_Right),
m_world           (this, 570, 265, ""),
m_label_nr_players(this, 560, 285, _("Players:"), Align_Right),
m_nr_players      (this, 570, 285, ""),
m_label_descr     (this, 560, 305, _("Descr:"),   Align_Right),
m_descr           (this, 570, 305, 200, 190, ""),
m_back
	(this,
	 570, 505, 200, 26,
	 0,
	 &Fullscreen_Menu_MapSelect::end_modal, this, 0,
	 _("Back")),
m_ok
	(this,
	 570, 535, 200, 26,
	 2,
	 &Fullscreen_Menu_MapSelect::ok, this,
	 _("OK"),
	 std::string(),
	 false),
m_load_map_as_scenario
	(this,
	 Point
	 	(m_label_load_map_as_scenario.get_x() +
	 	 m_label_load_map_as_scenario.get_w() + 10,
	 	 170)),
m_list(this, 15, 205, 455, 365),
m_curdir("maps"), m_basedir("maps")
{
	m_title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	m_load_map_as_scenario.set_state(false);

	m_list.selected.set(this, &Fullscreen_Menu_MapSelect::map_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_MapSelect::double_clicked);

	fill_list();
}

bool Fullscreen_Menu_MapSelect::is_scenario()
{
	return m_load_map_as_scenario.get_state();
}

void Fullscreen_Menu_MapSelect::setScenarioSelectionVisible(bool vis)
{
	m_load_map_as_scenario.set_visible(vis);
	m_label_load_map_as_scenario.set_visible(vis);
}

const MapData* Fullscreen_Menu_MapSelect::get_map() const
{
	return m_list.has_selection() ? &m_list.get_selected() : 0;
}


void Fullscreen_Menu_MapSelect::ok()
{
	MapData const & mapdata = m_list.get_selected();

	if (!mapdata.width) {
		m_curdir = g_fs->FS_CanonicalizeName(mapdata.filename);
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
	MapData const & map = m_list.get_selected();

	if (map.width) {
		char buf[256];

		// get translated tribesname
		std::string worldpath("worlds/" + map.world);
		i18n::grab_textdomain(worldpath);
		Profile prof((worldpath + "/conf").c_str());
		Section & global = prof.get_safe_section("world");
		std::string world(global.get_safe_string("name"));
		i18n::release_textdomain();

		m_name      .set_text(map.name);
		m_author    .set_text(map.author);
		sprintf(buf, "%-4ux%4u", map.width, map.height);
		m_size      .set_text(buf);
		sprintf(buf, "%i", map.nrplayers);
		m_nr_players.set_text(buf);
		m_descr     .set_text(map.description);
		m_world     .set_text(world);
	} else {
		// Directory
		m_name      .set_text("(directory)");
		m_author    .set_text("");
		m_size      .set_text("");
		m_nr_players.set_text("");
		m_descr     .set_text("");
		m_world     .set_text("");
	}
	m_ok.set_enabled(true);
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
	m_list.clear();

	//  Fill it with all files we find in all directories.
	filenameset_t files;
	g_fs->FindFiles(m_curdir, "*", &files);

	int32_t ndirs=0;

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (m_curdir != m_basedir) {
		MapData map;
		map.filename = g_fs->FS_CanonicalizeName(m_curdir + "/..");

		m_list.add
			("<parent>",
			 map,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add subdirectories to the list (except for uncompressed maps)
	for
		(filenameset_t::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		const char * const name = pname->c_str();
		if (!strcmp(FileSystem::FS_Filename(name), ".")) continue;
		if (!strcmp(FileSystem::FS_Filename(name), "..")) continue; // Upsy, appeared again. ignore
		if (!strcmp(FileSystem::FS_Filename(name), ".svn")) continue; // HACK: we skip .svn dir (which is in normal checkout present) for esthetic reasons
		if (!g_fs->IsDirectory(name)) continue;
		if (WL_Map_Loader::is_widelands_map(name)) continue;

		MapData dir;
		dir.filename = name;
		m_list.add
			(FileSystem::FS_Filename(name),
			 dir,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add map files(compressed maps) and directories(uncompressed)
	{
		Widelands::Map map; //  Map_Loader needs a place to put it's preload data

		for
			(filenameset_t::iterator pname = files.begin();
			 pname != files.end();
			 ++pname)
		{
			const char *name = pname->c_str();

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
				if (!mapdata.width || !mapdata.height)
					continue;

				m_list.add
					(mapdata.name.c_str(),
					 mapdata,
					 g_gr->get_picture
					 	(PicMod_Game,
					 	 dynamic_cast<WL_Map_Loader const *>(ml) ?
					 	 "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const std::exception & e) {
				log("Mapselect: Skip %s due to preload error: %s\n", name, e.what());
			} catch (...) {
				log("Mapselect: Skip %s due to unknown exception\n", name);
			}

			delete ml;
		}
	}

	m_list.sort(0, ndirs);
	m_list.sort(ndirs);

	if (m_list.size())
		m_list.select(0);
}
