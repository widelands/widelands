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

#include "fullscreen_menu_editor_mapselect.h"

#include "editor_game_base.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "map.h"
#include "profile.h"
#include "s2map.h"
#include "wexception.h"
#include "widelands_map_loader.h"

#include "log.h"

#include <stdio.h>

using Widelands::WL_Map_Loader;


Fullscreen_Menu_Editor_MapSelect::Fullscreen_Menu_Editor_MapSelect()
:
Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
m_xres
	(gr_x()),
m_yres
	(gr_y()),
m_butw
	(m_xres*0.25),
m_buth
	(m_yres*0.045),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Text labels
m_title
	(this,
	 m_xres / 2, m_yres*0.18,
	 _("Choose your map!"),
	 Align_HCenter),
m_label_name
	(this,
	 m_xres*0.7, m_yres*0.34,
	 _("Name:"),
	 Align_Right),
m_name (this, m_xres*0.71, m_yres*0.34, std::string()),
m_label_author
	(this,
	 m_xres*0.7, m_yres*0.375,
	 _("Author:"),
	 Align_Right),
m_author (this, m_xres*0.71, m_yres*0.375, std::string()),
m_label_size
	(this,
	 m_xres*0.7, m_yres*0.41,
	 _("Size:"),
	 Align_Right),
m_size (this, m_xres*0.71, m_yres*0.41, std::string()),
m_label_world
	(this,
	 m_xres*0.7, m_yres*0.445,
	 _("World:"),
	 Align_Right),
m_world (this, m_xres*0.71, m_yres*0.445, std::string()),
m_label_nr_players
	(this,
	 m_xres*0.7, m_yres*0.48,
	 _("Players:"),
	 Align_Right),
m_nr_players (this, m_xres*0.71, m_yres*0.48, std::string()),
m_label_descr
	(this,
	 m_xres*0.7, m_yres*0.515,
	 _("Descr:"),
	 Align_Right),
m_descr (this, m_xres*0.71, m_yres*0.52, m_xres*0.25, m_yres*0.315, std::string()),

// Buttons
m_back
	(this,
	 m_xres*0.71, m_yres*0.85, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_Editor_MapSelect::end_modal, this, 0,
	 _("Back"), std::string(), true, false,
	 m_fn, m_fs),
m_ok
	(this,
	 m_xres*0.71, m_yres*0.9, m_butw, m_buth,
	 2,
	 &Fullscreen_Menu_Editor_MapSelect::ok, this,
	 _("OK"), std::string(), false, false,
	 m_fn, m_fs),

// Map list
m_list(this, m_xres*0.0188, m_yres*0.3417, m_xres*0.5688, m_yres*0.6083),

// Runtime variables
m_curdir("maps"), m_basedir("maps")
{
	m_title           .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_name      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_name            .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_author    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_author          .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_size      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_size            .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_world     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_world           .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_nr_players.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_nr_players      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_descr     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_descr           .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list            .set_font(m_fn, m_fs);

	m_list.selected.set(this, &Fullscreen_Menu_Editor_MapSelect::map_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_Editor_MapSelect::double_clicked);

	fill_list();
}

std::string Fullscreen_Menu_Editor_MapSelect::get_map()
{
	return m_list.has_selection() ? m_list.get_selected() : 0;
}

void Fullscreen_Menu_Editor_MapSelect::ok()
{
	std::string filename(m_list.get_selected());

	if
		(g_fs->IsDirectory(filename.c_str())
		 &&
		 !WL_Map_Loader::is_widelands_map(filename))
	{

		m_curdir = filename;
		m_list.clear();
		m_mapfiles.clear();
		fill_list();
	} else
		end_modal(1);
}


/**
 * Called when a different entry in the listbox gets selected.
 * When this happens, the information display at the right needs to be
 * refreshed.
 */
void Fullscreen_Menu_Editor_MapSelect::map_selected(uint32_t)
{
	std::string name = m_list.get_selected();

	m_ok.set_enabled(true);

	if (!g_fs->IsDirectory(name) || WL_Map_Loader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			Widelands::Map_Loader * const m_ml = map.get_correct_loader(name.c_str());
			m_ml->preload_map(true); //  This has worked before, no problem.
			delete m_ml;
		}

		m_name  .set_text(map.get_name       ());
		m_author.set_text(map.get_author     ());
		m_descr .set_text(map.get_description());
		m_world .set_text(map.get_world_name ());

		char buf[200];
		sprintf(buf, "%i", map.get_nrplayers());
		m_nr_players .set_text(buf);

		sprintf(buf, "%ix%i", map.get_width(), map.get_height());
		m_size      .set_text(buf);
	} else {
		m_name      .set_text(std::string());
		m_author    .set_text(std::string());
		m_descr     .set_text(std::string());
		m_world     .set_text(std::string());
		m_nr_players.set_text(std::string());
		m_size      .set_text(std::string());
	}
}

/**
 * listbox got double clicked
 */
void Fullscreen_Menu_Editor_MapSelect::double_clicked(uint32_t) {ok();}

/**
 * fill the file list
 */
void Fullscreen_Menu_Editor_MapSelect::fill_list()
{
	//  Fill it with all files we find.
	g_fs->FindFiles(m_curdir, "*", &m_mapfiles, 1);

	//  First, we add all directories. We manually add the parent directory.
	if (m_curdir != m_basedir) {
#ifndef __WIN32__
		m_parentdir = m_curdir.substr(0, m_curdir.rfind("/"));
#else
		m_parentdir = m_curdir.substr(0, m_curdir.rfind("\\"));
#endif
		m_list.add
			(_("<parent>"),
			 m_parentdir.c_str(),
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
	}

	const filenameset_t::const_iterator mapfiles_end = m_mapfiles.end();
	for
		(filenameset_t::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
		const char * const name = pname->c_str();
		if
			(strcmp(FileSystem::FS_Filename(name), ".")    and
			 strcmp(FileSystem::FS_Filename(name), "..")   and // Upsy, appeared again. ignore
			 strcmp(FileSystem::FS_Filename(name), ".svn") and // HACK: we skip .svn dir (which is in normal checkout present) for esthetic reasons
			 g_fs->IsDirectory(name)                       and
			 not WL_Map_Loader::is_widelands_map(name))

		m_list.add
			(FileSystem::FS_Filename(name),
			 name,
			 g_gr->get_picture(PicMod_Game, "pics/ls_dir.png"));
	}

	Widelands::Map map;

	for
		(filenameset_t::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
		char const * const name = pname->c_str();

		if (Widelands::Map_Loader * const m_ml = map.get_correct_loader(name)) {
			try {
				m_ml->preload_map(true);
				m_list.add
					(FileSystem::FS_Filename(name),
					 name,
					 g_gr->get_picture
					 	(PicMod_Game,
					 	 dynamic_cast<WL_Map_Loader const *>(m_ml) ?
					 	 "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (_wexception&) {} //  we simply skip illegal entries
			delete m_ml;
		}
	}

	if (m_list.size())
		m_list.select(0);
}
