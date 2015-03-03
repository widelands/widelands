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
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/widelands_map_loader.h"
#include "ui_basic/box.h"

using Widelands::WidelandsMapLoader;

FullscreenMenuMapSelect::FullscreenMenuMapSelect
		(GameSettingsProvider* const settings, GameController* const ctrl) :
	FullscreenMenuLoadMapOrGame(),
	checkbox_space_(25),
	checkboxes_y_(tabley_ - 120),

	// Main title
	title_
		(this, get_w() / 2, checkboxes_y_ / 3,
		 _("Choose a map"),
		 UI::Align_HCenter),

	table_(this, tablex_, tabley_, tablew_, tableh_, MapTable::Type::kMapnames, false),
	map_details_(this, right_column_x_, tabley_,
					 get_right_column_w(right_column_x_ + indent_),
					 tableh_ - buth_ - 4 * padding_),

	basedir_("maps"),
	settings_(settings),
	ctrl_(ctrl),
	is_scenario_(false),
	has_translated_mapname_(false)
{
	curdir_ = basedir_,
	title_.set_textstyle(UI::TextStyle::ui_big());
	if (settings_->settings().multiplayer) {
		back_.set_tooltip(_("Return to the multiplayer game setup"));
	} else {
		back_.set_tooltip(_("Return to the single player menu"));
	}

	back_.sigclicked.connect(boost::bind(&FullscreenMenuMapSelect::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuMapSelect::clicked_ok, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuMapSelect::entry_selected, this));
	table_.double_clicked.connect(boost::bind(&FullscreenMenuMapSelect::clicked_ok, boost::ref(*this)));

	UI::Box* vbox = new UI::Box(this, tablex_, checkboxes_y_,
										 UI::Box::Horizontal, checkbox_space_, get_w());

	// Must be initialized before tag checkboxes
	cb_dont_localize_mapnames_ = new UI::Checkbox(vbox, Point(0, 0));
	cb_dont_localize_mapnames_->set_state(false);
	cb_dont_localize_mapnames_->changedto.connect
			(boost::bind(&FullscreenMenuMapSelect::fill_table, boost::ref(*this)));

	cb_show_all_maps_ = _add_tag_checkbox(vbox, "blumba", _("Show all maps"));
	tags_checkboxes_.clear(); // Remove this again, it is a special tag checkbox
	cb_show_all_maps_->set_state(true);

	vbox->add(cb_dont_localize_mapnames_, UI::Box::AlignLeft, true);
	UI::Textarea * ta_dont_localize_mapnames =
			/** TRANSLATORS: Checkbox title. If this checkbox is enabled, map names aren't translated. */
			new UI::Textarea(vbox, _("Show original map names"), UI::Align_CenterLeft);
	vbox->add_space(padding_);
	vbox->add(ta_dont_localize_mapnames, UI::Box::AlignLeft);
	vbox->add_space(checkbox_space_);
	vbox->set_size(get_w() - 2 * tablex_, checkbox_space_);

	vbox = new UI::Box(this,
							 tablex_, vbox->get_y() + vbox->get_h() + padding_,
							 UI::Box::Horizontal, checkbox_space_, get_w());
	_add_tag_checkbox(vbox, "official", _("Official"));
	_add_tag_checkbox(vbox, "unbalanced", _("Unbalanced"));
	_add_tag_checkbox(vbox, "seafaring", _("Seafaring"));
	_add_tag_checkbox(vbox, "scenario", _("Scenario"));
	vbox->set_size(get_w() - 2 * tablex_, checkbox_space_);

	vbox = new UI::Box(this,
							 tablex_, vbox->get_y() + vbox->get_h() + padding_,
							 UI::Box::Horizontal, checkbox_space_, get_w());
	_add_tag_checkbox(vbox, "ffa", _("Free for all"));
	_add_tag_checkbox(vbox, "1v1", _("1v1"));

	vbox->set_size(get_w() - 2 * tablex_, checkbox_space_);

	vbox = new UI::Box(this,
							 tablex_, vbox->get_y() + vbox->get_h() + padding_,
							 UI::Box::Horizontal, checkbox_space_, get_w());
	_add_tag_checkbox(vbox, "2teams", _("Teams of 2"));
	_add_tag_checkbox(vbox, "3teams", _("Teams of 3"));
	_add_tag_checkbox(vbox, "4teams", _("Teams of 4"));
	vbox->set_size(get_w() - 2 * tablex_, checkbox_space_);

	scenario_types_ = settings_->settings().multiplayer ? Map::MP_SCENARIO : Map::SP_SCENARIO;

	table_.focus();
	fill_table();

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	ta_dont_localize_mapnames->set_visible(has_translated_mapname_);
}

void FullscreenMenuMapSelect::think()
{
	if (ctrl_) {
		ctrl_->think();
	}
}

bool FullscreenMenuMapSelect::is_scenario()
{
	return is_scenario_;
}


MapData const * FullscreenMenuMapSelect::get_map() const
{
	return table_.get_map();
}


void FullscreenMenuMapSelect::clicked_ok()
{
	assert(table_.has_selection());
	const MapData& mapdata = *table_.get_map();

	if (!mapdata.width) {
		curdir_ = mapdata.filename;
		fill_table();
	} else {
		end_modal(1 + is_scenario());
	}
}

bool FullscreenMenuMapSelect::set_has_selection()
{
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);

	if (!has_selection) {
		map_details_.clear();
	}
	return has_selection;
}


void FullscreenMenuMapSelect::entry_selected()
{
	if (set_has_selection()) {
		map_details_.update(*table_.get_map(), !cb_dont_localize_mapnames_->get_state());
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
 *
 * \note special case is, if this is a multiplayer game on a dedicated server and
 * the client wants to change the map - in that case the maps available on the server are shown.
 */
void FullscreenMenuMapSelect::fill_table()
{
	std::vector<MapData> maps_data;
	table_.clear();
	has_translated_mapname_ = false;

	if (settings_->settings().maps.empty()) {
		// This is the normal case

		//  Fill it with all files we find in all directories.
		FilenameSet files = g_fs->list_directory(curdir_);

		//If we are not at the top of the map directory hierarchy (we're not talking
		//about the absolute filesystem top!) we manually add ".."
		if (curdir_ != basedir_) {
			MapData mapdata;
	#ifndef _WIN32
			mapdata.filename = curdir_.substr(0, curdir_.rfind('/'));
	#else
			mapdata.filename = curdir_.substr(0, curdir_.rfind('\\'));
	#endif
			mapdata.localized_name = (boost::format("\\<%s\\>") % _("parent")).str();
			mapdata.maptype = MapData::MapType::kDirectory;
			maps_data.push_back(mapdata);
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
			if (WidelandsMapLoader::is_widelands_map(name))
				continue;

			MapData mapdata;
			mapdata.filename = name;
			if (strcmp (name, "maps/MP Scenarios") == 0) {
				/** TRANSLATORS: Directory name for MP Scenarios in map selection */
				mapdata.localized_name = _("Multiplayer Scenarios");
			} else {
				mapdata.localized_name = FileSystem::fs_filename(name);
			}
			mapdata.maptype = MapData::MapType::kDirectory;
			maps_data.push_back(mapdata);
		}

		//Add map files(compressed maps) and directories(uncompressed)
		{
			Widelands::Map map; //  MapLoader needs a place to put its preload data

			for (const std::string& mapfilename : files) {

				std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
				if (!ml) {
					continue;
				}

				try {
					map.set_filename(mapfilename);
					ml->preload_map(true);

					i18n::Textdomain td("maps");

					MapData mapdata;
					mapdata.filename       = mapfilename;
					mapdata.name           = map.get_name();
					mapdata.localized_name = mapdata.name.empty() ? "" : _(mapdata.name);
					mapdata.authors.parse(map.get_author());
					mapdata.description    = map.get_description().empty() ? "" : _(map.get_description());
					mapdata.hint           = map.get_hint().empty() ? "" : _(map.get_hint());
					mapdata.nrplayers      = map.get_nrplayers();
					mapdata.width          = map.get_width();
					mapdata.height         = map.get_height();
					mapdata.tags           = map.get_tags();
					mapdata.suggested_teams = map.get_suggested_teams();

					if (map.scenario_types() & scenario_types_) {
						mapdata.maptype = MapData::MapType::kScenario;
						mapdata.tags.insert("scenario");
					} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
						mapdata.maptype = MapData::MapType::kNormal;
					} else {
						mapdata.maptype = MapData::MapType::kSettlers2;
					}

					has_translated_mapname_ =
							has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

					if (!mapdata.width || !mapdata.height) {
						continue;
					}

					bool has_all_tags = true;
					for (std::set<uint32_t>::const_iterator it = req_tags_.begin(); it != req_tags_.end(); ++it)
						has_all_tags &= mapdata.tags.count(tags_ordered_[*it]);
					if (!has_all_tags) {
						continue;
					}
					maps_data.push_back(mapdata);
				} catch (const std::exception & e) {
					log("Mapselect: Skip %s due to preload error: %s\n", mapfilename.c_str(), e.what());
				} catch (...) {
					log("Mapselect: Skip %s due to unknown exception\n", mapfilename.c_str());
				}
			}
		}
	} else {
		//client changing maps on dedicated server
		for (uint16_t i = 0; i < settings_->settings().maps.size(); ++i) {
			Widelands::Map map; //  MapLoader needs a place to put its preload data

			const DedicatedMapInfos & dmap = settings_->settings().maps.at(i);
			const std::string& mapfilename = dmap.path;
			std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(mapfilename));

			i18n::Textdomain td("maps");
			MapData mapdata;
			try {
				if (!ml) {
					throw wexception("Mapselect: No MapLoader");
				}

				map.set_filename(mapfilename);
				ml->preload_map(true);

				mapdata.filename    = mapfilename;
				mapdata.name        = map.get_name();
				mapdata.authors.parse(map.get_author());
				mapdata.description = map.get_description();
				mapdata.hint        = map.get_hint();
				mapdata.nrplayers   = map.get_nrplayers();
				mapdata.width       = map.get_width();
				mapdata.height      = map.get_height();

				if (map.scenario_types() & scenario_types_) {
					mapdata.maptype = MapData::MapType::kScenario;
					mapdata.tags.insert("scenario");
				} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
					mapdata.maptype = MapData::MapType::kSettlers2;
				} else {
					mapdata.maptype = MapData::MapType::kNormal;
				}

				if (mapdata.nrplayers != dmap.players ||
					 (mapdata.maptype == MapData::MapType::kScenario) != dmap.scenario) {
					throw wexception("Mapselect: Number of players or scenario doesn't match");
				}

				if (!mapdata.width || !mapdata.height) {
					throw wexception("Mapselect: Map has no size");
				}

				// Finally write the entry to the list
				maps_data.push_back(mapdata);
			} catch (...) {
				log("Mapselect: Skipped reading locale data for file %s - not valid.\n", mapfilename.c_str());

				// Fill in the data we got from the dedicated server
				mapdata.filename    = mapfilename;
				mapdata.name        = mapfilename.substr(5, mapfilename.size() - 1);
				mapdata.authors.parse(_("Nobody"));
				mapdata.description = _("This map file is not present in your filesystem."
							" The data shown here was sent by the server.");
				mapdata.hint        = "";
				mapdata.nrplayers   = dmap.players;
				mapdata.width       = 1;
				mapdata.height      = 0;

				if (dmap.scenario) {
					mapdata.maptype = MapData::MapType::kScenario;
					mapdata.tags.insert("scenario");
				} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
					mapdata.maptype = MapData::MapType::kSettlers2;
				} else {
					mapdata.maptype = MapData::MapType::kNormal;
				}

				// Finally write the entry to the list
				maps_data.push_back(mapdata);
			}
		}
	}
	table_.fill(maps_data, !cb_dont_localize_mapnames_->get_state());
	set_has_selection();
}

/*
 * Add a tag to the checkboxes
 */
UI::Checkbox * FullscreenMenuMapSelect::_add_tag_checkbox
	(UI::Box * box, std::string tag, std::string displ_name)
{
	int32_t id = tags_ordered_.size();
	tags_ordered_.push_back(tag);

	UI::Checkbox * cb = new UI::Checkbox(box, Point(0, 0));
	cb->changedto.connect
		(boost::bind(&FullscreenMenuMapSelect::_tagbox_changed, this, id, _1));

	box->add(cb, UI::Box::AlignLeft, true);
	UI::Textarea * ta = new UI::Textarea(box, displ_name, UI::Align_CenterLeft);
	box->add_space(padding_);
	box->add(ta, UI::Box::AlignLeft);
	box->add_space(checkbox_space_);

	tags_checkboxes_.push_back(cb);

	return cb;
}

/*
 * One of the tagboxes has changed
 */
void FullscreenMenuMapSelect::_tagbox_changed(int32_t id, bool to) {
	if (id == 0) { // Show all maps checbox
		if (to) {
			for (UI::Checkbox * checkbox : tags_checkboxes_) {
				checkbox->set_state(false);
			}
		}
	} else { // Any tag
		if (to) {
			req_tags_.insert(id);
		}
		else {
			req_tags_.erase(id);
		}
	}
	if (req_tags_.empty()) {
		cb_show_all_maps_->set_state(true);
	}
	else {
		cb_show_all_maps_->set_state(false);
	}

	fill_table();
}
