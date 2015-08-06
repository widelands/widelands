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

#include <memory>
#include <string>
#include <vector>

#include <SDL_keycode.h>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/scoped_timer.h"
#include "base/warning.h"
#include "editor/tools/editor_delete_immovable_tool.h"
#include "editor/ui_menus/editor_main_menu.h"
#include "editor/ui_menus/editor_main_menu_load_map.h"
#include "editor/ui_menus/editor_main_menu_save_map.h"
#include "editor/ui_menus/editor_player_menu.h"
#include "editor/ui_menus/editor_tool_menu.h"
#include "editor/ui_menus/editor_toolsize_menu.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/world/resource_description.h"
#include "logic/world/world.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_base.h"
#include "wui/overlay_manager.h"

namespace {

using Widelands::Building;

// Load all tribes from disk.
void load_all_tribes(Widelands::EditorGameBase* egbase, UI::ProgressWindow* loader_ui) {
	for (const std::string& tribename : Widelands::TribeDescr::get_all_tribenames()) {
		ScopedTimer timer((boost::format("Loading %s took %%ums.") % tribename).str());
		loader_ui->stepf(_("Loading tribe: %s"), tribename.c_str());
		egbase->manually_load_tribe(tribename);
	}
}

}  // namespace

EditorInteractive::EditorInteractive(Widelands::EditorGameBase & e) :
	InteractiveBase(e, g_options.pull_section("global")),
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
	m_toggle_main_menu.sigclicked.connect(boost::bind(&EditorInteractive::toggle_mainmenu, this));
	m_toggle_tool_menu.sigclicked.connect(boost::bind(&EditorInteractive::tool_menu_btn, this));
	m_toggle_toolsize_menu.sigclicked.connect(boost::bind(&EditorInteractive::toolsize_menu_btn, this));
	m_toggle_minimap.sigclicked.connect(boost::bind(&EditorInteractive::toggle_minimap, this));
	m_toggle_buildhelp.sigclicked.connect(boost::bind(&EditorInteractive::toggle_buildhelp, this));
	m_toggle_player_menu.sigclicked.connect(boost::bind(&EditorInteractive::toggle_playermenu, this));
	m_undo.sigclicked.connect(
		boost::bind(&EditorHistory::undo_action, &m_history, boost::cref(egbase().world())));
	m_redo.sigclicked.connect(
		boost::bind(&EditorHistory::redo_action, &m_history, boost::cref(egbase().world())));

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
	set_display_flag(InteractiveBase::dfDebug, true);
#else
	set_display_flag(InteractiveBase::dfDebug, false);
#endif

	fieldclicked.connect(boost::bind(&EditorInteractive::map_clicked, this, false));
}

void EditorInteractive::register_overlays() {
	Widelands::Map & map = egbase().map();

	//  Starting locations
	Widelands::PlayerNumber const nr_players = map.get_nrplayers();
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
	OverlayManager& overlay_manager = map.overlay_manager();
	Widelands::Extent const extent = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		if (uint8_t const amount = fc.field->get_resources_amount()) {
			const std::string& immname =
			   egbase().world().get_resource(fc.field->get_resources())->get_editor_pic(amount);
			if (immname.size())
				overlay_manager.register_overlay(fc, g_gr->images().get(immname), 4);
		}
	}
}


void EditorInteractive::load(const std::string & filename) {
	assert(filename.size());

	Widelands::Map & map = egbase().map();

	// TODO(unknown): get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	egbase().cleanup_for_load();
	m_history.reset();

	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(filename));
	if (!ml.get())
		throw WLWarning
			(_("Unsupported format"),
			 _("Widelands could not load the file \"%s\". The file format seems to be incompatible."),
			 filename.c_str());
	ml->preload_map(true);

	UI::ProgressWindow loader_ui("pics/editor.jpg");
	std::vector<std::string> tipstext;
	tipstext.push_back("editor");

	m_history.reset();

	GameTips editortips(loader_ui, tipstext);

	load_all_tribes(&egbase(), &loader_ui);

	// Create the players. TODO(SirVer): this must be managed better
	loader_ui.step(_("Creating players"));
	iterate_player_numbers(p, map.get_nrplayers()) {
		egbase().add_player(p, 0, map.get_scenario_player_tribe(p), map.get_scenario_player_name(p));
	}

	ml->load_map_complete(egbase(), true);
	loader_ui.step(_("Loading graphics..."));
	egbase().load_graphics(loader_ui);

	register_overlays();

	set_need_save(false);
}


/// Called just before the editor starts, after postload, init and gfxload.
void EditorInteractive::start() {
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
void EditorInteractive::think() {
	InteractiveBase::think();

	int32_t lasttime = m_realtime;
	int32_t frametime;

	m_realtime = WLApplication::get()->get_time();
	frametime = m_realtime - lasttime;

	egbase().get_gametime_pointer() += frametime;
}



void EditorInteractive::exit() {
	if (m_need_save) {
		if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
		} else {
			UI::WLMessageBox mmb
			(this,
			 _("Unsaved Map"),
			 _("The map has not been saved, do you really want to quit?"),
			 UI::WLMessageBox::MBoxType::kOkCancel);
			if (mmb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack)
				return;
		}
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void EditorInteractive::toggle_mainmenu() {
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new EditorMainMenu(*this, m_mainmenu);
}

void EditorInteractive::map_clicked(bool should_draw) {
	m_history.do_action
		(tools.current(),
		 tools.use_tool, egbase().map(), egbase().world(),
	     get_sel_pos(), *this, should_draw);
	set_need_save(true);
}

bool EditorInteractive::handle_mouserelease(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		m_left_mouse_button_is_down = false;
	}
	return InteractiveBase::handle_mouserelease(btn, x, y);
}

bool EditorInteractive::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		m_left_mouse_button_is_down = true;
	}
	return InteractiveBase::handle_mousepress(btn, x, y);
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void EditorInteractive::set_sel_pos(Widelands::NodeAndTriangle<> const sel) {
	bool const target_changed =
	    tools.current().operates_on_triangles() ?
	    sel.triangle != get_sel_pos().triangle : sel.node != get_sel_pos().node;
	InteractiveBase::set_sel_pos(sel);
	if (target_changed && m_left_mouse_button_is_down)
		map_clicked(true);
}

void EditorInteractive::toggle_buildhelp() {
	egbase().map().overlay_manager().toggle_buildhelp();
}


void EditorInteractive::tool_menu_btn() {
	if (m_toolmenu.window)
		delete m_toolmenu.window;
	else
		new EditorToolMenu(*this, m_toolmenu);
}


void EditorInteractive::toggle_playermenu() {
	if (m_playermenu.window)
		delete m_playermenu.window;
	else {
		select_tool(tools.set_starting_pos, EditorTool::First);
		new EditorPlayerMenu(*this, m_playermenu);
	}

}

void EditorInteractive::toolsize_menu_btn() {
	if (m_toolsizemenu.window)
		delete m_toolsizemenu.window;
	else
		new EditorToolsizeMenu(*this, m_toolsizemenu);
}

void EditorInteractive::set_sel_radius_and_update_menu(uint32_t const val) {
	if (tools.current().has_size_one()) {
		set_sel_radius(0);
		return;
	}
	if (UI::UniqueWindow * const w = m_toolsizemenu.window) {
		dynamic_cast<EditorToolsizeMenu&>(*w).update(val);
	} else {
		set_sel_radius(val);
	}
}


bool EditorInteractive::handle_key(bool const down, SDL_Keysym const code) {
	bool handled = InteractiveBase::handle_key(down, code);

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
			if (tools.use_tool == EditorTool::First)
				select_tool(tools.current(), EditorTool::Second);
			handled = true;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools.use_tool == EditorTool::First)
				select_tool(tools.current(), EditorTool::Third);
			handled = true;
			break;

		case SDLK_SPACE:
			toggle_buildhelp();
			handled = true;
			break;

		case SDLK_c:
			set_display_flag
			(InteractiveBase::dfShowCensus,
			 !get_display_flag(InteractiveBase::dfShowCensus));
			handled = true;
			break;

		case SDLK_h:
			toggle_mainmenu();
			handled = true;
			break;

		case SDLK_i:
			select_tool(tools.info, EditorTool::First);
			handled = true;
			break;

		case SDLK_m:
			toggle_minimap();
			handled = true;
			break;

		case SDLK_l:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new MainMenuLoadMap(*this);
			handled = true;
			break;

		case SDLK_p:
			toggle_playermenu();
			handled = true;
			break;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new MainMenuSaveMap(*this);
			handled = true;
			break;

		case SDLK_t:
			tool_menu_btn();
			handled = true;
			break;

		case SDLK_z:
			if ((code.mod & (KMOD_LCTRL | KMOD_RCTRL)) && (code.mod & (KMOD_LSHIFT | KMOD_RSHIFT)))
				m_history.redo_action(egbase().world());
			else if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_history.undo_action(egbase().world());
			handled = true;
			break;

		case SDLK_y:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_history.redo_action(egbase().world());
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
			if (tools.use_tool != EditorTool::First)
				select_tool(tools.current(), EditorTool::First);
			handled = true;
			break;
		default:
			break;
		}
	}
	return handled;
}


void EditorInteractive::select_tool
(EditorTool & primary, EditorTool::ToolIndex const which) {
	if (which == EditorTool::First && & primary != tools.current_pointer) {
		if (primary.has_size_one()) {
			set_sel_radius(0);
			if (UI::UniqueWindow * const w = m_toolsizemenu.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.set_buttons_enabled(false);
			}
		} else {
			if (UI::UniqueWindow * const w = m_toolsizemenu.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.update(toolsize_menu.value());
			}
		}
		Widelands::Map & map = egbase().map();
		//  A new tool has been selected. Remove all registered overlay callback
		//  functions.
		map.overlay_manager().remove_overlay_callback_function();
		map.recalc_whole_map(egbase().world());
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
void EditorInteractive::reference_player_tribe
(Widelands::PlayerNumber player, void const * const data) {
	assert(0 < player);
	assert(player <= egbase().map().get_nrplayers());

	PlayerReferences r;
	r.player = player;
	r.object = data;

	m_player_tribe_references.push_back(r);
}

/// Unreference !once!, if referenced many times, this will leak a reference.
void EditorInteractive::unreference_player_tribe
(Widelands::PlayerNumber const player, void const * const data) {
	assert(player <= egbase().map().get_nrplayers());
	assert(data);

	std::vector<PlayerReferences> & references = m_player_tribe_references;
	std::vector<PlayerReferences>::iterator it = references.begin();
	std::vector<PlayerReferences>::const_iterator references_end =
	    references.end();
	if (player) {
		for (; it < references_end; ++it)
			if (it->player == player && it->object == data) {
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

bool EditorInteractive::is_player_tribe_referenced
(Widelands::PlayerNumber const  player) {
	assert(0 < player);
	assert(player <= egbase().map().get_nrplayers());

	for (uint32_t i = 0; i < m_player_tribe_references.size(); ++i)
		if (m_player_tribe_references[i].player == player)
			return true;

	return false;
}

void EditorInteractive::run_editor(const std::string& filename, const std::string& script_to_run) {
	Widelands::EditorGameBase editor(nullptr);
	EditorInteractive eia(editor);
	editor.set_ibase(&eia); // TODO(unknown): get rid of this
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
				map.create_empty_map(
				   editor.world(),
				   64,
				   64,
				   _("No Name"),
				   g_options.pull_section("global").get_string("realname", _("Unknown")));

				load_all_tribes(&editor, &loader_ui);

				loader_ui.step(_("Loading graphics..."));
				editor.load_graphics(loader_ui);
				loader_ui.step(std::string());
			} else {
				loader_ui.stepf("Loading map \"%s\"...", filename.c_str());
				eia.load(filename);
			}
		}

		eia.select_tool(eia.tools.increase_height, EditorTool::First);
		editor.postload();
		eia.start();

		if (!script_to_run.empty()) {
			eia.egbase().lua().run_script(script_to_run);
		}
	}
	eia.run<UI::Panel::Returncodes>();

	editor.cleanup_objects();
}
