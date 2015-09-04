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
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/editor_main_menu_save_map_make_directory.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_filesystem.h"
#include "map_io/map_saver.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

inline EditorInteractive & MainMenuSaveMap::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}


MainMenuSaveMap::MainMenuSaveMap(EditorInteractive & parent)
	: UI::Window(&parent, "save_map_menu", 0, 0, 560, 330, _("Save Map"))
{
	int32_t const spacing =  5;
	int32_t const offsx   = spacing;
	int32_t const offsy   = 10;
	int32_t posx          = offsx;
	int32_t posy          = offsy;
	int32_t const descr_label_w = 100;

	m_ls =
		new UI::Listselect<const char *>
			(this,
			 posx, posy,
			 get_inner_w() / 2 - spacing, get_inner_h() - spacing - offsy - 60);
	m_ls->clicked.connect(boost::bind(&MainMenuSaveMap::clicked_item, this, _1));
	m_ls->double_clicked.connect(boost::bind(&MainMenuSaveMap::double_clicked_item, this, _1));
	m_ls->focus();
	m_editbox =
		new UI::EditBox
			(this,
			 posx, posy + get_inner_h() - spacing - offsy - 60 + 3,
			 get_inner_w() / 2 - spacing, 20,
			 g_gr->images().get("pics/but1.png"), UI::Align::Align_Left);
	m_editbox->set_text(parent.egbase().map().get_name());
	m_editbox->changed.connect(boost::bind(&MainMenuSaveMap::edit_box_changed, this));

	posx = get_inner_w() / 2 + spacing;
	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Name:"), UI::Align_CenterLeft);
	m_name =
		new UI::MultilineTextarea
			(this, posx + descr_label_w, posy, 200, 40, "---", UI::Align_CenterLeft);
	posy += 40 + spacing;

	new UI::Textarea
		(this, posx, posy, descr_label_w, 20, _("Authors:"), UI::Align_CenterLeft);
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
		 _("OK"));
	m_ok_btn->sigclicked.connect(boost::bind(&MainMenuSaveMap::clicked_ok, boost::ref(*this)));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 posx + get_inner_w() / 4 - spacing / 2, posy,
		 get_inner_w() / 4 - 1.5 * spacing, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&MainMenuSaveMap::die, boost::ref(*this)));

	UI::Button * make_directorybtn = new UI::Button
		(this, "make_directory",
		 spacing, posy, 185, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Make Directory"));
	make_directorybtn->sigclicked.connect
		(boost::bind(&MainMenuSaveMap::clicked_make_directory, boost::ref(*this)));


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
void MainMenuSaveMap::clicked_ok() {
	assert(m_ok_btn->enabled());
	std::string filename = m_editbox->text();

	if (filename == "") //  Maybe a directory is selected.
		filename = m_ls->get_selected();

	if
		(g_fs->is_directory(filename.c_str())
		 &&
		 !Widelands::WidelandsMapLoader::is_widelands_map(filename))
	{
		m_curdir = g_fs->canonicalize_name(filename);
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	} else { //  Ok, save this map
		Widelands::Map & map = eia().egbase().map();
		if (map.get_name() != _("No Name")) {
			std::string::size_type const filename_size = filename.size();
			map.set_name
				((4 <= filename_size
				  && filename[filename_size - 1] == 'f'
				  && filename[filename_size - 2] == 'm'
				  && filename[filename_size - 3] == 'w'
				  && filename[filename_size - 4] == '.'
				  ?
				  filename.substr(0, filename_size - 4) : filename)
				 .c_str());
		}

		// check if map has at least two port spaces that are reachable for each other
		if (map.allows_seafaring())
			map.add_tag("seafaring");
		else
			map.delete_tag("seafaring");

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
void MainMenuSaveMap::clicked_make_directory() {
	MainMenuSaveMapMakeDirectory md(this, _("unnamed"));
	if (md.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		g_fs->ensure_directory_exists(m_basedir);
		//  create directory
		std::string fullname = m_curdir;
		fullname            += "/";
		fullname            += md.get_dirname();
		g_fs->make_directory(fullname);
		m_ls->clear();
		m_mapfiles.clear();
		fill_list();
	}
}

/**
 * called when an item was selected
 */
void MainMenuSaveMap::clicked_item(uint32_t) {
	const char * const name = m_ls->get_selected();

	if (Widelands::WidelandsMapLoader::is_widelands_map(name)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::MapLoader> const ml
				(map.get_correct_loader(name));
			ml->preload_map(true); // This has worked before, no problem
		}

		m_editbox->set_text(FileSystem::fs_filename(name));

		m_name  ->set_text(map.get_name       ());
		m_name  ->set_tooltip(map.get_name    ());
		m_author->set_text(map.get_author     ());
		m_descr ->set_text(map.get_description());

		m_nrplayers->set_text(std::to_string(static_cast<unsigned int>(map.get_nrplayers())));

		m_size->set_text((boost::format(_("%1$ix%2$i"))
								% map.get_width() % map.get_height()).str());
	} else {
		m_name     ->set_text(FileSystem::fs_filename(name));
		m_name     ->set_tooltip("");
		m_author   ->set_text("");
		m_nrplayers->set_text("");
		m_size     ->set_text("");
		if (g_fs->is_directory(name)) {
			m_name->set_tooltip((boost::format(_("Directory: %s"))
										% FileSystem::fs_filename(name)).str());
			m_descr->set_text((boost::format("\\<%s\\>") % _("directory")).str());
		} else {
			const std::string not_map_string = _("Not a map file");
			m_name->set_tooltip(not_map_string);
			m_descr->set_text((boost::format("\\<%s\\>") % not_map_string).str());
		}

	}
	edit_box_changed();
}

/**
 * An Item has been doubleclicked
 */
void MainMenuSaveMap::double_clicked_item(uint32_t) {
	const char * const name = m_ls->get_selected();

	if (g_fs->is_directory(name) && !Widelands::WidelandsMapLoader::is_widelands_map(name)) {
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
void MainMenuSaveMap::fill_list() {
	// Fill it with all files we find.
	m_mapfiles = g_fs->list_directory(m_curdir);

	// First, we add all directories. We manually add the parent directory
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
			 !Widelands::WidelandsMapLoader::is_widelands_map(name))

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

		// we do not list S2 files since we only write wmf
		std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(name));
		if (upcast(Widelands::WidelandsMapLoader, wml, ml.get())) {
			try {
				wml->preload_map(true);
				m_ls->add
					(FileSystem::filename_without_ext(name),
					 name,
					 g_gr->images().get("pics/ls_wlmap.png"));
			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}
	if (m_ls->size())
		m_ls->select(0);
}

/**
 * The editbox was changed. Enable ok button
 */
void MainMenuSaveMap::edit_box_changed() {
	m_ok_btn->set_enabled(m_editbox->text().size());
}

/**
 * Save the map in the current directory with
 * the current filename
 *
 * returns true if dialog should close, false if it
 * should stay open
 */
bool MainMenuSaveMap::save_map(std::string filename, bool binary) {
	//  Make sure that the base directory exists.
	g_fs->ensure_directory_exists(m_basedir);

	//  OK, first check if the extension matches (ignoring case).
	if (!boost::iends_with(filename, WLMF_SUFFIX))
		filename += WLMF_SUFFIX;

	//  append directory name
	std::string complete_filename = m_curdir;
	complete_filename            += "/";
	complete_filename            += filename;

	//  Check if file exists. If so, show a warning.
	if (g_fs->file_exists(complete_filename)) {
		std::string s =
			(boost::format(_("A file with the name ‘%s’ already exists. Overwrite?"))
				% FileSystem::fs_filename(filename.c_str())).str();
		UI::WLMessageBox mbox
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::MBoxType::kOkCancel);
		if (mbox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack)
			return false;

		g_fs->fs_unlink(complete_filename);
	}

	std::unique_ptr<FileSystem> fs
			(g_fs->create_sub_file_system(complete_filename, binary ? FileSystem::ZIP : FileSystem::DIR));
	Widelands::MapSaver wms(*fs, eia().egbase());
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
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
	}
	die();

	return true;
}
