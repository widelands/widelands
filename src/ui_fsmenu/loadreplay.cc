/*
 * Copyright (C) 2007-2011 by the Widelands Development Team
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

#include "ui_fsmenu/loadreplay.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/replay.h"
#include "ui_basic/messagebox.h"
#include "wui/text_constants.h"

FullscreenMenuLoadReplay::FullscreenMenuLoadReplay(Widelands::Game & g) :
	FullscreenMenuLoadMapOrGame(),

	// Main title
	m_title
		(this, get_w() / 2, m_maplisty / 3,
		 _("Choose a replay"), UI::Align_HCenter),

	// Replay description
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

	// Replay list
	m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth),

	// "Data container" for the replay information
	m_game(g)
{
	m_title.set_textstyle(ts_big());
	m_back.set_tooltip(_("Return to the main menu"));
	m_ok.set_tooltip(_("Load this replay"));
	m_ta_mapname.set_tooltip(_("The map that this replay is based on"));
	m_ta_gametime.set_tooltip(_("The time that elapsed inside this game"));
	m_ta_players.set_tooltip(_("The number of players"));
	m_ta_win_condition.set_tooltip(_("The win condition that was set for this game"));
	m_delete.set_tooltip(_("Delete this replay"));

	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLoadReplay::clicked_back, boost::ref(*this)));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuLoadReplay::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
		 	 (&FullscreenMenuLoadReplay::clicked_delete, boost::ref(*this)));
	m_list.selected.connect(boost::bind(&FullscreenMenuLoadReplay::replay_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&FullscreenMenuLoadReplay::clicked_ok, boost::ref(*this)));

	m_list.focus();
	fill_list();
}


void FullscreenMenuLoadReplay::clicked_ok()
{
	if (!m_list.has_selection()) {
		return;
	}

	m_filename = m_list.get_selected();
	end_modal(1);
}

void FullscreenMenuLoadReplay::clicked_delete()
{
	if (!m_list.has_selection()) {
		return;
	}

	std::string fname = m_list.get_selected();
	UI::WLMessageBox confirmationBox
		(this,
		 _("Delete file"),
		 (boost::format(_("Do you really want to delete %s?")) % fname).str(),
		 UI::WLMessageBox::YESNO);
	if (confirmationBox.run()) {
		g_fs->fs_unlink(m_list.get_selected());
		g_fs->fs_unlink(m_list.get_selected() + WLGF_SUFFIX);
		m_list.clear();
		fill_list();
		if (m_list.empty()) {
			//  else fill_list() already selected the first entry
			no_selection();
		}
	}
}

/**
 * Update buttons and labels to reflect that no loadable replay is selected.
 */
void FullscreenMenuLoadReplay::no_selection()
{
	m_ok.set_enabled(false);
	m_delete.set_enabled(false);

	m_ta_mapname .set_text(std::string());
	m_ta_gametime.set_text(std::string());
}

void FullscreenMenuLoadReplay::replay_selected(uint32_t const selected)
{
	if (!m_list.has_selection()) {
		no_selection();
		return;
	}

	if (m_list.has_selection()) {
		std::string name = m_list.get_selected() + WLGF_SUFFIX;
		Widelands::GamePreloadPacket gpdp;

		try {
			Widelands::GameLoader gl(name, m_game);
			gl.preload_game(gpdp);
		} catch (const WException & e) {
			log("Replay '%s' must have changed from under us\nException: %s\n", name.c_str(), e.what());
			m_list.remove(selected);
			return;
		}

		m_ok.set_enabled(true);
		m_delete.set_enabled(true);
		m_ta_mapname.set_text(gpdp.get_mapname());

		uint32_t gametime = gpdp.get_gametime();
		m_ta_gametime.set_text(gametimestring(gametime));

		uint8_t number_of_players = gpdp.get_number_of_players();
		if (number_of_players > 0) {
			m_ta_players.set_text((boost::format("%u") % static_cast<unsigned int>(number_of_players)).str());
		} else {
			m_ta_players.set_text(_("Unknown"));
		}

		m_ta_win_condition.set_text(gpdp.get_win_condition());
	} else {
		no_selection();
	}
}


/**
 * Fill the file list by simply fetching all files that end with the
 * replay suffix and have a valid associated savegame.
 */
void FullscreenMenuLoadReplay::fill_list()
{
	FilenameSet files;

	files = filter(g_fs->list_directory(REPLAY_DIR),
	               [](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX);});

	Widelands::GamePreloadPacket gpdp;
	for (const std::string& replayfilename : files) {
		std::string savename = replayfilename + WLGF_SUFFIX;

		if (!g_fs->file_exists(savename)) {
			continue;
		}

		try {
			Widelands::GameLoader gl(savename, m_game);
			gl.preload_game(gpdp);

			m_list.add
				(FileSystem::filename_without_ext(replayfilename.c_str()).c_str(), replayfilename);
		} catch (const WException &) {} //  we simply skip illegal entries
	}

	if (m_list.size()) {
		m_list.select(0);
	}
}

bool FullscreenMenuLoadReplay::handle_key(bool down, SDL_keysym code)
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
			break; // not handled
	}

	return FullscreenMenuLoadMapOrGame::handle_key(down, code);
}
