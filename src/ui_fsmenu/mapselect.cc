/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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
 */

#include "ui_fsmenu/mapselect.h"

#include <cstdio>
#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/widelands_map_loader.h"
#include "wui/map_tags.h"

// TODO(GunChleoc): Arabic: line height broken for descriptions for Arabic.
// Fix align for table headings & entries and for wordwrap.

using Widelands::WidelandsMapLoader;

FullscreenMenuMapSelect::FullscreenMenuMapSelect(GameSettingsProvider* const settings,
                                                 GameController* const ctrl)
   : FullscreenMenuLoadMapOrGame(),
     checkbox_space_(25),
     // Less padding for big fonts; space is tight.
     checkbox_padding_(UI::g_fh1->fontset()->size_offset() > 0 ? 0 : 2 * padding_),

     // Main title
     title_(this, 0, 0, _("Choose a map"), UI::Align::kHCenter),
     checkboxes_(this, 0, 0, UI::Box::Vertical, 0, 0, 2 * padding_),
     table_(this, tablex_, tabley_, tablew_, tableh_, false),
     map_details_(this,
                  right_column_x_,
                  tabley_,
                  get_right_column_w(right_column_x_),
                  tableh_ - buth_ - 4 * padding_,
                  MapDetails::Style::kFsMenu),

     basedir_("maps"),
     settings_(settings),
     ctrl_(ctrl),
     has_translated_mapname_(false) {
	curdir_ = basedir_;
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	if (settings_->settings().multiplayer) {
		back_.set_tooltip(_("Return to the multiplayer game setup"));
	} else {
		back_.set_tooltip(_("Return to the single player menu"));
	}

	back_.sigclicked.connect(boost::bind(&FullscreenMenuMapSelect::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuMapSelect::clicked_ok, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuMapSelect::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuMapSelect::clicked_ok, boost::ref(*this)));
	table_.set_column_compare(
	   0, boost::bind(&FullscreenMenuMapSelect::compare_players, this, _1, _2));
	table_.set_column_compare(
	   1, boost::bind(&FullscreenMenuMapSelect::compare_mapnames, this, _1, _2));
	table_.set_column_compare(2, boost::bind(&FullscreenMenuMapSelect::compare_size, this, _1, _2));

	UI::Box* hbox = new UI::Box(&checkboxes_, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());

	// Must be initialized before tag checkboxes
	cb_dont_localize_mapnames_ =
	   new UI::Checkbox(hbox, Vector2i(0, 0), _("Show original map names"));
	cb_dont_localize_mapnames_->set_state(false);
	cb_dont_localize_mapnames_->changedto.connect(
	   boost::bind(&FullscreenMenuMapSelect::fill_table, boost::ref(*this)));

	cb_show_all_maps_ = add_tag_checkbox(hbox, "blumba", _("Show all maps"));
	tags_checkboxes_.clear();  // Remove this again, it is a special tag checkbox
	cb_show_all_maps_->set_state(true);

	hbox->add(cb_dont_localize_mapnames_, UI::Align::kLeft, true);
	checkboxes_.add(hbox, UI::Align::kLeft, true);

	hbox = new UI::Box(&checkboxes_, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());
	add_tag_checkbox(hbox, "official", localize_tag("official"));
	add_tag_checkbox(hbox, "unbalanced", localize_tag("unbalanced"));
	add_tag_checkbox(hbox, "seafaring", localize_tag("seafaring"));
	add_tag_checkbox(hbox, "artifacts", localize_tag("artifacts"));
	add_tag_checkbox(hbox, "scenario", localize_tag("scenario"));
	checkboxes_.add(hbox, UI::Align::kLeft, true);

	hbox = new UI::Box(&checkboxes_, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());
	add_tag_checkbox(hbox, "ffa", localize_tag("ffa"));
	add_tag_checkbox(hbox, "1v1", localize_tag("1v1"));
	checkboxes_.add(hbox, UI::Align::kLeft, true);

	hbox = new UI::Box(&checkboxes_, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());
	add_tag_checkbox(hbox, "2teams", localize_tag("2teams"));
	add_tag_checkbox(hbox, "3teams", localize_tag("3teams"));
	add_tag_checkbox(hbox, "4teams", localize_tag("4teams"));
	checkboxes_.add(hbox, UI::Align::kLeft, true);

	scenario_types_ = settings_->settings().multiplayer ? Map::MP_SCENARIO : Map::SP_SCENARIO;

	table_.focus();
	fill_table();

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	layout();
}

void FullscreenMenuMapSelect::layout() {
	title_.set_size(get_w(), title_.get_h());
	FullscreenMenuLoadMapOrGame::layout();
	checkboxes_y_ = tabley_ - 4 * (cb_show_all_maps_->get_h() + checkbox_padding_) - 2 * padding_;
	title_.set_pos(Vector2i(0, checkboxes_y_ / 3));
	checkboxes_.set_pos(Vector2i(tablex_, checkboxes_y_));
	checkboxes_.set_size(get_w() - 2 * tablex_, tabley_ - checkboxes_y_);
	table_.set_size(tablew_, tableh_);
	table_.set_pos(Vector2i(tablex_, tabley_));
	map_details_.set_size(get_right_column_w(right_column_x_), tableh_ - buth_ - 4 * padding_);
	map_details_.set_pos(Vector2i(right_column_x_, tabley_));
}

void FullscreenMenuMapSelect::think() {
	if (ctrl_) {
		ctrl_->think();
	}
}

bool FullscreenMenuMapSelect::compare_players(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_players(maps_data_[table_[rowb]]);
}

bool FullscreenMenuMapSelect::compare_mapnames(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_names(maps_data_[table_[rowb]]);
}

bool FullscreenMenuMapSelect::compare_size(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_size(maps_data_[table_[rowb]]);
}

MapData const* FullscreenMenuMapSelect::get_map() const {
	if (!table_.has_selection()) {
		return nullptr;
	}
	return &maps_data_[table_.get_selected()];
}

void FullscreenMenuMapSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	const MapData& mapdata = maps_data_[table_.get_selected()];

	if (!mapdata.width) {
		curdir_ = mapdata.filename;
		fill_table();
	} else {
		if (maps_data_[table_.get_selected()].maptype == MapData::MapType::kScenario) {
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kScenarioGame);
		} else {
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
		}
	}
}

bool FullscreenMenuMapSelect::set_has_selection() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);

	if (!has_selection) {
		map_details_.clear();
	}
	return has_selection;
}

void FullscreenMenuMapSelect::entry_selected() {
	if (set_has_selection()) {
		map_details_.update(
		   maps_data_[table_.get_selected()], !cb_dont_localize_mapnames_->get_state());
	}
}

/**
 * Fill the list with maps that can be opened.
 *
 *
 * At first, only the subdirectories are added to the list, then the normal
 * files follow. This is done to make navigation easier.
 *
 * To make things more difficult, we have to support compressed and uncompressed
 * map files here - the former are files, the latter are directories. Care must
 * be taken to sort uncompressed maps (which look like and really are
 * directories) with the files.
 *
 * The search starts in \ref curdir_ ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 */
void FullscreenMenuMapSelect::fill_table() {
	has_translated_mapname_ = false;

	maps_data_.clear();

	MapData::DisplayType display_type;
	if (cb_dont_localize_mapnames_->get_state()) {
		display_type = MapData::DisplayType::kMapnames;
	} else {
		display_type = MapData::DisplayType::kMapnamesLocalized;
	}

	// This is the normal case

	//  Fill it with all files we find in all directories.
	FilenameSet files = g_fs->list_directory(curdir_);

	// If we are not at the top of the map directory hierarchy (we're not talking
	// about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		maps_data_.push_back(MapData::create_parent_dir(curdir_));
	}

	Widelands::Map map;  //  MapLoader needs a place to put its preload data

	for (const std::string& mapfilename : files) {
		// Add map file (compressed) or map directory (uncompressed)
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
		if (ml != nullptr) {
			try {
				map.set_filename(mapfilename);
				ml->preload_map(true);

				if (!map.get_width() || !map.get_height()) {
					continue;
				}

				MapData::MapType maptype;
				if (map.scenario_types() & scenario_types_) {
					maptype = MapData::MapType::kScenario;
				} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
					maptype = MapData::MapType::kNormal;
				} else {
					maptype = MapData::MapType::kSettlers2;
				}

				MapData mapdata(map, mapfilename, maptype, display_type);

				has_translated_mapname_ =
				   has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

				bool has_all_tags = true;
				for (std::set<uint32_t>::const_iterator it = req_tags_.begin(); it != req_tags_.end();
				     ++it)
					has_all_tags &= mapdata.tags.count(tags_ordered_[*it]);
				if (!has_all_tags) {
					continue;
				}
				maps_data_.push_back(mapdata);
			} catch (const std::exception& e) {
				log("Mapselect: Skip %s due to preload error: %s\n", mapfilename.c_str(), e.what());
			} catch (...) {
				log("Mapselect: Skip %s due to unknown exception\n", mapfilename.c_str());
			}
		} else if (g_fs->is_directory(mapfilename)) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, ".."))
				continue;
			maps_data_.push_back(MapData::create_directory(mapfilename));
		}
	}

	table_.fill(maps_data_, display_type);
	if (!table_.empty()) {
		table_.select(0);
	}
	set_has_selection();
}

/*
 * Add a tag to the checkboxes
 */
UI::Checkbox*
FullscreenMenuMapSelect::add_tag_checkbox(UI::Box* box, std::string tag, std::string displ_name) {
	int32_t id = tags_ordered_.size();
	tags_ordered_.push_back(tag);

	UI::Checkbox* cb = new UI::Checkbox(box, Vector2i(0, 0), displ_name);
	cb->changedto.connect(boost::bind(&FullscreenMenuMapSelect::tagbox_changed, this, id, _1));

	box->add(cb, UI::Align::kLeft, true);
	box->add_space(checkbox_space_);
	tags_checkboxes_.push_back(cb);

	return cb;
}

/*
 * One of the tagboxes has changed
 */
void FullscreenMenuMapSelect::tagbox_changed(int32_t id, bool to) {
	if (id == 0) {  // Show all maps checbox
		if (to) {
			for (UI::Checkbox* checkbox : tags_checkboxes_) {
				checkbox->set_state(false);
			}
		}
	} else {  // Any tag
		if (to) {
			req_tags_.insert(id);
		} else {
			req_tags_.erase(id);
		}
	}
	if (req_tags_.empty()) {
		cb_show_all_maps_->set_state(true);
	} else {
		cb_show_all_maps_->set_state(false);
	}

	fill_table();
}
