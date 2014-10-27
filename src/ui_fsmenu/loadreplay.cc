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

#include <string>

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
	FullscreenMenuBase("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 19 / 400),

// Buttons
	m_back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 15 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("OK"), std::string(), false, false),
	m_delete
		(this, "delete",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Delete"), std::string(), false, false),

// Replay list
	m_list
		(this,
		 get_w() *  47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000),

// Text area
	m_title
		(this,
		 get_w() / 2, get_h() * 3 / 20,
		 _("Choose a replay"), UI::Align_HCenter),
	m_label_mapname
		(this,
		 get_w() * 7 / 10,  get_h() * 17 / 50,
		 _("Map Name:"), UI::Align_Right),
	m_tamapname(this, get_w() * 71 / 100, get_h() * 17 / 50),
	m_label_gametime
		(this,
		 get_w() * 7 / 10,  get_h() * 3 / 8,
		 _("Gametime:"), UI::Align_Right),
	m_tagametime(this, get_w() * 71 / 100, get_h() * 3 / 8),
	m_label_players
		(this,
		 get_w() * 7 / 10,  get_h() * 41 / 100,
		 _("Players:"), UI::Align_Right),
	m_ta_players
		(this, get_w() * 71 / 100, get_h() * 41 / 100),
	m_ta_win_condition
		(this, get_w() * 71 / 100, get_h() * 9 / 20),
	m_game(g)
{
	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLoadReplay::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuLoadReplay::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
		 	 (&FullscreenMenuLoadReplay::clicked_delete, boost::ref(*this)));

	m_list.set_font(ui_fn(), fs_small());
	m_list.selected.connect(boost::bind(&FullscreenMenuLoadReplay::replay_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&FullscreenMenuLoadReplay::double_clicked, this, _1));

	m_title         .set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);
	m_label_mapname .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_tamapname     .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_label_gametime.set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_tagametime    .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_label_players .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_ta_players    .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_ta_win_condition.set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_list          .set_font(ui_fn(), fs_small());
	m_back.set_font(font_small());
	m_ok.set_font(font_small());
	m_delete.set_font(font_small());

	fill_list();
}


void FullscreenMenuLoadReplay::clicked_ok()
{
	if (!m_list.has_selection())
		return;

	m_filename = m_list.get_selected();
	end_modal(1);
}

void FullscreenMenuLoadReplay::double_clicked(uint32_t)
{
	clicked_ok();
}

void FullscreenMenuLoadReplay::clicked_delete()
{
	if (!m_list.has_selection())
		return;

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

	m_tamapname .set_text(std::string());
	m_tagametime.set_text(std::string());
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
		m_tamapname.set_text(gpdp.get_mapname());

		uint32_t gametime = gpdp.get_gametime();
		m_tagametime.set_text(gametimestring(gametime));

		if (gpdp.get_number_of_players() > 0) {
			m_ta_players.set_text(std::to_string(static_cast<unsigned int>(gpdp.get_number_of_players())));
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
	for
		(FilenameSet::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		std::string savename = *pname + WLGF_SUFFIX;

		if (!g_fs->file_exists(savename))
			continue;

		try {
			Widelands::GameLoader gl(savename, m_game);
			gl.preload_game(gpdp);

			// TODO(GunChleoc): Do something more structured with this information
			// in the ui_fsmenu refactoring branch
			const std::string fs_filename = FileSystem::filename_without_ext(pname->c_str());

			// Begin localization section
			std::string displaytitle = fs_filename;
			if ((is_timestring(fs_filename) || fs_filename == "wl_autosave")
				 && gpdp.get_saveyear() > 0
				 && gpdp.get_savemonth() > 0
				 && gpdp.get_saveday() > 0) {

				switch (gpdp.get_gametype()) {
					case GameController::GameType::SINGLEPLAYER:
						/** TRANSLATORS: Gametype used in filenames for loading games */
						displaytitle = _("Single Player");
						break;
					case GameController::GameType::NETHOST:
						/** TRANSLATORS: Gametype used in filenames for loading games */
						/** TRANSLATORS: %1% is the number of players */
						displaytitle = (boost::format(_("Multiplayer (%1%, Host)"))
											 % static_cast<unsigned int>(gpdp.get_number_of_players())).str();
						break;
					case GameController::GameType::NETCLIENT:
						/** TRANSLATORS: Gametype used in filenames for loading games */
						/** TRANSLATORS: %1% is the number of players */
						displaytitle = (boost::format(_("Multiplayer (%1%)"))
											% static_cast<unsigned int>(gpdp.get_number_of_players())).str();
						break;
					default:
						// TODO(GunChleoc): Localize this
						displaytitle = ("Unknown game type");
				}

				/** TRANSLATORS: Filenames for loading games */
				/** TRANSLATORS: month day, year hour:minute gametype – mapname */
				/** TRANSLATORS: The mapname should always come last, because it */
				/** TRANSLATORS: can be longer than the space we have */
				// TODO(GunChleoc): Localize this
				displaytitle = (boost::format("%1$s %2$u, %3$u %4$u:%5$u %6$s – %7$s")
							 % localize_month(gpdp.get_savemonth())
							 % static_cast<unsigned int>(gpdp.get_saveday())
							 % static_cast<unsigned int>(gpdp.get_saveyear())
							 % static_cast<unsigned int>(gpdp.get_savehour())
							 % static_cast<unsigned int>(gpdp.get_saveminute())
							 % displaytitle
							 % gpdp.get_mapname()).str();

				if (fs_filename == "wl_autosave") {
					/** TRANSLATORS: Used in filenames for loading games */
					// TODO(GunChleoc): Localize this
					displaytitle = (boost::format(("Autosave: %1%")) % displaytitle).str();
				}
			}
			// End localization section

			m_list.add(displaytitle, *pname);
		} catch (const WException &) {} //  we simply skip illegal entries
	}

	if (m_list.size())
		m_list.select(0);
}

bool FullscreenMenuLoadReplay::handle_key(bool down, SDL_keysym code)
{
	if (!down)
		return false;

	switch (code.sym)
	{
	case SDLK_KP2:
		if (code.mod & KMOD_NUM)
			break;
		/* no break */
	case SDLK_DOWN:
	case SDLK_KP8:
		if (code.mod & KMOD_NUM)
			break;
		/* no break */
	case SDLK_UP:
		m_list.handle_key(down, code);
		return true;
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		clicked_ok();
		return true;
	case SDLK_KP_PERIOD:
		if (code.mod & KMOD_NUM)
			break;
		/* no break */
	case SDLK_DELETE:
		clicked_delete();
		return true;
	case SDLK_ESCAPE:
		end_modal(0);
		return true;
	default:
		break; // not handled
	}

	return FullscreenMenuBase::handle_key(down, code);
}
