/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "fullscreen_menu_loadreplay.h"

#include "game.h"
#include "game_loader.h"
#include "game_preload_data_packet.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "replay.h"


Fullscreen_Menu_LoadReplay::Fullscreen_Menu_LoadReplay(Game* g) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

	m_game(g),

// UI::Buttons
	m_back(this, 570, 505, 200, 26, 0,
	       &Fullscreen_Menu_LoadReplay::end_modal, this, 0,
	       _("Back")),

	m_ok(this, 570, 535, 200, 26, 2,
	     &Fullscreen_Menu_LoadReplay::clicked_ok, this,
	     _("OK")),

// Create the list area
	m_list(this, 15, 205, 455, 365),

	m_title(this, MENU_XRES / 2, 90, _("Choose a replay!"), Align_HCenter)
{
	m_title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	m_list.selected.set(this, &Fullscreen_Menu_LoadReplay::replay_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_LoadReplay::double_clicked);
	fill_list();
}

Fullscreen_Menu_LoadReplay::~Fullscreen_Menu_LoadReplay()
{
}

void Fullscreen_Menu_LoadReplay::clicked_ok()
{
	m_filename = m_list.get_selected();
	end_modal(1);
}

void Fullscreen_Menu_LoadReplay::double_clicked(uint32_t)
{
	clicked_ok();
}


void Fullscreen_Menu_LoadReplay::replay_selected(uint32_t)
{
	// TODO: Extract quick info about the replay
}


/**
 * Fill the file list by simply fetching all files that end with the
 * replay suffix and have a valid associated savegame.
 */
void Fullscreen_Menu_LoadReplay::fill_list()
{
	filenameset_t files;

	g_fs->FindFiles(REPLAY_DIR, "*" REPLAY_SUFFIX, &files, 1);

	for (filenameset_t::iterator pname = files.begin(); pname != files.end(); ++pname) {
		std::string savename = *pname + WLGF_SUFFIX;

		if (!g_fs->FileExists(savename))
			continue;

		FileSystem* fs = 0;

		try {
			Game_Preload_Data_Packet gpdp;
			fs = g_fs->MakeSubFileSystem(savename);
			Game_Loader gl(*fs, m_game);
			gl.preload_game(&gpdp);

			char* fname = strdup(FileSystem::FS_Filename(pname->c_str()));
			FileSystem::FS_StripExtension(fname);
			m_list.add(fname, *pname);
			free(fname);
		} catch (_wexception&) {
			// we simply skip illegal entries
		}

		delete fs;
	}

	if (m_list.size())
		m_list.select(0);
}
