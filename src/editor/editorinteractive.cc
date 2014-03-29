/*
 * Copyright (C) 2002-2003, 2006-2011, 2013 by the Widelands Development Team
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

#include "editor/editorinteractive.h"

#include <string>
#include <vector>

#include <SDL_keysym.h>
#include <boost/format.hpp>

#include "editor/tools/editor_delete_immovable_tool.h"
#include "editor/ui_menus/editor_main_menu.h"
#include "editor/ui_menus/editor_main_menu_load_map.h"
#include "editor/ui_menus/editor_main_menu_save_map.h"
#include "editor/ui_menus/editor_player_menu.h"
#include "editor/ui_menus/editor_tool_menu.h"
#include "editor/ui_menus/editor_toolsize_menu.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "scoped_timer.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "warning.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_base.h"
#include "wui/overlay_manager.h"

namespace {

using Widelands::Building;

// Load all tribes from disk.
void load_all_tribes(Widelands::Editor_Game_Base* egbase, UI::ProgressWindow* loader_ui) {
	for (const std::string& tribename : Widelands::Tribe_Descr::get_all_tribenames()) {
		ScopedTimer timer((boost::format("Loading %s took %%ums.") % tribename).str());
		loader_ui->stepf(_("Loading tribe: %s"), tribename.c_str());
		egbase->manually_load_tribe(tribename);
	}
}

}  // namespace

Editor_Interactive::Editor_Interactive(Widelands::Editor_Game_Base & e) :
	Interactive_Base(e, g_options.pull_section("global")),
	m_need_save(false),
	m_realtime(WLApplication::get()->get_time()),
	m_left_mouse_button_is_down(false),
	m_history(m_undo, m_redo),

#define INIT_BUTTON(picture, name, tooltip)                         \
	TOOLBAR_BUTTON_COMMON_PARAMETERS(name),                                      \
	g_gr->images().get("pics/" picture ".png"),                      \
	tooltip                                                                      \

	m_toggle_main_menu
	(INIT_BUTTON
	 ("menu_toggle_menu", "menu", _("Menu"))),
	m_toggle_tool_menu
	(INIT_BUTTON
	 ("editor_menu_toggle_tool_menu", "tools", _("Tools"))),
	m_toggle_toolsize_menu
	(INIT_BUTTON
	 ("editor_menu_set_toolsize_menu", "toolsize",
	  _("Tool Size"))),
	m_toggle_minimap
	(INIT_BUTTON
	 ("menu_toggle_minimap", "minimap", _("Minimap"))),
	m_toggle_buildhelp
	(INIT_BUTTON
	 ("menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"))),
	m_toggle_player_menu
	(INIT_BUTTON
	 ("editor_menu_player_menu", "players", _("Players"))),
	m_undo
	(INIT_BUTTON
	 ("editor_undo", "undo", _("Undo"))),
	m_redo
	(INIT_BUTTON
	 ("editor_redo", "redo", _("Redo")))
{
	m_toggle_main_menu.sigclicked.connect(boost::bind(&Editor_Interactive::toggle_mainmenu, this));
	m_toggle_tool_menu.sigclicked.connect(boost::bind(&Editor_Interactive::tool_menu_btn, this));
	m_toggle_toolsize_menu.sigclicked.connect(boost::bind(&Editor_Interactive::toolsize_menu_btn, this));
	m_toggle_minimap.sigclicked.connect(boost::bind(&Editor_Interactive::toggle_minimap, this));
	m_toggle_buildhelp.sigclicked.connect(boost::bind(&Editor_Interactive::toggle_buildhelp, this));
	m_toggle_player_menu.sigclicked.connect(boost::bind(&Editor_Interactive::toggle_playermenu, this));
	m_undo.sigclicked.connect(boost::bind(&Editor_History::undo_action, &m_history));
	m_redo.sigclicked.connect(boost::bind(&Editor_History::redo_action, &m_history));

	m_toolbar.set_layout_toplevel(true);
	m_toolbar.add(&m_toggle_main_menu,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_tool_menu,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_toolsize_menu,   UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,       UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_player_menu,     UI::Box::AlignLeft);
	m_toolbar.add(&m_undo,                   UI::Box::AlignLeft);
	m_toolbar.add(&m_redo,                   UI::Box::AlignLeft);
	adjust_toolbar_position();

	m_undo.set_enabled(false);
	m_redo.set_enabled(false);

#ifndef NDEBUG
	set_display_flag(Interactive_Base::dfDebug, true);
#else
	set_display_flag(Interactive_Base::dfDebug, false);
#endif

	fieldclicked.connect(boost::bind(&Editor_Interactive::map_clicked, this, false));
}

void Editor_Interactive::register_overlays() {
	Widelands::Map & map = egbase().map();

	//  Starting locations
	Widelands::Player_Number const nr_players = map.get_nrplayers();
	assert(nr_players <= 99); //  2 decimal digits
	char fname[] = "pics/editor_player_00_starting_pos.png";
	iterate_player_numbers(p, nr_players) {
		if (fname[20] == '9') {fname[20] = '0'; ++fname[19];} else ++fname[20];
		if (Widelands::Coords const sp = map.get_starting_pos(p)) {
			const Image* pic = g_gr->images().get(fname);
			assert(pic);
			map.overlay_manager().register_overlay
				(sp, pic, 8, Point(pic->width() / 2, STARTING_POS_HOTSPOT_Y));
		}
	}

	//  Resources: we do not calculate default resources, therefore we do not
	//  expect to meet them here.
	const Widelands::World    &    world           = map.world();
	Overlay_Manager        &       overlay_manager = map.overlay_manager();
	Widelands::Extent        const extent          = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		if (uint8_t const amount = fc.field->get_resources_amount()) {
			const std::string & immname =
			    world.get_resource(fc.field->get_resources())->get_editor_pic
			    (amount);
			if (immname.size())
				overlay_manager.register_overlay
				(fc, g_gr->images().get(immname), 4);
		}
	}

	need_complete_redraw();
}


void Editor_Interactive::load(const std::string & filename) {
	assert(filename.size());

	Widelands::Map & map = egbase().map();

	// TODO: get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	egbase().cleanup_for_load();
	m_history.reset();

	std::unique_ptr<Widelands::Map_Loader> const ml(map.get_correct_loader(filename));
	if (not ml.get())
		throw warning
			(_("Unsupported format"),
			 _("Widelands could not load the file \"%s\". The file format seems to be incompatible."),
			 filename.c_str());

	UI::ProgressWindow loader_ui("pics/editor.jpg");
	std::vector<std::string> tipstext;
	tipstext.push_back("editor");
	GameTips editortips(loader_ui, tipstext);

	{
		std::string const old_world_name = map.get_world_name();
		ml->preload_map(true);
		if (strcmp(map.get_world_name(), old_world_name.c_str()))
			change_world();
	}
	load_all_tribes(&egbase(), &loader_ui);

	// Create the players. TODO SirVer this must be managed better
	loader_ui.step(_("Creating players"));
	iterate_player_numbers(p, map.get_nrplayers()) {
		egbase().add_player
		(p, 0, map.get_scenario_player_tribe(p),
		 map.get_scenario_player_name(p));
	}

	loader_ui.step(_("Loading world data"));
	ml->load_world();
	ml->load_map_complete(egbase(), true);
	loader_ui.step(_("Loading graphics..."));
	egbase().load_graphics(loader_ui);

	register_overlays();

	set_need_save(false);
}


/// Called just before the editor starts, after postload, init and gfxload.
void Editor_Interactive::start() {
	// Run the editor initialization script, if any
	try {
		egbase().lua().run_script("map:scripting/editor_init.lua");
	} catch (LuaScriptNotExistingError &) {
		// do nothing.
	}
	egbase().map().overlay_manager().show_buildhelp(true);
}


/**
 * Called every frame.
 *
 * Advance the timecounter and animate textures.
 */
void Editor_Interactive::think() {
	Interactive_Base::think();

	int32_t lasttime = m_realtime;
	int32_t frametime;

	m_realtime = WLApplication::get()->get_time();
	frametime = m_realtime - lasttime;

	egbase().get_game_time_pointer() += frametime;

	g_gr->animate_maptextures(egbase().get_gametime());
}



void Editor_Interactive::exit() {
	if (m_need_save) {
		UI::WLMessageBox mmb
		(this,
		 _("Unsaved Map"),
		 _("The map has not been saved, do you really want to quit?"),
		 UI::WLMessageBox::YESNO);
		if (mmb.run() == 0)
			return;
	}
	end_modal(0);
}

void Editor_Interactive::toggle_mainmenu() {
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new Editor_Main_Menu(*this, m_mainmenu);
}

void Editor_Interactive::map_clicked(bool should_draw) {
	m_history.do_action
		(tools.current(),
		 tools.use_tool, egbase().map(),
	     get_sel_pos(), *this, should_draw);
	need_complete_redraw();
	set_need_save(true);
}

bool Editor_Interactive::handle_mouserelease(Uint8 btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		m_left_mouse_button_is_down = false;
	}
	return Interactive_Base::handle_mouserelease(btn, x, y);
}

bool Editor_Interactive::handle_mousepress(Uint8 btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		m_left_mouse_button_is_down = true;
	}
	return Interactive_Base::handle_mousepress(btn, x, y);
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void Editor_Interactive::set_sel_pos(Widelands::Node_and_Triangle<> const sel) {
	bool const target_changed =
	    tools.current().operates_on_triangles() ?
	    sel.triangle != get_sel_pos().triangle : sel.node != get_sel_pos().node;
	Interactive_Base::set_sel_pos(sel);
	if (target_changed and m_left_mouse_button_is_down)
		map_clicked(true);
}

void Editor_Interactive::toggle_buildhelp() {
	egbase().map().overlay_manager().toggle_buildhelp();
}


void Editor_Interactive::tool_menu_btn() {
	if (m_toolmenu.window)
		delete m_toolmenu.window;
	else
		new Editor_Tool_Menu(*this, m_toolmenu);
}


void Editor_Interactive::toggle_playermenu() {
	if (m_playermenu.window)
		delete m_playermenu.window;
	else {
		select_tool(tools.set_starting_pos, Editor_Tool::First);
		new Editor_Player_Menu(*this, m_playermenu);
	}

}

void Editor_Interactive::toolsize_menu_btn() {
	if (m_toolsizemenu.window)
		delete m_toolsizemenu.window;
	else
		new Editor_Toolsize_Menu(*this, m_toolsizemenu);
}

void Editor_Interactive::set_sel_radius_and_update_menu(uint32_t const val) {
	if (tools.current().has_size_one())
		return;
	if (UI::UniqueWindow * const w = m_toolsizemenu.window)
		ref_cast<Editor_Toolsize_Menu, UI::UniqueWindow>(*w).update(val);
	else
		set_sel_radius(val);
}


bool Editor_Interactive::handle_key(bool const down, SDL_keysym const code) {
	bool handled = Interactive_Base::handle_key(down, code);

	if (down) {
		// only on down events

		switch (code.sym) {
			// Sel radius
		case SDLK_1:
			set_sel_radius_and_update_menu(0);
			handled = true;
			break;
		case SDLK_2:
			set_sel_radius_and_update_menu(1);
			handled = true;
			break;
		case SDLK_3:
			set_sel_radius_and_update_menu(2);
			handled = true;
			break;
		case SDLK_4:
			set_sel_radius_and_update_menu(3);
			handled = true;
			break;
		case SDLK_5:
			set_sel_radius_and_update_menu(4);
			handled = true;
			break;
		case SDLK_6:
			set_sel_radius_and_update_menu(5);
			handled = true;
			break;
		case SDLK_7:
			set_sel_radius_and_update_menu(6);
			handled = true;
			break;
		case SDLK_8:
			set_sel_radius_and_update_menu(7);
			handled = true;
			break;
		case SDLK_9:
			set_sel_radius_and_update_menu(8);
			handled = true;
			break;
		case SDLK_0:
			set_sel_radius_and_update_menu(9);
			handled = true;
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (tools.use_tool == Editor_Tool::First)
				select_tool(tools.current(), Editor_Tool::Second);
			handled = true;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools.use_tool == Editor_Tool::First)
				select_tool(tools.current(), Editor_Tool::Third);
			handled = true;
			break;

		case SDLK_SPACE:
			toggle_buildhelp();
			handled = true;
			break;

		case SDLK_c:
			set_display_flag
			(Interactive_Base::dfShowCensus,
			 !get_display_flag(Interactive_Base::dfShowCensus));
			handled = true;
			break;

		case SDLK_f:
			g_gr->toggle_fullscreen();
			handled = true;
			break;

		case SDLK_h:
			toggle_mainmenu();
			handled = true;
			break;

		case SDLK_i:
			select_tool(tools.info, Editor_Tool::First);
			handled = true;
			break;

		case SDLK_m:
			toggle_minimap();
			handled = true;
			break;

		case SDLK_l:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new Main_Menu_Load_Map(*this);
			handled = true;
			break;

		case SDLK_p:
			toggle_playermenu();
			handled = true;
			break;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new Main_Menu_Save_Map(*this);
			handled = true;
			break;

		case SDLK_t:
			tool_menu_btn();
			handled = true;
			break;

		case SDLK_z:
			if ((code.mod & (KMOD_LCTRL | KMOD_RCTRL)) && (code.mod & (KMOD_LSHIFT | KMOD_RSHIFT)))
				m_history.redo_action();
			else if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_history.undo_action();
			handled = true;
			break;
		case SDLK_y:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_history.redo_action();
			handled = true;
			break;
		default:
			break;

		}
	} else {
		// key up events
		switch (code.sym) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools.use_tool != Editor_Tool::First)
				select_tool(tools.current(), Editor_Tool::First);
			handled = true;
			break;
		default:
			break;
		}
	}
	return handled;
}


void Editor_Interactive::select_tool
(Editor_Tool & primary, Editor_Tool::Tool_Index const which) {
	if (which == Editor_Tool::First and & primary != tools.current_pointer) {
		if (primary.has_size_one())
			set_sel_radius_and_update_menu(0);
		Widelands::Map & map = egbase().map();
		//  A new tool has been selected. Remove all registered overlay callback
		//  functions.
		map.overlay_manager().remove_overlay_callback_function();
		map.recalc_whole_map();

	}
	tools.current_pointer = &primary;
	tools.use_tool        = which;

	if (char const * const sel_pic = primary.get_sel(which))
		set_sel_picture(sel_pic);
	else
		unset_sel_picture();
	set_sel_triangles(primary.operates_on_triangles());
}

/**
 * Reference functions
 *
 *  data is a pointer to a tribe (for buildings)
 */
void Editor_Interactive::reference_player_tribe
(Widelands::Player_Number player, void const * const data) {
	assert(0 < player);
	assert(player <= egbase().map().get_nrplayers());

	Player_References r;
	r.player = player;
	r.object = data;

	m_player_tribe_references.push_back(r);
}

/// Unreference !once!, if referenced many times, this will leak a reference.
void Editor_Interactive::unreference_player_tribe
(Widelands::Player_Number const player, void const * const data) {
	assert(player <= egbase().map().get_nrplayers());
	assert(data);

	std::vector<Player_References> & references = m_player_tribe_references;
	std::vector<Player_References>::iterator it = references.begin();
	std::vector<Player_References>::const_iterator references_end =
	    references.end();
	if (player) {
		for (; it < references_end; ++it)
			if (it->player == player and it->object == data) {
				references.erase(it);
				break;
			}
	} else //  Player is invalid. Remove all references from this object.
		while (it < references_end)
			if (it->object == data) {
				it = references.erase(it);
				references_end = references.end();
			} else
				++it;
}

bool Editor_Interactive::is_player_tribe_referenced
(Widelands::Player_Number const  player) {
	assert(0 < player);
	assert(player <= egbase().map().get_nrplayers());

	for (uint32_t i = 0; i < m_player_tribe_references.size(); ++i)
		if (m_player_tribe_references[i].player == player)
			return true;

	return false;
}


void Editor_Interactive::change_world() {
	m_history.reset();
	delete m_terrainmenu  .window;
	delete m_immovablemenu.window;
	delete m_bobmenu      .window;
	delete m_resourcesmenu.window;
}


void Editor_Interactive::run_editor(const std::string & filename, const std::string& script_to_run) {
	Widelands::Editor_Game_Base editor(nullptr);
	Editor_Interactive eia(editor);
	editor.set_ibase(&eia); // TODO get rid of this
	{
		UI::ProgressWindow loader_ui("pics/editor.jpg");
		std::vector<std::string> tipstext;
		tipstext.push_back("editor");
		GameTips editortips(loader_ui, tipstext);

		{
			Widelands::Map & map = *new Widelands::Map;
			editor.set_map(&map);
			if (filename.empty()) {
				loader_ui.step("Creating empty map...");
				map.create_empty_map
				(64, 64, "greenland", _("No Name"),
				 g_options.pull_section("global").get_string
				 ("realname", _("Unknown")));

				load_all_tribes(&editor, &loader_ui);

				loader_ui.step(_("Loading graphics..."));
				editor.load_graphics(loader_ui);
				loader_ui.step(std::string());
			} else {
				loader_ui.stepf("Loading map \"%s\"...", filename.c_str());
				eia.load(filename);
			}
		}

		eia.select_tool(eia.tools.increase_height, Editor_Tool::First);
		editor.postload();
		eia.start();

		if (!script_to_run.empty()) {
			eia.egbase().lua().run_script(script_to_run);
		}
	}
	eia.run();

	editor.cleanup_objects();
}
