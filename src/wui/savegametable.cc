#include "wui/savegametable.h"

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"

SavegameTable::SavegameTable(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave)
   : UI::Table<uintptr_t>(parent, 0, 0, 0, 0, style, UI::TableRows::kMultiDescending),
     localize_autosave_(localize_autosave) {
}

const std::string SavegameTable::map_filename(const std::string& filename,
                                              const std::string& mapname) const {
	std::string result = FileSystem::filename_without_ext(filename.c_str());

	if (localize_autosave_ && boost::starts_with(result, kAutosavePrefix)) {
		std::vector<std::string> autosave_name;
		boost::split(autosave_name, result, boost::is_any_of("_"));
		if (autosave_name.empty() || autosave_name.size() < 3) {
			/** TRANSLATORS: %1% is a map's name. */
			result = (boost::format(_("Autosave: %1%")) % mapname).str();
		} else {
			/** TRANSLATORS: %1% is a number, %2% a map's name. */
			result = (boost::format(_("Autosave %1%: %2%")) % autosave_name.back() % mapname).str();
		}
	} else if (!(boost::starts_with(result, mapname))) {
		/** TRANSLATORS: %1% is a filename, %2% a map's name. */
		result = (boost::format(pgettext("filename_mapname", "%1%: %2%")) % result % mapname).str();
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
		return (boost::format(_("H (%1%)")) % savegame.nrplayers).str();
	}
	if (savegame.is_multiplayer_client()) {
		/** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
		/** TRANSLATORS: "Keep this to 2 letters maximum. */
		/** TRANSLATORS: A tooltip will explain the abbreviation. */
		/** TRANSLATORS: Make sure that this translation is consistent with the
		        tooltip. */
		/** TRANSLATORS: %1% is the number of players */
		return (boost::format(_("MP (%1%)")) % savegame.nrplayers).str();
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
	te.set_string(
	   last_column_index, (boost::format(_("Incompatible: %s")) % savegame.mapname).str());
}

void SavegameTable::create_directory_entry(UI::Table<const uintptr_t>::EntryRecord& te,
                                           const SavegameData& savegame) {
	size_t last_column_index = number_of_columns() - 1;
	for (size_t col_index = 0; col_index < last_column_index; ++col_index) {
		te.set_string(col_index, "");
	}
	if (savegame.is_parent_directory()) {
		te.set_picture(last_column_index, FN_GET_IMAGE("images/ui_basic/ls_dir.png"),
		               /** TRANSLATORS: Parent directory/folder */
		               (boost::format("<%s>") % _("parent")).str());
	} else if (savegame.is_sub_directory()) {
		te.set_picture(last_column_index, FN_GET_IMAGE("images/ui_basic/ls_dir.png"),
		               FileSystem::filename_without_ext(savegame.filename.c_str()));
	}
}

void SavegameTable::set_show_filenames(bool) {
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
		te.set_picture(1, FN_GET_IMAGE("images/ui_basic/ls_wlmap.png"),
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
	std::string game_mode_tooltip = "";
	/** TRANSLATORS: Tooltip header for the "Mode" column when choosing a game/replay to
	load.*/
	g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(_("Game Mode"));

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("SP = Single Player"), UI::FontStyle::kTooltip);

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("MP = Multiplayer"), UI::FontStyle::kTooltip);
	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("H = Multiplayer (Host)"), UI::FontStyle::kTooltip);

	game_mode_tooltip += g_gr->styles()
	                        .font_style(UI::FontStyle::kTooltip)
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
		te.set_picture(2, FN_GET_IMAGE("images/ui_basic/ls_wlmap.png"),
		               (boost::format(pgettext("mapname_gametime", "%1% (%2%)")) % map_basename %
		                savegame.gametime)
		                  .str());
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
	std::string game_mode_tooltip = "";
	/** TRANSLATORS: Tooltip header for the "Mode" column when choosing a game/replay to
	load.*/
	g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(_("Game Mode"));

	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("MP = Multiplayer"), UI::FontStyle::kTooltip);
	/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	game_mode_tooltip += as_listitem(_("H = Multiplayer (Host)"), UI::FontStyle::kTooltip);

	game_mode_tooltip += g_gr->styles()
	                        .font_style(UI::FontStyle::kTooltip)
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
		te.set_picture(2, FN_GET_IMAGE("images/ui_basic/ls_wlmap.png"),
		               map_filename(savegame.filename, savegame.mapname));
	}
}
