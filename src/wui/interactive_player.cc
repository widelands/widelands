/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "logic/building.h"
#include "building_statistics_menu.h"
#include "chat.h"
#include "logic/cmd_queue.h"
#include "constructionsite.h"
#include "debugconsole.h"
#include "economy/flag.h"
#include "encyclopedia_window.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "game_chat_menu.h"
#include "game_io/game_loader.h"
#include "game_main_menu.h"
#include "game_main_menu_save_game.h"
#include "game_objectives_menu.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "helper.h"
#include "i18n.h"
#include "immovable.h"
#include "graphic/overlay_manager.h"
#include "logic/player.h"
#include "productionsite.h"
#include "profile/profile.h"
#include "soldier.h"
#include "stock_menu.h"
#include "tribe.h"
#include "upcast.h"
#include "ware_statistics_menu.h"
#include "wlapplication.h"

#include "ui_basic/editbox.h"
#include "ui_basic/unique_window.h"


using Widelands::Building;
using Widelands::Map;
using boost::format;


#define CHAT_DISPLAY_TIME 5000 // Show chat messages as overlay for 5 seconds

ChatDisplay::ChatDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
	: UI::Panel(parent, x, y, w, h), m_chat(0)
{}
ChatDisplay::~ChatDisplay()
{
	// Finally delete all left message pictures
	for (uint32_t i = 0; i < m_cache_id.size(); ++i)
		if (m_cache_id[i])
			g_fh->delete_widget_cache(m_cache_id[i]);
}

void ChatDisplay::setChatProvider(ChatProvider & chat)
{
	m_chat = &chat;
}

struct Displayed {
	std::string text;
	int32_t h;
};

void ChatDisplay::draw(RenderTarget & dst)
{
	if (!m_chat)
		return;

	// delete pictures of all old messages that we won't use again
	// this is important to save space
	for (uint32_t i = 0; i < m_cache_id.size(); ++i)
		if (m_cache_id[i])
			g_fh->delete_widget_cache(m_cache_id[i]);
	m_cache_id.resize(0);

	int32_t now = WLApplication::get()->get_time();

	std::vector<ChatMessage> const & msgs = m_chat->getMessages();
	std::vector<Displayed> displaylist;
	uint32_t totalheight = 0;
	uint32_t idx = msgs.size();

	while (idx && now - msgs[idx - 1].time <= CHAT_DISPLAY_TIME) {
		int32_t w;

		Displayed d = {msgs[idx - 1].toPrintable(), 0};

		g_fh->get_size(UI_FONT_SMALL, d.text, &w, &d.h, get_w());
		if (d.h + static_cast<int32_t>(totalheight) > get_h())
			break;

		displaylist.push_back(d);
		--idx;
	}

	uint32_t y = 0;

	container_iterate_const(std::vector<Displayed>, displaylist, i) {
		uint32_t picid;
		g_fh->draw_richtext
			(dst,
			 RGBColor(55, 55, 55),
			 Point(0, get_inner_h() -55 -y),
			 "<rt>" + i.current->text + "</rt>",
			 get_w(),
			 m_cache_mode, &picid);
		y += i.current->h;
		m_cache_id.push_back(picid);
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
	(Widelands::Game        &       _game,
	 Section                &       global_s,
	 Widelands::Player_Number const plyn,
	 bool                     const scenario,
	 bool                     const multiplayer)
	:
	Interactive_GameBase (_game, global_s),
	m_auto_roadbuild_mode(global_s.get_bool("auto_roadbuild_mode", true)),
m_flag_to_connect(Widelands::Coords::Null()),

#define INIT_BTN(picture, callback, tooltip)                                  \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Interactive_Player::callback, *this,                                        \
 tooltip                                                                      \

m_toggle_chat
	(INIT_BTN("menu_chat",             toggle_chat,         _("Chat"))),
m_toggle_options_menu
	(INIT_BTN("menu_options_menu",     toggle_options_menu,    _("Options"))),
m_toggle_statistics_menu
	(INIT_BTN("menu_toggle_menu",      toggle_statistics_menu, _("Statistics"))),
m_toggle_objectives
	(INIT_BTN("menu_objectives",       toggle_objectives,      _("Objectives"))),
m_toggle_minimap
	(INIT_BTN("menu_toggle_minimap",   toggle_minimap,         _("Minimap"))),
m_toggle_buildhelp
	(INIT_BTN("menu_toggle_buildhelp", toggle_buildhelp,       _("Buildhelp"))),
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
	m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_options_menu,    UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,       UI::Box::AlignLeft);
	//m_toolbar.add(&m_toggle_resources,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_help,            UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_objectives,      UI::Box::AlignLeft);

	set_player_number(plyn);
	fieldclicked.set(this, &Interactive_Player::node_action);


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

	addCommand
		("switchplayer",
		 boost::bind(&Interactive_Player::cmdSwitchPlayer, this, _1));
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
	m_toggle_chat.set_visible(m_chatenabled);
	m_toggle_chat.set_enabled(m_chatenabled);
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
void Interactive_Player::toggle_chat() {
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
void Interactive_Player::toggle_statistics_menu() {
	if (m_statisticsmenu.window)
		delete m_statisticsmenu.window;
	else
		new GameMainMenu(*this, m_statisticsmenu, m_mainm_windows);
}
void Interactive_Player::toggle_objectives() {
	if (m_objectives.window)
		delete m_objectives.window;
	else
		new GameObjectivesMenu(*this, m_objectives);
}
void Interactive_Player::toggle_buildhelp() {
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


/// Player has clicked on the given node; bring up the context menu.
void Interactive_Player::node_action()
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
				set_display_flag
					(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_f:
			g_gr->toggle_fullscreen();
			return true;

		case SDLK_HOME:
			move_view_to(game().map().get_starting_pos(m_player_number));
			return true;

		case SDLK_RETURN:
			if (!m_chatProvider | !m_chatenabled)
				break;

			if (!m_chat.window)
				new GameChatMenu(this, m_chat, *m_chatProvider);

			dynamic_cast<GameChatMenu &>(*m_chat.window).enter_chat_message();
			return true;

		case SDLK_F6:
			if (get_display_flag(dfDebug)) {
				new GameChatMenu
					(this, m_debugconsole, *DebugConsole::getChatProvider());
				dynamic_cast<GameChatMenu &>(*m_debugconsole.window)
					.enter_chat_message(false);
			}
			return true;
#ifdef DEBUG //  only in debug builds
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
void Interactive_Player::set_player_number(uint32_t const n) {
	m_player_number = n;
}


/**
 * Cleanup any game-related data before loading a new game
 * while a game is currently playing.
 */
void Interactive_Player::cleanup_for_load() {}

void Interactive_Player::cmdSwitchPlayer(std::vector<std::string> const & args)
{
	if (args.size() != 2) {
		DebugConsole::write("Usage: switchplayer <nr>");
		return;
	}

	int n = atoi(args[1].c_str());
	if (n < 1 || n > MAX_PLAYERS || !game().get_player(n)) {
		DebugConsole::write(str(format("Player #%1% does not exist.") % n));
		return;
	}

	DebugConsole::write
		(str
		 	(format("Switching from #%1% to #%2%.")
		 	 % static_cast<int>(m_player_number) % n));
	m_player_number = n;
}
