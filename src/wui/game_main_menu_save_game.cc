/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011 by the Widelands Development Team
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

#include "game_main_menu_save_game.h"

#include "io/filesystem/filesystem.h"
#include "constants.h"
#include "logic/game.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "game_io/game_saver.h"
#include "interactive_gamebase.h"
#include "io/filesystem/layered_filesystem.h"
#include "profile/profile.h"

#include <boost/format.hpp>
using boost::format;

Interactive_GameBase & Game_Main_Menu_Save_Game::igbase() {
	return ref_cast<Interactive_GameBase, UI::Panel>(*get_parent());
}


Game_Main_Menu_Save_Game::Game_Main_Menu_Save_Game
	(Interactive_GameBase & parent, UI::UniqueWindow::Registry & registry)
:
#define WINDOW_WIDTH                                                        440
#define WINDOW_HEIGHT                                                       440
#define VMARGIN                                                               5
#define HMARGIN                                                               5
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
	UI::UniqueWindow
		(&parent, "save_game", &registry,
		 WINDOW_WIDTH, WINDOW_HEIGHT, _("Save Game")),
	m_ls     (this, HSPACING, VSPACING,  LIST_WIDTH, LIST_HEIGHT),
	m_name_label
		(this, DESCRIPTION_X,  5, 0, 20, _("Map Name: "),  UI::Align_CenterLeft),
	m_name
		(this, DESCRIPTION_X, 20, 0, 20, " ",              UI::Align_CenterLeft),
	m_gametime_label
		(this, DESCRIPTION_X, 45, 0, 20, _("Game Time: "), UI::Align_CenterLeft),
	m_gametime
		(this, DESCRIPTION_X, 60, 0, 20, " ",              UI::Align_CenterLeft),
	m_curdir(SaveHandler::get_base_dir())
{
	m_editbox =
		new UI::EditBox
			(this, HSPACING, EDITBOX_Y, LIST_WIDTH, EDITBOX_HEIGHT,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"));
	m_editbox->changed.connect(boost::bind(&Game_Main_Menu_Save_Game::edit_box_changed, this));
	m_editbox->ok.connect(boost::bind(&Game_Main_Menu_Save_Game::ok, this));

	m_button_ok =
		new UI::Button
			(this, "ok",
			 DESCRIPTION_X, OK_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
			 _("OK"),
			 std::string(),
			 false);
	m_button_ok->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::ok, this));

	UI::Button * cancelbtn =
		new UI::Button
			(this, "cancel",
			 DESCRIPTION_X, CANCEL_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
			 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::die, this));

	UI::Button * deletebtn =
		new UI::Button
			(this, "delete",
			 DESCRIPTION_X, DELETE_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
			 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
			 _("Delete"));
	deletebtn->sigclicked.connect(boost::bind(&Game_Main_Menu_Save_Game::delete_clicked, this));

	m_ls.selected.connect(boost::bind(&Game_Main_Menu_Save_Game::selected, this, _1));
	m_ls.double_clicked.connect(boost::bind(&Game_Main_Menu_Save_Game::double_clicked, this, _1));

	fill_list();

	center_to_parent();
	move_to_top();

	m_editbox->focus();
}


/**
 * called when a item is selected
 */
void Game_Main_Menu_Save_Game::selected(uint32_t) {
	std::string const & name = m_ls.get_selected();

	Widelands::Game_Loader gl(name, igbase().game());
	Widelands::Game_Preload_Data_Packet gpdp;
	gl.preload_game(gpdp); //  This has worked before, no problem

	{
		m_editbox->setText(FileSystem::FS_FilenameWoExt(name.c_str()));
	}
	m_button_ok->set_enabled(true);

	m_name.set_text(gpdp.get_mapname());
	char buf[200];
	uint32_t gametime = gpdp.get_gametime();
#define SPLIT_GAMETIME(unit, factor) \
   uint32_t const unit = gametime / factor; gametime %= factor;
	SPLIT_GAMETIME(days, 86400000);
	SPLIT_GAMETIME(hours, 3600000);
	SPLIT_GAMETIME(minutes, 60000);
	SPLIT_GAMETIME(seconds,  1000);
	sprintf
		(buf,
		 _("%02ud%02uh%02u'%02u\"%03u"),
		 days, hours, minutes, seconds, gametime);
	m_gametime.set_text(buf);
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
	filenameset_t m_gamefiles;

	//  Fill it with all files we find.
	g_fs->FindFiles(m_curdir, "*", &m_gamefiles, 0);

	Widelands::Game_Preload_Data_Packet gpdp;

	for
		(filenameset_t::iterator pname = m_gamefiles.begin();
		 pname != m_gamefiles.end();
		 ++pname)
	{
		char const * const name = pname->c_str();

		try {
			Widelands::Game_Loader gl(name, igbase().game());
			gl.preload_game(gpdp);
			m_ls.add(FileSystem::FS_FilenameWoExt(name).c_str(), name);
		} catch (_wexception const &) {} //  we simply skip illegal entries
	}

	if (m_ls.size())
		m_ls.select(0);
}

/*
 * The editbox was changed. Enable ok button
 */
void Game_Main_Menu_Save_Game::edit_box_changed() {
	m_button_ok->set_enabled(m_editbox->text().size());
}

static void dosave
	(Interactive_GameBase & igbase, std::string const & complete_filename)
{
	Widelands::Game & game = igbase.game();

	std::string error;
	if (!game.save_handler().save_game(game, complete_filename, &error)) {
		std::string s =
			_
			("Game Saving Error!\nSaved Game-File may be corrupt!\n\n"
			 "Reason given:\n");
		s += error;
		UI::WLMessageBox mbox
			(&igbase, _("Save Game Error!!"), s, UI::WLMessageBox::OK);
		mbox.run();
	}
}

struct SaveWarnMessageBox : public UI::WLMessageBox {
	SaveWarnMessageBox
		(Game_Main_Menu_Save_Game & parent, std::string const & filename)
		:
		UI::WLMessageBox
			(&parent,
			 _("Save Game Error!!"),
			 std::string(_("A File with the name "))
			 +
			 FileSystem::FS_Filename(filename.c_str())
			 +
			 _(" already exists. Overwrite?"),
			 YESNO),
		m_filename(filename)
	{}

	Game_Main_Menu_Save_Game & menu_save_game() {
		return ref_cast<Game_Main_Menu_Save_Game, UI::Panel>(*get_parent());
	}


	void pressedYes()
	{
		g_fs->Unlink(m_filename);
		dosave(menu_save_game().igbase(), m_filename);
		menu_save_game().die();
	}

	void pressedNo()
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


struct DeletionMessageBox : public UI::WLMessageBox {
	DeletionMessageBox
		(Game_Main_Menu_Save_Game & parent, std::string const & filename)
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

	void pressedYes()
	{
		g_fs->Unlink(m_filename);
		ref_cast<Game_Main_Menu_Save_Game, UI::Panel>(*get_parent()).fill_list();
		die();
	}

	void pressedNo()
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
