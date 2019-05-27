/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
#include "editor/tools/delete_immovable_tool.h"
#include "editor/ui_menus/help.h"
#include "editor/ui_menus/main_menu.h"
#include "editor/ui_menus/main_menu_load_map.h"
#include "editor/ui_menus/main_menu_save_map.h"
#include "editor/ui_menus/player_menu.h"
#include "editor/ui_menus/tool_menu.h"
#include "editor/ui_menus/toolsize_menu.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/maptriangleregion.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "wui/game_tips.h"
#include "wui/interactive_base.h"

namespace {
using Widelands::Building;

// Load all tribes from disk.
void load_all_tribes(Widelands::EditorGameBase* egbase, UI::ProgressWindow* loader_ui) {
	loader_ui->step(_("Loading tribes"));
	egbase->tribes();
}

}  // namespace

EditorInteractive::EditorInteractive(Widelands::EditorGameBase& e)
   : InteractiveBase(e, g_options.pull_section("global")),
     need_save_(false),
     realtime_(SDL_GetTicks()),
     is_painting_(false),
     undo_(nullptr),
     redo_(nullptr),
     tools_(new Tools(e.map())),
     history_(nullptr)  // history needs the undo/redo buttons
{
	add_toolbar_button("wui/menus/menu_toggle_menu", "menu", _("Main menu"), &mainmenu_, true);
	mainmenu_.open_window = [this] { new EditorMainMenu(*this, mainmenu_); };

	add_toolbar_button(
	   "wui/editor/editor_menu_toggle_tool_menu", "tools", _("Tools"), &toolmenu_, true);
	toolmenu_.open_window = [this] { new EditorToolMenu(*this, toolmenu_); };

	add_toolbar_button(
	   "wui/editor/editor_menu_set_toolsize_menu", "toolsize", _("Tool size"), &toolsizemenu_, true);
	toolsizemenu_.open_window = [this] { new EditorToolsizeMenu(*this, toolsizemenu_); };

	add_toolbar_button(
	   "wui/editor/editor_menu_player_menu", "players", _("Players"), &playermenu_, true);
	playermenu_.open_window = [this] {
		select_tool(tools_->set_starting_pos, EditorTool::First);
		new EditorPlayerMenu(*this, playermenu_);
	};

	toolbar()->add_space(15);

	toggle_buildhelp_ = add_toolbar_button(
	   "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show building spaces (on/off)"));
	toggle_buildhelp_->sigclicked.connect(boost::bind(&EditorInteractive::toggle_buildhelp, this));
	toggle_grid_ = add_toolbar_button("wui/menus/menu_toggle_grid", "grid", _("Show grid (on/off)"));
	toggle_grid_->set_perm_pressed(true);
	toggle_grid_->sigclicked.connect([this]() { toggle_grid(); });
	toggle_immovables_ = add_toolbar_button(
	   "wui/menus/menu_toggle_immovables", "immovables", _("Show immovables (on/off)"));
	toggle_immovables_->set_perm_pressed(true);
	toggle_immovables_->sigclicked.connect([this]() { toggle_immovables(); });
	toggle_bobs_ =
	   add_toolbar_button("wui/menus/menu_toggle_bobs", "animals", _("Show animals (on/off)"));
	toggle_bobs_->set_perm_pressed(true);
	toggle_bobs_->sigclicked.connect([this]() { toggle_bobs(); });
	toggle_resources_ = add_toolbar_button(
	   "wui/menus/menu_toggle_resources", "resources", _("Show resources (on/off)"));
	toggle_resources_->set_perm_pressed(true);
	toggle_resources_->sigclicked.connect([this]() { toggle_resources(); });

	toolbar()->add_space(15);

	add_toolbar_button(
	   "wui/menus/menu_toggle_minimap", "minimap", _("Minimap"), &minimap_registry(), true);
	minimap_registry().open_window = [this] { toggle_minimap(); };

	auto zoom = add_toolbar_button("wui/menus/menu_reset_zoom", "reset_zoom", _("Reset zoom"));
	zoom->sigclicked.connect([this] {
		map_view()->zoom_around(
		   1.f, Vector2f(get_w() / 2.f, get_h() / 2.f), MapView::Transition::Smooth);
	});

	toolbar()->add_space(15);

	undo_ = add_toolbar_button("wui/editor/editor_undo", "undo", _("Undo"));
	redo_ = add_toolbar_button("wui/editor/editor_redo", "redo", _("Redo"));

	history_.reset(new EditorHistory(*undo_, *redo_));

	undo_->sigclicked.connect([this] { history_->undo_action(egbase().world()); });
	redo_->sigclicked.connect([this] { history_->redo_action(egbase().world()); });

	toolbar()->add_space(15);

	add_toolbar_button("ui_basic/menu_help", "help", _("Help"), &helpmenu_, true);
	helpmenu_.open_window = [this] { new EditorHelp(*this, helpmenu_, &egbase().lua()); };

	adjust_toolbar_position();

#ifndef NDEBUG
	set_display_flag(InteractiveBase::dfDebug, true);
#else
	set_display_flag(InteractiveBase::dfDebug, false);
#endif

	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		map_clicked(node_and_triangle, false);
	});

	minimap_registry().minimap_type = MiniMapType::kStaticMap;
}

void EditorInteractive::load(const std::string& filename) {
	assert(filename.size());

	Widelands::Map* map = egbase().mutable_map();

	cleanup_for_load();

	std::unique_ptr<Widelands::MapLoader> ml(map->get_correct_loader(filename));
	if (!ml.get())
		throw WLWarning(
		   _("Unsupported Format"),
		   _("Widelands could not load the file \"%s\". The file format seems to be incompatible."),
		   filename.c_str());
	ml->preload_map(true);

	UI::ProgressWindow loader_ui("images/loadscreens/editor.jpg");
	std::vector<std::string> tipstext;
	tipstext.push_back("editor");

	GameTips editortips(loader_ui, tipstext);

	load_all_tribes(&egbase(), &loader_ui);

	// Create the players. TODO(SirVer): this must be managed better
	// TODO(GunChleoc): Ugly - we only need this for the test suite right now
	loader_ui.step(_("Creating players"));
	iterate_player_numbers(p, map->get_nrplayers()) {
		if (!map->get_scenario_player_tribe(p).empty()) {
			egbase().add_player(
			   p, 0, map->get_scenario_player_tribe(p), map->get_scenario_player_name(p));
		}
	}

	ml->load_map_complete(egbase(), Widelands::MapLoader::LoadType::kEditor);
	egbase().postload();
	egbase().load_graphics(loader_ui);
	map_changed(MapWas::kReplaced);
}

void EditorInteractive::cleanup_for_load() {
	// TODO(unknown): get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	egbase().cleanup_for_load();
}

/// Called just before the editor starts, after postload, init and gfxload.
void EditorInteractive::start() {
	// Run the editor initialization script, if any
	try {
		egbase().lua().run_script("map:scripting/editor_init.lua");
	} catch (LuaScriptNotExistingError&) {
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
		if (SDL_GetModState() & KMOD_CTRL) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
		} else {
			UI::WLMessageBox mmb(this, _("Unsaved Map"),
			                     _("The map has not been saved, do you really want to quit?"),
			                     UI::WLMessageBox::MBoxType::kOkCancel);
			if (mmb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack)
				return;
		}
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void EditorInteractive::map_clicked(const Widelands::NodeAndTriangle<>& node_and_triangle,
                                    const bool should_draw) {
	history_->do_action(tools_->current(), tools_->use_tool, *egbase().mutable_map(),
	                    egbase().world(), node_and_triangle, *this, should_draw);
	set_need_save(true);
}

bool EditorInteractive::handle_mouserelease(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		is_painting_ = false;
	}
	return InteractiveBase::handle_mouserelease(btn, x, y);
}

bool EditorInteractive::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		is_painting_ = true;
	}
	return InteractiveBase::handle_mousepress(btn, x, y);
}

void EditorInteractive::draw(RenderTarget& dst) {
	const auto& ebase = egbase();
	auto* fields_to_draw = map_view()->draw_terrain(ebase, Workareas(), draw_grid_, &dst);

	const float scale = 1.f / map_view()->view().zoom;
	const uint32_t gametime = ebase.get_gametime();

	// The map provides a mapping from player number to Coords, while we require
	// the inverse here. We construct this, but this is done on every frame and
	// therefore potentially expensive - though it never showed up in any of my
	// profiles. We could change the Map should this become a bottleneck, since
	// plrnum -> coords is needed less often.
	const auto& map = ebase.map();
	std::map<Widelands::Coords, int> starting_positions;
	for (int i = 1; i <= map.get_nrplayers(); ++i) {
		starting_positions[map.get_starting_pos(i)] = i;
	}

	// Figure out which fields are currently under the selection.
	std::set<Widelands::Coords> selected_nodes;
	std::set<Widelands::TCoords<>> selected_triangles;
	if (!get_sel_triangles()) {
		Widelands::MapRegion<> mr(map, Widelands::Area<>(get_sel_pos().node, get_sel_radius()));
		do {
			selected_nodes.emplace(mr.location());
		} while (mr.advance(map));
	} else {
		Widelands::MapTriangleRegion<> mr(
		   map, Widelands::Area<Widelands::TCoords<>>(get_sel_pos().triangle, get_sel_radius()));
		do {
			selected_triangles.emplace(mr.location());
		} while (mr.advance(map));
	}

	const auto& world = ebase.world();
	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		const FieldsToDraw::Field& field = fields_to_draw->at(idx);
		if (draw_immovables_) {
			Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
			if (imm != nullptr && imm->get_positions(ebase).front() == field.fcoords) {
				imm->draw(
				   gametime, TextToDraw::kNone, field.rendertarget_pixel, field.fcoords, scale, &dst);
			}
		}

		if (draw_bobs_) {
			for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob;
			     bob = bob->get_next_bob()) {
				bob->draw(
				   ebase, TextToDraw::kNone, field.rendertarget_pixel, field.fcoords, scale, &dst);
			}
		}

		// Draw resource overlay.
		uint8_t const amount = field.fcoords.field->get_resources_amount();
		if (draw_resources_ && amount > 0) {
			const std::string& immname =
			   world.get_resource(field.fcoords.field->get_resources())->editor_image(amount);
			if (!immname.empty()) {
				const auto* pic = g_gr->images().get(immname);
				blit_field_overlay(
				   &dst, field, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
			}
		}

		// Draw build help.
		if (buildhelp()) {
			const auto* overlay =
			   get_buildhelp_overlay(tools_->current().nodecaps_for_buildhelp(field.fcoords, ebase));
			if (overlay != nullptr) {
				blit_field_overlay(&dst, field, overlay->pic, overlay->hotspot, scale);
			}
		}

		// Draw the player starting position overlays.
		const auto it = starting_positions.find(field.fcoords);
		if (it != starting_positions.end()) {
			const Image* player_image =
			   playercolor_image(it->second - 1, "images/players/player_position.png");
			assert(player_image != nullptr);
			constexpr int kStartingPosHotspotY = 55;
			blit_field_overlay(&dst, field, player_image,
			                   Vector2i(player_image->width() / 2, kStartingPosHotspotY), scale);
		}

		// Draw selection markers on the field.
		if (selected_nodes.count(field.fcoords) > 0) {
			const Image* pic = get_sel_picture();
			blit_field_overlay(&dst, field, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
		}

		// Draw selection markers on the triangles.
		if (field.all_neighbors_valid()) {
			const FieldsToDraw::Field& rn = fields_to_draw->at(field.rn_index);
			const FieldsToDraw::Field& brn = fields_to_draw->at(field.brn_index);
			const FieldsToDraw::Field& bln = fields_to_draw->at(field.bln_index);
			if (selected_triangles.count(
			       Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::R))) {
				const Vector2i tripos(
				   (field.rendertarget_pixel.x + rn.rendertarget_pixel.x + brn.rendertarget_pixel.x) /
				      3,
				   (field.rendertarget_pixel.y + rn.rendertarget_pixel.y + brn.rendertarget_pixel.y) /
				      3);
				const Image* pic = get_sel_picture();
				blit_overlay(&dst, tripos, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
			}
			if (selected_triangles.count(
			       Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::D))) {
				const Vector2i tripos(
				   (field.rendertarget_pixel.x + bln.rendertarget_pixel.x + brn.rendertarget_pixel.x) /
				      3,
				   (field.rendertarget_pixel.y + bln.rendertarget_pixel.y + brn.rendertarget_pixel.y) /
				      3);
				const Image* pic = get_sel_picture();
				blit_overlay(&dst, tripos, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
			}
		}
	}
}

/// Needed to get freehand painting tools (hold down mouse and move to edit).
void EditorInteractive::set_sel_pos(Widelands::NodeAndTriangle<> const sel) {
	bool const target_changed = tools_->current().operates_on_triangles() ?
	                               sel.triangle != get_sel_pos().triangle :
	                               sel.node != get_sel_pos().node;
	InteractiveBase::set_sel_pos(sel);
	if (target_changed && is_painting_) {
		map_clicked(sel, true);
	}
}

void EditorInteractive::set_sel_radius_and_update_menu(uint32_t const val) {
	if (tools_->current().has_size_one()) {
		set_sel_radius(0);
		return;
	}
	if (UI::UniqueWindow* const w = toolsizemenu_.window) {
		dynamic_cast<EditorToolsizeMenu&>(*w).update(val);
	} else {
		set_sel_radius(val);
	}
}

void EditorInteractive::stop_painting() {
	is_painting_ = false;
}

bool EditorInteractive::player_hears_field(const Widelands::Coords&) const {
	return true;
}

void EditorInteractive::on_buildhelp_changed(const bool value) {
	toggle_buildhelp_->set_perm_pressed(value);
}

void EditorInteractive::toggle_resources() {
	draw_resources_ = !draw_resources_;
	toggle_resources_->set_perm_pressed(draw_resources_);
}

void EditorInteractive::toggle_immovables() {
	draw_immovables_ = !draw_immovables_;
	toggle_immovables_->set_perm_pressed(draw_immovables_);
}

void EditorInteractive::toggle_bobs() {
	draw_bobs_ = !draw_bobs_;
	toggle_bobs_->set_perm_pressed(draw_bobs_);
}

void EditorInteractive::toggle_grid() {
	draw_grid_ = !draw_grid_;
	toggle_grid_->set_perm_pressed(draw_grid_);
}

bool EditorInteractive::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		// Sel radius
		case SDLK_1:
			if (code.mod & (KMOD_CTRL)) {
				toggle_buildhelp();
			} else {
				set_sel_radius_and_update_menu(0);
			}
			return true;
		case SDLK_2:
			if (code.mod & (KMOD_CTRL)) {
				toggle_immovables();
			} else {
				set_sel_radius_and_update_menu(1);
			}
			return true;
		case SDLK_3:
			if (code.mod & (KMOD_CTRL)) {
				toggle_bobs();
			} else {
				set_sel_radius_and_update_menu(2);
			}
			return true;
		case SDLK_4:
			if (code.mod & (KMOD_CTRL)) {
				toggle_resources();
			} else {
				set_sel_radius_and_update_menu(3);
			}
			return true;
		case SDLK_5:
			set_sel_radius_and_update_menu(4);
			return true;
		case SDLK_6:
			set_sel_radius_and_update_menu(5);
			return true;
		case SDLK_7:
			set_sel_radius_and_update_menu(6);
			return true;
		case SDLK_8:
			set_sel_radius_and_update_menu(7);
			return true;
		case SDLK_9:
			set_sel_radius_and_update_menu(8);
			return true;
		case SDLK_0:
			if (!(code.mod & KMOD_CTRL)) {
				set_sel_radius_and_update_menu(9);
				return true;
			}
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (tools_->use_tool == EditorTool::First)
				select_tool(tools_->current(), EditorTool::Second);
			return true;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
		// TODO(GunChleoc): Keeping ALT and MODE to make the transition easier. Remove for Build 20.
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools_->use_tool == EditorTool::First)
				select_tool(tools_->current(), EditorTool::Third);
			return true;

		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_g:
			toggle_grid();
			return true;

		case SDLK_c:
			set_display_flag(
			   InteractiveBase::dfShowCensus, !get_display_flag(InteractiveBase::dfShowCensus));
			return true;

		case SDLK_h:
			mainmenu_.toggle();
			return true;

		case SDLK_i:
			select_tool(tools_->info, EditorTool::First);
			return true;

		case SDLK_l:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new MainMenuLoadMap(*this);
			return true;

		case SDLK_m:
			minimap_registry().toggle();
			return true;

		case SDLK_p:
			playermenu_.toggle();
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new MainMenuSaveMap(*this);
			return true;

		case SDLK_t:
			toolmenu_.toggle();
			return true;

		case SDLK_y:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				history_->redo_action(egbase().world());
			return true;

		case SDLK_z:
			if ((code.mod & (KMOD_LCTRL | KMOD_RCTRL)) && (code.mod & (KMOD_LSHIFT | KMOD_RSHIFT)))
				history_->redo_action(egbase().world());
			else if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				history_->undo_action(egbase().world());
			return true;

		case SDLK_F1:
			helpmenu_.toggle();
			return true;

		default:
			break;
		}
	} else {
		// key up events
		switch (code.sym) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		// TODO(GunChleoc): Keeping ALT and MODE to make the transition easier. Remove for Build 20.
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools_->use_tool != EditorTool::First)
				select_tool(tools_->current(), EditorTool::First);
			return true;
		default:
			break;
		}
	}
	return InteractiveBase::handle_key(down, code);
}

void EditorInteractive::select_tool(EditorTool& primary, EditorTool::ToolIndex const which) {
	if (which == EditorTool::First && &primary != tools_->current_pointer) {
		if (primary.has_size_one()) {
			set_sel_radius(0);
			if (UI::UniqueWindow* const w = toolsizemenu_.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.set_buttons_enabled(false);
			}
		} else {
			if (UI::UniqueWindow* const w = toolsizemenu_.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.update(toolsize_menu.value());
			}
		}
		egbase().mutable_map()->recalc_whole_map(egbase().world());
	}
	tools_->current_pointer = &primary;
	tools_->use_tool = which;

	if (const Image* sel_pic = primary.get_sel(which)) {
		set_sel_picture(sel_pic);
	} else {
		unset_sel_picture();
	}
	set_sel_triangles(primary.operates_on_triangles());
}

void EditorInteractive::run_editor(const std::string& filename, const std::string& script_to_run) {
	Widelands::EditorGameBase egbase(nullptr);
	EditorInteractive& eia = *new EditorInteractive(egbase);
	egbase.set_ibase(&eia);  // TODO(unknown): get rid of this
	{
		UI::ProgressWindow loader_ui("images/loadscreens/editor.jpg");
		std::vector<std::string> tipstext;
		tipstext.push_back("editor");
		GameTips editortips(loader_ui, tipstext);

		{
			if (filename.empty()) {
				loader_ui.step(_("Creating empty map…"));
				egbase.mutable_map()->create_empty_map(
				   egbase.world(), 64, 64, 0,
				   /** TRANSLATORS: Default name for new map */
				   _("No Name"),
				   g_options.pull_section("global").get_string(
				      "realname",
				      /** TRANSLATORS: Map author name when it hasn't been set yet */
				      pgettext("author_name", "Unknown")));

				load_all_tribes(&egbase, &loader_ui);

				egbase.load_graphics(loader_ui);
				loader_ui.step(std::string());
			} else {
				loader_ui.step((boost::format(_("Loading map “%s”…")) % filename).str());
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
		history_.reset(new EditorHistory(*undo_, *redo_));
		undo_->set_enabled(false);
		redo_->set_enabled(false);

		tools_.reset(new Tools(egbase().map()));
		select_tool(tools_->info, EditorTool::First);
		set_sel_radius(0);

		set_need_save(false);
		show_buildhelp(true);

		// Close all windows.
		for (Panel* child = get_first_child(); child; child = child->get_next_sibling()) {
			if (is_a(UI::Window, child)) {
				child->die();
			}
		}

		// Make sure that we will start at coordinates (0,0).
		map_view()->set_view(MapView::View{Vector2f::zero(), 1.f}, MapView::Transition::Jump);
		set_sel_pos(Widelands::NodeAndTriangle<>{
		   Widelands::Coords(0, 0),
		   Widelands::TCoords<>(Widelands::Coords(0, 0), Widelands::TriangleIndex::D)});
		break;

	case MapWas::kGloballyMutated:
		break;
	}
}

EditorInteractive::Tools* EditorInteractive::tools() {
	return tools_.get();
}
