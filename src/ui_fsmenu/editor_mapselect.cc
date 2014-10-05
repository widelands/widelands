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
	FullscreenMenuLoadMapOrGame(),

	// Main title
	m_title
		(this,
		 get_w() / 2, m_maplisty / 3,
		 _("Choose a map"), UI::Align_HCenter),

	// Map description
	m_label_mapname
		(this,
		 m_butx, m_maplisty,
		 _("Map Name:"),
		 UI::Align_Left),
	m_ta_mapname(this, m_butx + m_indent, m_label_mapname.get_y() + m_label_mapname.get_h() + m_padding,
					get_w() - m_butx - m_indent - m_margin_right, 2 * m_label_height - m_padding),

	m_label_author
		(this,
		 m_butx, m_ta_mapname.get_y() + m_ta_mapname.get_h() + m_padding,
		 _("Authors:"),
		 UI::Align_Left),
	m_ta_author(this, m_description_column_tab, m_label_author.get_y(),
				get_w() - m_butx - m_margin_right, m_label_height),

	m_label_size
		(this,
		 m_butx, m_ta_author.get_y() + m_ta_author.get_h(),
		 _("Size:"),
		 UI::Align_Left),
	m_ta_size(this, m_description_column_tab, m_label_size.get_y(),
				 get_w() - m_butx - m_margin_right, m_label_height),

	m_label_players
		(this,
		 m_butx, m_ta_size.get_y() + m_ta_size.get_h(),
		 _("Players:"),
		 UI::Align_Left),
	m_ta_players(this, m_description_column_tab, m_label_players.get_y(),
					 get_w() - m_butx - m_margin_right, m_label_height),

	m_label_description
		(this,
		 m_butx, m_ta_players.get_y() + m_ta_players.get_h() + 3 * m_padding,
		 _("Description:"),
		 UI::Align_Left),
	m_ta_description
		(this,
		 m_butx + m_indent,
		 m_label_description.get_y() + m_label_description.get_h() + m_padding,
		 get_w() - m_butx - m_indent - m_margin_right,
		 m_buty - m_label_description.get_y() - m_label_description.get_h()  - 4 * m_padding),

	// Map table
	m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth),

	// Runtime variables
	m_curdir("maps"), m_basedir("maps")
{
	m_back.set_tooltip(_("Return to the main editor menu"));
	m_ok.set_tooltip(_("Edit this map"));
	m_ta_mapname.set_tooltip(_("The name of this map"));
	m_ta_author.set_tooltip(_("The designers of this map"));
	m_ta_players.set_tooltip(_("The number of players"));
	m_ta_size.set_tooltip(_("The size of this map (Width x Height)"));
	m_ta_description.set_tooltip(_("Story and hints"));

	m_back.sigclicked.connect(boost::bind(&FullscreenMenuEditorMapSelect::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuEditorMapSelect::ok, boost::ref(*this)));

	m_title.set_textstyle(ts_big());

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
	std::string mapfilename(m_list.get_selected());

	if (g_fs->is_directory(mapfilename.c_str())
		 &&
		 !WidelandsMapLoader::is_widelands_map(mapfilename)) {

		m_curdir = mapfilename;
		m_list.clear();
		m_mapfiles.clear();
		fill_list();
	} else {
		end_modal(1);
	}
}


/**
 * Called when a different entry in the listbox gets selected.
 * When this happens, the information display at the right needs to be
 * refreshed.
 */
void FullscreenMenuEditorMapSelect::map_selected(uint32_t)
{
	std::string mapfilename = m_list.get_selected();

	m_ok.set_enabled(true);

	if (!g_fs->is_directory(mapfilename) || WidelandsMapLoader::is_widelands_map(mapfilename)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
			ml->preload_map(true); //  This has worked before, no problem.
		}

		// Translate the map data
		i18n::Textdomain td("maps");
		m_ta_mapname.set_text(_(map.get_name()));
		m_ta_author.set_text(map.get_author());
		m_ta_description.set_text(_(map.get_description()) +
										  (map.get_hint().empty() ? "" : (std::string("\n\n") + _(map.get_hint()))));

		m_ta_players.set_text((boost::format(ngettext("%u Player", "%u Players", map.get_nrplayers()))
				% static_cast<unsigned int>(map.get_nrplayers())).str());
		m_ta_size.set_text((boost::format("%i  x  %i") % map.get_width() % map.get_height()).str());

	} else {
		m_ta_mapname.set_text(std::string());
		m_ta_author.set_text(std::string());
		m_ta_description.set_text(std::string());
		m_ta_players.set_text(std::string());
		m_ta_size.set_text(std::string());
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
		std::string parent_string =
				/** TRANSLATORS: Parent directory */
				(boost::format("\\<%s\\>") % _("parent")).str();
		m_list.add
			(parent_string.c_str(),
			 m_parentdir.c_str(),
			 g_gr->images().get("pics/ls_dir.png"));
	}

	// Now we add the other directories
	for (const std::string& dirfilename : m_mapfiles) {
		const char * const name = dirfilename.c_str();

		if (strcmp(FileSystem::fs_filename(name), ".")  &&
			 // Upsy, appeared again. ignore
			 strcmp(FileSystem::fs_filename(name), "..") &&
			 g_fs->is_directory(name)                    &&
			 !WidelandsMapLoader::is_widelands_map(name)) {

			m_list.add
				(FileSystem::fs_filename(name),
				 name,
				 g_gr->images().get("pics/ls_dir.png"));
		}
	}

	// Finally, we add the maps
	Widelands::Map map;

	for (const std::string& mapfilename : m_mapfiles) {
		char const * const name = mapfilename.c_str();
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

	if (m_list.size()) {
		m_list.select(0);
	}
}
