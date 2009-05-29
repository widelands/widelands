/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_MULTIPLAYER_H
#define FULLSCREEN_MENU_MULTIPLAYER_H

#include "network/network_ggz.h"

#if HAVE_GGZ

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

/**
 * Fullscreen Menu for MultiPlayer.
 * Here you select what game you want to play.
 */
struct Fullscreen_Menu_MultiPlayer : public Fullscreen_Menu_Base {
	Fullscreen_Menu_MultiPlayer();

	enum {Back = dying_code, Metaserver, Lan};

private:
	uint32_t                                            m_butw;
	uint32_t                                            m_buth;
	uint32_t                                            m_butx;
	uint32_t                                            m_fs;
	std::string                                         m_fn;
	UI::Textarea                                        title;
	UI::Callback_IDButton<Fullscreen_Menu_MultiPlayer, int32_t> metaserver;
	UI::Callback_IDButton<Fullscreen_Menu_MultiPlayer, int32_t> lan;
	UI::Callback_IDButton<Fullscreen_Menu_MultiPlayer, int32_t> back;
};

#endif // if HAVE_GGZ

#endif
