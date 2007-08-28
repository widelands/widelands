/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "interactive_player.h"

#include "building.h"
#include "building_statistics_menu.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "encyclopedia_window.h"
#include "game_loader.h"
#include "game_main_menu.h"
#include "graphic.h"
#include "general_statistics_menu.h"
#include "i18n.h"
#include "keycodes.h"
#include "immovable.h"
#include "network.h"
#include "player.h"
#include "productionsite.h"
#include "overlay_manager.h"
#include "soldier.h"
#include "stock_menu.h"
#include "transport.h"
#include "tribe.h"
#include "ware_statistics_menu.h"
#include "wlapplication.h"

#include "ui_editbox.h"
#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

#define CHAT_DISPLAY_TIME 5000 // Show chat messages as overlay for 5 seconds


// This function is the callback for recalculation of field overlays
int Int_Player_overlay_callback_function
(const TCoords<FCoords> c, void* data, int)
{
	return
		static_cast<const Interactive_Player *>(data)->get_player()->
		get_buildcaps(c);
}


/*
===============
Interactive_Player::Interactive_Player

Initialize
===============
*/
Interactive_Player::Interactive_Player(Game & g, const uchar plyn) :
Interactive_Base(g), m_game(&g)
{
	// Setup all screen elements
	set_player_number(plyn);

	fieldclicked.set(this, &Interactive_Player::field_action);

	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;

	new UI::Button<Interactive_Player>
		(this,
		 x + 34, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png"),
		 &Interactive_Player::main_menu_btn, this,
		 _("Menu"));

	new UI::Button<Interactive_Player>
		(this,
		 x + 68, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png"),
		 &Interactive_Player::toggle_minimap, this,
		 _("Minimap"));

	new UI::Button<Interactive_Player>
		(this,
		 x + 102, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png"),
		 &Interactive_Player::toggle_buildhelp, this,
		 _("Buildhelp"));

	new UI::Button<Interactive_Player>
		(this,
		 x + 136, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game,  "pics/menu_help.png"),
		 &Interactive_Player::open_encyclopedia, this,
		 _("Tribe's ware encyclopedia"));

	// Speed info
	m_label_speed = new UI::Textarea(this, get_w(), 0, 0, 0, "", Align_TopRight);

	// Chat Messages
	m_chat_messages = new UI::Multiline_Textarea(this, 10, 25, get_inner_w(), get_inner_h(), "", Align_TopLeft);
	m_type_message = new UI::Textarea(this, 10, get_inner_h()-50, get_inner_w(), 50, "", Align_TopLeft);

	m_is_typing_msg = false;
	m_do_chat_overlays = true;
}

/*
===============
Interactive_Player::~Interactive_Player

cleanups
===============
*/
Interactive_Player::~Interactive_Player()
{
}


/*
===============
Interactive_Player::think

Update the speed display, check for chatmessages.
===============
*/
void Interactive_Player::think()
{
	Interactive_Base::think();

	{//  draw speed display
		char buffer[32];
		if (uint speed = m_game->get_speed())
			snprintf(buffer, sizeof(buffer), _("%ux").c_str(), speed);
		else strncpy (buffer, _("PAUSE").c_str(), sizeof(buffer));
		m_label_speed->set_text(buffer);
	}

	// Check for chatmessages
	NetGame* ng = m_game->get_netgame();
	if (ng && ng->have_chat_message()) {
		NetGame::Chat_Message t = ng->get_chat_message();
		m_chatmsges.push_back(t);

		Overlay_Chat_Messages ov;
		ov.msg =  t;
		ov.starttime = WLApplication::get()->get_time();
		m_show_chatmsg.push_back(ov);
	}

	// If we have chat messages to overlay, show them now
	m_chat_messages->set_text("");
	if (m_show_chatmsg.size() && m_do_chat_overlays) {
		std::string str;
		for (uint i = 0; i < m_show_chatmsg.size(); i++) {
			const NetGame::Chat_Message& t = m_show_chatmsg[i].msg;
			str += get_game()->get_player(t.plrnum)->get_name();
			str += ": ";
			str += t.msg;
			str += "\n";

			if (WLApplication::get()->get_time() - m_show_chatmsg[i].starttime > CHAT_DISPLAY_TIME) {
				m_show_chatmsg.erase(m_show_chatmsg.begin() + i);
				i--;
			}
		}

		m_chat_messages->set_text(str.c_str());
	}

	// Is the user typing a message?
	m_type_message->set_text("");
	if (m_is_typing_msg) {
		std::string text = _("Message: ");
		text += m_typed_message;
		m_type_message->set_text(text.c_str());
	}
}


/*
===============
Interactive_Player::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Interactive_Player::start()
{
	postload();
}


/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void Interactive_Player::postload()
{
	Map & map = egbase().map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	overlay_manager.show_buildhelp(false);
	overlay_manager.register_overlay_callback_function
			(&Int_Player_overlay_callback_function, static_cast<void *>(this));

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map();

	// Close game-relevant UI windows (but keep main menu open)
	delete m_fieldaction.window;
	m_fieldaction.window = 0;

	hide_minimap();
}


/*
===============
Interactive_Player::main_menu_btn

Bring up or close the main menu
===============
*/
void Interactive_Player::main_menu_btn()
{
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new GameMainMenu(*this, m_mainmenu, m_mainm_windows);
}

//
// Toggles buildhelp rendering in the main MapView
//
void Interactive_Player::toggle_buildhelp()
{
	egbase().map().overlay_manager().toggle_buildhelp();
}

//
// Shows wares encyclopedia for wares
//
void Interactive_Player::open_encyclopedia()
{
	if (m_encyclopedia.window)
		delete m_encyclopedia.window;
	else
		new EncyclopediaWindow(*this, m_encyclopedia);
}

/*
===============
Interactive_Player::field_action

Player has clicked on the given field; bring up the context menu.
===============
*/
void Interactive_Player::field_action()
{
	const Map & map = egbase().map();

	if (player().vision(Map::get_index(get_sel_pos().node, map.get_width()))) {
		// Special case for buildings
		BaseImmovable * const imm = map.get_immovable(get_sel_pos().node);

		if (imm && imm->get_type() == Map_Object::BUILDING) {
			Building *building = (Building *)imm;

			if (building->get_owner()->get_player_number() == get_player_number()) {
				building->show_options(this);
				return;
			}
		}

		// everything else can bring up the temporary dialog
		show_field_action(this, get_player(), &m_fieldaction);
	}
}

/*
===============
Interactive_Player::handle_key

Global in-game keypresses:
Space: toggles buildhelp
F5: reveal map
===============
*/
bool Interactive_Player::handle_key(bool down, int code, char c)
{
	if (m_is_typing_msg && down) {
		if (c & 0x7f) {
			m_typed_message.append(1, c);
			return true;
		}
	}

	switch (code) {
	case KEY_SPACE:
		if (down)
			toggle_buildhelp();
		return true;

	case KEY_m:
		if (down)
			toggle_minimap();
		return true;

	case KEY_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		return true;

	case KEY_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;

	case KEY_f:
		if (down)
			g_gr->toggle_fullscreen();
		return true;

	case KEY_HOME:
		if (down) move_view_to(m_game->map().get_starting_pos(m_player_number));
		return true;

	case KEY_PAGEUP:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(speed + 1);
		}
		return true;

	case KEY_PAGEDOWN:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(std::max(0, speed-1));
		}
		return true;

	case KEY_BACKSPACE:
		if (down) {
			if (m_is_typing_msg && m_typed_message.size()) {
				m_typed_message.erase(m_typed_message.begin() + m_typed_message.size() - 1);
				return true;
			}
		}
		break;

	case KEY_ESCAPE:
		if (down) {
			if (m_is_typing_msg) {
				m_is_typing_msg = false;
				m_typed_message.clear();
				return true;
			}
		}
		break;

	case KEY_RETURN:
		if (down) {
			if (m_is_typing_msg && m_typed_message.size()) {
				if (m_game->get_netgame()) {
					NetGame::Chat_Message t;

					t.plrnum = get_player_number();
					t.msg = m_typed_message;
					m_game->get_netgame()->send_chat_message(t);
				}
				m_typed_message.clear();
				m_is_typing_msg = false;
			} else {
				// Begin writing a message
				m_is_typing_msg = true;
			}
		}
		return true;

#ifdef DEBUG
	// Only in debug builds
	case KEY_F5:
		if (down) player().set_see_all(not player().see_all());
		return true;
#endif
	}

	return false;
}

/*
 * set the player and the visibility to this
 * player
 */
void Interactive_Player::set_player_number(uint n) {
	m_player_number=n;
}


/**
 * Cleanup any game-related data before loading a new game
 * while a game is currently playing.
 */
void Interactive_Player::cleanup_for_load()
{
}
