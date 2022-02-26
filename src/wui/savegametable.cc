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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/savegametable.h"

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"

SavegameTable::SavegameTable(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave)
   : UI::Table<uintptr_t>(parent, 0, 0, 0, 0, style, UI::TableRows::kMultiDescending),
     tooltip_style_(style == UI::PanelStyle::kWui ? UI::FontStyle::kWuiTooltip :
                                                    UI::FontStyle::kFsTooltip),
     tooltip_header_style_(style == UI::PanelStyle::kWui ? UI::FontStyle::kWuiTooltipHeader :
                                                           UI::FontStyle::kFsTooltipHeader),
     localize_autosave_(localize_autosave) {
}

const std::string SavegameTable::map_filename(const std::string& filename,
                                              const std::string& mapname) const {
	std::string result = FileSystem::filename_without_ext(filename.c_str());

	if (localize_autosave_ && starts_with(result, kAutosavePrefix)) {
		std::vector<std::string> autosave_name;
		split(autosave_name, result, {'_'});
		if (autosave_name.empty() || autosave_name.size() < 3) {
			/** TRANSLATORS: %1% is a map's name. */
			result = format(_("Autosave: %1%"), mapname);
		} else {
			/** TRANSLATORS: %1% is a number, %2% a map's name. */
			result = format(_("Autosave %1%: %2%"), autosave_name.back(), mapname);
		}
	} else if (!starts_with(result, mapname)) {
		/** TRANSLATORS: %1% is a filename, %2% a map's name. */
		result = format(pgettext("filename_mapname", "%1%: %2%"), result, mapname);
	}
	return result;
}

const std::string SavegameTable::find_game_type(const SavegameData& savegame) const {
	if (savegame.is_singleplayer()) {
		/** TRANSLATORS: "Single Player" entry in the Game Mode table column. */
		/** TRANSLATORS: "Keep this to 6 letters maximum. */
		/** TRANSLATORS: A tooltip will explain the abbreviation. */
		/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
		return _("SP");
	}
	if (savegame.is_multiplayer_host()) {
		/** TRANSLATORS: "Multiplayer Host" entry in the Game Mode table column. */
		/** TRANSLATORS: "Keep this to 2 letters maximum. */
		/** TRANSLATORS: A tooltip will explain the abbreviation. */
		/** TRANSLATORS: Make sure that this translation is consistent with the
		        tooltip. */
		/** TRANSLATORS: %1% is the number of players */
		return format(_("H (%1%)"), savegame.nrplayers);
	}
	if (savegame.is_multiplayer_client()) {
		/** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
		/** TRANSLATORS: "Keep this to 2 letters maximum. */
		/** TRANSLATORS: A tooltip will explain the abbreviation. */
		/** TRANSLATORS: Make sure that this translation is consistent with the
		        tooltip. */
		/** TRANSLATORS: %1% is the number of players */
		return format(_("MP (%1%)"), savegame.nrplayers);
	}
	if (savegame.is_replay()) {
		return "";
	}
	NEVER_HERE();
}

void SavegameTable::fill(const std::vector<SavegameData>& entries) {
	clear();

	for (size_t i = 0; i < entries.size(); ++i) {
		const SavegameData& savegame = entries[i];
		UI::Table<uintptr_t const>::EntryRecord& te = add(i);

		if (savegame.errormessage.empty()) {
			create_valid_entry(te, savegame);
		} else {
			create_error_entry(te, savegame);
		}
	}
	sort();
	layout();
	focus();
}

void SavegameTable::create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
                                       const SavegameData& savegame) {
	size_t last_column_index = number_of_columns() - 1;
	for (size_t col_index = 0; col_index < last_column_index; col_index++) {
		te.set_string(col_index, "");
	}
	te.set_string(last_column_index, format(_("Incompatible: %s"), savegame.mapname));
}

void SavegameTable::create_directory_entry(UI::Table<const uintptr_t>::EntryRecord& te,
                                           const SavegameData& savegame) {
	size_t last_column_index = number_of_columns() - 1;
	for (size_t col_index = 0; col_index < last_column_index; ++col_index) {
		te.set_string(col_index, "");
	}
	if (savegame.is_parent_directory()) {
		te.set_picture(last_column_index, g_image_cache->get("images/ui_basic/ls_dir.png"),
		               /** TRANSLATORS: Parent directory/folder */
		               format("<%s>", _("parent")));
	} else if (savegame.is_sub_directory()) {
		te.set_picture(last_column_index, g_image_cache->get("images/ui_basic/ls_dir.png"),
		               FileSystem::filename_without_ext(savegame.filename.c_str()));
	}
}

void SavegameTable::set_show_filenames(bool /* show_filenames */) {
	// empty on purpose, must be overridden for tables that support showing/hiding filenames
}

SavegameTableSinglePlayer::SavegameTableSinglePlayer(UI::Panel* parent,
                                                     UI::PanelStyle style,
                                                     bool localize_autosave)
   : SavegameTable(parent, style, localize_autosave) {

	add_columns();
}

void SavegameTableSinglePlayer::add_columns() {
	add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	add_column(0, _("Description"),
	           _("The filename that the game was saved under followed by the map’s name, "
	             "or the map’s name followed by the last objective achieved."),
	           UI::Align::kLeft, UI::TableColumnType::kFlexible);

	set_sort_column(0);
}

void SavegameTableSinglePlayer::create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
                                                   const SavegameData& savegame) {
	if (savegame.is_directory()) {
		create_directory_entry(te, savegame);
	} else {
		te.set_string(0, savegame.savedatestring);
		te.set_picture(1, g_image_cache->get("images/ui_basic/ls_wlmap.png"),
		               map_filename(savegame.filename, savegame.mapname));
	}
}

SavegameTableReplay::SavegameTableReplay(UI::Panel* parent,
                                         UI::PanelStyle style,
                                         bool localize_autosave)
   : SavegameTable(parent, style, localize_autosave), show_filenames_(false) {
	add_columns();
}

void SavegameTableReplay::add_columns() {
	add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	std::string game_mode_tooltip;
	/** TRANSLATORS: Tooltip header for the "Mode" column when choosing a game/replay to
	load.*/
	g_style_manager->font_style(tooltip_header_style_).as_font_tag(_("Game Mode"));

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("SP = Single Player"), tooltip_style_);

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("MP = Multiplayer"), tooltip_style_);
	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("H = Multiplayer (Host)"), tooltip_style_);

	game_mode_tooltip += g_style_manager->font_style(tooltip_style_)
	                        .as_font_tag(_("Numbers are the number of players."));

	add_column(65,
	           /** TRANSLATORS: Game Mode table column when choosing a game/replay to load. */
	           /** TRANSLATORS: Keep this to 5 letters maximum. */
	           /** TRANSLATORS: A tooltip will explain if you need to use an abbreviation. */
	           _("Mode"), game_mode_tooltip);
	add_column(0, _("Description"),
	           _("The filename that the game was saved under followed by the map’s name, "
	             "or the map’s name followed by the last objective achieved."),
	           UI::Align::kLeft, UI::TableColumnType::kFlexible);

	set_sort_column(0);
}

void SavegameTableReplay::create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
                                             const SavegameData& savegame) {
	if (savegame.is_directory()) {
		create_directory_entry(te, savegame);

	} else {
		te.set_string(0, savegame.savedatestring);
		te.set_string(1, find_game_type(savegame));
		const std::string map_basename =
		   show_filenames_ ? map_filename(savegame.filename, savegame.mapname) : savegame.mapname;
		te.set_picture(
		   2, g_image_cache->get("images/ui_basic/ls_wlmap.png"),
		   format(pgettext("mapname_gametime", "%1% (%2%)"), map_basename, savegame.gametime));
	}
}

void SavegameTableReplay::set_show_filenames(bool show_filenames) {
	show_filenames_ = show_filenames;
	set_column_tooltip(2, show_filenames ? _("Filename: Map name (start of replay)") :
                                          _("Map name (start of replay)"));
}

SavegameTableMultiplayer::SavegameTableMultiplayer(UI::Panel* parent,
                                                   UI::PanelStyle style,
                                                   bool localize_autosave)
   : SavegameTable(parent, style, localize_autosave) {
	add_columns();
}

void SavegameTableMultiplayer::add_columns() {
	add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	std::string game_mode_tooltip;
	/** TRANSLATORS: Tooltip header for the "Mode" column when choosing a game/replay to
	load.*/
	g_style_manager->font_style(tooltip_header_style_).as_font_tag(_("Game Mode"));

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("MP = Multiplayer"), tooltip_style_);
	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("H = Multiplayer (Host)"), tooltip_style_);

	game_mode_tooltip += g_style_manager->font_style(tooltip_style_)
	                        .as_font_tag(_("Numbers are the number of players."));

	add_column(65,
	           /** TRANSLATORS: Game Mode table column when choosing a game/replay to load. */
	           /** TRANSLATORS: Keep this to 5 letters maximum. */
	           /** TRANSLATORS: A tooltip will explain if you need to use an abbreviation. */
	           _("Mode"), game_mode_tooltip);
	add_column(0, _("Description"),
	           _("The filename that the game was saved under followed by the map’s name, "
	             "or the map’s name followed by the last objective achieved."),
	           UI::Align::kLeft, UI::TableColumnType::kFlexible);

	set_sort_column(0);
}

void SavegameTableMultiplayer::create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
                                                  const SavegameData& savegame) {
	if (savegame.is_directory()) {
		create_directory_entry(te, savegame);
	} else {
		te.set_string(0, savegame.savedatestring);
		te.set_string(1, find_game_type(savegame));
		te.set_picture(2, g_image_cache->get("images/ui_basic/ls_wlmap.png"),
		               map_filename(savegame.filename, savegame.mapname));
	}
}
