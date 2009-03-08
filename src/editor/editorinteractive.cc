/*
 * Copyright (C) 2002-2003, 2006-2009 by the Widelands Development Team
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
#include "game_tips.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "overlay_manager.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "warning.h"
#include "widelands_map_loader.h"
#include "wlapplication.h"

#include "ui_messagebox.h"
#include "ui_progresswindow.h"

#include <SDL_keysym.h>

using Widelands::Building;

Editor_Interactive::Editor_Interactive(Widelands::Editor_Game_Base & e) :
Interactive_Base(e),
m_need_save     (false),
m_realtime      (WLApplication::get()->get_time()),

#define INIT_BUTTON(picture, callback, tooltip)                               \
 TOOLBAR_BUTTON_COMMON_PARAMETERS,                                            \
 g_gr->get_picture(PicMod_Game, "pics/" picture ".png"),                      \
 &Editor_Interactive::callback, this,                                         \
 tooltip                                                                      \

m_toggle_main_menu
	(INIT_BUTTON
	 ("menu_toggle_menu",              toggle_mainmenu,       _("Menu"))),
m_toggle_tool_menu
	(INIT_BUTTON
	 ("editor_menu_toggle_tool_menu",  tool_menu_btn,         _("Tool"))),
m_toggle_toolsize_menu
	(INIT_BUTTON
	 ("editor_menu_set_toolsize_menu", toolsize_menu_btn,     _("Toolsize"))),
m_toggle_minimap
	(INIT_BUTTON
	 ("menu_toggle_minimap",           toggle_minimap,        _("Minimap"))),
m_toggle_buildhelp
	(INIT_BUTTON
	 ("menu_toggle_buildhelp",         toggle_buildhelp,      _("Buildhelp"))),
m_toggle_player_menu
	(INIT_BUTTON
	 ("editor_menu_player_menu",       toggle_playermenu,     _("Players"))),
m_toggle_event_menu
	(INIT_BUTTON
	 ("menu_toggle_event_menu",        toggle_eventmenu,      _("Events"))),
m_toggle_variables_menu
	(INIT_BUTTON
	 ("menu_toggle_variables_menu",    toggle_variablesmenu,  _("Variables"))),
m_toggle_objectives_menu
	(INIT_BUTTON
	 ("menu_toggle_objectives_menu",   toggle_objectivesmenu, _("Objectives")))
{
	m_toolbar.add(&m_toggle_main_menu,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_tool_menu,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_toolsize_menu,   UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_player_menu,     UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_event_menu,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_variables_menu,  UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_objectives_menu, UI::Box::AlignLeft);
	m_toolbar.resize();
	adjust_toolbar_position();

	//  Disable debug. It is no use for editor.
#ifndef DEBUG
	set_display_flag(Interactive_Base::dfDebug, false);
#else
	set_display_flag(Interactive_Base::dfDebug, true);
#endif

	fieldclicked.set(this, &Editor_Interactive::map_clicked);
}


void Editor_Interactive::load(std::string const & filename) {
	assert(filename.size());

	Widelands::Map & map = egbase().map();

	// TODO: get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	egbase().cleanup_for_load(true, false);

	Widelands::Map_Loader * const ml = map.get_correct_loader(filename.c_str());
	if (not ml)
		throw warning
			(_("Unsupported format"),
			 (_("Widelands could not load the file \"") + filename +
			  _("\". The file format seems to be incompatible."))
			 .c_str());

	UI::ProgressWindow loader_ui("pics/editor.jpg");
	GameTips editortips (loader_ui, "txts/editortips");
	{
		std::string const old_world_name = map.get_world_name();
		ml->preload_map(true);
		if (strcmp(map.get_world_name(), old_world_name.c_str()))
			change_world();
	}

	loader_ui.step (_("Loading world data"));
	ml->load_world();
	ml->load_map_complete(&egbase(), true);
	egbase().load_graphics(loader_ui);

	//  update all the visualizations
	// Player positions
	std::string text;
	Widelands::Player_Number const nr_players = map.get_nrplayers();
	assert(nr_players <= 99); //  2 decimal digits
	char fname[] ="pics/editor_player_00_starting_pos.png";
	iterate_player_numbers(p, nr_players) {
		if (fname[20] == '9') {fname[20] = '0'; ++fname[19];} else ++fname[20];
		if (Widelands::Coords const sp = map.get_starting_pos(p))
			//  Have overlay on starting position only when it has no building.
			if (not dynamic_cast<const Building *>(map[sp].get_immovable())) {
				uint32_t const picid = g_gr->get_picture(PicMod_Game, fname);
				uint32_t w, h;
				g_gr->get_picture_size(picid, w, h);
				map.overlay_manager().register_overlay
					(sp, picid, 8, Point(w / 2, STARTING_POS_HOTSPOT_Y));
			}
	}

	//  Resources. we do not calculate default resources, therefore we do not
	//  expect to meet them here.
	Widelands::World const &       world           = map.world();
	Overlay_Manager        &       overlay_manager = map.overlay_manager();
	Widelands::Extent        const extent          = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		if (const uint8_t amount = fc.field->get_resources_amount()) {
			const std::string & immname =
				world.get_resource(fc.field->get_resources())->get_editor_pic
				(amount);
			if (immname.size())
				overlay_manager.register_overlay
					(fc, g_gr->get_picture(PicMod_Game, immname.c_str()), 4);
		}
	}

	set_need_save(false);
	need_complete_redraw();

	delete ml;
}


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
		UI::MessageBox mmb
			(this,
			 _("Map unsaved"),
			 _("The Map is unsaved, do you really want to quit?"),
			 UI::MessageBox::YESNO);
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
	else new Editor_Variables_Menu(*this, &m_variablesmenu);
}


void Editor_Interactive::toggle_eventmenu() {
	if (m_eventmenu.window) delete m_eventmenu.window;
	else new Editor_Event_Menu(*this, &m_eventmenu);
}

void Editor_Interactive::map_clicked() {
	tools.current()
		.handle_click(tools.use_tool, egbase().map(), get_sel_pos(), *this);
	need_complete_redraw();
	set_need_save(true);
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void Editor_Interactive::set_sel_pos(Widelands::Node_and_Triangle<> const sel)
{
	const bool target_changed = tools.current().operates_on_triangles() ?
		sel.triangle != get_sel_pos().triangle : sel.node != get_sel_pos().node;
	Interactive_Base::set_sel_pos(sel);
	int32_t mask = SDL_BUTTON_LMASK;
#ifdef __APPLE__
	// workaround for SDLs middle button emulation
	mask |= SDL_BUTTON_MMASK;
#endif
	if (target_changed and SDL_GetMouseState(0, 0) & mask)
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


void Editor_Interactive::set_sel_radius_and_update_menu(uint32_t const val) {
	if (UI::UniqueWindow * const w = m_toolsizemenu.window)
		dynamic_cast<Editor_Toolsize_Menu &>(*w).update(val);
	else
		set_sel_radius(val);
}


bool Editor_Interactive::handle_key(bool down, SDL_keysym code)
{
	bool handled = Interactive_Base::handle_key(down, code);

	if (down)
		// only on down events
		switch (code.sym) {
		// Sel radius
		case SDLK_1:
			set_sel_radius_and_update_menu (0);
			handled=true;
			break;
		case SDLK_2:
			set_sel_radius_and_update_menu (1);
			handled=true;
			break;
		case SDLK_3:
			set_sel_radius_and_update_menu (2);
			handled=true;
			break;
		case SDLK_4:
			set_sel_radius_and_update_menu (3);
			handled=true;
			break;
		case SDLK_5:
			set_sel_radius_and_update_menu (4);
			handled=true;
			break;
		case SDLK_6:
			set_sel_radius_and_update_menu (5);
			handled=true;
			break;
		case SDLK_7:
			set_sel_radius_and_update_menu (6);
			handled=true;
			break;
		case SDLK_8:
			set_sel_radius_and_update_menu (7);
			handled=true;
			break;
		case SDLK_9:
			set_sel_radius_and_update_menu (8);
			handled=true;
			break;
		case SDLK_0:
			set_sel_radius_and_update_menu (9);
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
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new Main_Menu_Load_Map (this);
			handled=true;
			break;

		case SDLK_p:
			toggle_playermenu();
			handled=true;
			break;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
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
	else
		// key up events
		switch (code.sym) {
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

	return handled;
}


void Editor_Interactive::select_tool
	(Editor_Tool & primary, Editor_Tool::Tool_Index const which)
{
	if (which == Editor_Tool::First and &primary != tools.current_pointer) {
		Widelands::Map & map = egbase().map();
		//  A new tool has been selected. Remove all registered overlay callback
		//  functions.
		map.overlay_manager().register_overlay_callback_function(0, 0);
		map.recalc_whole_map();

	}
	tools.current_pointer = &primary;
	tools.use_tool        = which;

	if (const char * sel_pic = primary.get_sel(which)) set_sel_picture(sel_pic);
	else                                             unset_sel_picture();
	set_sel_triangles(primary.operates_on_triangles());
}

/**
 * Reference functions
 *
 *  data is either a pointer to a trigger, event
 *  or a tribe (for buildings)
 */
void Editor_Interactive::reference_player_tribe
	(Widelands::Player_Number const player, void const * const data)
{
	assert(0 < player);
	assert    (player <= egbase().map().get_nrplayers());

	Player_References r;
	r.player = player;
	r.object = data;

	m_player_tribe_references.push_back(r);
}

/**
 * Unreference !once!, if referenced many times, this
 * will leace a reference
 */
void Editor_Interactive::unreference_player_tribe
	(Widelands::Player_Number const player, void const * const data)
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
		if (m_player_tribe_references[i].player == player)
			return true;

	return false;
}


void Editor_Interactive::change_world() {
	delete m_terrainmenu  .window;
	delete m_immovablemenu.window;
	delete m_bobmenu      .window;
	delete m_resourcesmenu.window;
}


/**
 * Public static method to create an instance of the editor
 * and run it. This takes care of all the setup and teardown.
 */
void Editor_Interactive::run_editor(std::string const & filename)
{
	Widelands::Editor_Game_Base editor;
	Editor_Interactive eia(editor);
	editor.set_iabase(&eia); //TODO: get rid of this
	{
		UI::ProgressWindow loader_ui("pics/editor.jpg");
		GameTips editortips (loader_ui, "txts/editortips");
		g_gr->flush(PicMod_Menu);

		{
			Widelands::Map & map = *new Widelands::Map;
			editor.set_map(&map);
			if (filename.empty()) {
				loader_ui.step("Creating empty map...");
				map.create_empty_map
					(64, 64, "greenland", _("No Name"),
					 g_options.pull_section("global").get_string
					 	("realname", _("Unknown")));
				editor.load_graphics(loader_ui);
			} else {
				loader_ui.stepf("Loading map \"%s\"...", filename.c_str());
				eia.load(filename);
			}
		}

		{ //  Load all tribes into memory
			std::vector<std::string> tribenames;
			Widelands::Tribe_Descr::get_all_tribenames(tribenames);
			container_iterate_const(std::vector<std::string>, tribenames, i)
				editor.manually_load_tribe(*i.current);
		}

		eia.select_tool(eia.tools.increase_height, Editor_Tool::First);
		editor.postload();
		eia.start();
	}
	eia.run();

	editor.cleanup_objects();

	g_gr->flush(PicMod_Game);
	g_anim.flush();
}

