/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_save_map.h"

#include <cstdio>
#include <cstring>
#include <string>

#include <boost/format.hpp>

#include "constants.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/editor_main_menu_save_map_make_directory.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_filesystem.h"
#include "map_io/widelands_map_loader.h"
#include "map_io/widelands_map_saver.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "upcast.h"
#include "wexception.h"

inline Editor_Interactive & Main_Menu_Save_Map::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Main_Menu_Save_Map::Main_Menu_Save_Map(Editor_Interactive & parent)
	: UI::Window(&parent, "save_map_menu", 0, 0, 500, 330, _("Save Map"))
{
	int32_t const spacing =  5;
	int32_t const offsx   = spacing;
	int32_t const offsy   = 30;
	int32_t posx          = offsx;
	int32_t posy          = offsy;

	m_ls =
		new UI::Listselect<const char *>
			(this,
			 posx, posy,
			 get_inner_w() / 2 - spacing, get_inner_h() - spacing - offsy - 60);
	m_ls->clicked.connect(boost::bind(&Main_Menu_Save_Map::clicked_item, this, _1));
	m_ls->double_clicked.connect(boost::bind(&Main_Menu_Save_Map::double_clicked_item, this, _1));
	m_editbox =
		new UI::EditBox
			(this,
			 posx, posy + get_inner_h() - spacing - offsy - 60 + 3,
			 get_inner_w() / 2 - spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_editbox->setText(parent.egbase().map().get_name());
	m_editbox->changed.connect(boost::bind(&Main_Menu_Save_Map::edit_box_changed, this));

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
		(this, posx, posy, 70, 20, _("Descr: "), UI::Align_CenterLeft);
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
		 _("OK"));
	m_ok_btn->sigclicked.connect(boost::bind(&Main_Menu_Save_Map::clicked_ok, boost::ref(*this)));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 posx, posy, 80, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&Main_Menu_Save_Map::die, boost::ref(*this)));

	UI::Button * make_directorybtn = new UI::Button
		(this, "make_directory",
		 spacing, posy, 120, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Make Directory"));
	make_directorybtn->sigclicked.connect
		(boost::bind(&Main_Menu_Save_Map::clicked_make_directory, boost::ref(*this)));


	m_basedir = "maps";
	m_curdir  = "maps";

	fill_list();
	edit_box_changed();

	center_to_parent();
	move_to_top();
}

/**
 * Called when the ok button was pressed or a file in list was double clicked.
 */
void Main_Menu_Save_Map::clicked_ok() {
	assert(m_ok_btn->enabled());
	std::string filename = m_editbox->text();

	if (filename == "") //  Maybe a directory is selected.
		filename = m_ls->get_selected();

	if
		(g_fs->IsDirectory(filename.c_str())
		 &&
		 !Widelands::WL_Map_Loader::is_widelands_map(filename))
	{
		m_curdir = g_fs->FS_CanonicalizeName(filename);
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else { //  Ok, save this map
		Widelands::Map & map = eia().egbase().map();
		if (not strcmp(map.get_name(), _("No Name"))) {
			std::string::size_type const filename_size = filename.size();
			map.set_name
				((4 <= filename_size
				  and filename[filename_size - 1] == 'f'
				  and filename[filename_size - 2] == 'm'
				  and filename[filename_size - 3] == 'w'
				  and filename[filename_size - 4] == '.'
				  ?
				  filename.substr(0, filename_size - 4) : filename)
				 .c_str());
		}
		if
			(save_map
			 	(filename,
			 	 ! g_options.pull_section("global").get_bool("nozip", false)))
			die();
	}
}

/**
 * Called, when the make directory button was clicked.
 */
void Main_Menu_Save_Map::clicked_make_directory() {
	Main_Menu_Save_Map_Make_Directory md(this, _("unnamed"));
	if (md.run()) {
		g_fs->EnsureDirectoryExists(m_basedir);
		//  create directory
		std::string fullname = m_curdir;
		fullname            += "/";
		fullname            += md.get_dirname();
		g_fs->MakeDirectory(fullname);
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	}
}

/**
 * called when an item was selected
 */
void Main_Menu_Save_Map::clicked_item(uint32_t) {
	const char * const name = m_ls->get_selected();

	if (Widelands::WL_Map_Loader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::Map_Loader> const ml
				(map.get_correct_loader(name));
			ml->preload_map(true); // This has worked before, no problem
		}

		m_editbox->setText(FileSystem::FS_Filename(name));

		m_name  ->set_text(map.get_name       ());
		m_author->set_text(map.get_author     ());
		m_descr ->set_text(map.get_description());
		m_world ->set_text(map.get_world_name ());

		char buf[200];
		sprintf(buf, "%i", map.get_nrplayers());
		m_nrplayers->set_text(buf);

		sprintf(buf, "%ix%i", map.get_width(), map.get_height());
		m_size->set_text(buf);
	} else {
		m_name     ->set_text(FileSystem::FS_Filename(name));
		m_author   ->set_text("");
		m_world    ->set_text("");
		m_nrplayers->set_text("");
		m_size     ->set_text("");
		if (g_fs->IsDirectory(name)) {
			std::string dir_string =
				(boost::format("\\<%s\\>") % _("directory")).str();
			m_descr    ->set_text(dir_string);
		} else {
			std::string not_map_string =
				(boost::format("\\<%s\\>") % _("Not a map file")).str();
			m_descr    ->set_text(not_map_string);
		}

	}
	edit_box_changed();
}

/**
 * An Item has been doubleclicked
 */
void Main_Menu_Save_Map::double_clicked_item(uint32_t) {
	const char * const name = m_ls->get_selected();

	if (g_fs->IsDirectory(name) && !Widelands::WL_Map_Loader::is_widelands_map(name)) {
		m_curdir = name;
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else
		clicked_ok();
}

/**
 * fill the file list
 */
void Main_Menu_Save_Map::fill_list() {
	// Fill it with all files we find.
	m_mapfiles = g_fs->ListDirectory(m_curdir);

	// First, we add all directories. We manually add the parent directory
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
			 not Widelands::WL_Map_Loader::is_widelands_map(name))

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

		// we do not list S2 files since we only write wmf
		std::unique_ptr<Widelands::Map_Loader> ml(map.get_correct_loader(name));
		if (upcast(Widelands::WL_Map_Loader, wml, ml.get())) {
			try {
				wml->preload_map(true);
				m_ls->add
					(FileSystem::FS_Filename(name),
					 name,
					 g_gr->images().get("pics/ls_wlmap.png"));
			} catch (const _wexception &) {} //  we simply skip illegal entries
		}
	}
	if (m_ls->size())
		m_ls->select(0);
}

/**
 * The editbox was changed. Enable ok button
 */
void Main_Menu_Save_Map::edit_box_changed() {
	m_ok_btn->set_enabled(m_editbox->text().size());
}

/**
 * Save the map in the current directory with
 * the current filename
 *
 * returns true if dialog should close, false if it
 * should stay open
 */
bool Main_Menu_Save_Map::save_map(std::string filename, bool binary) {
	//  Make sure that the base directory exists.
	g_fs->EnsureDirectoryExists(m_basedir);

	//  OK, first check if the extension matches (ignoring case).
	bool assign_extension = true;
	if (filename.size() >= strlen(WLMF_SUFFIX)) {
		char buffer[10]; //  enough for the extension
		filename.copy
			(buffer, sizeof(WLMF_SUFFIX), filename.size() - strlen(WLMF_SUFFIX));
		if (!strncasecmp(buffer, WLMF_SUFFIX, strlen(WLMF_SUFFIX)))
			assign_extension = false;
	}
	if (assign_extension)
		filename += WLMF_SUFFIX;

	//  append directory name
	std::string complete_filename = m_curdir;
	complete_filename            += "/";
	complete_filename            += filename;

	//  Check if file exists. If so, show a warning.
	if (g_fs->FileExists(complete_filename)) {
		std::string s =
			(boost::format(_("A file with the name ‘%s’ already exists. Overwrite?"))
				% FileSystem::FS_Filename(filename.c_str())).str();
		UI::WLMessageBox mbox
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::YESNO);
		if (not mbox.run())
			return false;

		g_fs->Unlink(complete_filename);
	}

	std::unique_ptr<FileSystem> fs
			(g_fs->CreateSubFileSystem(complete_filename, binary ? FileSystem::ZIP : FileSystem::DIR));
	Widelands::Map_Saver wms(*fs, eia().egbase());
	try {
		wms.save();
		eia().set_need_save(false);
	} catch (const std::exception & e) {
		std::string s =
			_
			("Error Saving Map!\nSaved map file may be corrupt!\n\nReason "
			 "given:\n");
		s += e.what();
		UI::WLMessageBox  mbox
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::OK);
		mbox.run();
	}
	die();

	return true;
}
