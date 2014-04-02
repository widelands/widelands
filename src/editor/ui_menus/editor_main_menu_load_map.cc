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

#include <boost/format.hpp>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_set_starting_pos_tool.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "logic/world.h"
#include "map_io/map_loader.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "wexception.h"
#include "wui/overlay_manager.h"

using Widelands::WL_Map_Loader;

/**
 * Create all the buttons etc...
*/
Main_Menu_Load_Map::Main_Menu_Load_Map(Editor_Interactive & parent)
	: UI::Window(&parent, "load_map_menu", 0, 0, 500, 300, _("Load Map"))
{
	int32_t const spacing =  5;
	int32_t const offsx   = spacing;
	int32_t const offsy   = 30;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	m_ls = new UI::Listselect<const char *>
		(this,
		 posx, posy,
		 get_inner_w() / 2 - spacing, get_inner_h() - spacing - offsy - 40);
	m_ls->selected.connect(boost::bind(&Main_Menu_Load_Map::selected, this, _1));
	m_ls->double_clicked.connect(boost::bind(&Main_Menu_Load_Map::double_clicked, this, _1));

	posx = get_inner_w() / 2 + spacing;
	posy += 20;
	new UI::Textarea
		(this, posx, posy, 150, 20, _("Name:"), UI::Align_CenterLeft);
	m_name =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 150, 20, _("Author:"), UI::Align_CenterLeft);
	m_author =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 70, 20, _("Size:"), UI::Align_CenterLeft);
	m_size =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 70, 20, _("World:"), UI::Align_CenterLeft);
	m_world =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 70, 20, _("Players:"), UI::Align_CenterLeft);
	m_nrplayers =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", UI::Align_CenterLeft);
	posy += 20 + spacing;


	new UI::Textarea
		(this, posx, posy, 70, 20, _("Descr:"), UI::Align_CenterLeft);
	m_descr =
		new UI::Multiline_Textarea
			(this,
			 posx + 70, posy,
			 get_inner_w() - posx - spacing - 70,
			 get_inner_h() - posy - spacing - 40,
			 "---", UI::Align_CenterLeft);

	posy = get_inner_h() - 30;

	m_ok_btn = new UI::Button
		(this, "ok",
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 g_gr->images().get("pics/but0.png"),
		 _("OK"),
		 std::string(),
		 false);
	m_ok_btn->sigclicked.connect(boost::bind(&Main_Menu_Load_Map::clicked_ok, this));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 posx, posy, 80, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&Main_Menu_Load_Map::die, this));

	m_basedir = "maps";
	m_curdir  = "maps";

	fill_list();

	center_to_parent();
	move_to_top();
}


void Main_Menu_Load_Map::clicked_ok() {
	const char * const filename(m_ls->get_selected());

	if (g_fs->IsDirectory(filename) && !WL_Map_Loader::is_widelands_map(filename)) {
		m_curdir = filename;
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else {
		ref_cast<Editor_Interactive, UI::Panel>(*get_parent()).load(filename);
		die();
	}
}

/**
 * Called when a entry is selected
 */
void Main_Menu_Load_Map::selected(uint32_t) {
	const char * const name = m_ls->get_selected();

	m_ok_btn->set_enabled(true);

	if (!g_fs->IsDirectory(name) || WL_Map_Loader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::Map_Loader> map_loader = map.get_correct_loader(name);
			map_loader->preload_map(true); //  This has worked before, no problem.
		}

		// get translated worldsname
		std::string world(map.get_world_name());
		std::string worldpath("worlds/" + world);
		Profile prof((worldpath + "/conf").c_str(), nullptr, "world_" + world);
		Section & global = prof.get_safe_section("world");
		world = global.get_safe_string("name");

		// Translate the map data
		i18n::Textdomain td("maps");
		m_name  ->set_text(_(map.get_name()));
		m_author->set_text(map.get_author());
		m_descr ->set_text
			(_(map.get_description()) + (map.get_hint().empty() ? "" : (std::string("\n") + _(map.get_hint()))));
		m_world ->set_text(world);

		char buf[200];
		sprintf(buf, "%i", map.get_nrplayers());
		m_nrplayers->set_text(buf);

		sprintf(buf, "%ix%i", map.get_width(), map.get_height());
		m_size     ->set_text(buf);
	} else {
		m_name     ->set_text("");
		m_author   ->set_text("");
		m_descr    ->set_text("");
		m_world    ->set_text("");
		m_nrplayers->set_text("");
		m_size     ->set_text("");
	}
}

/**
 * An entry has been doubleclicked
 */
void Main_Menu_Load_Map::double_clicked(uint32_t) {clicked_ok();}

/**
 * fill the file list
 */
void Main_Menu_Load_Map::fill_list() {
	//  Fill it with all files we find.
	m_mapfiles = g_fs->ListDirectory(m_curdir);

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
		m_ls->add
			(parent_string.c_str(),
			 m_parentdir.c_str(),
			 g_gr->images().get("pics/ls_dir.png"));
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
			 strcmp(FileSystem::FS_Filename(name), "..")   and
			 g_fs->IsDirectory(name)                       and
			 not WL_Map_Loader::is_widelands_map(name))

		m_ls->add
			(FileSystem::FS_Filename(name),
			 name,
			 g_gr->images().get("pics/ls_dir.png"));
	}

	Widelands::Map map;

	for
		(filenameset_t::const_iterator pname = m_mapfiles.begin();
		 pname != mapfiles_end;
		 ++pname)
	{
		char const * const name = pname->c_str();
		std::unique_ptr<Widelands::Map_Loader> map_loader = map.get_correct_loader(name);
		if (map_loader.get() != nullptr) {
			try {
				map_loader->preload_map(true);
				m_ls->add
					(FileSystem::FS_Filename(name),
					 name,
					 g_gr->images().get
						 (dynamic_cast<WL_Map_Loader*>(map_loader.get())
								? "pics/ls_wlmap.png" : "pics/ls_s2map.png"));
			} catch (const _wexception &) {} //  we simply skip illegal entries
		}
	}

	if (m_ls->size())
		m_ls->select(0);
}
