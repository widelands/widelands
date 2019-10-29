#include "savegametable.h"

SavegameTable::SavegameTable(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : UI::Table<uintptr_t>(parent, x, y, w, h, style) {

	add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	add_column(0, _("Description"),
	           _("The filename that the game was saved under followed by the map’s name, "
	             "or the map’s name followed by the last objective achieved."),
	           UI::Align::kLeft, UI::TableColumnType::kFlexible);

	//    /** TRANSLATORS: Column title for number of players in map list */
	//    add_column(35, _("Pl."), _("Number of players"), UI::Align::kCenter);
	//    add_column(0, _("Filename"), _("The name of the map or scenario"), UI::Align::kLeft,
	//               UI::TableColumnType::kFlexible);
	//    add_column(90, _("Size"), _("The size of the map (Width x Height)"));

	set_sort_column(0);
}

void SavegameTable::fill(const std::vector<SavegameData>& entries) {
	clear();

	for (size_t i = 0; i < entries.size(); ++i) {
		const SavegameData& savegame = entries[i];
		UI::Table<uintptr_t const>::EntryRecord& te = add(i);

		te.set_string(0, savegame.savedatestring);

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

		te.set_string(1, "komplizierter name?");
		//        te.set_string(1, map_filename(savegame.filename, savegame.mapname,
		//        localize_autosave_));

		//        if(savegame.is_directory()) {
		//            //add directory
		//        }
		//        else {
		//            //add savegame data
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
