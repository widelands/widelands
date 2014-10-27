/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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

#include "ui_fsmenu/editor_mapselect.h"

#include <cstdio>
#include <memory>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "wui/text_constants.h"

using Widelands::WidelandsMapLoader;

FullscreenMenuEditorMapSelect::FullscreenMenuEditorMapSelect() :
	FullscreenMenuBase("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Text labels
	m_title
		(this,
		 get_w() / 2, get_h() * 9 / 50,
		 _("Choose a map"), UI::Align_HCenter),
	m_label_name
		(this,
		 get_w() * 7 / 10, get_h() * 17 / 50,
		 _("Name:"), UI::Align_Right),
	m_name (this, get_w() * 71 / 100, get_h() * 17 / 50, std::string()),
	m_label_author
		(this,
		 get_w() * 7 / 10, get_h() * 3 / 8,
		 _("Author:"), UI::Align_Right),
	m_author (this, get_w() * 71 / 100, get_h() * 3 / 8, std::string()),
	m_label_size
		(this,
		 get_w() * 7 / 10, get_h() * 41 / 100,
		 _("Size:"), UI::Align_Right),
	m_size (this, get_w() * 71 / 100, get_h() * 41 / 100, std::string()),
	m_label_nr_players
		(this,
		 get_w() * 7 / 10, get_h() * 12 / 25,
		 _("Players:"), UI::Align_Right),
	m_nr_players (this, get_w() * 71 / 100, get_h() * 12 / 25, std::string()),
	m_label_descr
		(this,
		 get_w() * 7 / 10, get_h() * 103 / 200,
		 _("Descr:"), UI::Align_Right),
	m_descr
		(this,
		 get_w() * 71 / 100, get_h() * 13 / 25, get_w() / 4, get_h() * 63 / 200),

// Buttons
	m_back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("OK"), std::string(), false, false),

// Map list
	m_list
		(this,
		 get_w() *  47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000),

// Runtime variables
	m_curdir("maps"), m_basedir("maps")
{
	m_back.sigclicked.connect(boost::bind(&FullscreenMenuEditorMapSelect::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuEditorMapSelect::ok, boost::ref(*this)));

	m_back.set_font(font_small());
	m_ok.set_font(font_small());

	m_title           .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_name      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_name            .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_author    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_author          .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_size      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_size            .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_nr_players.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_nr_players      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_descr     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_descr           .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list            .set_font(m_fn, m_fs);

	m_list.selected.connect(boost::bind(&FullscreenMenuEditorMapSelect::map_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&FullscreenMenuEditorMapSelect::double_clicked, this, _1));

	fill_list();
}

std::string FullscreenMenuEditorMapSelect::get_map()
{
	return m_list.has_selection() ? m_list.get_selected() : nullptr;
}

void FullscreenMenuEditorMapSelect::ok()
{
	std::string filename(m_list.get_selected());

	if
		(g_fs->is_directory(filename.c_str())
		 &&
		 !WidelandsMapLoader::is_widelands_map(filename))
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
void FullscreenMenuEditorMapSelect::map_selected(uint32_t)
{
	std::string name = m_list.get_selected();

	m_ok.set_enabled(true);

	if (!g_fs->is_directory(name) || WidelandsMapLoader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(name);
			ml->preload_map(true); //  This has worked before, no problem.
		}

		// Translate the map data
		i18n::Textdomain td("maps");
		m_name  .set_text(_(map.get_name()));
		m_author.set_text(map.get_author());
		m_descr .set_text
			(_(map.get_description()) + (map.get_hint().empty() ? "" : (std::string("\n") + _(map.get_hint()))));

		m_nr_players.set_text(std::to_string(static_cast<unsigned int>(map.get_nrplayers())));

		/** TRANSLATORS: These are map coordinates */
		m_size      .set_text((boost::format(_("%1$ix%2$i")) % map.get_width() % map.get_height()).str());
	} else {
		m_name      .set_text(std::string());
		m_author    .set_text(std::string());
		m_descr     .set_text(std::string());
		m_nr_players.set_text(std::string());
		m_size      .set_text(std::string());
	}
}

/**
 * listbox got double clicked
 */
void FullscreenMenuEditorMapSelect::double_clicked(uint32_t) {ok();}

/**
 * fill the file list
 */
void FullscreenMenuEditorMapSelect::fill_list()
{
	//  Fill it with all files we find.
	m_mapfiles = g_fs->list_directory(m_curdir);

	//  First, we add all directories. We manually add the parent directory.
	if (m_curdir != m_basedir) {
#ifndef _WIN32
		m_parentdir = m_curdir.substr(0, m_curdir.rfind('/'));
#else
		m_parentdir = m_curdir.substr(0, m_curdir.rfind('\\'));
#endif

		m_list.add
				/** TRANSLATORS: Parent directory */
				((boost::format("\\<%s\\>") % _("parent")).str(),
				 m_parentdir.c_str(),
			 g_gr->images().get("pics/ls_dir.png"));
	}

	const FilenameSet::const_iterator mapfiles_end = m_mapfiles.end();
	for
		(FilenameSet::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
		const char * const name = pname->c_str();
		if
			(strcmp(FileSystem::fs_filename(name), ".")    &&
			 // Upsy, appeared again. ignore
			 strcmp(FileSystem::fs_filename(name), "..")   &&
			 g_fs->is_directory(name)                       &&
			 !WidelandsMapLoader::is_widelands_map(name))

		m_list.add
			(FileSystem::fs_filename(name),
			 name,
			 g_gr->images().get("pics/ls_dir.png"));
	}

	Widelands::Map map;

	for
		(FilenameSet::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
		char const * const name = pname->c_str();
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(name);
		if (ml.get() != nullptr) {
			try {
				ml->preload_map(true);
				m_list.add
					(FileSystem::fs_filename(name),
					 name,
					 g_gr->images().get
						 (dynamic_cast<WidelandsMapLoader*>(ml.get()) ? "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}

	if (m_list.size())
		m_list.select(0);
}
