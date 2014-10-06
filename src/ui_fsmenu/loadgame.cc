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
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "ui_basic/icon.h"
#include "ui_basic/messagebox.h"
#include "wui/text_constants.h"


FullscreenMenuLoadGame::FullscreenMenuLoadGame
	(Widelands::Game & g, GameSettingsProvider * gsp, GameController * gc) :
	FullscreenMenuLoadMapOrGame(),

	// Main title
	m_title
		(this, get_w() / 2, m_maplisty / 3,
		 _("Choose a saved game"), UI::Align_HCenter),

	// Savegame description
	m_label_mapname
		(this, m_right_column_x, m_maplisty,
		 _("Map Name:"),
		 UI::Align_Left),
	m_ta_mapname(this,
					 m_right_column_x + m_indent, get_y_from_preceding(m_label_mapname) + m_padding,
					 get_right_column_w(m_right_column_x + m_indent), 2 * m_label_height - m_padding),

	m_label_gametime
		(this, m_right_column_x, get_y_from_preceding(m_ta_mapname) + 3 * m_padding,
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
		(this, m_right_column_x, get_y_from_preceding(m_ta_players),
		 _("Win Condition:"),
		 UI::Align_Left),
	m_ta_win_condition(this,
							 m_right_column_tab, m_label_win_condition.get_y(),
							 get_right_column_w(m_right_column_tab), m_label_height),

	m_delete
		(this, "delete",
		 m_right_column_x, get_y_from_preceding(m_ta_win_condition) + 3 * m_padding,
		 m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Delete"), std::string(), false, false),

	m_minimap_max_width(get_right_column_w(m_right_column_x)),
	m_minimap_max_height(m_buty - get_y_from_preceding(m_delete) - 6 * m_padding),
	m_minimap_icon(this,
						m_right_column_x, get_y_from_preceding(m_delete) + 3 * m_padding,
						m_minimap_max_width, m_minimap_max_height, nullptr),

	// Savegame list
	m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth),

	// "Data container" for the savegame information
	m_game(g),
	m_settings(gsp),
	m_ctrl(gc)
{
	m_title.set_textstyle(ts_big());
	m_back.set_tooltip(_("Return to the single player menu"));
	m_ok.set_tooltip(_("Load this game"));
	m_ta_mapname.set_tooltip(_("The map that this game is based on"));
	m_ta_gametime.set_tooltip(_("The time that elapsed inside this game"));
	m_ta_players.set_tooltip(_("The number of players"));
	m_ta_win_condition.set_tooltip(_("The win condition that was set for this game"));
	m_delete.set_tooltip(_("Delete this game"));
	m_minimap_icon.set_visible(false);

	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::end_modal, boost::ref(*this), 0));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	m_delete.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLoadGame::clicked_delete, boost::ref(*this)));
	m_list.selected.connect(boost::bind(&FullscreenMenuLoadGame::map_selected, this, _1));
	m_list.double_clicked.connect(boost::bind(&FullscreenMenuLoadGame::double_clicked, this, _1));

	m_list.focus();
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
	m_filename = m_list.get_selected();
	end_modal(1);
}

void FullscreenMenuLoadGame::clicked_delete()
{
	std::string fname = m_list.get_selected();
	UI::WLMessageBox confirmationBox
		(this,
		 _("Delete file"),
		 (boost::format(_("Do you really want to delete %s?")) % fname).str(),
		 UI::WLMessageBox::YESNO);
	if (confirmationBox.run()) {
		g_fs->fs_unlink(m_list.get_selected());
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


void FullscreenMenuLoadGame::map_selected(uint32_t selected)
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

	Widelands::GamePreloadPacket gpdp;
	Widelands::GameLoader gl(name, m_game);

	try {
		gl.preload_game(gpdp);
	} catch (const WException & e) {
		if (!m_settings || m_settings->settings().saved_games.empty()) {
			log("Save game '%s' must have changed from under us\nException: %s\n", name, e.what());
			m_list.remove(selected);
			return;

		} else {
			m_ok.set_enabled(true);
			m_delete.set_enabled(false);
			m_ta_mapname .set_text(_("Savegame from dedicated server"));
			m_ta_gametime.set_text(_("Unknown gametime"));
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
		m_ta_mapname.set_text(_(gpdp.get_mapname()));
	}

	uint32_t gametime = gpdp.get_gametime();
	m_ta_gametime.set_text(gametimestring(gametime));

	uint8_t number_of_players = gpdp.get_number_of_players();
	if (number_of_players > 0) {
		m_ta_players.set_text((boost::format(ngettext("%u Player", "%u Players", number_of_players))
				% static_cast<unsigned int>(number_of_players)).str());
	} else {
		m_ta_players.set_text(_("Unknown"));
	}

	m_ta_win_condition.set_text(gpdp.get_win_condition());

	std::string minimap_path = gpdp.get_minimap_path();
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
			   minimap_path, std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(name)).get()));

			m_minimap_image.reset(new_in_memory_image(std::string(name + minimap_path), surface.release()));

			// Scale it
			double scale = double(m_minimap_max_width) / m_minimap_image->width();
			double scaleY = double(m_minimap_max_height) / m_minimap_image->height();
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
			int32_t xpos = m_right_column_x + (get_w() - m_right_column_x - m_right_column_margin - w) / 2;
			int32_t ypos = m_delete.get_y() + m_delete.get_h() + 3 * m_padding;

			// Set small minimaps higher up for a more harmonious look
			if (h < m_minimap_max_height * 2 / 3) {
				ypos += (m_minimap_max_height - h) / 3;
			} else {
				ypos += (m_minimap_max_height - h) / 2;
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
 * Listbox got double clicked
 */
void FullscreenMenuLoadGame::double_clicked(uint32_t) {
	clicked_ok();
}

/**
 * Fill the file list
 */
void FullscreenMenuLoadGame::fill_list() {
	if (m_settings && !m_settings->settings().saved_games.empty()) {
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i) {
			const char * path = m_settings->settings().saved_games.at(i).path.c_str();
			m_list.add(FileSystem::filename_without_ext(path).c_str(), path);
		}
	} else { // Normal case
		// Fill it with all files we find.
		m_gamefiles = g_fs->list_directory("save");

		Widelands::GamePreloadPacket gpdp;

		const FilenameSet & gamefiles = m_gamefiles;
		for (const std::string& gamefile : gamefiles) {
			char const * const name = gamefile.c_str();

			try {
				Widelands::GameLoader gl(name, m_game);
				gl.preload_game(gpdp);

				m_list.add(FileSystem::filename_without_ext(name).c_str(), name);
			} catch (const WException &) {
				//  we simply skip illegal entries
			}
		}
	}

	if (m_list.size()) {
		m_list.select(0);
	}
}

bool FullscreenMenuLoadGame::handle_key(bool down, SDL_keysym code)
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

	return FullscreenMenuLoadMapOrGame::handle_key(down, code);
}
