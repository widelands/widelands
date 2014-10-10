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
#include "graphic/image_transformations.h"
#include "graphic/in_memory_image.h"
#include "graphic/surface.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/replay.h"
#include "ui_basic/icon.h"
#include "ui_basic/messagebox.h"
#include "wui/text_constants.h"


FullscreenMenuLoadGame::FullscreenMenuLoadGame
	(Widelands::Game & g, GameSettingsProvider * gsp, GameController * gc, bool is_replay) :
	FullscreenMenuLoadMapOrGame(),

	m_is_replay(is_replay),
	// Main title
	m_title
		(this, get_w() / 2, m_maplisty / 3,
		 m_is_replay ? _("Choose a replay") : _("Choose a saved game"), UI::Align_HCenter),

	// Savegame description
	m_label_mapname
		(this, m_right_column_x, m_maplisty,
		 _("Map Name:"),
		 UI::Align_Left),
	m_ta_mapname(this,
					 m_right_column_x + m_indent, get_y_from_preceding(m_label_mapname) + m_padding,
					 get_right_column_w(m_right_column_x + m_indent), 2 * m_label_height - m_padding),

	m_label_gametime
		(this, m_right_column_x, get_y_from_preceding(m_ta_mapname) + 2 * m_padding,
		 _("Gametime:"),
		 UI::Align_Left),
	m_ta_gametime(this,
					  m_right_column_tab, m_label_gametime.get_y(),
					  get_right_column_w(m_right_column_tab), m_label_height),

	m_label_players
		(this, m_right_column_x, get_y_from_preceding(m_ta_gametime),
		 _("Players:"),
		 UI::Align_Left),
	m_ta_players(this,
					 m_right_column_tab, m_label_players.get_y(),
					 get_right_column_w(m_right_column_tab), m_label_height),

	m_label_win_condition
		(this, m_right_column_x, get_y_from_preceding(m_ta_players) + 3 * m_padding,
		 _("Win Condition:"),
		 UI::Align_Left),
	m_ta_win_condition(this,
							 m_right_column_x + m_indent, get_y_from_preceding(m_label_win_condition) + m_padding,
							 get_right_column_w(m_right_column_x + m_indent), m_label_height),

	m_delete
		(this, "delete",
		 m_right_column_x, m_buty - m_buth - 2 * m_padding,
		 m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Delete"), std::string(), false, false),

	m_minimap_y(get_y_from_preceding(m_ta_win_condition) + 3 * m_padding),
	m_minimap_w(get_right_column_w(m_right_column_x)),
	m_minimap_h(m_delete.get_y() - get_y_from_preceding(m_ta_win_condition) - 6 * m_padding),
	m_minimap_icon(this,
						m_right_column_x, get_y_from_preceding(m_ta_win_condition) + 3 * m_padding,
						m_minimap_w, m_minimap_h, nullptr),

	// Savegame table
	m_table(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth),

	// "Data container" for the savegame information
	m_game(g),
	m_settings(gsp),
	m_ctrl(gc)
{
	m_title.set_textstyle(ts_big());
	m_ta_gametime.set_tooltip(_("The time that elapsed inside this game"));
	m_ta_players.set_tooltip(_("The number of players"));
	m_ta_win_condition.set_tooltip(_("The win condition that was set for this game"));

	if (m_is_replay) {
		m_back.set_tooltip(_("Return to the main menu"));
		m_ok.set_tooltip(_("Load this replay"));
		m_ta_mapname.set_tooltip(_("The map that this replay is based on"));
		m_delete.set_tooltip(_("Delete this replay"));
	} else {
		m_back.set_tooltip(_("Return to the single player menu"));
		m_ok.set_tooltip(_("Load this game"));
		m_ta_mapname.set_tooltip(_("The map that this game is based on"));
		m_delete.set_tooltip(_("Delete this game"));
	}
	m_minimap_icon.set_visible(false);

	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_back, boost::ref(*this)));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLoadGame::clicked_delete, boost::ref(*this)));
	m_table.add_column(m_table.get_w(), _("Filename"), _("Filename"), UI::Align_Left);
	m_table.selected.connect(boost::bind(&FullscreenMenuLoadGame::map_selected, this, _1));
	m_table.double_clicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	m_table.set_sort_column(0);
	m_table.focus();
	fill_list();
}

void FullscreenMenuLoadGame::think()
{
	if (m_ctrl) {
		m_ctrl->think();
	}
}


void FullscreenMenuLoadGame::clicked_ok()
{
	m_filename = m_table.get_selected();
	end_modal(1);
}

void FullscreenMenuLoadGame::clicked_delete()
{
	if (!m_table.has_selection()) {
		return;
	}
	const SavegameData & gamedata = m_games_data[m_table.get_selected()];

	std::string fname = gamedata.filename;
	UI::WLMessageBox confirmationBox
		(this,
		 _("Delete file"),
		 (boost::format(_("Do you really want to delete %s?")) % fname).str(),
		 UI::WLMessageBox::YESNO);
	if (confirmationBox.run()) {
		g_fs->fs_unlink(fname);
		if (m_is_replay) {
			g_fs->fs_unlink(fname + WLGF_SUFFIX);
		}
		fill_list();
	}
}

/**
 * Update buttons and labels to reflect that no loadable game is selected.
 */
void FullscreenMenuLoadGame::no_selection()
{
	m_ok.set_enabled(false);
	m_delete.set_enabled(false);

	m_ta_mapname .set_text(std::string());
	m_ta_gametime.set_text(std::string());
	m_ta_players.set_text(std::string());
	m_ta_win_condition.set_text(std::string());
	m_minimap_icon.set_icon(nullptr);
	m_minimap_icon.set_visible(false);
	m_minimap_icon.set_no_frame();
	m_minimap_image.reset();
}


void FullscreenMenuLoadGame::map_selected(uint32_t)
{
	if (!m_table.has_selection()) {
		no_selection();
		return;
	}

	const SavegameData & gamedata = m_games_data[m_table.get_selected()];

	m_ok.set_enabled(true);
	m_delete.set_enabled(true);

	m_ta_mapname.set_text(gamedata.name);
	m_ta_gametime.set_text(gametimestring(gamedata.gametime));

	uint8_t number_of_players = gamedata.nrplayers;
	if (number_of_players > 0) {
		m_ta_players.set_text((boost::format("%u") % static_cast<unsigned int>(number_of_players)).str());
	} else {
		m_ta_players.set_text(_("Unknown"));
	}

	m_ta_win_condition.set_text(gamedata.wincondition);

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
			std::unique_ptr<Surface> surface(load_image(
				minimap_path, std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get()));

			m_minimap_image.reset(new_in_memory_image(std::string(gamedata.filename + minimap_path), surface.release()));

			// Scale it
			double scale = double(m_minimap_w) / m_minimap_image->width();
			double scaleY = double(m_minimap_h) / m_minimap_image->height();
			if (scaleY < scale) {
				scale = scaleY;
			}
			if (scale > 1.0) scale = 1.0; // Don't make the image too big; fuzziness will result
			uint16_t w = scale * m_minimap_image->width();
			uint16_t h = scale * m_minimap_image->height();
			const Image* resized = ImageTransformations::resize(m_minimap_image.get(), w, h);
			// keeps our in_memory_image around and give to icon the one
			// from resize that is handled by the cache. It is still linked to our
			// surface
			m_minimap_icon.set_size(w, h);

			// Center the minimap in the available space
			int32_t xpos = m_right_column_x + (get_w() - m_right_column_margin - w - m_right_column_x) / 2;
			int32_t ypos = m_minimap_y;

			// Set small minimaps higher up for a more harmonious look
			if (h < m_minimap_h * 2 / 3) {
				ypos += (m_minimap_h - h) / 3;
			} else {
				ypos += (m_minimap_h - h) / 2;
			}

			m_minimap_icon.set_pos(Point(xpos, ypos));
			m_minimap_icon.set_frame(UI_FONT_CLR_FG);
			m_minimap_icon.set_visible(true);
			m_minimap_icon.set_icon(resized);
		} catch (const std::exception & e) {
			log("Failed to load the minimap image : %s\n", e.what());
		}
	}
}


/**
 * Fill the file list
 */
void FullscreenMenuLoadGame::fill_list() {

	m_games_data.clear();
	m_table.clear();

	SavegameData* gamedata = new SavegameData();

	if (m_settings && !m_settings->settings().saved_games.empty()) {
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i) {
			gamedata->filename = m_settings->settings().saved_games.at(i).path;
			m_games_data.push_back(*gamedata);

			UI::Table<uintptr_t const>::EntryRecord & te =
				m_table.add(m_games_data.size() - 1);
			te.set_string(0, FileSystem::filename_without_ext(gamedata->filename.c_str()).c_str());
		}
	} else { // Normal case
		// Fill it with all files we find.

		if (m_is_replay) {
			m_gamefiles = filter(g_fs->list_directory(REPLAY_DIR),
								[](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX);});
		} else {
			m_gamefiles = g_fs->list_directory("save");
		}

		Widelands::GamePreloadPacket gpdp;

		const FilenameSet & gamefiles = m_gamefiles;

		for (const std::string& gamefilename : gamefiles) {

			gamedata = new SavegameData();

			std::string savename = gamefilename;
			if (m_is_replay) savename += WLGF_SUFFIX;

			if (!g_fs->file_exists(savename.c_str())) {
				continue;
			}

			gamedata->filename = gamefilename;

			try {
				Widelands::GameLoader gl(savename.c_str(), m_game);
				gl.preload_game(gpdp);
				{
					i18n::Textdomain td("maps");
					gamedata->name = _(gpdp.get_mapname());
				}
				gamedata->gametime = gpdp.get_gametime();
				gamedata->nrplayers = gpdp.get_number_of_players();
				{
					i18n::Textdomain td("win_conditions");
					gamedata->wincondition = _(gpdp.get_win_condition());
				}
				gamedata->minimap_path = gpdp.get_minimap_path();
				m_games_data.push_back(*gamedata);

				UI::Table<uintptr_t const>::EntryRecord & te =
					m_table.add(m_games_data.size() - 1);
				te.set_string(0, FileSystem::filename_without_ext(gamefilename.c_str()).c_str());

			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}
	m_table.sort();

	if (m_table.size()) {
		m_table.select(0);
	} else {
		no_selection();
	}
}

bool FullscreenMenuLoadGame::handle_key(bool down, SDL_keysym code)
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
