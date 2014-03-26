/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2013 by the Widelands Development Team
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

#include "wui/game_main_menu_save_game.h"

#include <boost/format.hpp>
#include <libintl.h>

#include "constants.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "game_io/game_saver.h"
#include "gamecontroller.h"
#include "i18n.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/playersmanager.h"
#include "profile/profile.h"
#include "timestring.h"
#include "wui/interactive_gamebase.h"

using boost::format;

Interactive_GameBase & Game_Main_Menu_Save_Game::igbase() {
	return ref_cast<Interactive_GameBase, UI::Panel>(*get_parent());
}

#define WINDOW_WIDTH                                                        440
#define WINDOW_HEIGHT                                                       440
#define VMARGIN                                                               5
#define VSPACING                                                              5
#define HSPACING                                                              5
#define EDITBOX_HEIGHT                                                       20
#define BUTTON_HEIGHT                                                        20
#define LIST_WIDTH                                                          280
#define LIST_HEIGHT   (WINDOW_HEIGHT - 2 * VMARGIN - VSPACING - EDITBOX_HEIGHT)
#define EDITBOX_Y                    (WINDOW_HEIGHT - EDITBOX_HEIGHT - VMARGIN)
#define DESCRIPTION_X                         (VMARGIN + LIST_WIDTH + VSPACING)
#define DESCRIPTION_WIDTH              (WINDOW_WIDTH - DESCRIPTION_X - VMARGIN)
#define CANCEL_Y                      (WINDOW_HEIGHT - BUTTON_HEIGHT - VMARGIN)
#define DELETE_Y                          (CANCEL_Y - BUTTON_HEIGHT - VSPACING)
#define OK_Y                              (DELETE_Y - BUTTON_HEIGHT - VSPACING)

Game_Main_Menu_Save_Game::Game_Main_Menu_Save_Game
	(Interactive_GameBase & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "save_game", &registry,
		 WINDOW_WIDTH, WINDOW_HEIGHT, _("Save Game")),
	m_ls     (this, HSPACING, VSPACING,  LIST_WIDTH, LIST_HEIGHT),
	m_name_label
		(this, DESCRIPTION_X,  5, 0, 20, _("Map Name:"),  UI::Align_CenterLeft),
	m_mapname
		(this, DESCRIPTION_X, 20, 0, 20, " ",              UI::Align_CenterLeft),
	m_gametime_label
		(this, DESCRIPTION_X, 45, 0, 20, _("Game Time:"), UI::Align_CenterLeft),
	m_gametime
		(this, DESCRIPTION_X, 60, 0, 20, " ",              UI::Align_CenterLeft),
	m_players_label
		(this, DESCRIPTION_X, 85, 0, 20, " ",              UI::Align_CenterLeft),
	m_win_condition_label
		(this, DESCRIPTION_X, 110, 0, 20, _("Win condition:"), UI::Align_CenterLeft),
	m_win_condition
		(this, DESCRIPTION_X, 125, 0, 20, " ",             UI::Align_CenterLeft),
	m_curdir(SaveHandler::get_base_dir())
{
	m_editbox =
		new UI::EditBox
			(this, HSPACING, EDITBOX_Y, LIST_WIDTH, EDITBOX_HEIGHT,
			 g_gr->images().get("pics/but1.png"));
	m_editbox->changed.connect(boost::bind(&Game_Main_Menu_Save_Game::edit_box_changed, this));
	m_editbox->ok.connect(boost::bind(&Game_Main_Menu_Save_Game::ok, this));

	m_button_ok =
		new UI::Button
			(this, "ok",
			 DESCRIPTION_X, OK_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->images().get("pics/but4.png"),
			 _("OK"),
			 std::string(),
			 false);
	m_button_ok->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::ok, this));

	UI::Button * cancelbtn =
		new UI::Button
			(this, "cancel",
			 DESCRIPTION_X, CANCEL_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->images().get("pics/but4.png"),
			 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::die, this));

	UI::Button * deletebtn =
		new UI::Button
			(this, "delete",
			 DESCRIPTION_X, DELETE_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->images().get("pics/but4.png"),
			 _("Delete"));
	deletebtn->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::delete_clicked, this));

	m_ls.selected.connect(boost::bind(&Game_Main_Menu_Save_Game::selected, this, _1));
	m_ls.double_clicked.connect(boost::bind(&Game_Main_Menu_Save_Game::double_clicked, this, _1));

	fill_list();

	center_to_parent();
	move_to_top();

	std::string cur_filename = parent.game().save_handler().get_cur_filename();
	if (!cur_filename.empty()) {
		select_by_name(cur_filename);
	} else {
		// Display current game infos
		{
			//Try to translate the map name.
			i18n::Textdomain td("maps");
			m_mapname.set_text(_(parent.game().get_map()->get_name()));
		}
		uint32_t gametime = parent.game().get_gametime();
		m_gametime.set_text(gametimestring(gametime));

		int player_nr = parent.game().player_manager()->get_number_of_players();
		// TODO: This should be ngettext(" %i player" etc. with boost::format, but it refuses to work
		/** TRANSLATORS: This is preceded by a number */
		m_players_label.set_text(
		   (boost::format(ngettext("%i player", "%i players", player_nr)) % player_nr).str());
		m_win_condition.set_text(parent.game().get_win_condition_displayname());
	}

	m_editbox->focus();
	pause_game(true);
}


/**
 * called when a item is selected
 */
void Game_Main_Menu_Save_Game::selected(uint32_t) {
	const std::string & name = m_ls.get_selected();

	Widelands::Game_Loader gl(name, igbase().game());
	Widelands::Game_Preload_Data_Packet gpdp;
	gl.preload_game(gpdp); //  This has worked before, no problem

	{
		m_editbox->setText(FileSystem::FS_FilenameWoExt(name.c_str()));
	}
	m_button_ok->set_enabled(true);

	//Try to translate the map name.
	{
		i18n::Textdomain td("maps");
		m_mapname.set_text(_(gpdp.get_mapname()));
	}

	uint32_t gametime = gpdp.get_gametime();
	m_gametime.set_text(gametimestring(gametime));

	if (gpdp.get_number_of_players() > 0) {
		char buf[200];
		sprintf
			(buf, "%i %s", gpdp.get_number_of_players(),
			// TODO: This should be ngettext(" %i player" etc. with boost::format, but it refuses to work
			/** TRANSLATORS: This is preceded by a number */
			ngettext("player", "players", gpdp.get_number_of_players()));
			m_players_label.set_text(buf);
	} else {
		// Keep label empty
		m_players_label.set_text("");
	}
	m_win_condition.set_text(gpdp.get_win_condition());
}

/**
 * An Item has been doubleclicked
 */
void Game_Main_Menu_Save_Game::double_clicked(uint32_t) {
	ok();
}

/*
 * fill the file list
 */
void Game_Main_Menu_Save_Game::fill_list() {
	m_ls.clear();
	filenameset_t gamefiles;

	//  Fill it with all files we find.
	gamefiles = g_fs->ListDirectory(m_curdir);

	Widelands::Game_Preload_Data_Packet gpdp;

	for
		(filenameset_t::iterator pname = gamefiles.begin();
		 pname != gamefiles.end();
		 ++pname)
	{
		char const * const name = pname->c_str();

		try {
			Widelands::Game_Loader gl(name, igbase().game());
			gl.preload_game(gpdp);
			m_ls.add(FileSystem::FS_FilenameWoExt(name).c_str(), name);
		} catch (const _wexception &) {} //  we simply skip illegal entries
	}
}

void Game_Main_Menu_Save_Game::select_by_name(std::string name)
{
	for (uint32_t idx = 0; idx < m_ls.size(); idx++) {
		const std::string val = m_ls[idx];
		if (name == val) {
			m_ls.select(idx);
			return;
		}
	}
}

/*
 * The editbox was changed. Enable ok button
 */
void Game_Main_Menu_Save_Game::edit_box_changed() {
	m_button_ok->set_enabled(m_editbox->text().size());
}

static void dosave
	(Interactive_GameBase & igbase, const std::string & complete_filename)
{
	Widelands::Game & game = igbase.game();

	std::string error;
	if (!game.save_handler().save_game(game, complete_filename, &error)) {
		std::string s =
			_
			("Game Saving Error!\nSaved game file may be corrupt!\n\n"
			 "Reason given:\n");
		s += error;
		UI::WLMessageBox mbox
			(&igbase, _("Save Game Error!"), s, UI::WLMessageBox::OK);
		mbox.run();
	}
	game.save_handler().set_current_filename(complete_filename);
}

struct SaveWarnMessageBox : public UI::WLMessageBox {
	SaveWarnMessageBox
		(Game_Main_Menu_Save_Game & parent, const std::string & filename)
		:
		UI::WLMessageBox
			(&parent,
			 _("Save Game Error!"),
			(boost::format(_("A file with the name ‘%s’ already exists. Overwrite?"))
				% FileSystem::FS_Filename(filename.c_str())).str(),
			 YESNO),
		m_filename(filename)
	{}

	Game_Main_Menu_Save_Game & menu_save_game() {
		return ref_cast<Game_Main_Menu_Save_Game, UI::Panel>(*get_parent());
	}


	void pressedYes() override
	{
		g_fs->Unlink(m_filename);
		dosave(menu_save_game().igbase(), m_filename);
		menu_save_game().die();
	}

	void pressedNo() override
	{
		die();
	}

private:
	std::string const m_filename;
};

/**
 * Called when the Ok button is clicked or the Return key pressed in the edit box.
 */
void Game_Main_Menu_Save_Game::ok()
{
	if (m_editbox->text().empty())
		return;

	std::string const complete_filename =
		igbase().game().save_handler().create_file_name
			(m_curdir, m_editbox->text());

	//  Check if file exists. If it does, show a warning.
	if (g_fs->FileExists(complete_filename)) {
		new SaveWarnMessageBox(*this, complete_filename);
	} else {
		dosave(igbase(), complete_filename);
		die();
	}
}

void Game_Main_Menu_Save_Game::die()
{
	pause_game(false);
	UI::UniqueWindow::die();
}



struct DeletionMessageBox : public UI::WLMessageBox {
	DeletionMessageBox
		(Game_Main_Menu_Save_Game & parent, const std::string & filename)
		:
		UI::WLMessageBox
			(&parent,
			 _("File deletion"),
			 str
				 (format(_("Do you really want to delete the file %s?")) %
				  FileSystem::FS_Filename(filename.c_str())),
			 YESNO),
		m_filename(filename)
	{}

	void pressedYes() override
	{
		g_fs->Unlink(m_filename);
		ref_cast<Game_Main_Menu_Save_Game, UI::Panel>(*get_parent()).fill_list();
		die();
	}

	void pressedNo() override
	{
		die();
	}

private:
	std::string const m_filename;
};


/**
 * Called when the delete button has been clicked
 */
void Game_Main_Menu_Save_Game::delete_clicked()
{
	std::string const complete_filename =
		igbase().game().save_handler().create_file_name
			(m_curdir, m_editbox->text());

	//  Check if file exists. If it does, let the user confirm the deletion.
	if (g_fs->FileExists(complete_filename))
		new DeletionMessageBox(*this, complete_filename);
}

void Game_Main_Menu_Save_Game::pause_game(bool paused)
{
	if (igbase().is_multiplayer()) {
		return;
	}
	igbase().game().gameController()->setPaused(paused);
}
