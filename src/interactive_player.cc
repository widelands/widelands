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

#include "interactive_player.h"

#include "building.h"
#include "building_statistics_menu.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "encyclopedia_window.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "game_loader.h"
#include "game_main_menu.h"
#include "game_chat_menu.h"
#include "game_main_menu_save_game.h"
#include "game_objectives_menu.h"
#include "game_options_menu.h"
#include "graphic.h"
#include "general_statistics_menu.h"
#include "helper.h"
#include "i18n.h"
#include "immovable.h"
#include "chat.h"
#include "player.h"
#include "profile.h"
#include "productionsite.h"
#include "overlay_manager.h"
#include "soldier.h"
#include "stock_menu.h"
#include "transport.h"
#include "tribe.h"
#include "ware_statistics_menu.h"
#include "wlapplication.h"

#include "ui_editbox.h"
#include "ui_unique_window.h"

#include "upcast.h"

using Widelands::Building;
using Widelands::Map;


#define CHAT_DISPLAY_TIME 5000 // Show chat messages as overlay for 5 seconds

ChatDisplay::ChatDisplay(UI::Panel* parent, int32_t x, int32_t y, int32_t w, int32_t h)
: UI::Panel(parent, x, y, w, h)
{
	m_chat = 0;
}

void ChatDisplay::setChatProvider(ChatProvider* chat)
{
	m_chat = chat;
}

struct Displayed {
	std::string text;
	int32_t h;
};

void ChatDisplay::draw(RenderTarget* dst)
{
	if (!m_chat)
		return;

	int32_t now = WLApplication::get()->get_time();

	const std::vector<ChatMessage>& msgs = m_chat->getMessages();
	std::vector<Displayed> displaylist;
	uint32_t totalheight = 0;
	uint32_t idx = msgs.size();

	while (idx && now - msgs[idx - 1].time <= CHAT_DISPLAY_TIME) {
		int32_t w;

		Displayed d = {msgs[idx-1].toPrintable(), 0};

		g_fh->get_size(UI_FONT_SMALL, d.text, &w, &d.h, get_w());
		if (d.h+static_cast<int32_t>(totalheight) > get_h())
			break;

		displaylist.push_back(d);
		--idx;
	}

	uint32_t y = 0;

	for
		(std::vector<Displayed>::const_iterator it = displaylist.begin();
		 it != displaylist.end();
		 ++it)
	{
		g_fh->draw_string
			(*dst,
			 UI_FONT_SMALL, UI_FONT_SMALL_CLR,
			 Point(0, get_inner_h() -55 -y),
			 it->text,
			 Align_Left);
		y += it->h;
	}
}


// This function is the callback for recalculation of field overlays
int32_t Int_Player_overlay_callback_function
	(Widelands::TCoords<Widelands::FCoords> const c, void * data, int32_t)
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
Interactive_Player::Interactive_Player
		(Widelands::Game & g, uint8_t const plyn, bool scenario, bool multiplayer)
:
Interactive_GameBase(g),
m_chatProvider   (0),
m_chatDisplay    (0),
m_flag_to_connect(Widelands::Coords::Null()),

#define INIT_BTN(picture, callback, tooltip)                                  \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Interactive_Player::callback, this,                                         \
 tooltip                                                                      \

m_toggle_chat
	(INIT_BTN("menu_chat",             toggle_chat,         _("Chat"))),
m_toggle_options_menu
	(INIT_BTN("menu_options_menu",     toggle_options_menu, _("Options"))),
m_toggle_main_menu
	(INIT_BTN("menu_toggle_menu",      toggle_main_menu,    _("Statistics"))),
m_toggle_objectives
	(INIT_BTN("menu_objectives",       toggle_objectives,   _("Objectives"))),
m_toggle_minimap
	(INIT_BTN("menu_toggle_minimap",   toggle_minimap,      _("Minimap"))),
m_toggle_buildhelp
	(INIT_BTN("menu_toggle_buildhelp", toggle_buildhelp,    _("Buildhelp"))),
#if 0
m_toggle_resources
	(INIT_BTN
	 	("editor_menu_tool_change_resources",
	 	 toggle_resources,
	 	 _("Resource information"))),
#endif
m_toggle_help
	(INIT_BTN("menu_help",             toggle_help,         _("Ware help")))
{
	m_auto_roadbuild_mode =
		g_options.pull_section("global")->get_bool("auto_roadbuild_mode", true);

	m_toolbar.add(&m_toggle_chat,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_options_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_main_menu,    UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,    UI::Box::AlignLeft);
	//m_toolbar.add(&m_toggle_resources,    UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_help,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_objectives,   UI::Box::AlignLeft);

	set_player_number(plyn);
	fieldclicked.set(this, &Interactive_Player::field_action);


	// TODO : instead of making unneeded buttons invisible after generation,
	// they should not at all be generated. -> implement more dynamic toolbar UI
	if (multiplayer) {
		m_chatDisplay =
			new ChatDisplay(this, 10, 25, get_w() - 10, get_h() - 25);
		m_toggle_chat.set_visible(false);
		m_toggle_chat.set_enabled(false);
	} else
		m_toggle_chat.set_visible(false);
	if (not scenario)
		m_toggle_objectives.set_visible(false);

	m_toolbar.resize();
	adjust_toolbar_position();

	set_display_flag(dfSpeed, true);
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

void Interactive_Player::think()
{
	Interactive_Base::think();

	if (m_flag_to_connect) {
		Widelands::Field & field = egbase().map()[m_flag_to_connect];
		if (upcast(Widelands::Flag const, flag, field.get_immovable())) {
			if (not flag->has_road() and not is_building_road())
				if (m_auto_roadbuild_mode) {
					//  There might be a fieldaction window open, showing a button
					//  for roadbuilding. If that dialog remains open so that the
					//  button is clicked, we would enter roadbuilding mode while
					//  we are already in roadbuilding mode from the call below.
					//  That is not allowed. Therefore we must delete the
					//  fieldaction window before entering roadbuilding mode here.
					delete m_fieldaction.window;
					m_fieldaction.window = 0;
					start_build_road(m_flag_to_connect, field.get_owned_by());
				}
			m_flag_to_connect = Widelands::Coords::Null();
		}
	}
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


//  Toolbar button callback functions.
void Interactive_Player::toggle_chat        () {
	if (m_chat.window)
		delete m_chat.window;
	else if (m_chatProvider)
		new GameChatMenu(this, m_chat, *m_chatProvider);
}
void Interactive_Player::toggle_options_menu() {
	if (m_options.window)
		delete m_options.window;
	else
		new GameOptionsMenu(*this, m_options, m_mainm_windows);
}
void Interactive_Player::toggle_main_menu   () {
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new GameMainMenu(*this, m_mainmenu, m_mainm_windows);
}
void Interactive_Player::toggle_objectives  () {
	if (m_objectives.window)
		delete m_objectives.window;
	else
		new GameObjectivesMenu(*this, m_objectives);
}
void Interactive_Player::toggle_buildhelp   () {
	egbase().map().overlay_manager().toggle_buildhelp();
}
void Interactive_Player::toggle_resources   () {
}
void Interactive_Player::toggle_help        () {
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
	Map const & map = egbase().map();
#ifndef DEBUG
	if (player().vision(Map::get_index(get_sel_pos().node, map.get_width())))
#endif
	{
		// Special case for buildings
		if (upcast(Building, building, map.get_immovable(get_sel_pos().node)))
			if (building->owner().get_player_number() == get_player_number()) {
				building->show_options(this);
				return;
			}

		// everything else can bring up the temporary dialog
		show_field_action(this, get_player(), &m_fieldaction);
	}
}


void Interactive_Player::set_chat_provider(ChatProvider* chat)
{
	m_chatProvider = chat;
	m_chatDisplay->setChatProvider(chat);

	m_toggle_chat.set_visible(chat);
	m_toggle_chat.set_enabled(chat);
}

ChatProvider* Interactive_Player::get_chat_provider()
{
	return m_chatProvider;
}


/**
 * Global in-game keypresses:
 * \li Space: toggles buildhelp
 * \li m: show minimap
 * \li o: show objectives window
 * \li c: toggle census
 * \li s: toggle building statistics
 * \li Home: go to starting position
 * \li PageUp/PageDown: change game speed
 * \li Return: write chat message
 * \li F5: reveal map (in debug builds)
*/
bool Interactive_Player::handle_key(bool down, SDL_keysym code)
{
	if (down)
		switch (code.sym) {
		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_m:
			toggle_minimap();
			return true;

		case SDLK_o:
			toggle_objectives();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new Game_Main_Menu_Save_Game(*this, m_mainm_windows.savegame);
			else
				set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_f:
			g_gr->toggle_fullscreen();
			return true;

		case SDLK_HOME:
			move_view_to(game().map().get_starting_pos(m_player_number));
			return true;

		case SDLK_RETURN:
			if (!m_chatProvider)
				break;

			if (!m_chat.window)
				new GameChatMenu(this, m_chat, *m_chatProvider);

			dynamic_cast<GameChatMenu &>(*m_chat.window).enter_chat_message();
			return true;

#ifdef DEBUG
	// Only in debug builds
		case SDLK_F5:
			player().set_see_all(not player().see_all());
			return true;
#endif

		default:
			break;
		}

	return Interactive_GameBase::handle_key(down, code);
}

/**
 * Set the player and the visibility to this
 * player
 */
void Interactive_Player::set_player_number(uint32_t n) {
	m_player_number=n;
}


/**
 * Cleanup any game-related data before loading a new game
 * while a game is currently playing.
 */
void Interactive_Player::cleanup_for_load() {}
