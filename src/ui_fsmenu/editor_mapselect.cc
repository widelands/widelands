/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "editor_mapselect.h"

#include "logic/editor_game_base.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "map.h"
#include "profile/profile.h"
#include "s2map.h"
#include "wexception.h"
#include "map_io/widelands_map_loader.h"

#include "log.h"

#include <cstdio>

using Widelands::WL_Map_Loader;

Fullscreen_Menu_Editor_MapSelect::Fullscreen_Menu_Editor_MapSelect() :
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
		 _("Choose your map!"), UI::Align_HCenter),
	m_label_name
		(this,
		 m_xres * 7 / 10, m_yres * 17 / 50,
		 _("Name:"), UI::Align_Right),
	m_name (this, m_xres * 71 / 100, m_yres * 17 / 50, std::string()),
	m_label_author
		(this,
		 m_xres * 7 / 10, m_yres * 3 / 8,
		 _("Author:"), UI::Align_Right),
	m_author (this, m_xres * 71 / 100, m_yres * 3 / 8, std::string()),
	m_label_size
		(this,
		 m_xres * 7 / 10, m_yres * 41 / 100,
		 _("Size:"), UI::Align_Right),
	m_size (this, m_xres * 71 / 100, m_yres * 41 / 100, std::string()),
	m_label_world
		(this,
		 m_xres * 7 / 10, m_yres * 89 / 200,
		 _("World:"), UI::Align_Right),
	m_world (this, m_xres * 71 / 100, m_yres * 89 / 200, std::string()),
	m_label_nr_players
		(this,
		 m_xres * 7 / 10, m_yres * 12 / 25,
		 _("Players:"), UI::Align_Right),
	m_nr_players (this, m_xres * 71 / 100, m_yres * 12 / 25, std::string()),
	m_label_descr
		(this,
		 m_xres * 7 / 10, m_yres * 103 / 200,
		 _("Descr:"), UI::Align_Right),
	m_descr
		(this,
		 m_xres * 71 / 100, m_yres * 13 / 25, m_xres / 4, m_yres * 63 / 200),

// Buttons
	m_back
		(this,
		 m_xres * 71 / 100, m_yres * 17 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_Editor_MapSelect::end_modal, *this, 0,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_ok
		(this,
		 m_xres * 71 / 100, m_yres * 9 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 &Fullscreen_Menu_Editor_MapSelect::ok, *this,
		 _("OK"), std::string(), false, false,
		 m_fn, m_fs),

// Map list
	m_list
		(this,
		 m_xres *  47 / 2500, m_yres * 3417 / 10000,
		 m_xres * 711 / 1250, m_yres * 6083 / 10000),

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
	m_list.double_clicked.set
		(this, &Fullscreen_Menu_Editor_MapSelect::double_clicked);

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
			Widelands::Map_Loader * const m_ml =
				map.get_correct_loader(name.c_str());
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
#ifndef WIN32
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
			 // Upsy, appeared again. ignore
			 strcmp(FileSystem::FS_Filename(name), "..")   and
			 // HACK: we skip .svn dir (which is in normal checkout present) for
			 // esthetic reasons
			 strcmp(FileSystem::FS_Filename(name), ".svn") and
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
			} catch (_wexception const &) {} //  we simply skip illegal entries
			delete m_ml;
		}
	}

	if (m_list.size())
		m_list.select(0);
}
