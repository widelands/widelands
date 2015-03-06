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
#include "editor/ui_menus/editor_main_menu_map_options.h"
#include "editor/ui_menus/editor_main_menu_save_map_make_directory.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_filesystem.h"
#include "map_io/map_saver.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "ui_basic/messagebox.h"

inline EditorInteractive & MainMenuSaveMap::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

MainMenuSaveMap::MainMenuSaveMap(EditorInteractive & parent)
	: UI::Window(&parent, "save_map_menu",
					 0, 0, parent.get_inner_w() - 40, parent.get_inner_h() - 40,
					 _("Save Map")),

	  // Values for alignment and size
	  padding_(4),
	  buth_(20),
	  tablex_(padding_),
	  tabley_(buth_ + 2 * padding_),
	  tablew_(get_inner_w() * 7 / 12),
	  tableh_(get_inner_h() - tabley_ - 3 * buth_ - 2 * padding_),
	  right_column_x_(tablew_ + 2 * padding_),
	  butw_((get_inner_w() - right_column_x_ - 2 * padding_) / 2),

	  table_(this, tablex_, tabley_, tablew_, tableh_, false),
	  map_details_(
		  this, right_column_x_, tabley_,
		  get_inner_w() - right_column_x_ - padding_,
		  tableh_ - buth_ - padding_),
	  ok_(
		  this, "ok",
		  get_inner_w() / 2  - butw_ - padding_, get_inner_h() - padding_ - buth_,
		  butw_, buth_,
		  g_gr->images().get("pics/but0.png"),
		  _("OK")),
	  cancel_(
		  this, "cancel",
		  get_inner_w() / 2  + padding_, get_inner_h() - padding_ - buth_,
		  butw_, buth_,
		  g_gr->images().get("pics/but1.png"),
		  _("Cancel")),
	  make_directory_(
		  this, "make_directory",
		  right_column_x_, tabley_ + tableh_ + 3 * padding_ - 1,
		  get_inner_w() - right_column_x_ - padding_, buth_,
		  g_gr->images().get("pics/but1.png"),
		  _("Make Directory")),
	  edit_options_(
		  this, "edit_options",
		  right_column_x_, tabley_ + tableh_ - buth_,
		  get_inner_w() - right_column_x_ - padding_, buth_,
		  g_gr->images().get("pics/but0.png"),
		  _("Map Options")),
	  editbox_label_(this, padding_, tabley_ + tableh_ + 3 * padding_,
						  butw_, buth_,
						  _("Filename:"), UI::Align::Align_Left),
	  basedir_("maps"),
	  has_translated_mapname_(false),
	  showing_mapames_(false) {
	curdir_ = basedir_;

	UI::Box* vbox = new UI::Box(this, tablex_, padding_,
										 UI::Box::Horizontal, padding_, get_w());
	show_mapnames_ = new UI::Button(vbox, "show_mapnames", 0, 0, butw_, buth_,
												g_gr->images().get("pics/but1.png"),
												_("Show Map Names"));
	show_mapnames_->sigclicked.connect
			(boost::bind(&MainMenuSaveMap::toggle_mapnames, boost::ref(*this)));
	vbox->add(show_mapnames_, UI::Box::AlignLeft, true);

	cb_dont_localize_mapnames_ = new UI::Checkbox(vbox, Point(0, 0));
	cb_dont_localize_mapnames_->set_state(false);
	cb_dont_localize_mapnames_->changedto.connect
			(boost::bind(&MainMenuSaveMap::fill_table, boost::ref(*this)));
	vbox->add_space(2 * padding_);
	vbox->add(cb_dont_localize_mapnames_, UI::Box::AlignLeft, true);
	vbox->add_space(padding_);
	UI::Textarea * ta_dont_localize_mapnames =
			/** TRANSLATORS: Checkbox title. If this checkbox is enabled, map names aren't translated. */
			new UI::Textarea(vbox, _("Show original map names"), UI::Align_CenterLeft);
	vbox->add(ta_dont_localize_mapnames, UI::Box::AlignLeft);
	vbox->set_size(get_inner_w(), buth_);

	table_.selected.connect(boost::bind(&MainMenuSaveMap::clicked_item, boost::ref(*this)));
	table_.double_clicked.connect(boost::bind(&MainMenuSaveMap::double_clicked_item, boost::ref(*this)));
	table_.focus();
	fill_table();

	editbox_ = new UI::EditBox(
					  this, editbox_label_.get_x() + editbox_label_.get_w() + padding_, editbox_label_.get_y(),
					  tablew_ - editbox_label_.get_w() - padding_ + 1, buth_,
					  g_gr->images().get("pics/but1.png"), UI::Align::Align_Left);

	editbox_->set_text(parent.egbase().map().get_name());
	editbox_->changed.connect(boost::bind(&MainMenuSaveMap::edit_box_changed, this));
	edit_box_changed();

	ok_.sigclicked.connect(boost::bind(&MainMenuSaveMap::clicked_ok, boost::ref(*this)));
	cancel_.sigclicked.connect(boost::bind(&MainMenuSaveMap::die, boost::ref(*this)));
	make_directory_.sigclicked.connect
		(boost::bind(&MainMenuSaveMap::clicked_make_directory, boost::ref(*this)));
	edit_options_.sigclicked.connect
		(boost::bind(&MainMenuSaveMap::clicked_edit_options, boost::ref(*this)));

	// We always want the current map's data here
	const Widelands::Map& map = parent.egbase().map();
	MapData::MapType maptype;

	if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
		 map.scenario_types() & Widelands::Map::SP_SCENARIO) {
		maptype = MapData::MapType::kScenario;
	} else {
		maptype = MapData::MapType::kNormal;
	}

	MapData mapdata(map, "", maptype);

	map_details_.update(mapdata, false);

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	ta_dont_localize_mapnames->set_visible(has_translated_mapname_);

	center_to_parent();
	move_to_top();
}

/**
 * Called when the ok button was pressed or a file in list was double clicked.
 */
void MainMenuSaveMap::clicked_ok() {
	assert(ok_.enabled());
	std::string filename = editbox_->text();

	if (filename == "") { //  Maybe a directory is selected.
		filename = table_.get_map()->filename;
	}

	if (g_fs->is_directory(filename.c_str())
		 &&
		 !Widelands::WidelandsMapLoader::is_widelands_map(filename)) {
		curdir_ = g_fs->canonicalize_name(filename);
		fill_table();
	} else { //  Ok, save this map
		Widelands::Map& map = eia().egbase().map();
		if (map.get_name() == _("No Name")) {
			std::string::size_type const filename_size = filename.size();
			map.set_name
				(4 <= filename_size && boost::iends_with(filename, WLMF_SUFFIX) ?
				  filename.substr(0, filename_size - 4) : filename);
		}
		if (save_map(filename, !g_options.pull_section("global").get_bool("nozip", false))) {
			die();
		}
	}
}

/**
 * Called, when the make directory button was clicked.
 */
void MainMenuSaveMap::clicked_make_directory() {
	MainMenuSaveMapMakeDirectory md(this, _("unnamed"));
	if (md.run()) {
		g_fs->ensure_directory_exists(basedir_);
		//  create directory
		std::string fullname = curdir_;
		fullname            += "/";
		fullname            += md.get_dirname();
		g_fs->make_directory(fullname);
		fill_table();
	}
}

void MainMenuSaveMap::clicked_edit_options() {
	EditorInteractive& parent = dynamic_cast<EditorInteractive&>(*get_parent());
	MainMenuMapOptions mo(parent, true);
	if (mo.run()) {
		const Widelands::Map& map = parent.egbase().map();
		MapData::MapType maptype;

		if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
			 map.scenario_types() & Widelands::Map::SP_SCENARIO) {
			maptype = MapData::MapType::kScenario;
		} else {
			maptype = MapData::MapType::kNormal;
		}

		MapData mapdata(map, editbox_->text(), maptype);

		map_details_.update(mapdata, false);
	}
}

/**
 * called when an item was selected
 */
void MainMenuSaveMap::clicked_item() {
	// Only change editbox contents
	if (table_.has_selection()) {
		const MapData& mapdata = *table_.get_map();
		if (mapdata.maptype != MapData::MapType::kDirectory) {
			editbox_->set_text(FileSystem::fs_filename(table_.get_map()->filename.c_str()));
			edit_box_changed();
		}
	}
}

/**
 * An Item has been doubleclicked
 */
void MainMenuSaveMap::double_clicked_item() {
	const MapData& mapdata = *table_.get_map();
	if (mapdata.maptype == MapData::MapType::kDirectory) {
		curdir_ = mapdata.filename;
		fill_table();
	} else {
		clicked_ok();
	}
}

void MainMenuSaveMap::toggle_mapnames() {
	if (showing_mapames_) {
		show_mapnames_->set_title(_("Show Map Names"));
	} else {
		show_mapnames_->set_title(_("Show Filenames"));
	}
	showing_mapames_ = !showing_mapames_;
	fill_table();
}


/**
 * fill the file list
 */
void MainMenuSaveMap::fill_table() {
	std::vector<MapData> maps_data;

	//  Fill it with all files we find.
	FilenameSet files = g_fs->list_directory(curdir_);

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		maps_data.push_back(MapData::create_parent_dir(curdir_));
	}

	Widelands::Map map;

	for (const std::string& mapfilename : files) {

		// Add map file (compressed) or map directory (uncompressed)
		if (Widelands::WidelandsMapLoader::is_widelands_map(mapfilename)) {
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
			if (ml.get() != nullptr) {
				try {
					ml->preload_map(true);

					if (!map.get_width() || !map.get_height()) {
						continue;
					}

					MapData::MapType maptype;

					if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
						 map.scenario_types() & Widelands::Map::SP_SCENARIO) {
						maptype = MapData::MapType::kScenario;
						} else if (dynamic_cast<Widelands::WidelandsMapLoader*>(ml.get())) {
						maptype = MapData::MapType::kNormal;
					} else {
						maptype = MapData::MapType::kSettlers2;
					}

					MapData mapdata(map, mapfilename, maptype);

					has_translated_mapname_ =
							has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

					maps_data.push_back(mapdata);

				} catch (const WException &) {} //  we simply skip illegal entries
			}
		} else if (g_fs->is_directory(mapfilename)) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, ".."))
				continue;
			maps_data.push_back(MapData::create_directory(mapfilename));
		}
	}
	if (!showing_mapames_) {
		table_.fill(maps_data, MapTable::Type::kFilenames);
	} else if (cb_dont_localize_mapnames_->get_state()) {
		table_.fill(maps_data, MapTable::Type::kMapnames);
	} else {
		table_.fill(maps_data, MapTable::Type::kMapnamesLocalized);
	}
}

/**
 * The editbox was changed. Enable ok button
 */
void MainMenuSaveMap::edit_box_changed() {
	ok_.set_enabled(!editbox_->text().empty());
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
	g_fs->ensure_directory_exists(basedir_);

	//  OK, first check if the extension matches (ignoring case).
	if (!boost::iends_with(filename, WLMF_SUFFIX))
		filename += WLMF_SUFFIX;

	//  append directory name
	std::string complete_filename = curdir_;
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
