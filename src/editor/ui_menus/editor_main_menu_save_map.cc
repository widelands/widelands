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
	: UI::Window(&parent, "save_map_menu",
					 0, 0, parent.get_inner_w() - 80, parent.get_inner_h() - 80,
					 _("Save Map")),

	  // Values for alignment and size
	  padding_(5),
	  butw_(get_inner_w() / 4 - 1.5 * padding_),
	  buth_(20),
	  details_box_x_(get_inner_w() * 2 / 3),
	  details_box_y_(2 * padding_),
	  details_box_w_(get_inner_w() - details_box_x_ - 2 * padding_),
	  details_box_h_(get_inner_h() - 2 * details_box_y_ - 2 * buth_ - 4 * padding_),
	  details_label_w_(details_box_w_ / 2 - padding_),

	  details_box_(this, details_box_x_, details_box_y_, UI::Box::Vertical,
			 details_box_w_, details_box_h_, padding_ / 2),

	  name_box_(&details_box_, 0, 0, UI::Box::Horizontal,
			 details_box_w_, 3 * buth_ + padding_, padding_ / 2),
	  name_label_(&details_box_, 0, 0, details_label_w_, buth_, _("Name:")),
	  name_(&name_box_, 0, 0, details_box_w_ - padding_, 2 * buth_, "---"),

	  author_box_(&details_box_, 0, 0, UI::Box::Horizontal,
			 details_box_w_, buth_ + padding_, padding_ / 2),
	  author_label_(&details_box_, 0, 0, details_label_w_, buth_, _("Authors:")),
	  author_(&author_box_, 0, 0, details_box_w_ - padding_, buth_, "---"),

	  size_box_(&details_box_, 0, 0, UI::Box::Horizontal,
			 details_box_w_, buth_ + padding_, padding_ / 2),
	  size_label_(&size_box_, 0, 0, details_label_w_, buth_, _("Size:")),
	  size_(&size_box_, 0, 0, details_box_w_ - details_label_w_ - padding_, buth_, "---"),

	  nrplayers_box_(&details_box_, 0, 0, UI::Box::Horizontal,
			 details_box_w_, buth_ + padding_, padding_ / 2),
	  nrplayers_label_(&nrplayers_box_, 0, 0, details_label_w_, buth_, _("Players:")),
	  nrplayers_(&nrplayers_box_, 0, 0, details_box_w_ - details_label_w_ - padding_, buth_, "---"),

	  descr_box_(&details_box_, 0, 0, UI::Box::Horizontal,
			 details_box_w_, buth_ + padding_, padding_ / 2),
	  descr_label_(&details_box_, 0, 0, details_box_w_, buth_, _("Descr:")),
	  descr_(&descr_box_, padding_, 0, details_box_w_ + padding_, 5 * buth_, "---"),

	  list_box_(this, padding_, details_box_y_, UI::Box::Vertical,
			 details_box_x_ - padding_, details_box_h_, padding_ / 2),

	  editbox_label_(this, padding_, details_box_y_ + details_box_h_ + 3 * padding_,
						  details_box_w_, buth_, _("Filename:"), UI::Align::Align_Right),

	  m_table(&list_box_, 0, 0, details_box_x_ - padding_, details_box_h_)
{

	m_table.add_column(m_table.get_w() / 2,
							 _("Filename"),
							 _("The filename that the map was saved under."),
								 UI::Align_Left);
	m_table.add_column(m_table.get_w() / 2,
							 _("Map Name"),
							 _("The map’s name."),
								 UI::Align_Left);

	m_table.selected.connect(boost::bind(&MainMenuSaveMap::clicked_item, boost::ref(*this)));
	m_table.double_clicked.connect(boost::bind(&MainMenuSaveMap::double_clicked_item, boost::ref(*this)));
	m_table.set_column_compare
		(0,
		 boost::bind(&MainMenuSaveMap::compare_filenames, this, _1, _2));
	m_table.set_column_compare
		(1,
		 boost::bind(&MainMenuSaveMap::compare_mapnames, this, _1, _2));
	m_table.set_sort_column(0);

	m_table.focus();

	editbox_ = new UI::EditBox(this, details_box_x_, details_box_y_ + details_box_h_ + 3 * padding_,
										details_box_w_, buth_,
										g_gr->images().get("pics/but1.png"), UI::Align::Align_Left);

	details_box_.add(&name_label_, UI::Box::AlignLeft);
	name_box_.add_space(2 * padding_);
	name_box_.add(&name_, UI::Box::AlignLeft);
	details_box_.add(&name_box_, UI::Box::AlignLeft);
	details_box_.add_space(padding_);

	details_box_.add(&author_label_, UI::Box::AlignLeft);
	author_box_.add_space(2 * padding_);
	author_box_.add(&author_, UI::Box::AlignLeft);
	details_box_.add(&author_box_, UI::Box::AlignLeft);
	details_box_.add_space(padding_);

	size_box_.add(&size_label_, UI::Box::AlignLeft);
	size_box_.add(&size_, UI::Box::AlignLeft);
	details_box_.add(&size_box_, UI::Box::AlignLeft);
	details_box_.add_space(padding_);

	nrplayers_box_.add(&nrplayers_label_, UI::Box::AlignLeft);
	nrplayers_box_.add(&nrplayers_, UI::Box::AlignLeft);
	details_box_.add(&nrplayers_box_, UI::Box::AlignLeft);
	details_box_.add_space(padding_);

	details_box_.add(&descr_label_, UI::Box::AlignLeft);
	descr_box_.add_space(2 * padding_);
	descr_box_.add(&descr_, UI::Box::AlignLeft);
	details_box_.add(&descr_box_, UI::Box::AlignLeft);
	details_box_.add_space(padding_);

	details_box_.set_size(details_box_w_, get_h() - details_box_.get_y());
	descr_box_.set_size(details_box_.get_inner_w(),
							  details_box_h_ - descr_label_.get_h() - descr_label_.get_y());
	descr_.set_size(descr_box_.get_inner_w() - 3 * padding_, descr_box_.get_inner_h());

	list_box_.add(&m_table, UI::Box::AlignLeft);
	list_box_.set_size(details_box_x_ - padding_, get_h() - list_box_.get_y());

	editbox_->set_text(parent.egbase().map().get_name());
	editbox_->changed.connect(boost::bind(&MainMenuSaveMap::edit_box_changed, this));

	m_ok_btn = new UI::Button
		(this, "ok",
		 get_inner_w() * 3 / 4 + padding_, get_inner_h() - padding_ - buth_,
		 butw_, buth_,
		 g_gr->images().get("pics/but0.png"),
		 _("OK"));
	m_ok_btn->sigclicked.connect(boost::bind(&MainMenuSaveMap::clicked_ok, boost::ref(*this)));

	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 get_inner_w() * 1 / 2 + padding_, get_inner_h() - padding_ - buth_,
		 butw_, buth_,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&MainMenuSaveMap::die, boost::ref(*this)));

	UI::Button * make_directorybtn = new UI::Button
		(this, "make_directory",
		 padding_, get_inner_h() - padding_ - buth_,
		 185, buth_,
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
	std::string filename = editbox_->text();

	if (filename == "") { //  Maybe a directory is selected.
		filename = m_maps_data[m_table.get_selected()].filename;
	}

	if
		(g_fs->is_directory(filename.c_str())
		 &&
		 !Widelands::WidelandsMapLoader::is_widelands_map(filename))
	{
		m_curdir = g_fs->canonicalize_name(filename);
		m_table.clear();
		m_maps_data.clear();
		fill_list();
	} else { //  Ok, save this map
		Widelands::Map & map = eia().egbase().map();
		if (map.get_name() != _("No Name")) {
			std::string::size_type const filename_size = filename.size();
			map.set_name
				(4 <= filename_size && boost::iends_with(filename, WLMF_SUFFIX) ?
				  filename.substr(0, filename_size - 4) : filename);
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
void MainMenuSaveMap::clicked_make_directory() {
	// NOCOM make this much bigger
	MainMenuSaveMapMakeDirectory md(this, _("unnamed"));
	if (md.run()) {
		g_fs->ensure_directory_exists(m_basedir);
		//  create directory
		std::string fullname = m_curdir;
		fullname            += "/";
		fullname            += md.get_dirname();
		g_fs->make_directory(fullname);
		m_table.clear();
		fill_list();
	}
}

/**
 * called when an item was selected
 */
void MainMenuSaveMap::clicked_item() {
	const SaveMapData& mapdata = m_maps_data[m_table.get_selected()];
	const std::string& filename = m_maps_data[m_table.get_selected()].filename;

	if (Widelands::WidelandsMapLoader::is_widelands_map(filename)) {
		Widelands::Map map;
		{
			std::unique_ptr<Widelands::MapLoader> const ml
				(map.get_correct_loader(filename));
			ml->preload_map(true); // This has worked before, no problem
		}

		editbox_->set_text(FileSystem::fs_filename(filename.c_str()));

		{
			i18n::Textdomain td("maps");
			const std::string& localized_name = _(mapdata.mapname);
			if (localized_name == mapdata.mapname) {
				name_.set_text(mapdata.mapname);
			} else {
				name_.set_text((boost::format("%s (%s)") % mapdata.mapname % localized_name).str());
			}
		}
		name_.set_tooltip(mapdata.mapname);
		author_.set_text(mapdata.authors);
		descr_.set_text(mapdata.description);

		nrplayers_.set_text(std::to_string(mapdata.nrplayers));

		size_.set_text((boost::format(_("%1$ix%2$i"))
							 % mapdata.width % mapdata.height).str());
	} else {
		name_.set_text(FileSystem::fs_filename(filename.c_str()));
		name_.set_tooltip("");
		author_.set_text("");
		nrplayers_.set_text("");
		size_.set_text("");
		if (g_fs->is_directory(filename)) {
			name_.set_tooltip((boost::format(_("Directory: %s"))
									 % FileSystem::fs_filename(filename.c_str())).str());
			descr_.set_text((boost::format("\\<%s\\>") % _("directory")).str());
		} else {
			const std::string not_map_string = _("Not a map file");
			name_.set_tooltip(not_map_string);
			descr_.set_text((boost::format("\\<%s\\>") % not_map_string).str());
		}

	}
	edit_box_changed();
}

/**
 * An Item has been doubleclicked
 */
void MainMenuSaveMap::double_clicked_item() {
	const std::string& filename = m_maps_data[m_table.get_selected()].filename;

	if (g_fs->is_directory(filename) && !Widelands::WidelandsMapLoader::is_widelands_map(filename)) {
		m_curdir = filename;
		m_table.clear();
		m_maps_data.clear();
		fill_list();
	} else
		clicked_ok();
}


bool MainMenuSaveMap::compare_filenames(uint32_t rowa, uint32_t rowb)
{ // NOCOM fixme
	const SaveMapData & r1 = m_maps_data[m_table[rowa]];
	const SaveMapData & r2 = m_maps_data[m_table[rowb]];

	if (r1.isdir && !r2.isdir) {
		return true;
	}
	return r1.filename < r2.filename;
}

bool MainMenuSaveMap::compare_mapnames(uint32_t rowa, uint32_t rowb)
{ // NOCOM fixme
	const SaveMapData & r1 = m_maps_data[m_table[rowa]];
	const SaveMapData & r2 = m_maps_data[m_table[rowb]];

	if (r1.isdir && !r2.isdir) {
		return true;
	}
	return r1.mapname < r2.mapname;
}


/**
 * fill the file list
 */
void MainMenuSaveMap::fill_list() {

	//  Fill it with all files we find in all directories.
	FilenameSet files = g_fs->list_directory(m_curdir);

	// First, we add all directories. We manually add the parent directory
	if (m_curdir != m_basedir) {
		SaveMapData mapdata;
#ifndef _WIN32
		mapdata.filename = m_curdir.substr(0, m_curdir.rfind('/'));
#else
		mapdata.filename = m_curdir.substr(0, m_curdir.rfind('\\'));
#endif
		mapdata.isdir = true;
		m_maps_data.push_back(mapdata);
		UI::Table<uintptr_t const>::EntryRecord& te = m_table.add(m_maps_data.size() - 1);
		/** TRANSLATORS: Parent directory */
		te.set_picture(0, g_gr->images().get("pics/ls_dir.png"), (boost::format("\\<%s\\>") % _("parent")).str());
		te.set_string(1, "");
	}

	//Add subdirectories to the list (except for uncompressed maps)
	for (const std::string& mapfilename : files) {
		char const * const name = mapfilename.c_str();
		if (!strcmp(FileSystem::fs_filename(name), "."))
			continue;
		// Upsy, appeared again. ignore
		if (!strcmp(FileSystem::fs_filename(name), ".."))
			continue;
		if (!g_fs->is_directory(name))
			continue;
		if (Widelands::WidelandsMapLoader::is_widelands_map(name))
			continue;

		SaveMapData mapdata;
		mapdata.filename = name;
		mapdata.isdir = true;

		m_maps_data.push_back(mapdata);
		UI::Table<uintptr_t const>::EntryRecord& te = m_table.add(m_maps_data.size() - 1);
		te.set_picture(0, g_gr->images().get("pics/ls_dir.png"), FileSystem::fs_filename(name));
		te.set_string(1, "");
	}


	Widelands::Map map;

	for (const std::string& pname : files) {
		char const * const name = pname.c_str();

		// we do not list S2 files since we only write wmf
		std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(pname));

		if (upcast(Widelands::WidelandsMapLoader, wml, ml.get())) {
			try {
				wml->preload_map(true);
				SaveMapData mapdata;
				mapdata.filename    = pname;
				mapdata.mapname     = map.get_name();
				mapdata.authors     = map.get_author();
				mapdata.description = map.get_description();
				mapdata.nrplayers   = map.get_nrplayers();
				mapdata.width       = map.get_width();
				mapdata.height      = map.get_height();
				mapdata.isdir       = false;

				// Finally write the entry to the list
				m_maps_data.push_back(mapdata);
				UI::Table<uintptr_t const>::EntryRecord& te = m_table.add(m_maps_data.size() - 1);
				te.set_picture(0, g_gr->images().get("pics/ls_wlmap.png"), FileSystem::fs_filename(name));
				te.set_string(1, mapdata.mapname);

			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}
	m_table.sort();

	if (m_table.size()) {
		m_table.select(0);
	}
}

/**
 * The editbox was changed. Enable ok button
 */
void MainMenuSaveMap::edit_box_changed() {
	m_ok_btn->set_enabled(!editbox_->text().empty());
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
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::YESNO);
		if (!mbox.run())
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
			(&eia(), _("Error Saving Map!"), s, UI::WLMessageBox::OK);
		mbox.run();
	}
	die();

	return true;
}
