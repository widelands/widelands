/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "loadgame.h"

#include "gamecontroller.h"
#include "gamesettings.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/game.h"
#include "ui_basic/messagebox.h"

#include <cstdio>

Fullscreen_Menu_LoadGame::Fullscreen_Menu_LoadGame
	(Widelands::Game & g, GameSettingsProvider * gsp, GameController * gc) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// "Data holder" for the savegame information
	m_game(g),

// Buttons
	m_back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind(&Fullscreen_Menu_LoadGame::end_modal, boost::ref(*this), 0),
		 _("Back"), std::string(), true, false),
	m_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 15 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 boost::bind(&Fullscreen_Menu_LoadGame::clicked_ok, boost::ref(*this)),
		 _("OK"), std::string(), false, false),
	m_delete
		(this, "delete",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind
			 (&Fullscreen_Menu_LoadGame::clicked_delete, boost::ref(*this)),
		 _("Delete"), std::string(), false, false),

// Savegame list
	m_list
		(this, get_w() * 47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000),

// Text areas
	m_title
		(this,
		 get_w() / 2, get_h() * 3 / 20,
		 _("Choose saved game!"), UI::Align_HCenter),
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

	m_settings(gsp),
	m_ctrl(gc)
{
	m_back.set_font(font_small());
	m_ok.set_font(font_small());
	m_delete.set_font(font_small());

	m_title         .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_mapname .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_tamapname     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_gametime.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_tagametime    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list          .set_font(m_fn, m_fs);
	m_list.selected.set(this, &Fullscreen_Menu_LoadGame::map_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_LoadGame::double_clicked);
	fill_list();
}

void Fullscreen_Menu_LoadGame::think()
{
	if (m_ctrl)
		m_ctrl->think();
}


void Fullscreen_Menu_LoadGame::clicked_ok()
{
	m_filename = m_list.get_selected();
	end_modal(1);
}

void Fullscreen_Menu_LoadGame::clicked_delete()
{
	std::string fname = m_list.get_selected();
	UI::WLMessageBox confirmationBox
		(this,
		 _("Delete file"),
		 _("Do you really want to delete ") + fname + "?",
		 UI::WLMessageBox::YESNO);
	if (confirmationBox.run()) {
		g_fs->Unlink(m_list.get_selected());
		m_list.clear();
		fill_list();
		if (m_list.empty()) {
			//  else fill_list() already selected the first entry
			no_selection();
		}
	}
}

/**
 * Update buttons and labels to reflect that no loadable game is selected.
 */
void Fullscreen_Menu_LoadGame::no_selection()
{
	m_ok.set_enabled(false);
	m_delete.set_enabled(false);

	m_tamapname .set_text(std::string());
	m_tagametime.set_text(std::string());
}


void Fullscreen_Menu_LoadGame::map_selected(uint32_t selected)
{
	if (!m_list.has_selection()) {
		no_selection();
		return;
	}

	if (const char * const name = m_list.get_selected()) {
		Widelands::Game_Preload_Data_Packet gpdp;

		try {
			Widelands::Game_Loader gl(name, m_game);
			gl.preload_game(gpdp);
		} catch (const _wexception & e) {
			if (!m_settings || m_settings->settings().saved_games.empty()) {
				log("Save game '%s' must have changed from under us\nException: %s\n", name, e.what());
				m_list.remove(selected);
				return;
			} else {
				m_ok.set_enabled(true);
				m_delete.set_enabled(false);
				m_tamapname .set_text(_("Savegame from dedicated server"));
				m_tagametime.set_text(_("Unknown gametime"));
				return;
			}
		}

		m_ok.set_enabled(true);
		m_delete.set_enabled(true);
		m_tamapname.set_text(gpdp.get_mapname());

		char buf[200];
		uint32_t gametime = gpdp.get_gametime();

		int32_t hours = gametime / 3600000;
		gametime -= hours * 3600000;
		int32_t minutes = gametime / 60000;

		sprintf(buf, "%02i:%02i", hours, minutes);
		m_tagametime.set_text(buf);
	} else {
		no_selection();
	}
}

/**
 * Listbox got double clicked
 */
void Fullscreen_Menu_LoadGame::double_clicked(uint32_t) {
	clicked_ok();
}

/**
 * Fill the file list
 */
void Fullscreen_Menu_LoadGame::fill_list() {
	if (m_settings && !m_settings->settings().saved_games.empty()) {
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i) {
			const char * path = m_settings->settings().saved_games.at(i).path.c_str();
			m_list.add(FileSystem::FS_FilenameWoExt(path).c_str(), path);
		}
	} else { // Normal case
		// Fill it with all files we find.
		g_fs->FindFiles("save", "*", &m_gamefiles, 0);

		Widelands::Game_Preload_Data_Packet gpdp;

		const filenameset_t & gamefiles = m_gamefiles;
		container_iterate_const(filenameset_t, gamefiles, i) {
			char const * const name = i.current->c_str();

			try {
				Widelands::Game_Loader gl(name, m_game);
				gl.preload_game(gpdp);

				m_list.add(FileSystem::FS_FilenameWoExt(name).c_str(), name);
			} catch (_wexception const & e) {
				//  we simply skip illegal entries
			}
		}
	}

	if (m_list.size())
		m_list.select(0);
}

bool Fullscreen_Menu_LoadGame::handle_key(bool down, SDL_keysym code)
{
	if (!down)
		return false;

	switch (code.sym)
	{
	case SDLK_KP2:
		if (code.mod & KMOD_NUM)
			break;
	case SDLK_DOWN:
	case SDLK_KP8:
		if (code.mod & KMOD_NUM)
			break;
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
	case SDLK_DELETE:
		clicked_delete();
		return true;
	case SDLK_ESCAPE:
		end_modal(0);
		return true;
	default:
		break;
	}

	return Fullscreen_Menu_Base::handle_key(down, code);
}
