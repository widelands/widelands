/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include <memory>

#include "logic/filesystem_constants.h"

LoadOrSaveGame::LoadOrSaveGame(UI::Panel* parent,
                               Widelands::Game& g,
                               FileType filetype,
                               UI::PanelStyle style,
                               UI::WindowStyle ws,
                               bool localize_autosave,
                               UI::Panel* table_parent,
                               UI::Panel* delete_button_parent)
   : table_box_(new UI::Box(table_parent ? table_parent : parent, style, 0, 0, UI::Box::Vertical)),
     filetype_(filetype),

     // Savegame description
     game_details_(
        parent,
        style,
        filetype == FileType::kReplay ? GameDetails::Mode::kReplay : GameDetails::Mode::kSavegame,
        g),
     delete_(
        new UI::Button(delete_button_parent ? delete_button_parent : game_details()->button_box(),
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
	case FileType::kReplay:
		table_ = new SavegameTableReplay(table_box_, style, localize_autosave);
		savegame_deleter_.reset(new ReplayDeleter(parent, ws));
		savegame_loader_.reset(new ReplayLoader(g));
		break;
	case FileType::kGameSinglePlayer:
		table_ = new SavegameTableSinglePlayer(table_box_, style, localize_autosave);
		savegame_deleter_.reset(new SavegameDeleter(parent, ws));
		savegame_loader_.reset((new SinglePlayerLoader(g)));
		break;
	case FileType::kGameMultiPlayer:
		table_ = new SavegameTableMultiplayer(table_box_, style, localize_autosave);
		savegame_deleter_.reset(new SavegameDeleter(parent, ws));
		savegame_loader_.reset(new MultiPlayerLoader(g));
		break;
	case FileType::kShowAll:
		table_ = new SavegameTableMultiplayer(
		   table_box_, style, localize_autosave);  // wrong? showAll = save window -> "accidental"
		                                           // same table as multiplayer
		savegame_deleter_.reset(new SavegameDeleter(parent, ws));
		savegame_loader_.reset(new EverythingLoader(g));
		break;
	}

	table_->set_column_compare(
	   0, [this](uint32_t a, uint32_t b) { return compare_save_time(a, b); });

	table_->set_column_compare(table_->number_of_columns() - 1,
	                           [this](uint32_t a, uint32_t b) { return compare_map_name(a, b); });

	table_box_->add(table_, UI::Box::Resizing::kExpandBoth);
	if (!delete_button_parent) {
		game_details_.button_box()->add(delete_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	}
	delete_->set_enabled(false);
	delete_->sigclicked.connect([this] { clicked_delete(); });

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

const std::vector<SavegameData> LoadOrSaveGame::get_selected_savegames() const {
	const std::set<uint32_t> selections = table_->selections();
	std::vector<SavegameData> savegames(selections.size());
	size_t i = 0;
	for (const uint32_t index : selections) {
		savegames.at(i) = get_savegame(index);
		++i;
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

	set_tooltips_of_buttons(table_->selections().size());

	const std::vector<SavegameData> savegames = get_selected_savegames();
	game_details_.display(savegames);
	if (!table_->selections().empty()) {
		delete_->set_enabled(!selection_contains_directory());
		result.reset(new SavegameData(savegames[0]));
	} else {
		delete_->set_enabled(false);
		delete_->set_tooltip("");
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

void LoadOrSaveGame::select_item_and_scroll_to_it(std::set<uint32_t>& selections) {
	if (table_->empty()) {
		return;
	}
	const uint32_t selectme = *selections.begin();
	table_->select(std::min(selectme, table_->size() - 1));
	table_->scroll_to_item(table_->selection_index() + 1);
}

void LoadOrSaveGame::clicked_delete() {
	if (!has_selection()) {
		return;
	}
	const std::vector<SavegameData> selected = get_selected_savegames();

	std::set<uint32_t> selections = table_->selections();
	if (savegame_deleter_->delete_savegames(selected)) {
		fill_table();
		select_item_and_scroll_to_it(selections);
		// Make sure that the game details are updated
		entry_selected();
	}
}

UI::Button* LoadOrSaveGame::delete_button() {
	return delete_;
}

void LoadOrSaveGame::fill_table() {
	clear_selections();
	games_data_ = savegame_loader_->load_files(curdir_);

	// If we are not in basedir we are in a sub-dir so we need to add parent dir
	if (curdir_ != basedir_) {
		games_data_.push_back(SavegameData::create_parent_dir(curdir_));
	}

	table_->fill(games_data_);
}

void LoadOrSaveGame::set_show_filenames(bool show_filenames) {
	if (filetype_ != FileType::kReplay) {
		return;
	}
	table_->set_show_filenames(show_filenames);
}

void LoadOrSaveGame::change_directory_to(const std::string& directory) {
	curdir_ = directory;
	fill_table();
}
