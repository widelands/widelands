/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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
#include "widelands_map_loader.h"
#include "s2map.h"
#include "wexception.h"
#include "widelands_map_loader.h"

#include "log.h"

#include <stdio.h>


Fullscreen_Menu_MapSelect::Fullscreen_Menu_MapSelect(Editor_Game_Base *g, Map_Loader** ml)
:
	Fullscreen_Menu_Base("choosemapmenu.jpg"),
	egbase(g),

	// Text
	title(this, MENU_XRES / 2, 110, _("Choose your map!"), Align_HCenter),

	// Checkbox
	label_load_map_as_scenario
	(this, MENU_XRES - 300, 180, _("Load Map as scenario: "), Align_VCenter),
	load_map_as_scenario
	(this,
	label_load_map_as_scenario.get_x() + label_load_map_as_scenario.get_w() + 10,
	170),


	// Create the list area
	list(this, 15, 205, 455, 365),

	// Info fields
	label_name    (this, 560, 205, _("Name:"),    Align_Right),
	taname        (this, 570, 205, ""),
	label_author  (this, 560, 225, _("Author:"),  Align_Right),
	taauthor      (this, 570, 225, ""),
	label_size    (this, 560, 245, _("Size:"),    Align_Right),
	tasize        (this, 570, 245, ""),
	label_world   (this, 560, 265, _("World:"),   Align_Right),
	taworld       (this, 570, 265, ""),
	label_nplayers(this, 560, 285, _("Players:"), Align_Right),
	tanplayers    (this, 570, 285, ""),
	label_descr   (this, 560, 305, _("Descr:"),   Align_Right),
	tadescr       (this, 570, 305, 200, 190, ""),

	back
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

	m_ml         (ml),
	m_map        (new Map),
	m_is_scenario(false),

	m_curdir ("maps"),
	m_basedir("maps")

{
	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	load_map_as_scenario.changedto.set
		(this, &Fullscreen_Menu_MapSelect::changed);
	load_map_as_scenario.set_state(m_is_scenario);
	list.selected.set(this, &Fullscreen_Menu_MapSelect::map_selected);
	list.double_clicked.set(this, &Fullscreen_Menu_MapSelect::double_clicked);
	delete *m_ml;
	*m_ml=0;
	fill_list();
}

Fullscreen_Menu_MapSelect::~Fullscreen_Menu_MapSelect()
{
	// if m_map is != 0, obviously ok was not pressed
	delete m_map;
	m_map=0;
}

/**
 * Gets called when the Checkbox changes
 */
void Fullscreen_Menu_MapSelect::changed(bool t)
{
	m_is_scenario=t;
}

void Fullscreen_Menu_MapSelect::ok()
{
	const std::string filename = list.get_selected();

	if (g_fs->IsDirectory(filename.c_str()) && !Widelands_Map_Loader::is_widelands_map(filename)) {
		m_curdir=g_fs->FS_CanonicalizeName(filename);
		list.clear();
		m_mapfiles.clear();
		fill_list();
	} else {
		if (m_map)
		{
			assert(*m_ml);

			egbase->set_map((*m_ml)->get_map());
			(*m_ml)->preload_map(m_is_scenario);
			(*m_ml)->load_world();
			m_map = 0;
		}

		if (m_is_scenario)
			end_modal(2);
		else
		end_modal(1);
	}
}

/**
 * Called when a different entry in the listbox gets selected.
 * When this happens, the information display at the right needs to be
 * refreshed.
 */
void Fullscreen_Menu_MapSelect::map_selected(uint32_t)
{
	const char * const name = list.get_selected();

	if (!g_fs->IsDirectory(name) || Widelands_Map_Loader::is_widelands_map(name)) {
		// No directory
		delete *m_ml;
		*m_ml = 0;

		if (get_mapname())
		{
			assert(m_map);

			try {
				*m_ml = m_map->get_correct_loader(get_mapname()); //  FIXME memory leak!
				(*m_ml)->preload_map(m_is_scenario);
				m_map->set_filename(name);

				char buf[256];
				taname    .set_text(m_map->get_name());
				taauthor  .set_text(m_map->get_author());
				sprintf(buf, "%-4ix%4i", m_map->get_width(), m_map->get_height());
				tasize    .set_text(buf);
				sprintf(buf, "%i", m_map->get_nrplayers());
				tanplayers.set_text(buf);
				tadescr   .set_text(m_map->get_description());
				taworld   .set_text(World::World(m_map->get_world_name()).get_name());
				m_ok.set_enabled(true);
			} catch (std::exception& e) {
				log("Failed to load map %s: %s\n", get_mapname(), e.what());

				taname    .set_text("(bad map file)");
				taauthor  .set_text("");
				tasize    .set_text("");
				tanplayers.set_text("");
				tadescr   .set_text("");
				taworld   .set_text("");
				m_ok.set_enabled(false);
			}
		} else {
			// Directory
			taname    .set_text("(bad map file)");
			taauthor  .set_text("");
			tasize    .set_text("");
			tanplayers.set_text("");
			tadescr   .set_text("");
			taworld   .set_text("");
		}
	}
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
 * \todo This has not been accomplished yet
 *
 * The search starts in \ref m_curdir ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 */
void Fullscreen_Menu_MapSelect::fill_list()
{
	// Fill it with all files we find in all directorys
	g_fs->FindFiles(m_curdir, "*", &m_mapfiles);

	int32_t ndirs=0;

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (m_curdir!=m_basedir) {
		m_parentdir=g_fs->FS_CanonicalizeName(m_curdir+"/..");
		list.add("<parent>",
		         m_parentdir.c_str(),
		         g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add subdirectories to the list
	//TODO: but skip uncompressed maps (which look like directories)
	for (filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
		const char *name = pname->c_str();
		if (!strcmp(FileSystem::FS_Filename(name), ".")) continue;
		if (!strcmp(FileSystem::FS_Filename(name), "..")) continue; // Upsy, appeared again. ignore
		if (!strcmp(FileSystem::FS_Filename(name), ".svn")) continue; // HACK: we skip .svn dir (which is in normal checkout present) for aesthetic reasons
		if (!g_fs->IsDirectory(name)) continue;
		if (Widelands_Map_Loader::is_widelands_map(name)) continue;

		list.add(FileSystem::FS_Filename(name),
		         name,
		         g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
		++ndirs;
	}

	//Add map files(compressed maps) and directories(uncompressed)
	{
		Map map; //Map_Loader needs a place to put it's preload data

		for (filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
			const char *name = pname->c_str();

			Map_Loader * const ml = map.get_correct_loader(name);
			if (!ml)
				continue;

			try {
				map.set_filename(name);
				ml->preload_map(true);
				list.add(map.get_name(),
				         name,
				         g_gr->get_picture(PicMod_Game,
				         dynamic_cast<const Widelands_Map_Loader *>(ml) ? "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const std::exception & e) {
				log("Mapselect: Skip %s due to preload error: %s\n", name, e.what());
			} catch (...) {
				log("Mapselect: Skip %s due to unknown exception\n", name);
			}

			delete ml;
		}
	}

	list.sort(0, ndirs);
	list.sort(ndirs);

	if (list.size()) list.select(0);
}
