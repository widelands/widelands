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

#include "editor/ui_menus/editor_main_menu_load_map.h"

#include <cstdio>
#include <memory>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/editor_set_starting_pos_tool.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "map_io/map_loader.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "wui/overlay_manager.h"

using Widelands::WidelandsMapLoader;

/**
 * Create all the buttons etc...
*/
MainMenuLoadMap::MainMenuLoadMap(EditorInteractive & parent)
	: UI::Window(&parent, "load_map_menu", 0, 0, 560, 300, _("Load Map"))
{
	int32_t const spacing =  5;
	int32_t const offsx   = spacing;
	int32_t const offsy   = 10;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	int32_t const descr_label_w = 100;

	m_ls = new UI::Listselect<const char *>
		(this,
		 posx, posy,
		 get_inner_w() / 2 - spacing, get_inner_h() - spacing - offsy - 40);
	m_ls->selected.connect(boost::bind(&MainMenuLoadMap::selected, this, _1));
	m_ls->double_clicked.connect(boost::bind(&MainMenuLoadMap::double_clicked, this, _1));
	m_ls->focus();

	posx = get_inner_w() / 2 + spacing;
	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Name:"), UI::Align_CenterLeft);
	m_name =
		new UI::MultilineTextarea
			(this, posx + descr_label_w, posy, 200, 40, "---", UI::Align_CenterLeft);
	posy += 40 + spacing;

	new UI::Textarea
		(this, posx, posy, 150, 20, _("Authors:"), UI::Align_CenterLeft);
	m_author =
		new UI::Textarea
			(this, posx + descr_label_w, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Size:"), UI::Align_CenterLeft);
	m_size =
		new UI::Textarea
			(this, posx + descr_label_w, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Players:"), UI::Align_CenterLeft);
	m_nrplayers =
		new UI::Textarea
			(this, posx + descr_label_w, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;


	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Descr:"), UI::Align_CenterLeft);
	m_descr =
		new UI::MultilineTextarea
			(this,
			 posx + descr_label_w, posy,
			 get_inner_w() - posx - spacing - descr_label_w,
			 get_inner_h() - posy - spacing - 40,
			 "---", UI::Align_CenterLeft);

	posy = get_inner_h() - 30;

	m_ok_btn = new UI::Button
		(this, "ok",
		 posx, posy,
		 get_inner_w() / 4 - 1.5 * spacing, 20,
		 g_gr->images().get("pics/but0.png"),
		 _("OK"),
		 std::string(),
		 false);
	m_ok_btn->sigclicked.connect(boost::bind(&MainMenuLoadMap::clicked_ok, this));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 posx + get_inner_w() / 4 - spacing / 2, posy,
		 get_inner_w() / 4 - 1.5 * spacing, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&MainMenuLoadMap::die, this));

	m_basedir = "maps";
	m_curdir  = "maps";

	fill_list();

	center_to_parent();
	move_to_top();
}


void MainMenuLoadMap::clicked_ok() {
	const char * const filename(m_ls->get_selected());

	if (g_fs->is_directory(filename) && !WidelandsMapLoader::is_widelands_map(filename)) {
		m_curdir = filename;
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else {
		dynamic_cast<EditorInteractive&>(*get_parent()).load(filename);
		die();
	}
}

/**
 * Called when a entry is selected
 */
void MainMenuLoadMap::selected(uint32_t) {
	const char * const name = m_ls->get_selected();

	m_ok_btn->set_enabled(true);

	if (!g_fs->is_directory(name) || WidelandsMapLoader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::MapLoader> map_loader = map.get_correct_loader(name);
			map_loader->preload_map(true); //  This has worked before, no problem.
		}

		// Translate the map data
		i18n::Textdomain td("maps");
		m_name  ->set_text(map.get_name());
		m_name  ->set_tooltip(map.get_name());
		m_author->set_text(map.get_author());
		m_descr ->set_text
			(_(map.get_description()) +
			 (map.get_hint().empty() ? "" : (std::string("\n\n") + _(map.get_hint()))));

		m_nrplayers->set_text(std::to_string(static_cast<unsigned int>(map.get_nrplayers())));

		m_size     ->set_text((boost::format(_("%1$ix%2$i"))
									  % map.get_width()
									  % map.get_height()).str());
	} else {
		m_name     ->set_text("");
		m_name     ->set_tooltip("");
		m_author   ->set_text("");
		m_descr    ->set_text("");
		m_nrplayers->set_text("");
		m_size     ->set_text("");
	}
}

/**
 * An entry has been doubleclicked
 */
void MainMenuLoadMap::double_clicked(uint32_t) {clicked_ok();}

/**
 * fill the file list
 */
void MainMenuLoadMap::fill_list() {
	//  Fill it with all files we find.
	m_mapfiles = g_fs->list_directory(m_curdir);

	//  First, we add all directories. We manually add the parent directory.
	if (m_curdir != m_basedir) {
#ifndef _WIN32
		m_parentdir = m_curdir.substr(0, m_curdir.rfind('/'));
#else
		m_parentdir = m_curdir.substr(0, m_curdir.rfind('\\'));
#endif

		m_ls->add
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
			 strcmp(FileSystem::fs_filename(name), "..")   &&
			 g_fs->is_directory(name)                       &&
			 !WidelandsMapLoader::is_widelands_map(name))

		m_ls->add
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
		std::unique_ptr<Widelands::MapLoader> map_loader = map.get_correct_loader(name);
		if (map_loader.get() != nullptr) {
			try {
				map_loader->preload_map(true);
				m_ls->add
					(FileSystem::filename_without_ext(name),
					 name,
					 g_gr->images().get
						 (dynamic_cast<WidelandsMapLoader*>(map_loader.get())
							? "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}

	if (m_ls->size())
		m_ls->select(0);
}
