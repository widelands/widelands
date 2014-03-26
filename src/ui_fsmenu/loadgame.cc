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

#include <cstdio>

#include <boost/format.hpp>

#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "graphic/graphic.h"
#include "graphic/image_loader_impl.h"
#include "graphic/image_transformations.h"
#include "graphic/in_memory_image.h"
#include "graphic/surface.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/game.h"
#include "timestring.h"
#include "ui_basic/icon.h"
#include "ui_basic/messagebox.h"



Fullscreen_Menu_LoadGame::Fullscreen_Menu_LoadGame
	(Widelands::Game & g, GameSettingsProvider * gsp, GameController * gc) :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),
	m_minimap_max_size(get_w() * 15 / 100),

// "Data holder" for the savegame information
	m_game(g),

// Buttons
	m_back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 31 / 40, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("OK"), std::string(), false, false),
	m_delete
		(this, "delete",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Delete"), std::string(), false, false),

// Savegame list
	m_list
		(this, get_w() * 47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000),

// Text areas
	m_title
		(this,
		 get_w() / 2, get_h() * 3 / 20,
		 _("Choose saved game"), UI::Align_HCenter),
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
	m_label_minimap
		(this,
		 get_w() * 7 / 10,  get_h() * 10 / 20,
		 _("Minimap:"), UI::Align_Right),
	m_minimap_icon
		(this, get_w() * 71 / 100, get_h() * 10 / 20,
		 m_minimap_max_size, m_minimap_max_size, nullptr),
	m_settings(gsp),
	m_ctrl(gc),
	m_image_loader(new ImageLoaderImpl())
{
	m_back.sigclicked.connect(boost::bind(&Fullscreen_Menu_LoadGame::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&Fullscreen_Menu_LoadGame::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_LoadGame::clicked_delete, boost::ref(*this)));

	m_back.set_font(font_small());
	m_ok.set_font(font_small());
	m_delete.set_font(font_small());

	m_title         .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_mapname .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_tamapname     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_gametime.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_tagametime    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_players .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_ta_players    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_ta_win_condition.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_minimap_icon.set_visible(false);
	m_list          .set_font(m_fn, m_fs);
	m_list.selected.connect(boost::bind(&Fullscreen_Menu_LoadGame::map_selected, this, _1));
	m_list.double_clicked.connect(boost::bind(&Fullscreen_Menu_LoadGame::double_clicked, this, _1));
	m_list.focus();
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
		 (boost::format(_("Do you really want to delete %s?")) % fname).str(),
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
	m_ta_players.set_text(std::string());
	m_ta_win_condition.set_text(std::string());
	m_minimap_icon.setIcon(nullptr);
	m_minimap_icon.set_visible(false);
	m_minimap_icon.setNoFrame();
	m_minimap_image.reset();
}


void Fullscreen_Menu_LoadGame::map_selected(uint32_t selected)
{
	if (!m_list.has_selection()) {
		no_selection();
		return;
	}
	const char * const name = m_list.get_selected();
	if (!name) {
		no_selection();
		return;
	}

	Widelands::Game_Preload_Data_Packet gpdp;
	Widelands::Game_Loader gl(name, m_game);

	try {
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

	//Try to translate the map name.
	//This will work on every official map as expected
	//and 'fail silently' (not find a translation) for already translated campaign map names.
	//It will also translate 'false-positively' on any user-made map which shares a name with
	//the official maps, but this should not be a problem to worry about.
	{
		i18n::Textdomain td("maps");
		m_tamapname.set_text(_(gpdp.get_mapname()));
	}

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

	std::string minimap_path = gpdp.get_minimap_path();
	// Delete former image
	m_minimap_icon.setIcon(nullptr);
	m_minimap_icon.set_visible(false);
	m_minimap_icon.setNoFrame();
	m_minimap_image.reset();
	// Load the new one
	if (!minimap_path.empty()) {
		try {
			// Load the image
			FileSystem* save_fs = g_fs->MakeSubFileSystem(name);
			std::unique_ptr<Surface> surface(m_image_loader->load(minimap_path, save_fs));
			m_minimap_image.reset(new_in_memory_image(std::string(name + minimap_path), surface.release()));
			delete save_fs;
			// Scale it
			double scale = double(m_minimap_max_size) / m_minimap_image->width();
			double scaleY = double(m_minimap_max_size) / m_minimap_image->height();
			if (scaleY < scale) {
				scale = scaleY;
			}
			uint16_t w = scale * m_minimap_image->width();
			uint16_t h = scale * m_minimap_image->height();
			const Image* resized = ImageTransformations::resize(m_minimap_image.get(), w, h);
			// keeps our in_memory_image around and give to icon the one
			// from resize that is handled by the cache. It is still linked to our
			// surface
			m_minimap_icon.set_size(w, h);
			m_minimap_icon.setFrame(UI_FONT_CLR_FG);
			m_minimap_icon.set_visible(true);
			m_minimap_icon.setIcon(resized);
		} catch (const std::exception & e) {
			log("Failed to load the minimap image : %s\n", e.what());
		}
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
		m_gamefiles = g_fs->ListDirectory("save");

		Widelands::Game_Preload_Data_Packet gpdp;

		const filenameset_t & gamefiles = m_gamefiles;
		container_iterate_const(filenameset_t, gamefiles, i) {
			char const * const name = i.current->c_str();

			try {
				Widelands::Game_Loader gl(name, m_game);
				gl.preload_game(gpdp);

				m_list.add(FileSystem::FS_FilenameWoExt(name).c_str(), name);
			} catch (const _wexception &) {
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
		break;
	}

	return Fullscreen_Menu_Base::handle_key(down, code);
}
