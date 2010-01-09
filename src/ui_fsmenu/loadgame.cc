/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#include "logic/game.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "ui_basic/messagebox.h"

#include <cstdio>

Fullscreen_Menu_LoadGame::Fullscreen_Menu_LoadGame(Widelands::Game & g) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (m_xres / 4),
	m_buth (m_yres * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// "Data holder" for the savegame information
	m_game(g),

// Buttons
	m_back
		(this,
		 m_xres * 71 / 100, m_yres * 9 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_LoadGame::end_modal, *this, 0,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_ok
		(this,
		 m_xres * 71 / 100, m_yres * 15 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 &Fullscreen_Menu_LoadGame::clicked_ok, *this,
		 _("OK"), std::string(), false, false,
		 m_fn, m_fs),
	m_delete
		 (this,
		  m_xres * 71 / 100, m_yres * 17 / 20, m_butw, m_buth,
		  g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		  &Fullscreen_Menu_LoadGame::clicked_delete, *this,
		  _("Delete"), std::string(), false, false,
		  m_fn, m_fs),

// Replay list
	m_list
		(this, m_xres * 47 / 2500, m_yres * 3417 / 10000,
		 m_xres * 711 / 1250, m_yres * 6083 / 10000),

// Text areas
	m_title
		(this,
		 m_xres / 2, m_yres * 3 / 20,
		 _("Choose saved game!"), UI::Align_HCenter),
	m_label_mapname
		(this,
		 m_xres * 7 / 10,  m_yres * 17 / 50,
		 _("Map Name:"), UI::Align_Right),
	m_tamapname(this, m_xres * 71 / 100, m_yres * 17 / 50),
	m_label_gametime
		(this,
		 m_xres * 7 / 10,  m_yres * 3 / 8,
		 _("Gametime:"), UI::Align_Right),
	m_tagametime(this, m_xres * 71 / 100, m_yres * 3 / 8)
{
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
		if (!m_list.size()) { // else fill_list() already selected the first entry
			m_ok.set_enabled(false);
			m_delete.set_enabled(false);
		}
	}
}


void Fullscreen_Menu_LoadGame::map_selected(uint32_t) {
	if (const char * const name = m_list.get_selected()) {
		Widelands::Game_Loader gl(name, m_game);
		Widelands::Game_Preload_Data_Packet gpdp;
		gl.preload_game(gpdp); //  This has worked before, no problem

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
		m_tamapname .set_text(std::string());
		m_tagametime.set_text(std::string());
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
	//  Fill it with all files we find.
	g_fs->FindFiles("save", "*", &m_gamefiles, 0);

	Widelands::Game_Preload_Data_Packet gpdp;

	const filenameset_t & gamefiles = m_gamefiles;
	container_iterate_const(filenameset_t, gamefiles, i) {
		char const * const name = i.current->c_str();

		try {
			Widelands::Game_Loader gl(name, m_game);
			gl.preload_game(gpdp);

			char const * extension, * fname =
				FileSystem::FS_Filename(name, extension);
			char fname_without_extension[extension - fname + 1];
			for (char * p = fname_without_extension;; ++p, ++fname)
				if (fname == extension) {*p = '\0'; break;} else *p = *fname;
			m_list.add(fname_without_extension, name);
		} catch (_wexception const &) {} //  we simply skip illegal entries
	}

	if (m_list.size())
		m_list.select(0);
}
