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

#include "editor_main_menu_load_map.h"

#include "building.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "editor/editorinteractive.h"
#include "map_io/widelands_map_loader.h"
#include "wexception.h"
#include "editor_game_base.h"
#include "editor/tools/editor_set_starting_pos_tool.h"
#include "graphic/overlay_manager.h"
#include "world.h"
#include "map_io/map_loader.h"

#include "ui/ui_basic/ui_button.h"
#include "ui/ui_basic/ui_editbox.h"
#include "ui/ui_basic/ui_listselect.h"
#include "ui/ui_basic/ui_multilinetextarea.h"
#include "ui/ui_basic/ui_progresswindow.h"
#include "ui/ui_basic/ui_textarea.h"

#include <cstdio>

using Widelands::WL_Map_Loader;

/**
 * Create all the buttons etc...
*/
Main_Menu_Load_Map::Main_Menu_Load_Map(Editor_Interactive *parent)
:
	UI::Window(parent, 0, 0, 500, 300, _("Load Map"))
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
	m_ls->selected.set(this, &Main_Menu_Load_Map::selected);
	m_ls->double_clicked.set(this, &Main_Menu_Load_Map::double_clicked);

	posx = get_inner_w() / 2 + spacing;
	posy += 20;
	new UI::Textarea(this, posx, posy, 150, 20, _("Name: "), Align_CenterLeft);
	m_name =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 150, 20, _("Author: "), Align_CenterLeft);
	m_author =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea(this, posx, posy, 70, 20, _("Size: "), Align_CenterLeft);
	m_size =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea(this, posx, posy, 70, 20, _("World: "), Align_CenterLeft);
	m_world =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 70, 20, _("Players: "), Align_CenterLeft);
	m_nrplayers =
		new UI::Textarea
			(this, posx + 70, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;


	new UI::Textarea(this, posx, posy, 70, 20, _("Descr: "), Align_CenterLeft);
	m_descr =
		new UI::Multiline_Textarea
			(this,
			 posx + 70, posy,
			 get_inner_w() - posx - spacing - 70,
			 get_inner_h() - posy - spacing - 40,
			 "---", Align_CenterLeft);

	posx = 5;
	posy = get_inner_h() - 30;

	m_ok_btn = new UI::Callback_Button<Main_Menu_Load_Map>
		(this,
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 0,
		 &Main_Menu_Load_Map::clicked_ok, *this,
		 _("OK"),
		 std::string(),
		 false);

	new UI::Callback_Button<Main_Menu_Load_Map>
		(this,
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 1,
		 &Main_Menu_Load_Map::die, *this,
		 _("Cancel"));

	m_basedir = "maps";
	m_curdir  = "maps";

	fill_list();

	center_to_parent();
	move_to_top();
}


void Main_Menu_Load_Map::clicked_ok() {
	std::string filename(m_ls->get_selected());

	if
		(g_fs->IsDirectory(filename.c_str())
		 &&
		 !WL_Map_Loader::is_widelands_map(filename))
	{
		m_curdir = filename;
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else {
		dynamic_cast<Editor_Interactive &>(*get_parent()).load(filename);
		die();
	}
}

/**
 * Called when a item is selected
 */
void Main_Menu_Load_Map::selected(uint32_t) {
	const char * const name = m_ls->get_selected();

	m_ok_btn->set_enabled(true);

	if (!g_fs->IsDirectory(name) || WL_Map_Loader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			Widelands::Map_Loader * const m_ml = map.get_correct_loader(name);
			m_ml->preload_map(true); //  This has worked before, no problem.
			delete m_ml;
		}

		m_name  ->set_text(map.get_name       ());
		m_author->set_text(map.get_author     ());
		m_descr ->set_text(map.get_description());
		m_world ->set_text(map.get_world_name ());

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
 * An Item has been doubleclicked
 */
void Main_Menu_Load_Map::double_clicked(uint32_t) {clicked_ok();}

/**
 * fill the file list
 */
void Main_Menu_Load_Map::fill_list() {
	//  Fill it with all files we find.
	g_fs->FindFiles(m_curdir, "*", &m_mapfiles, 1);

	//  First, we add all directories. We manually add the parent directory.
	if (m_curdir != m_basedir) {
#ifndef WIN32
		m_parentdir = m_curdir.substr(0, m_curdir.rfind("/"));
#else
		m_parentdir = m_curdir.substr(0, m_curdir.rfind("\\"));
#endif
		m_ls->add
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
			 // aesthetic reasons
			 strcmp(FileSystem::FS_Filename(name), ".svn") and
			 g_fs->IsDirectory(name)                       and
			 not WL_Map_Loader::is_widelands_map(name))

		m_ls->add
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
				m_ls->add
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

	if (m_ls->size()) m_ls->select(0);
}
