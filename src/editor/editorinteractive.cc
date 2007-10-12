/*
 * Copyright (C) 2002-2003, 2006-2007 by the Widelands Development Team
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

#include "editorinteractive.h"
#include "editor_delete_immovable_tool.h"
#include "editor_event_menu.h"
#include "editor_objectives_menu.h"
#include "editor_variables_menu.h"
#include "editor_main_menu.h"
#include "editor_main_menu_load_map.h"
#include "editor_main_menu_save_map.h"
#include "editor_player_menu.h"
#include "editor_tool_menu.h"
#include "editor_toolsize_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "overlay_manager.h"
#include "player.h"
#include <SDL_keysym.h>
#include <stdint.h>
#include "tribe.h"
#include "ui_button.h"
#include "ui_modal_messagebox.h"
#include "ui_progresswindow.h"
#include "wlapplication.h"


Editor_Interactive::Editor_Interactive(Editor_Game_Base & e) :
Interactive_Base(e), m_egbase(e)
{

   // Disable debug. it is no use for editor
#ifndef DEBUG
   set_display_flag(Interactive_Base::dfDebug, false);
#else
   set_display_flag(Interactive_Base::dfDebug, true);
#endif

	m_realtime = WLApplication::get()->get_time();

	fieldclicked.set(this, &Editor_Interactive::map_clicked);

   // user interface buttons
   int32_t x = (get_w() - (7*34)) >> 1;
   int32_t y = get_h() - 34;

	new UI::Button<Editor_Interactive>
		(this,
		 x, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png"),
		 &Editor_Interactive::toggle_mainmenu, this,
		 _("Menu"));

	new UI::Button<Editor_Interactive>
		(this,
		 x + 34, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/editor_menu_toggle_tool_menu.png"),
		 &Editor_Interactive::tool_menu_btn, this,
		 _("Tool"));

	new UI::Button<Editor_Interactive>
		(this,
		 x + 68, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/editor_menu_set_toolsize_menu.png"),
		 &Editor_Interactive::toolsize_menu_btn, this,
		 _("Toolsize"));

	new UI::Button<Editor_Interactive>
		(this,
		 x + 102, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png"),
		 &Editor_Interactive::toggle_minimap, this,
		 _("Minimap"));

   new UI::Button<Editor_Interactive>
		(this,
		 x + 136, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png"),
		 &Editor_Interactive::toggle_buildhelp, this,
		 _("Buildhelp"));

	new UI::Button<Editor_Interactive>
		(this,
		 x + 170, y, 34, 43,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/editor_menu_player_menu.png"),
		 &Editor_Interactive::toggle_playermenu, this,
		 _("Players"));

   new UI::Button<Editor_Interactive>
		(this,
		 x + 204, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_event_menu.png"),
		 &Editor_Interactive::toggle_eventmenu, this,
		 _("Events"));

   new UI::Button<Editor_Interactive>
		(this,
		 x + 238, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_variables_menu.png"),
		 &Editor_Interactive::toggle_variablesmenu, this,
		 _("Variables"));

	new UI::Button<Editor_Interactive>
		(this,
		 x + 272, y, 34, 34,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_toggle_objectives_menu.png"),
		 &Editor_Interactive::toggle_objectivesmenu, this,
		 _("Objectives"));

   // Load all tribes into memory
   std::vector<std::string> tribes;
	Tribe_Descr::get_all_tribenames(tribes);
	for (uint32_t i = 0; i < tribes.size(); ++i)
		e.manually_load_tribe(tribes[i].c_str());

   m_need_save=false;
   m_ctrl_down=false;

	select_tool(tools.increase_height, Editor_Tool::First);
}

/// Restore default sel.
Editor_Interactive::~Editor_Interactive() {unset_sel_picture();}


/// Called just before the editor starts, after postload, init and gfxload.
void Editor_Interactive::start()
{egbase().map().overlay_manager().show_buildhelp(true);}


/**
 * Called every frame.
 *
 * Advance the timecounter and animate textures.
 */
void Editor_Interactive::think()
{
	Interactive_Base::think();

	int32_t lasttime = m_realtime;
	int32_t frametime;

	m_realtime = WLApplication::get()->get_time();
	frametime = m_realtime - lasttime;

	*egbase().get_game_time_pointer() += frametime;

	g_gr->animate_maptextures(egbase().get_gametime());
}



void Editor_Interactive::exit() {
	if (m_need_save) {
		UI::Modal_Message_Box mmb
			(this,
			 _("Map unsaved"),
			 _("The Map is unsaved, do you really want to quit?"),
			 UI::Modal_Message_Box::YESNO);
		if (mmb.run() == 0) return;
	}
   end_modal(0);
}

void Editor_Interactive::toggle_mainmenu() {
	if (m_mainmenu.window) delete m_mainmenu.window;
	else new Editor_Main_Menu(this, &m_mainmenu);
}


void Editor_Interactive::toggle_objectivesmenu() {
	if (m_objectivesmenu.window) delete m_objectivesmenu.window;
	else new Editor_Objectives_Menu(this, &m_objectivesmenu);
}


void Editor_Interactive::toggle_variablesmenu() {
	if (m_variablesmenu.window) delete m_variablesmenu.window;
	else new Editor_Variables_Menu(this, &m_variablesmenu);
}


void Editor_Interactive::toggle_eventmenu() {
	if (m_eventmenu.window) delete m_eventmenu.window;
	else new Editor_Event_Menu(this, &m_eventmenu);
}

void Editor_Interactive::map_clicked() {
	tools.current()
		.handle_click(tools.use_tool, egbase().map(), get_sel_pos(), *this);
	need_complete_redraw();
	set_need_save(true);
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void Editor_Interactive::set_sel_pos(const Node_and_Triangle<> sel) {
	const bool target_changed = tools.current().operates_on_triangles() ?
		sel.triangle != get_sel_pos().triangle : sel.node != get_sel_pos().node;
	Interactive_Base::set_sel_pos(sel);
	if (target_changed and SDL_GetMouseState(0, 0) & SDL_BUTTON(SDL_BUTTON_LEFT))
		map_clicked();
}


void Editor_Interactive::toggle_buildhelp()
{egbase().map().overlay_manager().toggle_buildhelp();}


void Editor_Interactive::tool_menu_btn() {
	if (m_toolmenu.window) delete m_toolmenu.window;
	else new Editor_Tool_Menu(*this, m_toolmenu);
}


void Editor_Interactive::toggle_playermenu() {
	if (m_playermenu.window) delete m_playermenu.window;
	else {
		select_tool(tools.set_starting_pos, Editor_Tool::First);
		new Editor_Player_Menu(*this, &m_playermenu);
	}

}


void Editor_Interactive::toolsize_menu_btn() {
	if (m_toolsizemenu.window) delete m_toolsizemenu.window;
	else new Editor_Toolsize_Menu(this, &m_toolsizemenu);
}


bool Editor_Interactive::handle_key(bool down, SDLKey code, char)
{
	bool handled=false;

	if (code==SDLK_LCTRL || code==SDLK_RCTRL) m_ctrl_down = down;

	if (down)
	{
		// only on down events
		switch (code)
		{
		// Sel radius
		case SDLK_1:
			set_sel_radius (0);
			handled=true;
			break;
		case SDLK_2:
			set_sel_radius (1);
			handled=true;
			break;
		case SDLK_3:
			set_sel_radius (2);
			handled=true;
			break;
		case SDLK_4:
			set_sel_radius (3);
			handled=true;
			break;
		case SDLK_5:
			set_sel_radius (4);
			handled=true;
			break;
		case SDLK_6:
			set_sel_radius (5);
			handled=true;
			break;
		case SDLK_7:
			set_sel_radius (6);
			handled=true;
			break;
		case SDLK_8:
			set_sel_radius (7);
			handled=true;
			break;
		case SDLK_9:
			set_sel_radius (8);
			handled=true;
			break;
		case SDLK_0:
			set_sel_radius (9);
			handled=true;
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (tools.use_tool == Editor_Tool::First)
				select_tool (tools.current(), Editor_Tool::Second);
			handled=true;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools.use_tool == Editor_Tool::First)
				select_tool (tools.current(), Editor_Tool::Third);
			handled=true;
			break;

		case SDLK_SPACE:
			toggle_buildhelp();
			handled=true;
			break;

		case SDLK_c:
			set_display_flag
				(Interactive_Base::dfShowCensus,
				 !get_display_flag (Interactive_Base::dfShowCensus));
			handled=true;
			break;

		case SDLK_e:
			toggle_eventmenu();
			handled=true;
			break;

		case SDLK_f:
			if (down)
				g_gr->toggle_fullscreen();
			handled=true;
			break;

		case SDLK_h:
			toggle_mainmenu();
			handled=true;
			break;

		case SDLK_i:
			select_tool (tools.info, Editor_Tool::First);
			handled=true;
			break;

		case SDLK_m:
			toggle_minimap();
			handled=true;
			break;

		case SDLK_l:
			if (m_ctrl_down)
				new Main_Menu_Load_Map (this);
			handled=true;
			break;

		case SDLK_p:
			toggle_playermenu();
			handled=true;
			break;

		case SDLK_s:
			if (m_ctrl_down)
				new Main_Menu_Save_Map (this);
			handled=true;
			break;

		case SDLK_t:
			tool_menu_btn();
			handled=true;
			break;

		default:
			break;

		}
	}
	else
	{
		// key up events
		switch (code)
		{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools.use_tool != Editor_Tool::First)
				select_tool (tools.current(), Editor_Tool::First);
			handled=true;
			break;
		default:
			break;
		}
	}

	return handled;
}


void Editor_Interactive::select_tool
(Editor_Tool & primary, const Editor_Tool::Tool_Index which)
{
	if (which == Editor_Tool::First and &primary != tools.current_pointer) {
		Map & map = egbase().map();
      // A new tool has been selected. Remove all
      // registered overlay callback functions
		map.overlay_manager().register_overlay_callback_function(0, 0);
		map.recalc_whole_map();

	}
	tools.current_pointer = &primary;
   tools.use_tool=which;

	if (const char * sel_pic = primary.get_sel(which)) set_sel_picture(sel_pic);
	else                                             unset_sel_picture();
	set_sel_triangles(primary.operates_on_triangles());
}

/*
 * Reference functions
 *
 *  data is either a pointer to a trigger, event
 *  or a tribe (for buildings)
 */
void Editor_Interactive::reference_player_tribe
(const Player_Number player, const void * const data)
{
	assert(0 < player);
	assert    (player <= egbase().map().get_nrplayers());

   Player_References r;
   r.player=player;
   r.object=data;

   m_player_tribe_references.push_back(r);
}

/*
 * unreference !once!, if referenced many times, this
 * will leace a reference
 */
void Editor_Interactive::unreference_player_tribe
(const Player_Number player, const void * const data)
{
	assert(player <= egbase().map().get_nrplayers());
   assert(data);

	std::vector<Player_References> & references = m_player_tribe_references;
	std::vector<Player_References>::iterator it = references.begin();
	std::vector<Player_References>::const_iterator references_end =
		references.end();
	if (player > 0) {
		for (; it < references_end; ++it)
			if (it->player == player and it->object == data) break;
		references.erase(it);
	} else //  Player is invalid. Remove all references from this object.
		for (; it < references_end; ++it)
			if (it->object == data) {references.erase(it); --it, --references_end;}
}

bool Editor_Interactive::is_player_tribe_referenced(int32_t player) {
	assert(0 < player);
	assert    (player <= egbase().map().get_nrplayers());

	for (uint32_t i = 0; i < m_player_tribe_references.size(); ++i)
         if (m_player_tribe_references[i].player==player) return true;

   return false;
}


/**
 * Public static method to create an instance of the editor
 * and run it. This takes care of all the setup and teardown.
 */
void Editor_Interactive::run_editor()
{
	Editor_Game_Base editor;
	UI::ProgressWindow loader_ui;

	Map* m = new Map;
	m->create_empty_map();
	editor.set_map(m);

	g_gr->flush(PicMod_Menu);

	Editor_Interactive eia(editor);
	editor.set_iabase(&eia); //TODO: get rid of this

	editor.postload();
	editor.load_graphics(loader_ui);

	eia.start();
	eia.run();

	editor.cleanup_objects();

	g_gr->flush(PicMod_Game);
	g_anim.flush();
}

