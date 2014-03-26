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

#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/game.h"
#include "logic/replay.h"
#include "timestring.h"
#include "ui_basic/messagebox.h"

Fullscreen_Menu_LoadReplay::Fullscreen_Menu_LoadReplay(Widelands::Game & g) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

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
	m_back.sigclicked.connect(boost::bind(&Fullscreen_Menu_LoadReplay::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&Fullscreen_Menu_LoadReplay::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
		 	 (&Fullscreen_Menu_LoadReplay::clicked_delete, boost::ref(*this)));

	m_list.set_font(ui_fn(), fs_small());
	m_list.selected.connect(boost::bind(&Fullscreen_Menu_LoadReplay::replay_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&Fullscreen_Menu_LoadReplay::double_clicked, this, _1));

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


void Fullscreen_Menu_LoadReplay::clicked_ok()
{
	if (!m_list.has_selection())
		return;

	m_filename = m_list.get_selected();
	end_modal(1);
}

void Fullscreen_Menu_LoadReplay::double_clicked(uint32_t)
{
	clicked_ok();
}

void Fullscreen_Menu_LoadReplay::clicked_delete()
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
		g_fs->Unlink(m_list.get_selected());
		g_fs->Unlink(m_list.get_selected() + WLGF_SUFFIX);
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
void Fullscreen_Menu_LoadReplay::no_selection()
{
	m_ok.set_enabled(false);
	m_delete.set_enabled(false);

	m_tamapname .set_text(std::string());
	m_tagametime.set_text(std::string());
}

void Fullscreen_Menu_LoadReplay::replay_selected(uint32_t const selected)
{
	if (!m_list.has_selection()) {
		no_selection();
		return;
	}


	if (m_list.has_selection()) {
		std::string name = m_list.get_selected() + WLGF_SUFFIX;
		Widelands::Game_Preload_Data_Packet gpdp;

		try {
			Widelands::Game_Loader gl(name, m_game);
			gl.preload_game(gpdp);
		} catch (const _wexception & e) {
			log("Replay '%s' must have changed from under us\nException: %s\n", name.c_str(), e.what());
			m_list.remove(selected);
			return;
		}

		m_ok.set_enabled(true);
		m_delete.set_enabled(true);
		m_tamapname.set_text(gpdp.get_mapname());

		char buf[20];
		uint32_t gametime = gpdp.get_gametime();
		m_tagametime.set_text(gametimestring(gametime));

		if (gpdp.get_number_of_players() > 0) {
			sprintf(buf, "%i", gpdp.get_number_of_players());
		} else {
			sprintf(buf, "%s", _("Unknown"));
		}
		m_ta_players.set_text(buf);

		m_ta_win_condition.set_text(gpdp.get_win_condition());
	} else {
		no_selection();
	}
}


/**
 * Fill the file list by simply fetching all files that end with the
 * replay suffix and have a valid associated savegame.
 */
void Fullscreen_Menu_LoadReplay::fill_list()
{
	filenameset_t files;

	files = filter(g_fs->ListDirectory(REPLAY_DIR),
	               [](const std::string& fn) {return boost::ends_with(fn, REPLAY_SUFFIX);});

	Widelands::Game_Preload_Data_Packet gpdp;
	for
		(filenameset_t::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		std::string savename = *pname + WLGF_SUFFIX;

		if (!g_fs->FileExists(savename))
			continue;

		try {
			Widelands::Game_Loader gl(savename, m_game);
			gl.preload_game(gpdp);

			m_list.add
				(FileSystem::FS_FilenameWoExt(pname->c_str()).c_str(), *pname);
		} catch (const _wexception &) {} //  we simply skip illegal entries
	}

	if (m_list.size())
		m_list.select(0);
}

bool Fullscreen_Menu_LoadReplay::handle_key(bool down, SDL_keysym code)
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

	return Fullscreen_Menu_Base::handle_key(down, code);
}
