/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "interactive_spectator.h"

#include "game.h"
#include "graphic.h"
#include "i18n.h"

#include "ui_editbox.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

/**
 * Setup the replay UI for the given game.
 */
Interactive_Spectator::Interactive_Spectator(Widelands::Game * const g)
:
Interactive_Base(*g),

#define INIT_BTN(picture, callback, tooltip)                                  \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Interactive_Spectator::callback, this,                                      \
 tooltip                                                                      \

m_exit          (INIT_BTN("menu_exit_game",      exit_btn, _("Exit Replay"))),
m_toggle_minimap(INIT_BTN("menu_toggle_minimap", toggle_minimap, _("Minimap")))
{
	m_toolbar.add(&m_exit,           UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap, UI::Box::AlignLeft);
	m_toolbar.resize();
	adjust_toolbar_position();

	// Setup all screen elements
	fieldclicked.set(this, &Interactive_Spectator::field_action);

	set_display_flag(dfSpeed, true);
}


/**
 * Cleanup all structures.
 */
Interactive_Spectator::~Interactive_Spectator()
{
}


/**
 * \return a pointer to the running \ref Game instance.
 */
Widelands::Game * Interactive_Spectator::get_game()
{
	return static_cast<Widelands::Game *>(&egbase());
}


/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Widelands::Player * Interactive_Spectator::get_player() const throw ()
{
	return 0;
}


/**
 * Called just before the game starts, after postload, init and gfxload
 */
void Interactive_Spectator::start()
{
	Widelands::Map & map = get_game()->map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	overlay_manager.show_buildhelp(false);

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map();
}


/**
 * Bring up or close the main menu.
 * Right now, this is kind of misleading since it just stops playback.
 */
void Interactive_Spectator::exit_btn()
{
	end_modal(0);
}


/**
 * Observer has clicked on the given field; bring up the context menu.
 * \todo Implement
 */
void Interactive_Spectator::field_action() {}


/**
 * Global in-game keypresses:
 */
bool Interactive_Spectator::handle_key(bool down, SDL_keysym code)
{
	switch (code.sym) {
	case SDLK_m:
		if (down)
			toggle_minimap();
		return true;

	case SDLK_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		return true;

	case SDLK_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;

	case SDLK_f:
		if (down)
			g_gr->toggle_fullscreen();
		return true;

	default:
		break;
	}

	return Interactive_Base::handle_key(down, code);
}

