/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "wui/load_or_save_game.h"

#include <ctime>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "helper.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/replay.h"
#include "ui_basic/messagebox.h"

LoadOrSaveGame::LoadOrSaveGame(UI::Panel* parent,
                               Widelands::Game& g,
                               FileType filetype,
                               UI::PanelStyle style,
                               bool localize_autosave)
   : parent_(parent),
     table_box_(new UI::Box(parent, 0, 0, UI::Box::Vertical)),
     filetype_(filetype),
     savegame_deleter_(parent),
     // Savegame description
     game_details_(
        parent,
        style,
        filetype == FileType::kReplay ? GameDetails::Mode::kReplay : GameDetails::Mode::kSavegame),
     delete_(new UI::Button(game_details()->button_box(),
                            "delete",
                            0,
                            0,
                            0,
                            0,
                            style == UI::PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
                                                               UI::ButtonStyle::kWuiSecondary,
                            _("Delete"))),
     basedir_(filetype_ == FileType::kReplay ? kReplayDir : kSaveDir),
     curdir_(basedir_),
     game_(g) {

	switch (filetype_) {
	case FileType::kGameSinglePlayer:
		table_ = new SavegameTableSinglePlayer(table_box_, style, localize_autosave);
		break;
	case FileType::kReplay:
		table_ = new SavegameTableReplay(table_box_, style, localize_autosave);
		break;
	default:
		table_ = new SavegameTableMultiplayer(table_box_, style, localize_autosave);
	}

	table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, this, _1, _2));

	table_->set_column_compare(table_->number_of_columns() - 1,
	                           boost::bind(&LoadOrSaveGame::compare_map_name, this, _1, _2));

	table_box_->add(table_, UI::Box::Resizing::kExpandBoth);
	game_details_.button_box()->add(delete_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	delete_->set_enabled(false);
	delete_->sigclicked.connect(boost::bind(&LoadOrSaveGame::clicked_delete, boost::ref(*this)));

	fill_table();
}

bool LoadOrSaveGame::selection_contains_directory() const {
	const std::set<uint32_t>& selections = table_->selections();
	for (const uint32_t index : selections) {
		const SavegameData& gamedata = get_savegame(index);
		if (gamedata.is_directory()) {
			return true;
		}
	}
	return false;
}

const SavegameData& LoadOrSaveGame::get_savegame(uint32_t index) const {
	return games_data_[(*table_)[index]];
}

const std::vector<SavegameData>
LoadOrSaveGame::get_selected_savegames(const std::set<uint32_t>& selections) const {
	std::vector<SavegameData> savegames;
	for (const uint32_t index : selections) {
		savegames.push_back(get_savegame(index));
	}
	return savegames;
}

bool LoadOrSaveGame::compare_save_time(uint32_t rowa, uint32_t rowb) const {
	return get_savegame(rowa).compare_save_time(get_savegame(rowb));
}

bool LoadOrSaveGame::compare_map_name(uint32_t rowa, uint32_t rowb) const {
	return get_savegame(rowa).compare_map_name(get_savegame(rowb));
}

std::unique_ptr<SavegameData> LoadOrSaveGame::entry_selected() {
	std::unique_ptr<SavegameData> result(new SavegameData());
	size_t selections = table_->selections().size();
	set_tooltips_of_buttons(selections);

	const std::vector<SavegameData> savegames = get_selected_savegames(table_->selections());
	game_details_.display(savegames);
	if (selections > 0 && !selection_contains_directory()) {
		delete_->set_enabled(true);
	} else {
		delete_->set_enabled(false);
		delete_->set_tooltip("");
	}
	// TODO(GunChleoc): Take care of the OK button too.
	if (selections > 0) {
		result.reset(new SavegameData(savegames[0]));
	}
	return result;
}
void LoadOrSaveGame::set_tooltips_of_buttons(size_t nr_of_selected_items) const {
	if (nr_of_selected_items == 1) {
		delete_->set_tooltip(
		   filetype_ == FileType::kReplay ?
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected 1 file */
		      _("Delete this replay") :
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected 1 file */
		      _("Delete this game"));
	} else if (nr_of_selected_items > 1) {
		delete_->set_tooltip(filetype_ == FileType::kReplay ?
		                        /** TRANSLATORS: Tooltip for the delete button. The user has
		                           selected multiple files */
		                        _("Delete these replays") :
		                        /** TRANSLATORS: Tooltip for the delete button. The user has
		                           selected multiple files */
		                        _("Delete these games"));
	} else {
		delete_->set_tooltip("");
	}
}

bool LoadOrSaveGame::has_selection() const {
	return table_->has_selection();
}

void LoadOrSaveGame::clear_selections() {
	table_->clear_selections();
	game_details_.clear();
}

void LoadOrSaveGame::select_by_name(const std::string& name) {
	table_->clear_selections();
	for (uint32_t idx = 0; idx < table_->size(); ++idx) {
		const SavegameData& gamedata = get_savegame(idx);
		if (name == gamedata.filename) {
			table_->select(idx);
			return;
		}
	}
}

SavegameTable& LoadOrSaveGame::table() {
	return *table_;
}

UI::Box* LoadOrSaveGame::table_box() {
	return table_box_;
}

GameDetails* LoadOrSaveGame::game_details() {
	return &game_details_;
}

void LoadOrSaveGame::update_table(std::set<uint32_t>& selections) {
	fill_table();

	// Select something meaningful if possible, then scroll to it.
	const uint32_t selectme = *selections.begin();
	if (selectme < table_->size() - 1) {
		table_->select(selectme);
	} else if (!table_->empty()) {
		table_->select(table_->size() - 1);
	}
	if (table_->has_selection()) {
		table_->scroll_to_item(table_->selection_index() + 1);
	}
	// Make sure that the game details are updated
	entry_selected();
}

void LoadOrSaveGame::clicked_delete() {
	if (!has_selection()) {
		return;
	}
	std::set<uint32_t> selections = table().selections();
	const std::vector<SavegameData> selected = get_selected_savegames(selections);

	if (savegame_deleter_.delete_savegames(selected)) {
		update_table(selections);
	}
}

UI::Button* LoadOrSaveGame::delete_button() {
	return delete_;
}

bool LoadOrSaveGame::is_valid_gametype(const SavegameData& gamedata) const {
	// Skip singleplayer games in multiplayer mode and vice versa
	if (filetype_ != FileType::kReplay && filetype_ != FileType::kShowAll) {
		if (filetype_ == FileType::kGameMultiPlayer) {
			if (gamedata.gametype == GameController::GameType::kSingleplayer) {
				return false;
			}
		} else if ((gamedata.gametype != GameController::GameType::kSingleplayer) &&
		           (gamedata.gametype != GameController::GameType::kReplay)) {
			return false;
		}
	}
	return true;
}

void LoadOrSaveGame::add_time_info(SavegameData& gamedata,
                                   const Widelands::GamePreloadPacket& gpdp) const {
	gamedata.savetimestamp = gpdp.get_savetimestamp();
	time_t t;
	time(&t);
	struct tm* currenttime = localtime(&t);
	// We need to put these into variables because of a sideeffect of the localtime function.
	int8_t current_year = currenttime->tm_year;
	int8_t current_month = currenttime->tm_mon;
	int8_t current_day = currenttime->tm_mday;

	struct tm* savedate = localtime(&gamedata.savetimestamp);

	if (gamedata.savetimestamp > 0) {
		if (savedate->tm_year == current_year && savedate->tm_mon == current_month &&
		    savedate->tm_mday == current_day) {  // Today

			// Adding the 0 padding in a separate statement so translators won't have to deal
			// with it
			const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute */
			   (boost::format(_("Today, %1%:%2%")) % savedate->tm_hour % minute).str();
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute. This is part of a list.
			    */
			   (boost::format(_("saved today at %1%:%2%")) % savedate->tm_hour % minute).str();
		} else if ((savedate->tm_year == current_year && savedate->tm_mon == current_month &&
		            savedate->tm_mday == current_day - 1) ||
		           (savedate->tm_year == current_year - 1 && savedate->tm_mon == 11 &&
		            current_month == 0 && savedate->tm_mday == 31 &&
		            current_day == 1)) {  // Yesterday
			// Adding the 0 padding in a separate statement so translators won't have to deal
			// with it
			const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute */
			   (boost::format(_("Yesterday, %1%:%2%")) % savedate->tm_hour % minute).str();
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute. This is part of a list.
			    */
			   (boost::format(_("saved yesterday at %1%:%2%")) % savedate->tm_hour % minute).str();
		} else {  // Older
			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month day, year */
			   (boost::format(_("%1% %2%, %3%")) % localize_month(savedate->tm_mon) %
			    savedate->tm_mday % (1900 + savedate->tm_year))
			      .str();
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month (short name) day (number),
			      year (number). This is part of a list. */
			   (boost::format(_("saved on %1% %2%, %3%")) % savedate->tm_mday %
			    localize_month(savedate->tm_mon) % (1900 + savedate->tm_year))
			      .str();
		}
	}
}

void LoadOrSaveGame::add_general_information(SavegameData& gamedata,
                                             const Widelands::GamePreloadPacket& gpdp) const {
	gamedata.set_mapname(gpdp.get_mapname());
	gamedata.set_gametime(gpdp.get_gametime());
	gamedata.set_nrplayers(gpdp.get_number_of_players());
	gamedata.version = gpdp.get_version();
	gamedata.wincondition = gpdp.get_localized_win_condition();
	gamedata.minimap_path = gpdp.get_minimap_path();
}

void LoadOrSaveGame::add_error_info(SavegameData& gamedata, std::string errormessage) const {
	boost::replace_all(errormessage, "\n", "<br>");
	gamedata.errormessage =
	   ((boost::format("<p>%s</p><p>%s</p><p>%s</p>"))
	    /** TRANSLATORS: Error message introduction for when an old savegame can't be loaded */
	    % _("This file has the wrong format and can’t be loaded."
	        " Maybe it was created with an older version of Widelands.")
	    /** TRANSLATORS: This text is on a separate line with an error message below */
	    % _("Error message:") % errormessage)
	      .str();

	gamedata.mapname = FileSystem::filename_without_ext(gamedata.filename.c_str());
}

void LoadOrSaveGame::add_sub_dir(const std::string& gamefilename) {
	// Add subdirectory to the list
	const char* fs_filename = FileSystem::fs_filename(gamefilename.c_str());
	if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, "..")) {
		return;
	}
	games_data_.push_back(SavegameData::create_sub_dir(gamefilename));
}

void LoadOrSaveGame::load_gamefile(const std::string& gamefilename) {

	Widelands::GamePreloadPacket gpdp;
	std::string savename = gamefilename;

	if (filetype_ == FileType::kReplay) {
		savename += kSavegameExtension;
	}
	if (!g_fs->file_exists(savename.c_str())) {
		return;
	}

	SavegameData gamedata(gamefilename);
	try {
		Widelands::GameLoader gl(savename.c_str(), game_);
		gl.preload_game(gpdp);

		gamedata.gametype = gpdp.get_gametype();
		if (!is_valid_gametype(gamedata)) {
			return;
		}
		add_general_information(gamedata, gpdp);
		add_time_info(gamedata, gpdp);

	} catch (const std::exception& e) {

		if (g_fs->is_directory(gamefilename)) {
			add_sub_dir(gamefilename);
			return;
		} else {
			add_error_info(gamedata, e.what());
		}
	}

	games_data_.push_back(gamedata);
}

void LoadOrSaveGame::fill_table() {
	clear_selections();
	games_data_.clear();

	FilenameSet gamefiles = g_fs->list_directory(curdir_);

	// If we are not in basedir we are in a sub-dir so we need to add parent dir
	if (curdir_ != basedir_) {
		games_data_.push_back(SavegameData::create_parent_dir(curdir_));
	}

	for (const std::string& gamefilename : gamefiles) {
		load_gamefile(gamefilename);
	}

	table_->fill(games_data_);
}

void LoadOrSaveGame::set_show_filenames(bool show_filenames) {
	if (filetype_ != FileType::kReplay) {
		return;
	}
	table_->set_show_filenames(show_filenames);
}

void LoadOrSaveGame::change_directory_to(std::string& directory) {
	curdir_ = directory;
	fill_table();
}
