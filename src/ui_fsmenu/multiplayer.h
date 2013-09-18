/*
 * Copyright (C) 2002, 2006, 2008, 2012 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_MULTIPLAYER_H
#define FULLSCREEN_MENU_MULTIPLAYER_H

#include "ui_fsmenu/base.h"
#include "network/internet_gaming.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

/**
 * Fullscreen Menu for MultiPlayer.
 * Here you select what game you want to play.
 */
struct Fullscreen_Menu_MultiPlayer : public Fullscreen_Menu_Base {
	Fullscreen_Menu_MultiPlayer();

	enum {Back = dying_code, Metaserver, Lan};

	void showInternetLogin();
	void internetLogin();
	std::string get_nickname() {return m_nickname;}
	std::string get_password() {return m_password;}
	bool registered()          {return m_register;}

private:
	uint32_t                                            m_butw;
	uint32_t                                            m_buth;
	uint32_t                                            m_butx;
	uint32_t                                            m_fs;
	std::string                                         m_fn;
	UI::Textarea                                        title;
	UI::Button                             metaserver;
	UI::Button                           * showloginbox;
	UI::Button                             lan;
	UI::Button                             back;

	// Values from internet login window
	std::string m_nickname;
	std::string m_password;
	bool        m_register;
	bool        m_auto_log;
};

#endif
