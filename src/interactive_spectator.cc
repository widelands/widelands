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

#include "interactive_spectator.h"

#include "game.h"
#include "graphic.h"
#include "i18n.h"

#include "ui_editbox.h"
#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

struct Interactive_Spectator::Internals {
	UI::Textarea* label_speed;
};


/**
 * Setup the replay UI for the given game.
 */
Interactive_Spectator::Interactive_Spectator(Game* g)
:
Interactive_Base(*g),
m(new Internals)
{
	// Setup all screen elements
	fieldclicked.set(this, &Interactive_Spectator::field_action);

	// user interface buttons
	int32_t x = (get_w() - (4*34)) >> 1;
	int32_t y = get_h() - 34;

	new UI::Button<Interactive_Spectator>
		(this,
		 x + 34, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.png"),
		 &Interactive_Spectator::exit_btn, this,
		 _("Menu"));

	new UI::Button<Interactive_Spectator>
		(this,
		 x + 68, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png"),
		 &Interactive_Spectator::toggle_minimap, this,
		 _("Minimap"));

	// Speed info
	m->label_speed = new UI::Textarea(this, get_w(), 0, 0, 0, "", Align_TopRight);
}


/**
 * Cleanup all structures.
 */
Interactive_Spectator::~Interactive_Spectator()
{
	delete m;
	m = 0;
}


/**
 * \return a pointer to the running \ref Game instance.
 */
Game* Interactive_Spectator::get_game()
{
	return static_cast<Game*>(&egbase());
}


/**
 * \return "our" player.
 *
 * \note We might want to implement a feature to watch a specific player,
 * including their vision. Then this should be changed.
 */
Player* Interactive_Spectator::get_player() const throw ()
{
	return 0;
}


/**
 * Update the speed display, check for chatmessages.
 */
void Interactive_Spectator::think()
{
	Interactive_Base::think();

	//  draw speed display
	{
		std::string text;

		if (uint32_t speed = get_game()->get_speed()) {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), _("%ux").c_str(), speed);
			text = buffer;
		} else {
			text = _("PAUSE");
		}

		m->label_speed->set_text(text);
	}
}


/**
 * Called by the game when the end of the game has been reached.
 */
void Interactive_Spectator::end_of_game()
{
	get_game()->set_speed(0);
}


/**
 * Called just before the game starts, after postload, init and gfxload
 */
void Interactive_Spectator::start()
{
	Map & map = get_game()->map();
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
void Interactive_Spectator::field_action()
{
}


/**
 * Global in-game keypresses:
 */
bool Interactive_Spectator::handle_key(bool down, SDL_keysym code)
{
	bool handled=false;

	switch (code.sym) {
	case SDLK_m:
		if (down)
			toggle_minimap();
		handled=true;
		break;

	case SDLK_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		handled=true;
		break;

	case SDLK_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		handled=true;
		break;

	case SDLK_f:
		if (down)
			g_gr->toggle_fullscreen();
		handled=true;
		break;

	case SDLK_PAGEUP:
		if (down) {
			int32_t speed = get_game()->get_speed();

			get_game()->set_speed(speed + 1);
		}
		handled=true;
		break;

	case SDLK_PAGEDOWN:
		if (down) {
			int32_t speed = get_game()->get_speed();

			get_game()->set_speed(std::max(0, speed-1));
		}
		handled=true;
		break;

	default:
		handled=false;
		break;
	}

	return handled;
}

