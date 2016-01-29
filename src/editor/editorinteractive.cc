/*
 * Copyright (C) 2002-2003, 2006-2011, 2013, 2015 by the Widelands Development Team
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
#include "editor/ui_menus/editor_help.h"
#include "editor/ui_menus/editor_main_menu.h"
#include "editor/ui_menus/editor_main_menu_load_map.h"
#include "editor/ui_menus/editor_main_menu_save_map.h"
#include "editor/ui_menus/editor_player_menu.h"
#include "editor/ui_menus/editor_tool_menu.h"
#include "editor/ui_menus/editor_toolsize_menu.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "wlapplication.h"
#include "wui/field_overlay_manager.h"
#include "wui/game_tips.h"
#include "wui/interactive_base.h"

namespace {

using Widelands::Building;

// Load all tribes from disk.
void load_all_tribes(Widelands::EditorGameBase* egbase, UI::ProgressWindow* loader_ui) {
	loader_ui->step(_("Loading tribes"));
	egbase->tribes();
}

// Updates the resources overlays after a field has changed.
void update_resource_overlay(const Widelands::NoteFieldResourceChanged& note,
                             const Widelands::World& world,
                             FieldOverlayManager* field_overlay_manager) {
	//  Ok, we're doing something. First remove the current overlays.
	if (note.old_resource != Widelands::kNoResource) {
		const std::string str =
		   world.get_resource(note.old_resource)->editor_image(note.old_amount);
		const Image* pic = g_gr->images().get(str);
		field_overlay_manager->remove_overlay(note.fc, pic);
	}

	const auto amount = note.fc.field->get_resources_amount();
	const auto resource_type = note.fc.field->get_resources();
	if (amount > 0 && resource_type != Widelands::kNoResource) {
		const std::string str =
		   world.get_resource(note.fc.field->get_resources())->editor_image(amount);
		const Image* pic = g_gr->images().get(str);
		field_overlay_manager->register_overlay(note.fc, pic, 0);
	}
}

}  // namespace

EditorInteractive::EditorInteractive(Widelands::EditorGameBase & e) :
	InteractiveBase(e, g_options.pull_section("global")),
	need_save_(false),
	realtime_(SDL_GetTicks()),
	left_mouse_button_is_down_(false),
	tools_(new Tools()),
	history_(new EditorHistory(undo_, redo_)),

#define INIT_BUTTON(picture, name, tooltip)                         \
	TOOLBAR_BUTTON_COMMON_PARAMETERS(name),                                      \
	g_gr->images().get("pics/" picture ".png"),                      \
	tooltip                                                                      \

	toggle_main_menu_
	(INIT_BUTTON
	 ("menu_toggle_menu", "menu", _("Menu"))),
	toggle_tool_menu_
	(INIT_BUTTON
	 ("editor_menu_toggle_tool_menu", "tools", _("Tools"))),
	toggle_toolsize_menu_
	(INIT_BUTTON
	 ("editor_menu_set_toolsize_menu", "toolsize",
	  _("Tool Size"))),
	toggle_minimap_
	(INIT_BUTTON
	 ("menu_toggle_minimap", "minimap", _("Minimap"))),
	toggle_buildhelp_
	(INIT_BUTTON
	 ("menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"))),
	toggle_player_menu_
	(INIT_BUTTON
	 ("editor_menu_player_menu", "players", _("Players"))),
	undo_
	(INIT_BUTTON
	 ("editor_undo", "undo", _("Undo"))),
	redo_
	(INIT_BUTTON
	 ("editor_redo", "redo", _("Redo"))),
	toggle_help_
	(INIT_BUTTON
	 ("menu_help", "help", _("Help")))
{
	toggle_main_menu_.sigclicked.connect(boost::bind(&EditorInteractive::toggle_mainmenu, this));
	toggle_tool_menu_.sigclicked.connect(boost::bind(&EditorInteractive::tool_menu_btn, this));
	toggle_toolsize_menu_.sigclicked.connect(boost::bind(&EditorInteractive::toolsize_menu_btn, this));
	toggle_minimap_.sigclicked.connect(boost::bind(&EditorInteractive::toggle_minimap, this));
	toggle_buildhelp_.sigclicked.connect(boost::bind(&EditorInteractive::toggle_buildhelp, this));
	toggle_player_menu_.sigclicked.connect(boost::bind(&EditorInteractive::toggle_playermenu, this));
	undo_.sigclicked.connect([this] {history_->undo_action(egbase().world());});
	redo_.sigclicked.connect([this] {history_->redo_action(egbase().world());});
	toggle_help_.sigclicked.connect(boost::bind(&EditorInteractive::toggle_help, this));

	toolbar_.set_layout_toplevel(true);
	toolbar_.add(&toggle_main_menu_,       UI::Align::kLeft);
	toolbar_.add(&toggle_tool_menu_,       UI::Align::kLeft);
	toolbar_.add(&toggle_toolsize_menu_,   UI::Align::kLeft);
	toolbar_.add(&toggle_minimap_,         UI::Align::kLeft);
	toolbar_.add(&toggle_buildhelp_,       UI::Align::kLeft);
	toolbar_.add(&toggle_player_menu_,     UI::Align::kLeft);
	toolbar_.add(&undo_,                   UI::Align::kLeft);
	toolbar_.add(&redo_,                   UI::Align::kLeft);
	toolbar_.add(&toggle_help_,            UI::Align::kLeft);
	adjust_toolbar_position();

#ifndef NDEBUG
	set_display_flag(InteractiveBase::dfDebug, true);
#else
	set_display_flag(InteractiveBase::dfDebug, false);
#endif

	fieldclicked.connect(boost::bind(&EditorInteractive::map_clicked, this, false));

	// Subscribe to changes of the resource type on a field..
	field_resource_changed_subscriber_ =
	   Notifications::subscribe<Widelands::NoteFieldResourceChanged>(
	      [this](const Widelands::NoteFieldResourceChanged& note) {
		      update_resource_overlay(note, egbase().world(), mutable_field_overlay_manager());
		   });
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
			mutable_field_overlay_manager()->register_overlay
				(sp, pic, 8, Point(pic->width() / 2, STARTING_POS_HOTSPOT_Y));
		}
	}

	//  Resources: we do not calculate default resources, therefore we do not
	//  expect to meet them here.
	Widelands::Extent const extent = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		if (uint8_t const amount = fc.field->get_resources_amount()) {
			const std::string& immname =
			   egbase().world().get_resource(fc.field->get_resources())->editor_image(amount);
			if (immname.size())
				mutable_field_overlay_manager()->register_overlay(fc, g_gr->images().get(immname), 4);
		}
	}
}


void EditorInteractive::load(const std::string & filename) {
	assert(filename.size());

	Widelands::Map & map = egbase().map();

	// TODO(unknown): get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	egbase().cleanup_for_load();

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

	GameTips editortips(loader_ui, tipstext);

	load_all_tribes(&egbase(), &loader_ui);

	// Create the players. TODO(SirVer): this must be managed better
	loader_ui.step(_("Creating players"));
	iterate_player_numbers(p, map.get_nrplayers()) {
		egbase().add_player(p, 0, map.get_scenario_player_tribe(p), map.get_scenario_player_name(p));
	}

	ml->load_map_complete(egbase(), true);
	egbase().load_graphics(loader_ui);
	map_changed(MapWas::kReplaced);
}


/// Called just before the editor starts, after postload, init and gfxload.
void EditorInteractive::start() {
	// Run the editor initialization script, if any
	try {
		egbase().lua().run_script("map:scripting/editor_init.lua");
	} catch (LuaScriptNotExistingError &) {
		// do nothing.
	}
	map_changed(MapWas::kReplaced);
}


/**
 * Called every frame.
 *
 * Advance the timecounter and animate textures.
 */
void EditorInteractive::think() {
	InteractiveBase::think();

	uint32_t lasttime = realtime_;

	realtime_ = SDL_GetTicks();

	egbase().get_gametime_pointer() += realtime_ - lasttime;
}



void EditorInteractive::exit() {
	if (need_save_) {
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
	if (mainmenu_.window)
		delete mainmenu_.window;
	else
		new EditorMainMenu(*this, mainmenu_);
}

void EditorInteractive::map_clicked(bool should_draw) {
	history_->do_action(tools_->current(), tools_->use_tool, egbase().map(), egbase().world(),
	                     get_sel_pos(), *this, should_draw);
	set_need_save(true);
}

bool EditorInteractive::handle_mouserelease(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		left_mouse_button_is_down_ = false;
	}
	return InteractiveBase::handle_mouserelease(btn, x, y);
}

bool EditorInteractive::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		left_mouse_button_is_down_ = true;
	}
	return InteractiveBase::handle_mousepress(btn, x, y);
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void EditorInteractive::set_sel_pos(Widelands::NodeAndTriangle<> const sel) {
	bool const target_changed =
	    tools_->current().operates_on_triangles() ?
	    sel.triangle != get_sel_pos().triangle : sel.node != get_sel_pos().node;
	InteractiveBase::set_sel_pos(sel);
	if (target_changed && left_mouse_button_is_down_)
		map_clicked(true);
}

void EditorInteractive::tool_menu_btn() {
	if (toolmenu_.window)
		delete toolmenu_.window;
	else
		new EditorToolMenu(*this, toolmenu_);
}


void EditorInteractive::toggle_playermenu() {
	if (playermenu_.window)
		delete playermenu_.window;
	else {
		select_tool(tools_->set_starting_pos, EditorTool::First);
		new EditorPlayerMenu(*this, playermenu_);
	}

}

void EditorInteractive::toolsize_menu_btn() {
	if (toolsizemenu_.window)
		delete toolsizemenu_.window;
	else
		new EditorToolsizeMenu(*this, toolsizemenu_);
}

void EditorInteractive::set_sel_radius_and_update_menu(uint32_t const val) {
	if (tools_->current().has_size_one()) {
		set_sel_radius(0);
		return;
	}
	if (UI::UniqueWindow * const w = toolsizemenu_.window) {
		dynamic_cast<EditorToolsizeMenu&>(*w).update(val);
	} else {
		set_sel_radius(val);
	}
}

void EditorInteractive::toggle_help() {
	if (helpmenu_.window)
		delete helpmenu_.window;
	else
		new EditorHelp(*this, helpmenu_);
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
			if (tools_->use_tool == EditorTool::First)
				select_tool(tools_->current(), EditorTool::Second);
			handled = true;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools_->use_tool == EditorTool::First)
				select_tool(tools_->current(), EditorTool::Third);
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
			select_tool(tools_->info, EditorTool::First);
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
				history_->redo_action(egbase().world());
			else if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				history_->undo_action(egbase().world());
			handled = true;
			break;

		case SDLK_y:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				history_->redo_action(egbase().world());
			handled = true;
			break;

		case SDLK_F1:
			toggle_help();
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
			if (tools_->use_tool != EditorTool::First)
				select_tool(tools_->current(), EditorTool::First);
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
	if (which == EditorTool::First && & primary != tools_->current_pointer) {
		if (primary.has_size_one()) {
			set_sel_radius(0);
			if (UI::UniqueWindow * const w = toolsizemenu_.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.set_buttons_enabled(false);
			}
		} else {
			if (UI::UniqueWindow * const w = toolsizemenu_.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.update(toolsize_menu.value());
			}
		}
		Widelands::Map & map = egbase().map();
		//  A new tool has been selected. Remove all registered overlay callback
		//  functions.
		mutable_field_overlay_manager()->register_overlay_callback_function(nullptr);
		map.recalc_whole_map(egbase().world());
	}
	tools_->current_pointer = &primary;
	tools_->use_tool        = which;

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

	player_tribe_references_.push_back(r);
}

/// Unreference !once!, if referenced many times, this will leak a reference.
void EditorInteractive::unreference_player_tribe
(Widelands::PlayerNumber const player, void const * const data) {
	assert(player <= egbase().map().get_nrplayers());
	assert(data);

	std::vector<PlayerReferences> & references = player_tribe_references_;
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

	for (uint32_t i = 0; i < player_tribe_references_.size(); ++i)
		if (player_tribe_references_[i].player == player)
			return true;

	return false;
}

void EditorInteractive::run_editor(const std::string& filename, const std::string& script_to_run) {
	Widelands::EditorGameBase egbase(nullptr);
	EditorInteractive eia(egbase);
	egbase.set_ibase(&eia); // TODO(unknown): get rid of this
	{
		UI::ProgressWindow loader_ui("pics/editor.jpg");
		std::vector<std::string> tipstext;
		tipstext.push_back("editor");
		GameTips editortips(loader_ui, tipstext);

		{
			Widelands::Map & map = *new Widelands::Map;
			egbase.set_map(&map);
			if (filename.empty()) {
				loader_ui.step("Creating empty map...");
				map.create_empty_map(
				   egbase.world(),
				   64,
				   64,
					0,
					/** TRANSLATORS: Default name for new map */
				   _("No Name"),
					g_options.pull_section("global").get_string("realname", pgettext("map_name", "Unknown")));

				load_all_tribes(&egbase, &loader_ui);

				egbase.load_graphics(loader_ui);
				loader_ui.step(std::string());
			} else {
				loader_ui.stepf("Loading map \"%s\"...", filename.c_str());
				eia.load(filename);
			}
		}

		egbase.postload();

		eia.start();

		if (!script_to_run.empty()) {
			eia.egbase().lua().run_script(script_to_run);
		}
	}
	eia.run<UI::Panel::Returncodes>();

	egbase.cleanup_objects();
}

void EditorInteractive::map_changed(const MapWas& action) {
	switch (action) {
		case MapWas::kReplaced:
			history_.reset(new EditorHistory(undo_, redo_));
			undo_.set_enabled(false);
			redo_.set_enabled(false);

			tools_.reset(new Tools());
			select_tool(tools_->increase_height, EditorTool::First);
			set_sel_radius(0);

			set_need_save(false);
			show_buildhelp(true);

			// Close all windows.
			for (Panel* child = get_first_child(); child; child = child->get_next_sibling()) {
				if (is_a(UI::Window, child)) {
					child->die();
				}
			}
			break;

		case MapWas::kGloballyMutated:
			break;
	}

	mutable_field_overlay_manager()->remove_all_overlays();
	register_overlays();
}

EditorInteractive::Tools* EditorInteractive::tools() {
	return tools_.get();
}
