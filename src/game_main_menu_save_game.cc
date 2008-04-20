/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "game_main_menu_save_game.h"

#include "filesystem.h"
#include "constants.h"
#include "game.h"
#include "game_loader.h"
#include "game_preload_data_packet.h"
#include "game_saver.h"
#include "i18n.h"
#include "interactive_player.h"
#include "layered_filesystem.h"
#include "profile.h"

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_messagebox.h"
#include "ui_textarea.h"

Interactive_Player & Game_Main_Menu_Save_Game::iaplayer() {
	return dynamic_cast<Interactive_Player &>(*get_parent());
}

/*
===============
Game_Main_Menu_Save_Game::Game_Main_Menu_Save_Game

Create all the buttons etc...
===============
*/
Game_Main_Menu_Save_Game::Game_Main_Menu_Save_Game(Interactive_Player* parent, UI::UniqueWindow::Registry* registry)
:
#define spacing 5
UI::UniqueWindow(parent, registry, 400, 270, _("Save Game")),
m_ls
	(this,
	 spacing, 30,
	 get_inner_w() / 2 - spacing, get_inner_h() - spacing - 90)
{
	int32_t const offsy   = 30;
	int32_t       posx    =  5;
	int32_t       posy    = offsy;

	m_ls.selected.set(this, &Game_Main_Menu_Save_Game::selected);
	m_ls.double_clicked.set(this, &Game_Main_Menu_Save_Game::double_clicked);
	m_editbox =
		new UI::EditBox
		(this,
		 posx, posy + get_inner_h() - spacing - offsy - 60 + 3,
		 get_inner_w() / 2 - spacing, 20,
		 1, 0);
	m_editbox->changed.set(this, &Game_Main_Menu_Save_Game::edit_box_changed);

	posx = get_inner_w() / 2 + spacing;
	posy += 20;
	new UI::Textarea
		(this, posx, posy, 150, 20, _("Map Name: "), Align_CenterLeft);
	m_name =
		new UI::Textarea(this, posx+90, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	new UI::Textarea
		(this, posx, posy, 150, 20, _("Game Time: "), Align_CenterLeft);
	m_gametime =
		new UI::Textarea(this, posx+90, posy, 200, 20, "---", Align_CenterLeft);
	posy += 20 + spacing;

	posx = 5;
	posy = get_inner_h() - 30;
	m_ok_btn = new UI::Button<Game_Main_Menu_Save_Game>
		(this,
		 get_inner_w() / 2 - spacing - 80, posy, 80, 20,
		 4,
		 &Game_Main_Menu_Save_Game::clicked_ok, this,
		 _("OK"),
		 std::string(),
		 false);
	new UI::Button<Game_Main_Menu_Save_Game>
		(this,
		 get_inner_w() / 2 + spacing, posy, 80, 20,
		 4,
		 &Game_Main_Menu_Save_Game::die, this,
		 _("Cancel"));

	m_curdir = SaveHandler::get_base_dir();

	fill_list();

	center_to_parent();
	move_to_top();
}

/*
===============
Unregister from the registry pointer
===============
*/
Game_Main_Menu_Save_Game::~Game_Main_Menu_Save_Game() {}


/**
 * called when a item is selected
 */
void Game_Main_Menu_Save_Game::selected(uint32_t) {
	std::string const & name = m_ls.get_selected();

	std::auto_ptr<FileSystem> const fs(g_fs->MakeSubFileSystem(name));
	Widelands::Game_Loader gl(*fs, iaplayer().get_game());
	Widelands::Game_Preload_Data_Packet gpdp;
	gl.preload_game(&gpdp); // This has worked before, no problem

	{
		char const * const fname = FileSystem::FS_Filename(name.c_str());
		char fname_without_extension[strlen(fname) + 1];
		strcpy(fname_without_extension, fname);
		FileSystem::FS_StripExtension(fname_without_extension);
		m_editbox->setText(fname_without_extension);
	}
	m_ok_btn->set_enabled(true);

	m_name->set_text(gpdp.get_mapname());

	char buf[200];
	uint32_t gametime = gpdp.get_gametime();

	int32_t hours = gametime / 3600000;
	gametime -= hours * 3600000;
	int32_t minutes = gametime / 60000;

	sprintf(buf, "%02i:%02i", hours, minutes);
	m_gametime->set_text(buf);
}

/**
 * An Item has been doubleclicked
 */
void Game_Main_Menu_Save_Game::double_clicked(uint32_t) {clicked_ok();}

/*
 * fill the file list
 */
void Game_Main_Menu_Save_Game::fill_list() {
	filenameset_t m_gamefiles;

	//  Fill it with all files we find.
	g_fs->FindFiles(m_curdir, "*", &m_gamefiles, 1);

	Widelands::Game_Preload_Data_Packet gpdp;

	for
		(filenameset_t::iterator pname = m_gamefiles.begin();
		 pname != m_gamefiles.end();
		 ++pname)
	{
		char const * const name = pname->c_str();

		try {
			std::auto_ptr<FileSystem> const fs(g_fs->MakeSubFileSystem(name));
			Widelands::Game_Loader gl(*fs, iaplayer().get_game());
			gl.preload_game(&gpdp);
			char const * const fname = FileSystem::FS_Filename(name);
			char fname_without_extension[strlen(fname) + 1];
			strcpy(fname_without_extension, fname);
			FileSystem::FS_StripExtension(fname_without_extension);
			m_ls.add(fname_without_extension, name);
		} catch (_wexception&) {} //  we simply skip illegal entries
	}

	if (m_ls.size())
		m_ls.select(0);
}

/*
 * The editbox was changed. Enable ok button
 */
void Game_Main_Menu_Save_Game::edit_box_changed() {
	m_ok_btn->set_enabled(true);
}


static void dosave
	(Interactive_Player & iaplayer, std::string const & complete_filename)
{
	Widelands::Game & game = iaplayer.game();

	std::string error;
	if (!game.get_save_handler()->save_game(game, complete_filename, &error))
	{
		std::string s =
			_
			("Game Saving Error!\nSaved Game-File may be corrupt!\n\n"
			 "Reason given:\n");
		s += error;
		UI::MessageBox mbox
			(&iaplayer, _("Save Game Error!!"), s, UI::MessageBox::OK);
		mbox.run();
	}
}

struct SaveWarnMessageBox : public UI::MessageBox {
	SaveWarnMessageBox
		(Game_Main_Menu_Save_Game & parent, std::string const & filename)
		:
		UI::MessageBox
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
		return dynamic_cast<Game_Main_Menu_Save_Game &>(*get_parent());
	}


	void pressedYes()
	{
		g_fs->Unlink(m_filename);
		dosave(menu_save_game().iaplayer(), m_filename);
		menu_save_game().die();
	}

	void pressedNo()
	{
		die();
	}

private:
	std::string const m_filename;
};

/*
===========
called when the ok button has been clicked
===========
*/
void Game_Main_Menu_Save_Game::clicked_ok() {
	std::string const complete_filename =
		iaplayer().game().get_save_handler()->create_file_name
			(m_curdir, m_editbox->text());

	//  Check if file exists. If it does, show a warning.
	if (g_fs->FileExists(complete_filename)) {
		new SaveWarnMessageBox(*this, complete_filename);
	} else {
		dosave(iaplayer(), complete_filename);
		die();
	}
}
