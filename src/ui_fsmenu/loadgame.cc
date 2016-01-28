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
 *
 */

#include "ui_fsmenu/loadgame.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <memory>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/text_constants.h"
#include "graphic/texture.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/replay.h"
#include "ui_basic/icon.h"
#include "ui_basic/messagebox.h"

// TODO(GunChleoc): Arabic: line height broken for descriptions for Arabic.
namespace {

// This function concatenates the filename and localized map name for a savegame/replay.
// If the filename starts with the map name, the map name is omitted.
// It also prefixes autosave files with a numbered and localized "Autosave" prefix.
std::string map_filename(const std::string& filename, const std::string& mapname) {
	std::string result = FileSystem::filename_without_ext(filename.c_str());
	std::string mapname_localized;
	{
		i18n::Textdomain td("maps");
		mapname_localized = _(mapname);
	}

	if (boost::starts_with(result, "wl_autosave")) {
		std::vector<std::string> autosave_name;
		boost::split(autosave_name, result, boost::is_any_of("_"));
		if (autosave_name.empty() || autosave_name.size() < 3) {
			/** TRANSLATORS: %1% is a map's name. */
			result = (boost::format(_("Autosave: %1%")) % mapname_localized).str();
		} else {
			/** TRANSLATORS: %1% is a number, %2% a map's name. */
			result = (boost::format(_("Autosave %1%: %2%")) % autosave_name.back() % mapname_localized).str();
		}
	} else if (!(boost::starts_with(result, mapname) || boost::starts_with(result, mapname_localized))) {
		/** TRANSLATORS: %1% is a filename, %2% a map's name. */
		result = (boost::format(_("%1% (%2%)")) % result % mapname_localized).str();
	}
	return result;
}

} // namespace

FullscreenMenuLoadGame::FullscreenMenuLoadGame
	(Widelands::Game & g, GameSettingsProvider * gsp, GameController * gc, bool is_replay) :
	FullscreenMenuLoadMapOrGame(),
	m_table(this, tablex_, tabley_, tablew_, tableh_, true),

	m_is_replay(is_replay),
	// Main title
	m_title
		(this, get_w() / 2, tabley_ / 3,
		 m_is_replay ? _("Choose a replay") : _("Choose a saved game"), UI::Align_HCenter),

	// Savegame description
	m_label_mapname
		(this, right_column_x_, tabley_, "", UI::Align_Left),
	m_ta_mapname(this,
					 right_column_x_ + indent_, get_y_from_preceding(m_label_mapname) + padding_,
					 get_right_column_w(right_column_x_ + indent_), 2 * m_label_height - padding_),

	m_label_gametime
		(this, right_column_x_, get_y_from_preceding(m_ta_mapname) + 2 * padding_,
		 "",
		 UI::Align_Left),
	m_ta_gametime(this,
					  m_right_column_tab, m_label_gametime.get_y(),
					  get_right_column_w(m_right_column_tab), m_label_height),

	m_label_players
		(this, right_column_x_, get_y_from_preceding(m_ta_gametime),
		 "",
		 UI::Align_Left),
	m_ta_players(this,
					 m_right_column_tab, m_label_players.get_y(),
					 get_right_column_w(m_right_column_tab), m_label_height),

	m_label_version
		(this, right_column_x_, get_y_from_preceding(m_ta_players),
		 "",
		 UI::Align_Left),
	m_ta_version(this,
					 m_right_column_tab, m_label_version.get_y(), "", UI::Align_Left),

	m_label_win_condition
		(this, right_column_x_, get_y_from_preceding(m_ta_version) + 3 * padding_,
		 "",
		 UI::Align_Left),
	m_ta_win_condition(this,
							 right_column_x_ + indent_, get_y_from_preceding(m_label_win_condition) + padding_,
							 get_right_column_w(right_column_x_ + indent_), m_label_height),

	m_delete
		(this, "delete",
		 right_column_x_, m_buty - buth_ - 2 * padding_,
		 m_butw, buth_,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 _("Delete"), std::string(), false, false),

	m_ta_errormessage
		(this,
		 right_column_x_,
		 get_y_from_preceding(m_ta_mapname) + 2 * padding_,
		 get_right_column_w(right_column_x_),
		 m_delete.get_y() - get_y_from_preceding(m_ta_mapname) - 6 * padding_),

	m_minimap_y(get_y_from_preceding(m_ta_win_condition) + 3 * padding_),
	m_minimap_w(get_right_column_w(right_column_x_)),
	m_minimap_h(m_delete.get_y() - get_y_from_preceding(m_ta_win_condition) - 6 * padding_),
	m_minimap_icon(this,
						right_column_x_, get_y_from_preceding(m_ta_win_condition) + 3 * padding_,
						m_minimap_w, m_minimap_h, nullptr),

	// "Data container" for the savegame information
	m_game(g),
	m_settings(gsp),
	m_ctrl(gc)
{
	m_title.set_textstyle(UI::TextStyle::ui_big());
	m_ta_gametime.set_tooltip(_("The time that elapsed inside this game"));
	m_ta_players.set_tooltip(_("The number of players"));
	m_ta_version.set_tooltip(_("The version of Widelands that this game was played under"));
	m_ta_win_condition.set_tooltip(_("The win condition that was set for this game"));

	if (m_is_replay) {
		back_.set_tooltip(_("Return to the main menu"));
		ok_.set_tooltip(_("Load this replay"));
		m_ta_mapname.set_tooltip(_("The map that this replay is based on"));
		m_delete.set_tooltip(_("Delete this replay"));
	} else {
		back_.set_tooltip(_("Return to the single player menu"));
		ok_.set_tooltip(_("Load this game"));
		m_ta_mapname.set_tooltip(_("The map that this game is based on"));
		m_delete.set_tooltip(_("Delete this game"));
	}
	m_minimap_icon.set_visible(false);

	back_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLoadGame::clicked_delete, boost::ref(*this)));
	m_table.add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align_Left);
	int used_width = 130;
	if (m_is_replay || m_settings->settings().multiplayer) {
		std::vector<std::string> modes;
		if (m_is_replay) {
			/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
			/** TRANSLATORS: Make sure that you keep consistency in your translation. */
			modes.push_back(_("SP = Single Player"));
		}
		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
		modes.push_back(_("MP = Multiplayer"));
		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
		modes.push_back(_("H = Multiplayer (Host)"));
		const std::string mode_tooltip_1 =
				/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
				/** TRANSLATORS: %s is a list of game modes. */
				((boost::format(_("Game Mode: %s."))
				  % i18n::localize_list(modes, i18n::ConcatenateWith::COMMA))).str();
		const std::string mode_tooltip_2 =
				_("Numbers are the number of players.");

		m_table.add_column(65,
								 /** TRANSLATORS: Game Mode table column when choosing a game/replay to load. */
								 /** TRANSLATORS: Keep this to 5 letters maximum. */
								 /** TRANSLATORS: A tooltip will explain if you need to use an abbreviation. */
								 _("Mode"),
								 (boost::format("%s %s") % mode_tooltip_1 % mode_tooltip_2).str(),
								 UI::Align_Left);
		used_width += 65;
	}
	m_table.add_column(m_table.get_w() - used_width,
							 _("Description"),
							 _("The filename that the game was saved under followed by the map’s name, "
								"or the map’s name followed by the last objective achieved."),
								 UI::Align_Left);
	m_table.set_column_compare
		(0,
		 boost::bind(&FullscreenMenuLoadGame::compare_date_descending, this, _1, _2));
	m_table.selected.connect(boost::bind(&FullscreenMenuLoadGame::entry_selected, this));
	m_table.double_clicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	m_table.set_sort_column(0);
	m_table.focus();
	fill_table();
}

void FullscreenMenuLoadGame::think()
{
	if (m_ctrl) {
		m_ctrl->think();
	}
}

// Reverse default sort order for save date column
bool FullscreenMenuLoadGame::compare_date_descending(uint32_t rowa, uint32_t rowb)
{
	const SavegameData & r1 = m_games_data[m_table[rowa]];
	const SavegameData & r2 = m_games_data[m_table[rowb]];

	return r1.savetimestamp < r2.savetimestamp;
}


void FullscreenMenuLoadGame::clicked_ok()
{
	const SavegameData & gamedata = m_games_data[m_table.get_selected()];
	if (gamedata.errormessage.empty()) {
		m_filename = gamedata.filename;
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}
}

void FullscreenMenuLoadGame::clicked_delete()
{
	if (!m_table.has_selection()) {
		return;
	}
	const SavegameData & gamedata = m_games_data[m_table.get_selected()];

	std::string message = (boost::format("%s %s\n")
				  % m_label_mapname.get_text() % gamedata.mapname).str();

	message = (boost::format("%s %s %s\n") % message
				  % m_label_win_condition.get_text() % gamedata.wincondition).str();

	message = (boost::format("%s %s %s\n") % message
				  % _("Save Date:") % gamedata.savedatestring).str();

	message = (boost::format("%s %s %s\n") % message
				  % m_label_gametime.get_text() % gametimestring(gamedata.gametime)).str();

	message = (boost::format("%s %s %s\n\n") % message
				  % m_label_players.get_text() % gamedata.nrplayers).str();

	message = (boost::format("%s %s %s\n") % message
				  % _("Filename:") % gamedata.filename).str();

	if (m_is_replay) {
		message = (boost::format("%s\n\n%s")
					  % _("Do you really want to delete this replay?") % message).str();
	} else {
		message = (boost::format("%s\n\n%s")
					  % _("Do you really want to delete this game?") % message).str();
	}

	UI::WLMessageBox confirmationBox
		(this, _("Confirm deleting file"), message, UI::WLMessageBox::MBoxType::kOkCancel);
	if (confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		g_fs->fs_unlink(gamedata.filename);
		if (m_is_replay) {
			g_fs->fs_unlink(gamedata.filename + WLGF_SUFFIX);
		}
		fill_table();
	}
}


bool FullscreenMenuLoadGame::set_has_selection()
{
	bool has_selection = m_table.has_selection();
	ok_.set_enabled(has_selection);
	m_delete.set_enabled(has_selection);

	if (!has_selection) {
		m_label_mapname .set_text(std::string());
		m_label_gametime.set_text(std::string());
		m_label_players.set_text(std::string());
		m_label_version.set_text(std::string());
		m_label_win_condition.set_text(std::string());

		m_ta_mapname .set_text(std::string());
		m_ta_gametime.set_text(std::string());
		m_ta_players.set_text(std::string());
		m_ta_version.set_text(std::string());
		m_ta_win_condition.set_text(std::string());
		m_minimap_icon.set_icon(nullptr);
		m_minimap_icon.set_visible(false);
		m_minimap_icon.set_no_frame();
		m_minimap_image.reset();
	} else {
		m_label_mapname .set_text(_("Map Name:"));
		m_label_gametime.set_text(_("Gametime:"));
		m_label_players.set_text(_("Players:"));
		m_label_win_condition.set_text(_("Win Condition:"));
	}
	return has_selection;
}


void FullscreenMenuLoadGame::entry_selected()
{
	if (set_has_selection()) {

		const SavegameData & gamedata = m_games_data[m_table.get_selected()];
		m_ta_errormessage.set_text(gamedata.errormessage);

		if (gamedata.errormessage.empty()) {
			m_ta_errormessage.set_visible(false);
			m_ta_mapname.set_text(gamedata.mapname);
			m_ta_gametime.set_text(gametimestring(gamedata.gametime));

			uint8_t number_of_players = gamedata.nrplayers;
			if (number_of_players > 0) {
				m_ta_players.set_text((boost::format("%u") % static_cast<unsigned int>(number_of_players)).str());
			} else {
				m_label_players.set_text("");
				m_ta_players.set_text("");
			}

			if (gamedata.version.empty()) {
				m_label_version.set_text("");
				m_ta_version.set_text("");
			} else {
				m_label_version.set_text(_("Widelands Version:"));
				m_ta_version.set_text(gamedata.version);
			}

			{
				i18n::Textdomain td("win_conditions");
				m_ta_win_condition.set_text(_(gamedata.wincondition));
			}

			std::string minimap_path = gamedata.minimap_path;
			// Delete former image
			m_minimap_icon.set_icon(nullptr);
			m_minimap_icon.set_visible(false);
			m_minimap_icon.set_no_frame();
			m_minimap_image.reset();
			// Load the new one
			if (!minimap_path.empty()) {
				try {
					// Load the image
					m_minimap_image = load_image(
					   minimap_path,
					   std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get());

					// Scale it
					double scale = double(m_minimap_w) / m_minimap_image->width();
					double scaleY = double(m_minimap_h) / m_minimap_image->height();
					if (scaleY < scale) {
						scale = scaleY;
					}
					if (scale > 1.0) scale = 1.0; // Don't make the image too big; fuzziness will result
					uint16_t w = scale * m_minimap_image->width();
					uint16_t h = scale * m_minimap_image->height();

					// Center the minimap in the available space
					int32_t xpos = right_column_x_ + (get_w() - m_right_column_margin - w - right_column_x_) / 2;
					int32_t ypos = m_minimap_y;

					// Set small minimaps higher up for a more harmonious look
					if (h < m_minimap_h * 2 / 3) {
						ypos += (m_minimap_h - h) / 3;
					} else {
						ypos += (m_minimap_h - h) / 2;
					}

					m_minimap_icon.set_size(w, h);
					m_minimap_icon.set_pos(Point(xpos, ypos));
					m_minimap_icon.set_frame(UI_FONT_CLR_FG);
					m_minimap_icon.set_visible(true);
					m_minimap_icon.set_icon(m_minimap_image.get());
				} catch (const std::exception & e) {
					log("Failed to load the minimap image : %s\n", e.what());
				}
			}
		} else {
			m_label_mapname.set_text(_("Filename:"));
			m_ta_mapname.set_text(gamedata.mapname);
			m_label_gametime.set_text("");
			m_ta_gametime.set_text("");
			m_label_players.set_text("");
			m_ta_players.set_text("");
			m_label_version.set_text("");
			m_ta_version.set_text("");
			m_label_win_condition.set_text("");
			m_ta_win_condition.set_text("");

			m_minimap_icon.set_icon(nullptr);
			m_minimap_icon.set_visible(false);
			m_minimap_icon.set_no_frame();
			m_minimap_image.reset();

			m_ta_errormessage.set_visible(true);
			ok_.set_enabled(false);
		}
	}
}

/**
 * Fill the file list
 */
void FullscreenMenuLoadGame::fill_table() {

	m_games_data.clear();
	m_table.clear();


	if (m_settings && !m_settings->settings().saved_games.empty()) {
		SavegameData gamedata;
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i) {
			gamedata.filename = m_settings->settings().saved_games.at(i).path;
			m_games_data.push_back(gamedata);

			UI::Table<uintptr_t const>::EntryRecord & te =
				m_table.add(m_games_data.size() - 1);
			te.set_string(0, FileSystem::filename_without_ext(gamedata.filename.c_str()).c_str());
		}
	} else { // Normal case
		// Fill it with all files we find.

		FilenameSet gamefiles;

		if (m_is_replay) {
			gamefiles = filter(g_fs->list_directory(REPLAY_DIR),
								[](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX);});
		} else {
			gamefiles = g_fs->list_directory("save");
		}

		Widelands::GamePreloadPacket gpdp;

		for (const std::string& gamefilename : gamefiles) {
			if (gamefilename == "save/campvis" || gamefilename == "save\\campvis") {
				continue;
			}

			SavegameData gamedata;

			std::string savename = gamefilename;
			if (m_is_replay) savename += WLGF_SUFFIX;

			if (!g_fs->file_exists(savename.c_str())) {
				continue;
			}

			gamedata.filename = gamefilename;

			try {
				Widelands::GameLoader gl(savename.c_str(), m_game);
				gl.preload_game(gpdp);

				gamedata.gametype = gpdp.get_gametype();

				if (!m_is_replay) {
					if (m_settings->settings().multiplayer) {
						if (gamedata.gametype == GameController::GameType::SINGLEPLAYER) {
							continue;
						}
					} else if (gamedata.gametype > GameController::GameType::SINGLEPLAYER) {
						continue;
					}
				}

				gamedata.mapname = gpdp.get_mapname();
				gamedata.gametime = gpdp.get_gametime();
				gamedata.nrplayers = gpdp.get_number_of_players();
				gamedata.version = gpdp.get_version();

				gamedata.savetimestamp = gpdp.get_savetimestamp();
				time_t t;
				time(&t);
				struct tm * currenttime  = localtime(&t);
				// We need to put these into variables because of a sideeffect of the localtime function.
				int8_t current_year = currenttime->tm_year;
				int8_t current_month = currenttime->tm_mon;
				int8_t current_day = currenttime->tm_mday;

				struct tm * savedate  = localtime(&gamedata.savetimestamp);

				if (gamedata.savetimestamp > 0) {
					if (savedate->tm_year == current_year &&
						 savedate->tm_mon == current_month &&
						 savedate->tm_mday == current_day) {  // Today

						// Adding the 0 padding in a separate statement so translators won't have to deal with it
						const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

						/** TRANSLATORS: Display date for choosing a savegame/replay */
						/** TRANSLATORS: hour:minute */
						gamedata.savedatestring = (boost::format(_("Today, %1%:%2%"))
															 % savedate->tm_hour % minute).str();
					} else if ((savedate->tm_year == current_year &&
									savedate->tm_mon == current_month &&
									savedate->tm_mday == current_day - 1) ||
								  (savedate->tm_year == current_year - 1 &&
									savedate->tm_mon == 11 && current_month == 0 &&
									savedate->tm_mday == 31 && current_day == 1)) {  // Yesterday
						// Adding the 0 padding in a separate statement so translators won't have to deal with it
						const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

						/** TRANSLATORS: Display date for choosing a savegame/replay */
						/** TRANSLATORS: hour:minute */
						gamedata.savedatestring = (boost::format(_("Yesterday, %1%:%2%"))
															 % savedate->tm_hour % minute).str();
					} else {  // Older

						/** TRANSLATORS: Display date for choosing a savegame/replay */
						/** TRANSLATORS: month day, year */
						gamedata.savedatestring = (boost::format(_("%2% %1%, %3%"))
							 % savedate->tm_mday
							 % localize_month(savedate->tm_mon)
							 % (1900 + savedate->tm_year)).str();
					}
				}

				{
					i18n::Textdomain td("win_conditions");
					gamedata.wincondition = _(gpdp.get_win_condition());
				}
				gamedata.minimap_path = gpdp.get_minimap_path();
				m_games_data.push_back(gamedata);

				UI::Table<uintptr_t const>::EntryRecord & te =
					m_table.add(m_games_data.size() - 1);
				te.set_string(0, gamedata.savedatestring);

				if (m_is_replay || m_settings->settings().multiplayer) {
					std::string gametypestring;
					switch (gamedata.gametype) {
						case GameController::GameType::SINGLEPLAYER:
							/** TRANSLATORS: "Single Player" entry in the Game Mode table column. */
							/** TRANSLATORS: "Keep this to 6 letters maximum. */
							/** TRANSLATORS: A tooltip will explain the abbreviation. */
							/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
							gametypestring = _("SP");
							break;
						case GameController::GameType::NETHOST:
							/** TRANSLATORS: "Multiplayer Host" entry in the Game Mode table column. */
							/** TRANSLATORS: "Keep this to 2 letters maximum. */
							/** TRANSLATORS: A tooltip will explain the abbreviation. */
							/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
							/** TRANSLATORS: %1% is the number of players */
							gametypestring = (boost::format(_("H (%1%)"))
												 % static_cast<unsigned int>(gamedata.nrplayers)).str();
							break;
						case GameController::GameType::NETCLIENT:
							/** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
							/** TRANSLATORS: "Keep this to 2 letters maximum. */
							/** TRANSLATORS: A tooltip will explain the abbreviation. */
							/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
							/** TRANSLATORS: %1% is the number of players */
							gametypestring = (boost::format(_("MP (%1%)"))
												% static_cast<unsigned int>(gamedata.nrplayers)).str();
							break;
						case GameController::GameType::REPLAY:
							gametypestring = "";
							break;
					}
					te.set_string(1, gametypestring);
					te.set_string(2, map_filename(gamedata.filename, gamedata.mapname));
				} else {
					te.set_string(1, map_filename(gamedata.filename, gamedata.mapname));
				}
			} catch (const WException & e) {
				//  we simply skip illegal entries
				gamedata.errormessage =
						((boost::format("%s\n\n%s\n\n%s"))
						 /** TRANSLATORS: Error message introduction for when an old savegame can't be loaded */
						 % _("This file has the wrong format and can’t be loaded."
							  " Maybe it was created with an older version of Widelands.")
						 /** TRANSLATORS: This text is on a separate line with an error message below */
						 % _("Error message:")
						 % e.what()).str();

				const std::string fs_filename = FileSystem::filename_without_ext(gamedata.filename.c_str());
				gamedata.mapname = fs_filename;
				m_games_data.push_back(gamedata);

				UI::Table<uintptr_t const>::EntryRecord & te =
					m_table.add(m_games_data.size() - 1);
				te.set_string(0, "");
				if (m_is_replay || m_settings->settings().multiplayer) {
					te.set_string(1, "");
					/** TRANSLATORS: Prefix for incompatible files in load game screens */
					te.set_string(2, (boost::format(_("Incompatible: %s")) % fs_filename).str());
				} else {
					te.set_string(1, (boost::format(_("Incompatible: %s")) % fs_filename).str());
				}
			}
		}
	}
	m_table.sort();

	if (m_table.size()) {
		m_table.select(0);
	}
}

bool FullscreenMenuLoadGame::handle_key(bool down, SDL_Keysym code)
{
	if (!down)
		return false;

	switch (code.sym)
	{
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		case SDLK_DELETE:
			clicked_delete();
			return true;
		default:
			break;
	}

	return FullscreenMenuLoadMapOrGame::handle_key(down, code);
}
