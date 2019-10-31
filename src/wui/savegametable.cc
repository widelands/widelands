#include "savegametable.h"

SavegameTable::SavegameTable(UI::Panel* parent,
                             int32_t x,
                             int32_t y,
                             uint32_t w,
                             uint32_t h,
                             UI::PanelStyle style,
                             bool is_replay)
   : UI::Table<uintptr_t>(parent, x, y, w, h, style, UI::TableRows::kMultiDescending) {
	is_replay_ = is_replay;

	add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	add_columns_special();
	add_column(260, _("Description"),
	           _("The filename that the game was saved under followed by the map’s name, "
	             "or the map’s name followed by the last objective achieved."),
	           UI::Align::kLeft, UI::TableColumnType::kFlexible);

	set_sort_column(0);
}

void SavegameTable::add_columns_special() {
	//    if (filetype_ != FileType::kGameSinglePlayer) {
	//		std::string game_mode_tooltip =
	//		   /** TRANSLATORS: Tooltip header for the "Mode" column when choosing a game/replay to
	//load.
	//		    */
	//		   /** TRANSLATORS: %s is a list of game modes. */
	//		   g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(_("Game Mode"));

	//		if (filetype_ == FileType::kReplay) {
	//			/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	//			/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	//			game_mode_tooltip += as_listitem(_("SP = Single Player"), UI::FontStyle::kTooltip);
	//		}
	//		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	//		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	//		game_mode_tooltip += as_listitem(_("MP = Multiplayer"), UI::FontStyle::kTooltip);
	//		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
	//		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
	//		game_mode_tooltip += as_listitem(_("H = Multiplayer (Host)"), UI::FontStyle::kTooltip);

	//		game_mode_tooltip += g_gr->styles()
	//		                        .font_style(UI::FontStyle::kTooltip)
	//		                        .as_font_tag(_("Numbers are the number of players."));

	//		table_.add_column(
	//		   65,
	//		   /** TRANSLATORS: Game Mode table column when choosing a game/replay to load. */
	//		   /** TRANSLATORS: Keep this to 5 letters maximum. */
	//		   /** TRANSLATORS: A tooltip will explain if you need to use an abbreviation. */
	//		   _("Mode"), game_mode_tooltip);
	//	}
}

const std::string SavegameTable::map_filename(const std::string& filename,
                                              const std::string& mapname) {
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

// bool SavegameTable::compare_date_descending(uint32_t rowa, uint32_t rowb) const {
//    const SavegameData& r1 = games_data_[table_[rowa]];
//    const SavegameData& r2 = games_data_[table_[rowb]];

//    return r1.savetimestamp < r2.savetimestamp;
//}

void SavegameTable::fill(const std::vector<SavegameData>& entries) {
	clear();

	for (size_t i = 0; i < entries.size(); ++i) {
		const SavegameData& savegame = entries[i];
		UI::Table<uintptr_t const>::EntryRecord& te = add(i);

		//        if (filetype_ != FileType::kGameSinglePlayer) {
		//            std::string gametypestring;
		//            switch (gamedata.gametype) {
		//            case GameController::GameType::kSingleplayer:
		//                /** TRANSLATORS: "Single Player" entry in the Game Mode table column. */
		//                /** TRANSLATORS: "Keep this to 6 letters maximum. */
		//                /** TRANSLATORS: A tooltip will explain the abbreviation. */
		//                /** TRANSLATORS: Make sure that this translation is consistent with the
		//                tooltip. */ gametypestring = _("SP"); break;
		//            case GameController::GameType::kNetHost:
		//                /** TRANSLATORS: "Multiplayer Host" entry in the Game Mode table column. */
		//                /** TRANSLATORS: "Keep this to 2 letters maximum. */
		//                /** TRANSLATORS: A tooltip will explain the abbreviation. */
		//                /** TRANSLATORS: Make sure that this translation is consistent with the
		//                tooltip. */
		//                /** TRANSLATORS: %1% is the number of players */
		//                gametypestring = (boost::format(_("H (%1%)")) % gamedata.nrplayers).str();
		//                break;
		//            case GameController::GameType::kNetClient:
		//                /** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
		//                /** TRANSLATORS: "Keep this to 2 letters maximum. */
		//                /** TRANSLATORS: A tooltip will explain the abbreviation. */
		//                /** TRANSLATORS: Make sure that this translation is consistent with the
		//                tooltip. */
		//                /** TRANSLATORS: %1% is the number of players */
		//                gametypestring = (boost::format(_("MP (%1%)")) % gamedata.nrplayers).str();
		//                break;
		//            case GameController::GameType::kReplay:
		//                gametypestring = "";
		//                break;
		//            case GameController::GameType::kUndefined:
		//                NEVER_HERE();
		//            }
		//            te.set_string(1, gametypestring);
		//            if (filetype_ == FileType::kReplay) {
		//                const std::string map_basename =
		//                   show_filenames_ ?
		//                      map_filename(gamedata.filename, gamedata.mapname, localize_autosave_)
		//                      : gamedata.mapname;
		//                te.set_string(2, (boost::format(pgettext("mapname_gametime", "%1% (%2%)")) %
		//                map_basename %
		//                                  gamedata.gametime)
		//                                    .str());
		//            } else {
		//                te.set_string(2, map_filename(gamedata.filename, gamedata.mapname,
		//                localize_autosave_));
		//            }
		//        } else {
		//            te.set_string(1, map_filename(gamedata.filename, gamedata.mapname,
		//            localize_autosave_));
		//        }

		//        if(savegame.is_directory()) {
		//            //add directory
		//        }
		//        else {
		//            //add savegame data
		te.set_string(0, savegame.savedatestring);
		te.set_string(1, map_filename(savegame.filename, savegame.mapname));
		//        }

		//		if (mapdata.maptype == MapData::MapType::kDirectory) {
		//			te.set_string(0, "");
		//			te.set_picture(
		//			   1, g_gr->images().get("images/ui_basic/ls_dir.png"), mapdata.localized_name);
		//			te.set_string(2, "");
		//		} else {
		//			te.set_string(0, (boost::format("(%i)") % mapdata.nrplayers).str());

		//			std::string picture = "images/ui_basic/ls_wlmap.png";
		//			if (mapdata.maptype == MapData::MapType::kScenario) {
		//				picture = "images/ui_basic/ls_wlscenario.png";
		//			} else if (mapdata.maptype == MapData::MapType::kSettlers2) {
		//				picture = "images/ui_basic/ls_s2map.png";
		//			}

		//			if (type == MapData::DisplayType::kFilenames) {
		//				set_column_title(1, _("Filename"));
		//				te.set_picture(1, g_gr->images().get(picture),
		//				               FileSystem::filename_without_ext(mapdata.filename.c_str()));
		//			} else {
		//				set_column_title(1, _("Map Name"));
		//				if (type == MapData::DisplayType::kMapnames) {
		//					te.set_picture(1, g_gr->images().get(picture), mapdata.name);
		//				} else {
		//					te.set_picture(1, g_gr->images().get(picture), mapdata.localized_name);
		//				}
		//			}

		//			te.set_string(2, (boost::format("%u x %u") % mapdata.width % mapdata.height).str());
		//		}
	}
	sort();
	layout();
}
