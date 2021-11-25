/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/warning.h"
#include "build_info.h"
#include "editor/tools/decrease_resources_tool.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/set_port_space_tool.h"
#include "editor/tools/set_terrain_tool.h"
#include "editor/ui_menus/help.h"
#include "editor/ui_menus/main_menu_load_map.h"
#include "editor/ui_menus/main_menu_map_options.h"
#include "editor/ui_menus/main_menu_new_map.h"
#include "editor/ui_menus/main_menu_random_map.h"
#include "editor/ui_menus/main_menu_save_map.h"
#include "editor/ui_menus/player_menu.h"
#include "editor/ui_menus/tool_change_height_options_menu.h"
#include "editor/ui_menus/tool_change_resources_options_menu.h"
#include "editor/ui_menus/tool_noise_height_options_menu.h"
#include "editor/ui_menus/tool_place_critter_options_menu.h"
#include "editor/ui_menus/tool_place_immovable_options_menu.h"
#include "editor/ui_menus/tool_resize_options_menu.h"
#include "editor/ui_menus/tool_set_terrain_options_menu.h"
#include "editor/ui_menus/toolsize_menu.h"
#include "graphic/mouse_cursor.h"
#include "graphic/playercolor.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/addons.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/map_object_type.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/mapregion.h"
#include "logic/maptriangleregion.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"
#include "wui/interactive_base.h"
#include "wui/toolbar.h"

std::string editor_splash_image() {
	return template_dir() + "loadscreens/editor.jpg";
}

EditorInteractive::EditorInteractive(Widelands::EditorGameBase& e)
   : InteractiveBase(e, get_config_section(), nullptr),
     need_save_(false),
     realtime_(SDL_GetTicks()),
     is_painting_(false),
     mainmenu_(toolbar(),
               "dropdown_menu_main",
               0,
               0,
               MainToolbar::kButtonSize,
               10,
               MainToolbar::kButtonSize,
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Title for the main menu button in the editor */
                  _("Main Menu"),
                  shortcut_string_for(KeyboardShortcut::kEditorMenu),
                  UI::PanelStyle::kWui),
               UI::DropdownType::kPictorialMenu,
               UI::PanelStyle::kWui,
               UI::ButtonStyle::kWuiPrimary),
     toolmenu_(toolbar(),
               "dropdown_menu_tools",
               0,
               0,
               MainToolbar::kButtonSize,
               12,
               MainToolbar::kButtonSize,
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Title for the tool menu button in the editor */
                  _("Tools"),
                  shortcut_string_for(KeyboardShortcut::kEditorTools),
                  UI::PanelStyle::kWui),
               UI::DropdownType::kPictorialMenu,
               UI::PanelStyle::kWui,
               UI::ButtonStyle::kWuiPrimary),
     showhidemenu_(toolbar(),
                   "dropdown_menu_showhide",
                   0,
                   0,
                   MainToolbar::kButtonSize,
                   10,
                   MainToolbar::kButtonSize,
                   /** TRANSLATORS: Title for a menu button in the editor. This menu will show/hide
                      building spaces, animals, immovables, resources */
                   _("Show / Hide"),
                   UI::DropdownType::kPictorialMenu,
                   UI::PanelStyle::kWui,
                   UI::ButtonStyle::kWuiPrimary),
     undo_(nullptr),
     redo_(nullptr),
     tools_(new Tools(e.map())),
     history_(nullptr)  // history needs the undo/redo buttons
{
	add_main_menu();
	add_tool_menu();

	add_toolbar_button(
	   "wui/editor/menus/toolsize", "toolsize", _("Tool size"), &menu_windows_.toolsize, true);
	menu_windows_.toolsize.open_window = [this] {
		new EditorToolsizeMenu(*this, menu_windows_.toolsize);
	};

	toolbar()->add_space(15);

	add_mapview_menu(MiniMapType::kStaticMap);
	add_showhide_menu();

	toolbar()->add_space(15);

	undo_ = add_toolbar_button(
	   "wui/editor/menus/undo", "undo",
	   as_tooltip_text_with_hotkey(
	      _("Undo"), shortcut_string_for(KeyboardShortcut::kEditorUndo), UI::PanelStyle::kWui));
	redo_ = add_toolbar_button(
	   "wui/editor/menus/redo", "redo",
	   as_tooltip_text_with_hotkey(
	      _("Redo"), shortcut_string_for(KeyboardShortcut::kEditorRedo), UI::PanelStyle::kWui));

	history_.reset(new EditorHistory(*undo_, *redo_));

	undo_->sigclicked.connect([this] { history_->undo_action(); });
	redo_->sigclicked.connect([this] { history_->redo_action(); });

	toolbar()->add_space(15);

	add_toolbar_button("ui_basic/menu_help", "help",
	                   as_tooltip_text_with_hotkey(
	                      _("Help"), shortcut_string_for(KeyboardShortcut::kCommonEncyclopedia),
	                      UI::PanelStyle::kWui),
	                   &menu_windows_.help, true);
	menu_windows_.help.open_window = [this] {
		new EditorHelp(*this, menu_windows_.help, &egbase().lua());
	};

	finalize_toolbar();

#ifndef NDEBUG
	set_display_flag(InteractiveBase::dfDebug, true);
#else
	set_display_flag(InteractiveBase::dfDebug, false);
#endif

	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		map_clicked(node_and_triangle, false);
	});

	initialization_complete();
}

void EditorInteractive::add_main_menu() {
	mainmenu_.set_image(g_image_cache->get("images/wui/editor/menus/main_menu.png"));

	menu_windows_.newmap.open_window = [this] { new MainMenuNewMap(*this, menu_windows_.newmap); };
	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("New Map"), MainMenuEntry::kNewMap,
	              g_image_cache->get("images/wui/editor/menus/new_map.png"));

	menu_windows_.newrandommap.open_window = [this] {
		new MainMenuNewRandomMap(*this, menu_windows_.newrandommap, egbase().map().get_width(),
		                         egbase().map().get_height());
	};
	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("New Random Map"), MainMenuEntry::kNewRandomMap,
	              g_image_cache->get("images/wui/editor/menus/new_random_map.png"));

	menu_windows_.loadmap.open_window = [this] {
		new MainMenuLoadMap(*this, menu_windows_.loadmap);
	};
	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("Load Map"), MainMenuEntry::kLoadMap,
	              g_image_cache->get("images/wui/editor/menus/load_map.png"), false, "",
	              shortcut_string_for(KeyboardShortcut::kEditorLoad));

	menu_windows_.savemap.open_window = [this] {
		new MainMenuSaveMap(*this, menu_windows_.savemap, menu_windows_.mapoptions);
	};
	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("Save Map"), MainMenuEntry::kSaveMap,
	              g_image_cache->get("images/wui/editor/menus/save_map.png"), false, "",
	              shortcut_string_for(KeyboardShortcut::kEditorSave));

	menu_windows_.mapoptions.open_window = [this] {
		new MainMenuMapOptions(*this, menu_windows_.mapoptions);
	};
	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("Map Options"), MainMenuEntry::kMapOptions,
	              g_image_cache->get("images/wui/editor/menus/map_options.png"));

	/** TRANSLATORS: An entry in the editor's main menu */
	mainmenu_.add(_("Exit Editor"), MainMenuEntry::kExitEditor,
	              g_image_cache->get("images/wui/menus/exit.png"));
	mainmenu_.selected.connect([this] { main_menu_selected(mainmenu_.get_selected()); });
	toolbar()->add(&mainmenu_);
}

void EditorInteractive::main_menu_selected(MainMenuEntry entry) {
	switch (entry) {
	case MainMenuEntry::kNewMap: {
		menu_windows_.newmap.toggle();
	} break;
	case MainMenuEntry::kNewRandomMap: {
		menu_windows_.newrandommap.toggle();
	} break;
	case MainMenuEntry::kLoadMap: {
		menu_windows_.loadmap.toggle();
	} break;
	case MainMenuEntry::kSaveMap: {
		menu_windows_.savemap.toggle();
	} break;
	case MainMenuEntry::kMapOptions: {
		menu_windows_.mapoptions.toggle();
	} break;
	case MainMenuEntry::kExitEditor: {
		exit();
	}
	}
}

void EditorInteractive::add_tool_menu() {
	toolmenu_.set_image(g_image_cache->get("images/wui/editor/menus/tools.png"));

	tool_windows_.height.open_window = [this] {
		new EditorToolChangeHeightOptionsMenu(*this, tools()->increase_height, tool_windows_.height);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Change height"), ToolMenuEntry::kChangeHeight,
	              g_image_cache->get("images/wui/editor/tools/height.png"), false,
	              /** TRANSLATORS: Tooltip for the change height tool in the editor */
	              _("Change the terrain height"));

	tool_windows_.noiseheight.open_window = [this] {
		new EditorToolNoiseHeightOptionsMenu(*this, tools()->noise_height, tool_windows_.noiseheight);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Random height"), ToolMenuEntry::kRandomHeight,
	              g_image_cache->get("images/wui/editor/tools/noise_height.png"), false,
	              /** TRANSLATORS: Tooltip for the random height tool in the editor */
	              _("Set the terrain height to random values"));

	tool_windows_.terrain.open_window = [this] {
		new EditorToolSetTerrainOptionsMenu(*this, tools()->set_terrain, tool_windows_.terrain);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Terrain"), ToolMenuEntry::kTerrain,
	              g_image_cache->get("images/wui/editor/tools/terrain.png"), false,
	              /** TRANSLATORS: Tooltip for the terrain tool in the editor */
	              _("Change the map’s terrain"));

	tool_windows_.immovables.open_window = [this] {
		new EditorToolPlaceImmovableOptionsMenu(
		   *this, tools()->place_immovable, tool_windows_.immovables);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Immovables"), ToolMenuEntry::kImmovables,
	              g_image_cache->get("images/wui/editor/tools/immovables.png"), false,
	              /** TRANSLATORS: Tooltip for the immovables tool in the editor */
	              _("Add or remove immovables"));

	tool_windows_.critters.open_window = [this] {
		new EditorToolPlaceCritterOptionsMenu(*this, tools()->place_critter, tool_windows_.critters);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Animals"), ToolMenuEntry::kAnimals,
	              g_image_cache->get("images/wui/editor/tools/critters.png"), false,
	              /** TRANSLATORS: Tooltip for the animals tool in the editor */
	              _("Add or remove animals"));

	tool_windows_.resources.open_window = [this] {
		new EditorToolChangeResourcesOptionsMenu(
		   *this, tools()->increase_resources, tool_windows_.resources);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Resources"), ToolMenuEntry::kResources,
	              g_image_cache->get("images/wui/editor/tools/resources.png"), false,
	              /** TRANSLATORS: Tooltip for the resources tool in the editor */
	              _("Set or change resources"));

	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Port spaces"), ToolMenuEntry::kPortSpace,
	              g_image_cache->get("images/wui/editor/tools/port_spaces.png"), false,
	              /** TRANSLATORS: Tooltip for the port spaces tool in the editor */
	              _("Add or remove port spaces"));

	tool_windows_.players.open_window = [this] {
		new EditorPlayerMenu(*this, tools()->set_starting_pos, tool_windows_.players);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Players"), ToolMenuEntry::kPlayers,
	              g_image_cache->get("images/wui/editor/tools/players.png"), false,
	              /** TRANSLATORS: Tooltip for the map size tool in the editor */
	              _("Set number of players and their names, tribes and starting positions"),
	              shortcut_string_for(KeyboardShortcut::kEditorPlayers));

	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Map origin"), ToolMenuEntry::kMapOrigin,
	              g_image_cache->get("images/wui/editor/tools/map_origin.png"), false,
	              /** TRANSLATORS: Tooltip for the map origin tool in the editor */
	              _("Set the position that will have the coordinates (0, 0). This will be the "
	                "top-left corner of a generated minimap."));

	tool_windows_.resizemap.open_window = [this] {
		new EditorToolResizeOptionsMenu(*this, tools()->resize, tool_windows_.resizemap);
	};
	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Map size"), ToolMenuEntry::kMapSize,
	              g_image_cache->get("images/wui/editor/tools/resize_map.png"), false,
	              /** TRANSLATORS: Tooltip for the map size tool in the editor */
	              _("Change the map’s size"));

	/** TRANSLATORS: An entry in the editor's tool menu */
	toolmenu_.add(_("Information"), ToolMenuEntry::kFieldInfo,
	              g_image_cache->get("images/wui/editor/fsel_editor_info.png"), false,
	              /** TRANSLATORS: Tooltip for the map information tool in the editor */
	              _("Click on a field to show information about it"),
	              shortcut_string_for(KeyboardShortcut::kEditorInfo));
	toolmenu_.selected.connect([this] { tool_menu_selected(toolmenu_.get_selected()); });
	toolbar()->add(&toolmenu_);
}

void EditorInteractive::tool_menu_selected(ToolMenuEntry entry) {
	switch (entry) {
	case ToolMenuEntry::kChangeHeight:
		tool_windows_.height.toggle();
		break;
	case ToolMenuEntry::kRandomHeight:
		tool_windows_.noiseheight.toggle();
		break;
	case ToolMenuEntry::kTerrain:
		tool_windows_.terrain.toggle();
		break;
	case ToolMenuEntry::kImmovables:
		tool_windows_.immovables.toggle();
		break;
	case ToolMenuEntry::kAnimals:
		tool_windows_.critters.toggle();
		break;
	case ToolMenuEntry::kResources:
		tool_windows_.resources.toggle();
		break;
	case ToolMenuEntry::kPortSpace:
		select_tool(tools()->set_port_space, EditorTool::First);
		break;
	case ToolMenuEntry::kPlayers:
		tool_windows_.players.toggle();
		break;
	case ToolMenuEntry::kMapOrigin:
		select_tool(tools()->set_origin, EditorTool::First);
		break;
	case ToolMenuEntry::kMapSize:
		tool_windows_.resizemap.toggle();
		break;
	case ToolMenuEntry::kFieldInfo:
		select_tool(tools()->info, EditorTool::First);
		break;
	}
	toolmenu_.toggle();
}

void EditorInteractive::add_showhide_menu() {
	showhidemenu_.set_image(g_image_cache->get("images/wui/menus/showhide.png"));
	toolbar()->add(&showhidemenu_);

	rebuild_showhide_menu();

	showhidemenu_.selected.connect([this] { showhide_menu_selected(showhidemenu_.get_selected()); });
}

void EditorInteractive::rebuild_showhide_menu() {
	const ShowHideEntry last_selection =
	   showhidemenu_.has_selection() ? showhidemenu_.get_selected() : ShowHideEntry::kBuildingSpaces;

	showhidemenu_.clear();

	/** TRANSLATORS: An entry in the editor's show/hide menu to toggle whether building spaces are
	 * shown */
	showhidemenu_.add(buildhelp() ? _("Hide Building Spaces") : _("Show Building Spaces"),
	                  ShowHideEntry::kBuildingSpaces,
	                  g_image_cache->get("images/wui/menus/toggle_buildhelp.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kCommonBuildhelp));

	/** TRANSLATORS: An entry in the editor's show/hide menu to toggle whether the map grid is shown
	 */
	showhidemenu_.add(draw_grid_ ? _("Hide Grid") : _("Show Grid"), ShowHideEntry::kGrid,
	                  g_image_cache->get("images/wui/menus/menu_toggle_grid.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kEditorShowhideGrid));

	/** TRANSLATORS: An entry in the editor's show/hide menu to toggle whether immovables (trees,
	 * rocks etc.) are shown */
	showhidemenu_.add(draw_immovables_ ? _("Hide Immovables") : _("Show Immovables"),
	                  ShowHideEntry::kImmovables,
	                  g_image_cache->get("images/wui/menus/toggle_immovables.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kEditorShowhideImmovables));

	/** TRANSLATORS: An entry in the editor's show/hide menu to toggle whether animals are shown */
	showhidemenu_.add(draw_bobs_ ? _("Hide Animals") : _("Show Animals"), ShowHideEntry::kAnimals,
	                  g_image_cache->get("images/wui/menus/toggle_bobs.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kEditorShowhideCritters));

	/** TRANSLATORS: An entry in the editor's show/hide menu to toggle whether resources are shown */
	showhidemenu_.add(draw_resources_ ? _("Hide Resources") : _("Show Resources"),
	                  ShowHideEntry::kResources,
	                  g_image_cache->get("images/wui/menus/toggle_resources.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kEditorShowhideResources));

	showhidemenu_.select(last_selection);
}

void EditorInteractive::showhide_menu_selected(ShowHideEntry entry) {
	switch (entry) {
	case ShowHideEntry::kBuildingSpaces: {
		toggle_buildhelp();
	} break;
	case ShowHideEntry::kGrid: {
		toggle_grid();
	} break;
	case ShowHideEntry::kImmovables: {
		toggle_immovables();
	} break;
	case ShowHideEntry::kAnimals: {
		toggle_bobs();
	} break;
	case ShowHideEntry::kResources: {
		toggle_resources();
	} break;
	}
	showhidemenu_.toggle();
}

void EditorInteractive::load(const std::string& filename) {
	assert(!filename.empty());
	assert(egbase().has_loader_ui());

	Widelands::Map* map = egbase().mutable_map();

	cleanup_for_load();

	std::unique_ptr<Widelands::MapLoader> ml(map->get_correct_loader(filename));
	if (!ml) {
		throw WLWarning(
		   _("Unsupported Format"),
		   _("Widelands could not load the file \"%s\". The file format seems to be incompatible."),
		   filename.c_str());
	}
	ml->preload_map(true, &egbase().enabled_addons());

	EditorInteractive::load_world_units(this, egbase());

	// Create the players. TODO(SirVer): this must be managed better
	// TODO(GunChleoc): Ugly - we only need this for the test suite right now
	iterate_player_numbers(p, map->get_nrplayers()) {
		if (!map->get_scenario_player_tribe(p).empty()) {
			egbase().add_player(p, 0, kPlayerColors[p - 1], map->get_scenario_player_tribe(p),
			                    map->get_scenario_player_name(p));
		}
	}

	ml->load_map_complete(egbase(), Widelands::MapLoader::LoadType::kEditor);
	egbase().create_tempfile_and_save_mapdata(FileSystem::ZIP);
	map_changed(MapWas::kReplaced);
}

void EditorInteractive::cleanup_for_load() {
	if (cleaning_up_) {
		return;
	}
	cleaning_up_ = true;

	// TODO(unknown): get rid of cleanup_for_load, it tends to be very messy
	// Instead, delete and re-create the egbase.
	// TODO(Nordfriese): …and then we can get rid of delete_world_and_tribes() as well
	egbase().cleanup_for_load();
	// This is needed so add-ons are configured correctly if current
	// and previous map had different world add-on settings
	egbase().delete_world_and_tribes();

	cleaning_up_ = false;
}

/// Called just before the editor starts, after postload, init and gfxload.
void EditorInteractive::start() {
	// Run the editor initialization script, if any
	try {
		g_sh->change_music("ingame", 1000);
		egbase().lua().run_script("map:scripting/editor_init.lua");
	} catch (LuaScriptNotExistingError&) {
		// do nothing.
	}
	map_changed(MapWas::kReplaced);
	if (registry_to_open_) {
		registry_to_open_->create();
		registry_to_open_ = nullptr;
	}
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

	egbase().get_gametime_pointer().increment(Duration(realtime_ - lasttime));
}

void EditorInteractive::exit() {
	if (need_save_) {
		if (SDL_GetModState() & KMOD_CTRL) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
		} else {
			UI::WLMessageBox mmb(this, UI::WindowStyle::kWui, _("Unsaved Map"),
			                     _("The map has not been saved, do you really want to quit?"),
			                     UI::WLMessageBox::MBoxType::kOkCancel);
			if (mmb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
				return;
			}
		}
	}
	g_sh->change_music("menu", 200);
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void EditorInteractive::map_clicked(const Widelands::NodeAndTriangle<>& node_and_triangle,
                                    const bool should_draw) {
	history_->do_action(tools_->current(), tools_->use_tool, *egbase().mutable_map(),
	                    node_and_triangle, *this, should_draw);
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
	auto* fields_to_draw = map_view()->draw_terrain(ebase, nullptr, Workareas(), draw_grid_, &dst);

	const float scale = 1.f / map_view()->view().zoom;
	const Time& gametime = ebase.get_gametime();

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
	if (g_mouse_cursor->is_visible()) {
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
	}

	for (size_t idx = 0; idx < fields_to_draw->size(); ++idx) {
		const FieldsToDraw::Field& field = fields_to_draw->at(idx);
		if (draw_immovables_) {
			Widelands::BaseImmovable* const imm = field.fcoords.field->get_immovable();
			if (imm != nullptr && imm->get_positions(ebase).front() == field.fcoords) {
				imm->draw(
				   gametime, InfoToDraw::kNone, field.rendertarget_pixel, field.fcoords, scale, &dst);
			}
		}

		if (draw_bobs_) {
			for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob;
			     bob = bob->get_next_bob()) {
				bob->draw(
				   ebase, InfoToDraw::kNone, field.rendertarget_pixel, field.fcoords, scale, &dst);
			}
		}

		// Draw resource overlay.
		uint8_t const amount = field.fcoords.field->get_resources_amount();
		if (draw_resources_ && amount > 0) {
			const std::string& immname = ebase.descriptions()
			                                .get_resource_descr(field.fcoords.field->get_resources())
			                                ->editor_image(amount);
			if (!immname.empty()) {
				const auto* pic = g_image_cache->get(immname);
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

		if (g_mouse_cursor->is_visible()) {
			// Draw selection markers on the field.
			if (selected_nodes.count(field.fcoords) > 0) {
				const Image* pic = get_sel_picture();
				blit_field_overlay(
				   &dst, field, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale);
			}

			// Draw selection markers on the triangles.
			if (field.all_neighbors_valid()) {
				const FieldsToDraw::Field& rn = fields_to_draw->at(field.rn_index);
				const FieldsToDraw::Field& brn = fields_to_draw->at(field.brn_index);
				const FieldsToDraw::Field& bln = fields_to_draw->at(field.bln_index);
				if (selected_triangles.count(
				       Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::R))) {
					const Vector2i tripos((field.rendertarget_pixel.x + rn.rendertarget_pixel.x +
					                       brn.rendertarget_pixel.x) /
					                         3,
					                      (field.rendertarget_pixel.y + rn.rendertarget_pixel.y +
					                       brn.rendertarget_pixel.y) /
					                         3);
					const Image* pic = get_sel_picture();
					blit_overlay(
					   &dst, tripos, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale, 1.f);
				}
				if (selected_triangles.count(
				       Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::D))) {
					const Vector2i tripos((field.rendertarget_pixel.x + bln.rendertarget_pixel.x +
					                       brn.rendertarget_pixel.x) /
					                         3,
					                      (field.rendertarget_pixel.y + bln.rendertarget_pixel.y +
					                       brn.rendertarget_pixel.y) /
					                         3);
					const Image* pic = get_sel_picture();
					blit_overlay(
					   &dst, tripos, pic, Vector2i(pic->width() / 2, pic->height() / 2), scale, 1.f);
				}
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
	if (UI::UniqueWindow* const w = menu_windows_.toolsize.window) {
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

void EditorInteractive::toggle_resources() {
	draw_resources_ = !draw_resources_;
	rebuild_showhide_menu();
}

void EditorInteractive::toggle_immovables() {
	draw_immovables_ = !draw_immovables_;
	rebuild_showhide_menu();
}

void EditorInteractive::toggle_bobs() {
	draw_bobs_ = !draw_bobs_;
	rebuild_showhide_menu();
}

void EditorInteractive::toggle_grid() {
	draw_grid_ = !draw_grid_;
	rebuild_showhide_menu();
}

bool EditorInteractive::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonEncyclopedia, code)) {
			menu_windows_.help.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorMenu, code)) {
			mainmenu_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorSave, code)) {
			menu_windows_.savemap.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorLoad, code)) {
			menu_windows_.loadmap.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorTools, code)) {
			toolmenu_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorInfo, code)) {
			select_tool(tools_->info, EditorTool::First);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorPlayers, code)) {
			tool_windows_.players.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorShowhideGrid, code)) {
			toggle_grid();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorShowhideCritters, code)) {
			toggle_bobs();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorShowhideImmovables, code)) {
			toggle_immovables();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorShowhideResources, code)) {
			toggle_resources();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorUndo, code)) {
			history_->undo_action();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kEditorRedo, code)) {
			history_->redo_action();
			return true;
		}

		for (int i = 0; i < 10; ++i) {
			if (matches_shortcut(static_cast<KeyboardShortcut>(
			                        static_cast<uint16_t>(KeyboardShortcut::kEditorToolsize1) + i),
			                     code)) {
				set_sel_radius_and_update_menu(i);
				return true;
			}
		}

		switch (code.sym) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (tools_->use_tool == EditorTool::First) {
				select_tool(tools_->current(), EditorTool::Second);
			}
			return true;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
		// TODO(GunChleoc): Keeping ALT and MODE to make the transition easier. Remove for Build 20.
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_MODE:
			if (tools_->use_tool == EditorTool::First) {
				select_tool(tools_->current(), EditorTool::Third);
			}
			return true;

		case SDLK_ESCAPE:
			mainmenu_.toggle();
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
			if (tools_->use_tool != EditorTool::First) {
				select_tool(tools_->current(), EditorTool::First);
			}
			return true;
		default:
			break;
		}
	}

	return InteractiveBase::handle_key(down, code);
}

bool EditorInteractive::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	int32_t change =
	   get_mousewheel_change(MousewheelHandlerConfigID::kEditorToolsize, x, y, modstate);
	if (change == 0) {
		return false;
	}
	set_sel_radius_and_update_menu(
	   std::max(0, std::min(static_cast<int32_t>(get_sel_radius()) + change, MAX_TOOL_AREA)));
	return true;
}

void EditorInteractive::select_tool(EditorTool& primary, EditorTool::ToolIndex const which) {
	if (which == EditorTool::First && &primary != tools_->current_pointer) {
		if (primary.has_size_one()) {
			set_sel_radius(0);
			if (UI::UniqueWindow* const w = menu_windows_.toolsize.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.set_buttons_enabled(false);
			}
		} else {
			if (UI::UniqueWindow* const w = menu_windows_.toolsize.window) {
				EditorToolsizeMenu& toolsize_menu = dynamic_cast<EditorToolsizeMenu&>(*w);
				toolsize_menu.update(toolsize_menu.value());
			}
		}
		egbase().mutable_map()->recalc_whole_map(egbase());
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

void EditorInteractive::run_editor(UI::Panel* error_message_parent,
                                   const EditorInteractive::Init init,
                                   const std::string& filename,
                                   const std::string& script_to_run) {
	try {
		EditorInteractive::do_run_editor(init, filename, script_to_run);
	} catch (const std::exception& e) {
		log_err("##############################\n"
		        "  FATAL EXCEPTION in editor: %s\n"
		        "##############################\n",
		        e.what());
		if (!error_message_parent) {
			return;
		}
		// Note: We don't necessarily want a bug report here, but the wording must
		// be EXACTLY LIKE THIS in v1.0 to avoid adding a new translatable string
		// during winter time freeze. We can consider rephrasing it after v1.0.
		UI::WLMessageBox m(
		   error_message_parent, UI::WindowStyle::kFsMenu, _("Error"),
		   bformat(
		      _("An error has occured. The error message is:\n\n%1$s\n\nPlease report "
		        "this problem to help us improve Widelands. You will find related messages in the "
		        "standard output (stdout.txt on Windows). You are using build %2$s "
		        "(%3$s).\nPlease add this information to your report."),
		      e.what(), build_id(), build_type()),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
	}
}

void EditorInteractive::do_run_editor(const EditorInteractive::Init init,
                                      const std::string& filename,
                                      const std::string& script_to_run) {
	Widelands::EditorGameBase egbase(nullptr);
	EditorInteractive& eia = *new EditorInteractive(egbase);
	egbase.set_ibase(&eia);  // TODO(unknown): get rid of this

	// We need to disable non-world add-ons in the editor
	for (auto it = egbase.enabled_addons().begin(); it != egbase.enabled_addons().end();) {
		if ((*it)->category != AddOns::AddOnCategory::kWorld) {
			it = egbase.enabled_addons().erase(it);
		} else {
			++it;
		}
	}

	egbase.create_loader_ui({"editor"}, true, "", editor_splash_image());
	EditorInteractive::load_world_units(&eia, egbase);

	if (init == EditorInteractive::Init::kLoadMapDirectly) {
		if (filename.empty()) {
			throw wexception("EditorInteractive::run_editor: Empty map file name");
		}

		Notifications::publish(UI::NoteLoadingMessage(bformat(_("Loading map “%s”…"), filename)));
		eia.load(filename);

		egbase.postload_addons();
		egbase.postload();
		eia.start();
		if (!script_to_run.empty()) {
			eia.egbase().lua().run_script(script_to_run);
		}
	} else {
		if (!filename.empty()) {
			throw wexception(
			   "EditorInteractive::run_editor: Map file name given when none was expected");
		}
		if (!script_to_run.empty()) {
			throw wexception("EditorInteractive::run_editor: Script given when none was expected");
		}

		egbase.postload_addons();
		egbase.postload();

		egbase.mutable_map()->create_empty_map(
		   egbase, 64, 64, 0,
		   /** TRANSLATORS: Default name for new map */
		   _("No Name"),
		   get_config_string("realname",
		                     /** TRANSLATORS: Map author name when it hasn't been set yet */
		                     pgettext("author_name", "Unknown")));

		switch (init) {
		case EditorInteractive::Init::kNew:
			eia.registry_to_open_ = &eia.menu_windows_.newmap;
			break;
		case EditorInteractive::Init::kRandom:
			eia.registry_to_open_ = &eia.menu_windows_.newrandommap;
			break;
		case EditorInteractive::Init::kLoad:
			eia.registry_to_open_ = &eia.menu_windows_.loadmap;
			break;
		default:
			break;
		}
	}

	egbase.remove_loader_ui();
	eia.run<UI::Panel::Returncodes>();
	egbase.cleanup_objects();
}

void EditorInteractive::load_world_units(EditorInteractive* eia,
                                         Widelands::EditorGameBase& egbase) {
	Notifications::publish(UI::NoteLoadingMessage(_("Loading world…")));
	Widelands::Descriptions* descriptions = egbase.mutable_descriptions();

	verb_log_info("┏━ Loading world\n");
	ScopedTimer timer("┗━ took %ums", true);

	if (eia) {
		// In order to ensure that items created by add-ons are properly
		// removed from the editor's object selection menus, we clear
		// and repopulate these menus every time the world is reloaded.
		eia->editor_categories_.clear();
	}

	std::unique_ptr<LuaTable> table(egbase.lua().run_script("world/init.lua"));

	auto load_category = [eia, descriptions](const LuaTable& t, const std::string& key,
	                                         Widelands::MapObjectType type) {
		for (const auto& category_table :
		     t.get_table(key)->array_entries<std::unique_ptr<LuaTable>>()) {
			// Even if we do not have an EditorInteractive, we still need to create the
			// Category because it will load all the map objects we are interested in
			std::unique_ptr<EditorCategory> c(
			   new EditorCategory(*category_table, type, *descriptions));
			if (eia) {
				eia->editor_categories_[type].push_back(std::move(c));
			}
		}
	};
	auto load_resources = [](const LuaTable& t) {
		for (const std::string& item : t.get_table("resources")->array_entries<std::string>()) {
			Notifications::publish(Widelands::NoteMapObjectDescription(
			   item, Widelands::NoteMapObjectDescription::LoadType::kObject));
		}
	};

	verb_log_info("┃    Critters");
	load_category(*table, "critters", Widelands::MapObjectType::CRITTER);

	verb_log_info("┃    Immovables");
	load_category(*table, "immovables", Widelands::MapObjectType::IMMOVABLE);

	verb_log_info("┃    Terrains");
	load_category(*table, "terrains", Widelands::MapObjectType::TERRAIN);

	verb_log_info("┃    Resources");
	load_resources(*table);

	for (const auto& info : egbase.enabled_addons()) {
		if (info->category == AddOns::AddOnCategory::kWorld) {
			verb_log_info("┃    Add-On ‘%s’", info->internal_name.c_str());
			table = egbase.lua().run_script(kAddOnDir + '/' + info->internal_name + "/editor.lua");
			load_category(*table, "critters", Widelands::MapObjectType::CRITTER);
			load_category(*table, "immovables", Widelands::MapObjectType::IMMOVABLE);
			load_category(*table, "terrains", Widelands::MapObjectType::TERRAIN);
			load_resources(*table);
		}
	}
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
			if (dynamic_cast<UI::Window*>(child) != nullptr) {
				child->die();
			}
		}

		// Make sure that we will start at coordinates (0,0).
		map_view()->set_view(MapView::View{Vector2f::zero(), 1.f}, MapView::Transition::Jump);
		set_sel_pos(Widelands::NodeAndTriangle<>{
		   Widelands::Coords(0, 0),
		   Widelands::TCoords<>(Widelands::Coords(0, 0), Widelands::TriangleIndex::D)});
		resize_minimap();
		break;

	case MapWas::kGloballyMutated:
		break;

	case MapWas::kResized:
		resize_minimap();
		break;
	}
}

EditorInteractive::Tools* EditorInteractive::tools() {
	return tools_.get();
}

const std::vector<std::unique_ptr<EditorCategory>>&
EditorInteractive::editor_categories(Widelands::MapObjectType type) const {
	assert(editor_categories_.count(type) == 1);
	return editor_categories_.at(type);
}
