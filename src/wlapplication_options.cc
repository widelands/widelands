/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wlapplication_options.h"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

#include "base/i18n.h"
#include "base/log.h"
#include "base/string.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "logic/filesystem_constants.h"

static Profile g_options(Profile::err_log);

static std::string config_file;
static std::unique_ptr<FileSystem> config_dir = nullptr;

void check_config_used() {
	g_options.check_used();
}

Section& get_config_section() {
	return g_options.pull_section("global");
}

Section& get_config_section(const std::string& section) {
	return g_options.pull_section(section.c_str());
}

Section* get_config_section_ptr(const std::string& section) {
	return g_options.get_section(section);
}

bool get_config_bool(const std::string& name, const bool dflt) {
	return g_options.pull_section("global").get_bool(name.c_str(), dflt);
}

bool get_config_bool(const std::string& section, const std::string& name, const bool dflt) {
	return g_options.pull_section(section.c_str()).get_bool(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& name, const int32_t dflt) {
	return g_options.pull_section("global").get_int(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& section, const std::string& name, const int32_t dflt) {
	return g_options.pull_section(section.c_str()).get_int(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& name, const uint32_t dflt) {
	return g_options.pull_section("global").get_natural(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& section, const std::string& name, uint32_t dflt) {
	return g_options.pull_section(section.c_str()).get_natural(name.c_str(), dflt);
}

std::string get_config_string(const std::string& name, const std::string& dflt) {
	return g_options.pull_section("global").get_string(name.c_str(), dflt.c_str());
}

std::string
get_config_string(const std::string& section, const std::string& name, const std::string& dflt) {
	return g_options.pull_section(section.c_str()).get_string(name.c_str(), dflt.c_str());
}

Section& get_config_safe_section() {
	return g_options.get_safe_section("global");
}

Section& get_config_safe_section(const std::string& section) {
	return g_options.get_safe_section(section);
}

void set_config_bool(const std::string& name, const bool value) {
	g_options.pull_section("global").set_bool(name.c_str(), value);
}

void set_config_bool(const std::string& section, const std::string& name, const bool value) {
	g_options.pull_section(section.c_str()).set_bool(name.c_str(), value);
}

void set_config_int(const std::string& name, int32_t value) {
	g_options.pull_section("global").set_int(name.c_str(), value);
}

void set_config_int(const std::string& section, const std::string& name, const int32_t value) {
	g_options.pull_section(section.c_str()).set_int(name.c_str(), value);
}

void set_config_string(const std::string& name, const std::string& value) {
	g_options.pull_section("global").set_string(name.c_str(), value.c_str());
}

void set_config_string(const std::string& section,
                       const std::string& name,
                       const std::string& value) {
	g_options.pull_section(section.c_str()).set_string(name.c_str(), value.c_str());
}

constexpr char kFastplaceNameSeparator = '$';

struct KeyboardShortcutInfo {
	enum class Scope {
		kGlobal,  // special value that intersects with all other scopes

		kMainMenu,
		kEditor,
		kGame,
	};

	const std::set<Scope> scopes;
	const SDL_Keysym default_shortcut;
	SDL_Keysym current_shortcut;
	const std::string internal_name;
	const std::function<std::string()> descname;
	std::map<std::string /* tribe */, std::string /* building */> fastplace;

	KeyboardShortcutInfo(const std::set<Scope>& s,
	                     const SDL_Keysym& sym,
	                     const std::string& n,
	                     const std::function<std::string()>& f)
	   : scopes(s), default_shortcut(sym), current_shortcut(sym), internal_name(n), descname(f) {
	}
};

// Use Cmd instead of Ctrl for default shortcuts on MacOS
#ifdef __APPLE__
constexpr uint16_t kDefaultCtrlModifier = KMOD_GUI;
#else
constexpr uint16_t kDefaultCtrlModifier = KMOD_CTRL;
#endif

static inline SDL_Keysym keysym(const SDL_Keycode c, uint16_t mod = 0) {
	return SDL_Keysym{SDL_GetScancodeFromKey(c), c, mod, 0};
}

static const std::vector<KeyboardShortcutInfo> kFastplaceDefaults = {
#define FP(name, descname)                                                                         \
	KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame}, keysym(SDLK_UNKNOWN),                \
	                     kFastplaceGroupPrefix + name, []() { return descname; })
   FP("warehouse", _("Warehouse")),
   FP("port", _("Port")),
   FP("training_small", _("Minor Training Site")),
   FP("training_large", _("Major Training Site")),
   FP("military_small_primary", _("Primary Small Military Site")),
   FP("military_small_secondary", _("Secondary Small Military Site")),
   FP("military_medium_primary", _("Primary Medium Military Site")),
   FP("military_medium_secondary", _("Secondary Medium Military Site")),
   FP("military_tower", _("Tower")),
   FP("military_fortress", _("Fortress")),
   FP("woodcutter", _("Woodcutter’s House")),
   FP("forester", _("Forester’s House")),
   FP("quarry", _("Quarry")),
   FP("building_materials_primary", _("Primary Building Materials Industry")),
   FP("building_materials_secondary", _("Secondary Building Materials Industry")),
   FP("building_materials_tertiary", _("Tertiary Building Materials Industry")),
   FP("fisher", _("Fishing")),
   FP("hunter", _("Hunting")),
   FP("fish_meat_replenisher", _("Fish/Meat Replenishing")),
   FP("well", _("Well")),
   FP("farm_primary", _("Primary Farm")),
   FP("farm_secondary", _("Secondary Farm")),
   FP("mill", _("Mill")),
   FP("bakery", _("Bakery")),
   FP("brewery", _("Brewery")),
   FP("smokery", _("Smokery")),
   FP("tavern", _("Tavern")),
   FP("smelting", _("Ore Smelting")),
   FP("tool_smithy", _("Tool Smithy")),
   FP("weapon_smithy", _("Weapon Smithy")),
   FP("armor_smithy", _("Armor Smithy")),
   FP("weaving_mill", _("Weaving Mill")),
   FP("shipyard", _("Shipyard")),
   FP("ferry_yard", _("Ferry Yard")),
   FP("scout", _("Scouting")),
   FP("barracks", _("Barracks")),
   FP("second_carrier", _("Second Carrier")),
   FP("charcoal", _("Charcoal")),
   FP("mine_stone", _("Stone Mine")),
   FP("mine_coal", _("Coal Mine")),
   FP("mine_iron", _("Iron Mine")),
   FP("mine_gold", _("Gold Mine")),
   FP("agriculture_producer", _("Agricultural Producer")),
   FP("agriculture_consumer_primary", _("Primary Agricultural Consumer")),
   FP("agriculture_consumer_secondary", _("Secondary Agricultural Consumer")),
   FP("industry_alternative", _("Alternative Industry")),
   FP("industry_supporter", _("Industry Support")),
   FP("terraforming", _("Terraforming")),
#undef FP
};

static std::map<KeyboardShortcut, KeyboardShortcutInfo> shortcuts_ = {
   {KeyboardShortcut::kMainMenuNew, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                         keysym(SDLK_n),
                                                         "mainmenu_new",
                                                         []() { return _("New Game"); })},
   {KeyboardShortcut::kMainMenuLoad, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                          keysym(SDLK_l),
                                                          "mainmenu_load",
                                                          []() { return _("Load Game"); })},
   {KeyboardShortcut::kMainMenuReplay,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_r),
                         "mainmenu_replay",
                         []() { return _("Watch Replay"); })},
   {KeyboardShortcut::kMainMenuRandomMatch,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_z),
                         "mainmenu_random",
                         []() { return _("New Random Game"); })},
   {KeyboardShortcut::kMainMenuTutorial,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_t),
                         "mainmenu_tutorial",
                         []() { return _("Tutorials"); })},
   {KeyboardShortcut::kMainMenuCampaign,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_h),
                         "mainmenu_campaign",
                         []() { return _("Campaigns"); })},
   {KeyboardShortcut::kMainMenuSP, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                        keysym(SDLK_s),
                                                        "mainmenu_sp",
                                                        []() { return _("Singleplayer"); })},
   {KeyboardShortcut::kMainMenuMP, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                        keysym(SDLK_m),
                                                        "mainmenu_mp",
                                                        []() { return _("Multiplayer"); })},
   {KeyboardShortcut::kMainMenuE, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                       keysym(SDLK_e),
                                                       "mainmenu_e",
                                                       []() { return _("Editor"); })},
   {KeyboardShortcut::kMainMenuEditorLoad,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_b),
                         "mainmenu_editor_load",
                         []() { return _("Editor – Load Map"); })},
   {KeyboardShortcut::kMainMenuEditorNew,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_k),
                         "mainmenu_editor_new",
                         []() { return _("Editor – New Map"); })},
   {KeyboardShortcut::kMainMenuEditorRandom,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_y),
                         "mainmenu_editor_random",
                         []() { return _("Editor – New Random Map"); })},
   {KeyboardShortcut::kMainMenuContinueEditing,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_w),
                         "mainmenu_editor_continue",
                         []() { return _("Continue Editing"); })},
   {KeyboardShortcut::kMainMenuContinuePlaying,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_c),
                         "mainmenu_continue",
                         []() { return _("Continue Playing"); })},
   {KeyboardShortcut::kMainMenuQuit, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                          keysym(SDLK_ESCAPE),
                                                          "mainmenu_quit",
                                                          []() { return _("Exit Widelands"); })},
   {KeyboardShortcut::kMainMenuAbout, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_F1),
                                                           "mainmenu_about",
                                                           []() { return _("About"); })},
   {KeyboardShortcut::kMainMenuAddons,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_a),
                         "mainmenu_addons",
                         []() { return _("Add-Ons"); })},
   {KeyboardShortcut::kMainMenuLAN, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                         keysym(SDLK_p),
                                                         "mainmenu_lan",
                                                         []() { return _("LAN / Direct IP"); })},
   {KeyboardShortcut::kMainMenuLobby, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_j),
                                                           "mainmenu_lobby",
                                                           []() { return _("Metaserver Lobby"); })},
   {KeyboardShortcut::kMainMenuLogin, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                                                           keysym(SDLK_u),
                                                           "mainmenu_login",
                                                           []() { return _("Internet Login"); })},
   {KeyboardShortcut::kMainMenuOptions,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kMainMenu},
                         keysym(SDLK_o),
                         "mainmenu_options",
                         []() { return _("Options"); })},

   {KeyboardShortcut::kCommonBuildhelp,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_SPACE),
                         "buildhelp",
                         []() { return _("Toggle Building Spaces"); })},
   {KeyboardShortcut::kCommonMinimap,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_m),
                         "minimap",
                         []() { return _("Toggle Minimap"); })},
   {KeyboardShortcut::kCommonEncyclopedia,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_F1),
                         "encyclopedia",
                         []() { return _("Encyclopedia"); })},
   {KeyboardShortcut::kCommonTextCut, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                                                           keysym(SDLK_x, kDefaultCtrlModifier),
                                                           "cut",
                                                           []() { return _("Cut Text"); })},
   {KeyboardShortcut::kCommonTextCopy, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                                                            keysym(SDLK_c, kDefaultCtrlModifier),
                                                            "copy",
                                                            []() { return _("Copy Text"); })},
   {KeyboardShortcut::kCommonTextPaste, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                                                             keysym(SDLK_v, kDefaultCtrlModifier),
                                                             "paste",
                                                             []() { return _("Paste Text"); })},
   {KeyboardShortcut::kCommonSelectAll, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                                                             keysym(SDLK_a, kDefaultCtrlModifier),
                                                             "selectall",
                                                             []() { return _("Select All"); })},
   {KeyboardShortcut::kCommonDeleteItem,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_DELETE),
                         "delete",
                         []() { return _("Delete Item"); })},
   {KeyboardShortcut::kCommonTooltipAccessibilityMode,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_F2),
                         "tt_access_mode",
                         []() { return _("Tooltip Accessibility Mode Key"); })},
   {KeyboardShortcut::kCommonFullscreen,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_f, kDefaultCtrlModifier),
                         "fullscreen",
                         []() { return _("Toggle Fullscreen"); })},
   {KeyboardShortcut::kCommonScreenshot,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGlobal},
                         keysym(SDLK_F11),
                         "screenshot",
                         []() { return _("Take Screenshot"); })},
   {KeyboardShortcut::kCommonZoomIn,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_PLUS, kDefaultCtrlModifier),
                         "zoom_in",
                         []() { return _("Zoom In"); })},
   {KeyboardShortcut::kCommonZoomOut,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_MINUS, kDefaultCtrlModifier),
                         "zoom_out",
                         []() { return _("Zoom Out"); })},
   {KeyboardShortcut::kCommonZoomReset,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_0, kDefaultCtrlModifier),
                         "zoom_reset",
                         []() { return _("Reset Zoom"); })},
   {KeyboardShortcut::kCommonQuicknavNext,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_PERIOD),
                         "quicknav_next",
                         []() { return _("Jump to Next Location"); })},
   {KeyboardShortcut::kCommonQuicknavPrev,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame, KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_COMMA),
                         "quicknav_prev",
                         []() { return _("Jump to Previous Location"); })},

   {KeyboardShortcut::kEditorMenu, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_h),
                                                        "editor_menu",
                                                        []() { return _("Menu"); })},
   {KeyboardShortcut::kEditorSave, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_s, kDefaultCtrlModifier),
                                                        "editor_save",
                                                        []() { return _("Save Map"); })},
   {KeyboardShortcut::kEditorLoad, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_l, kDefaultCtrlModifier),
                                                        "editor_load",
                                                        []() { return _("Load Map"); })},
   {KeyboardShortcut::kEditorUndo, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_z, kDefaultCtrlModifier),
                                                        "editor_undo",
                                                        []() { return _("Undo"); })},
   {KeyboardShortcut::kEditorRedo, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_y, kDefaultCtrlModifier),
                                                        "editor_redo",
                                                        []() { return _("Redo"); })},
   {KeyboardShortcut::kEditorTools, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                         keysym(SDLK_t),
                                                         "editor_tools",
                                                         []() { return _("Tools"); })},
   {KeyboardShortcut::kEditorInfo, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                        keysym(SDLK_i),
                                                        "editor_info",
                                                        []() { return _("Info Tool"); })},
   {KeyboardShortcut::kEditorPlayers, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                                                           keysym(SDLK_p),
                                                           "editor_players",
                                                           []() { return _("Players Menu"); })},
   {KeyboardShortcut::kEditorToolHistory,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_h, KMOD_SHIFT),
                         "editor_tool_history",
                         []() { return _("Tool History"); })},
   {KeyboardShortcut::kEditorShowhideGrid,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_g),
                         "editor_showhide_grid",
                         []() { return _("Toggle Grid"); })},
   {KeyboardShortcut::kEditorShowhideImmovables,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_o),
                         "editor_showhide_immovables",
                         []() { return _("Toggle Immovables"); })},
   {KeyboardShortcut::kEditorShowhideCritters,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_a),
                         "editor_showhide_critters",
                         []() { return _("Toggle Animals"); })},
   {KeyboardShortcut::kEditorShowhideResources,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_r),
                         "editor_showhide_resources",
                         []() { return _("Toggle Resources"); })},

   {KeyboardShortcut::kEditorShowhideMaximumBuildhelp,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor},
                         keysym(SDLK_SPACE, KMOD_SHIFT),
                         "editor_showhide_maximum_buildhelp",
                         []() { return _("Toggle Maximum Build Spaces"); })},

#define EDITOR_TOOLSIZE(radius, key)                                                               \
	{                                                                                               \
		KeyboardShortcut::kEditorToolsize##radius,                                                   \
		   KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kEditor}, keysym(SDLK_##key),          \
		                        "editor_toolsize" #radius,                                           \
		                        []() { return format(_("Set Toolsize to %d"), radius); })            \
	}
   EDITOR_TOOLSIZE(1, 1),
   EDITOR_TOOLSIZE(2, 2),
   EDITOR_TOOLSIZE(3, 3),
   EDITOR_TOOLSIZE(4, 4),
   EDITOR_TOOLSIZE(5, 5),
   EDITOR_TOOLSIZE(6, 6),
   EDITOR_TOOLSIZE(7, 7),
   EDITOR_TOOLSIZE(8, 8),
   EDITOR_TOOLSIZE(9, 9),
   EDITOR_TOOLSIZE(10, 0),
#undef EDITOR_TOOLSIZE

   {KeyboardShortcut::kInGameShowhideCensus,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_c),
                         "game_showhide_census",
                         []() { return _("Toggle Census"); })},
   {KeyboardShortcut::kInGameShowhideStats,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_s),
                         "game_showhide_stats",
                         []() { return _("Toggle Status Labels"); })},
   {KeyboardShortcut::kInGameShowhideSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_l),
                         "game_showhide_soldiers",
                         []() { return _("Toggle Soldier Levels"); })},
   {KeyboardShortcut::kInGameShowhideBuildings,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_u),
                         "game_showhide_buildings",
                         []() { return _("Toggle Buildings Visibility"); })},
   {KeyboardShortcut::kInGameShowhideWorkareas,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_w),
                         "game_showhide_workareas",
                         []() { return _("Toggle Overlapping Workareas"); })},
   {KeyboardShortcut::kInGameStatsGeneral,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_a),
                         "game_stats_general",
                         []() { return _("General Statistics"); })},
   {KeyboardShortcut::kInGameStatsWares,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_p),
                         "game_stats_wares",
                         []() { return _("Ware Statistics"); })},
   {KeyboardShortcut::kInGameStatsBuildings,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_b),
                         "game_stats_buildings",
                         []() { return _("Building Statistics"); })},
   {KeyboardShortcut::kInGameStatsStock,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_i),
                         "game_stats_stock",
                         []() { return _("Stock Inventory"); })},
   {KeyboardShortcut::kInGameStatsSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_x),
                         "game_stats_soldiers",
                         []() { return _("Soldier Statistics"); })},
   {KeyboardShortcut::kInGameStatsSeafaring,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_e),
                         "game_stats_seafaring",
                         []() { return _("Seafaring Statistics"); })},
   {KeyboardShortcut::kInGameObjectives, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                              keysym(SDLK_t),
                                                              "game_objectives",
                                                              []() { return _("Objectives"); })},
   {KeyboardShortcut::kInGameDiplomacy,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_d),
                         "game_diplomacy",
                         []() { return _("Diplomacy"); })},
   {KeyboardShortcut::kInGameMessages, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                            keysym(SDLK_n),
                                                            "game_messages",
                                                            []() { return _("Messages"); })},
   {KeyboardShortcut::kInGameSpeedDown,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN),
                         "game_speed_down",
                         []() { return _("Decrease Game Speed by 1×"); })},
   {KeyboardShortcut::kInGameSpeedDownSlow,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN, KMOD_SHIFT),
                         "game_speed_down_slow",
                         []() { return _("Decrease Game Speed by 0.25×"); })},
   {KeyboardShortcut::kInGameSpeedDownFast,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEDOWN, kDefaultCtrlModifier),
                         "game_speed_down_fast",
                         []() { return _("Decrease Game Speed by 10×"); })},
   {KeyboardShortcut::kInGameSpeedUp,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP),
                         "game_speed_up",
                         []() { return _("Increase Game Speed by 1×"); })},
   {KeyboardShortcut::kInGameSpeedUpSlow,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP, KMOD_SHIFT),
                         "game_speed_up_slow",
                         []() { return _("Increase Game Speed by 0.25×"); })},
   {KeyboardShortcut::kInGameSpeedUpFast,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAGEUP, kDefaultCtrlModifier),
                         "game_speed_up_fast",
                         []() { return _("Increase Game Speed by 10×"); })},
   {KeyboardShortcut::kInGameSpeedReset,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_PAUSE, KMOD_SHIFT),
                         "game_speed_reset",
                         []() { return _("Reset Game Speed"); })},
   {KeyboardShortcut::kInGamePause, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                         keysym(SDLK_PAUSE),
                                                         "game_pause",
                                                         []() { return _("Pause"); })},
   {KeyboardShortcut::kInGameScrollToHQ,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_HOME),
                         "game_hq",
                         []() { return _("Scroll to Starting Field"); })},
   {KeyboardShortcut::kInGameChat, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                        keysym(SDLK_RETURN),
                                                        "game_chat",
                                                        []() { return _("Chat"); })},
   {KeyboardShortcut::kInGameSave, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                        keysym(SDLK_s, kDefaultCtrlModifier),
                                                        "game_save",
                                                        []() { return _("Save Game"); })},
   {KeyboardShortcut::kInGameLoad, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                                                        keysym(SDLK_l, kDefaultCtrlModifier),
                                                        "game_load",
                                                        []() { return _("Load Game"); })},
   {KeyboardShortcut::kInGameMessagesGoto,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_g),
                         "game_msg_goto",
                         []() { return _("Messages: Go to Location"); })},
   {KeyboardShortcut::kInGameMessagesFilterAll,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_0, KMOD_ALT),
                         "game_msg_filter_all",
                         []() { return _("Messages: Show All"); })},
   {KeyboardShortcut::kInGameMessagesFilterGeologists,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_1, KMOD_ALT),
                         "game_msg_filter_geo",
                         []() { return _("Messages: Show Geologists’ Messages"); })},
   {KeyboardShortcut::kInGameMessagesFilterEconomy,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_2, KMOD_ALT),
                         "game_msg_filter_eco",
                         []() { return _("Messages: Show Economy Messages"); })},
   {KeyboardShortcut::kInGameMessagesFilterSeafaring,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_3, KMOD_ALT),
                         "game_msg_filter_seafaring",
                         []() { return _("Messages: Show Seafaring Messages"); })},
   {KeyboardShortcut::kInGameMessagesFilterWarfare,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_4, KMOD_ALT),
                         "game_msg_filter_warfare",
                         []() { return _("Messages: Show Military Messages"); })},
   {KeyboardShortcut::kInGameMessagesFilterScenario,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_5, KMOD_ALT),
                         "game_msg_filter_scenario",
                         []() { return _("Messages: Show Scenario Messages"); })},
   {KeyboardShortcut::kInGameSeafaringstatsGotoShip,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_j),
                         "game_sfstats_goto",
                         []() { return _("Seafaring: Go to Ship"); })},
   {KeyboardShortcut::kInGameSeafaringstatsWatchShip,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_f),
                         "game_sfstats_watch",
                         []() { return _("Seafaring: Watch Ship"); })},
   {KeyboardShortcut::kInGameSeafaringstatsOpenShipWindow,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_o),
                         "game_sfstats_open",
                         []() { return _("Seafaring: Open Ship Window"); })},
   {KeyboardShortcut::kInGameSeafaringstatsOpenShipWindowAndGoto,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_o, kDefaultCtrlModifier),
                         "game_sfstats_open_goto",
                         []() { return _("Seafaring: Open Ship Window And Go to Ship"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterAll,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_0, KMOD_SHIFT),
                         "game_sfstats_filter_all",
                         []() { return _("Seafaring: Show All Ships"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterIdle,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_1, KMOD_SHIFT),
                         "game_sfstats_filter_idle",
                         []() { return _("Seafaring: Show Idle Ships"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterShipping,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_2, KMOD_SHIFT),
                         "game_sfstats_filter_ship",
                         []() { return _("Seafaring: Show Transport Ships"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpWait,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_3, KMOD_SHIFT),
                         "game_sfstats_filter_wait",
                         []() { return _("Seafaring: Show Waiting Expeditions"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpScout,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_4, KMOD_SHIFT),
                         "game_sfstats_filter_scout",
                         []() { return _("Seafaring: Show Scouting Expeditions"); })},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpPortspace,
    KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},
                         keysym(SDLK_5, KMOD_SHIFT),
                         "game_sfstats_filter_port",
                         []() { return _("Seafaring: Show Expeditions with Port Spaces"); })},
#define QUICKNAV(i)                                                                                \
	{KeyboardShortcut::kInGameQuicknavSet##i,                                                       \
	 KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame},                                     \
	                      keysym(SDLK_##i, kDefaultCtrlModifier), "game_quicknav_set_" #i,          \
	                      []() { return format(_("Set Landmark #%d"), i); })},                      \
	{                                                                                               \
		KeyboardShortcut::kInGameQuicknavGoto##i,                                                    \
		   KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame}, keysym(SDLK_##i),              \
		                        "game_quicknav_goto_" #i,                                            \
		                        []() { return format(_("Go To Landmark #%d"), i); })                 \
	}
   QUICKNAV(1),
   QUICKNAV(2),
   QUICKNAV(3),
   QUICKNAV(4),
   QUICKNAV(5),
   QUICKNAV(6),
   QUICKNAV(7),
   QUICKNAV(8),
   QUICKNAV(9),
#undef QUICKNAV
};

void unset_shortcut(const KeyboardShortcut id) {
	set_shortcut(id, keysym(SDLK_UNKNOWN), nullptr);
}

void set_fastplace_shortcuts(KeyboardShortcut id, const std::map<std::string, std::string>& map) {
	assert(is_fastplace(id));
	KeyboardShortcutInfo& info = shortcuts_.at(id);

	auto config_key = [&info](const std::string& tribe) {
		std::string key = info.internal_name;
		key += kFastplaceNameSeparator;
		key += tribe;
		return key;
	};

	// Update or remove existing mapping
	for (auto& pair : info.fastplace) {
		const auto it = map.find(pair.first);
		if (it == map.end()) {
			pair.second.clear();
		} else {
			pair.second = it->second;
		}
		set_config_string("keyboard_fastplace", config_key(pair.first), pair.second);
	}

	// Add new mapping
	for (const auto& pair : map) {
		if (info.fastplace.count(pair.first) == 0u) {
			info.fastplace.emplace(pair);
			set_config_string("keyboard_fastplace", config_key(pair.first), pair.second);
		}
	}
}

const std::map<std::string, std::string>& get_fastplace_shortcuts(const KeyboardShortcut id) {
	assert(is_fastplace(id));
	return shortcuts_.at(id).fastplace;
}

const std::string& get_fastplace_group_name(const KeyboardShortcut id) {
	assert(is_fastplace(id));
	return shortcuts_.at(id).internal_name;
}

std::string to_string(const KeyboardShortcut id) {
	return shortcuts_.at(id).descname();
}

SDL_Keysym get_default_shortcut(const KeyboardShortcut id) {
	return shortcuts_.at(id).default_shortcut;
}

static void write_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	set_config_int("keyboard_sym", shortcuts_.at(id).internal_name, code.sym);
	set_config_int("keyboard_mod", shortcuts_.at(id).internal_name, code.mod);
}

static bool shared_scope(const std::set<KeyboardShortcutInfo::Scope>& scopes,
                         const KeyboardShortcutInfo& k) {
	if ((scopes.count(KeyboardShortcutInfo::Scope::kGlobal) != 0u) ||
	    (k.scopes.count(KeyboardShortcutInfo::Scope::kGlobal) != 0u)) {
		return true;
	}

	for (KeyboardShortcutInfo::Scope s : scopes) {
		if (k.scopes.count(s) != 0u) {
			return true;
		}
	}
	return false;
}

bool set_shortcut(const KeyboardShortcut id, const SDL_Keysym code, KeyboardShortcut* conflict) {
	const std::set<KeyboardShortcutInfo::Scope>& scopes = shortcuts_.at(id).scopes;

	for (auto& pair : shortcuts_) {
		if (pair.first != id && shared_scope(scopes, pair.second) &&
		    matches_shortcut(pair.first, code)) {
			if (conflict != nullptr) {
				*conflict = pair.first;
			}
			return false;
		}
	}

	shortcuts_.at(id).current_shortcut = code;
	write_shortcut(id, code);
	return true;
}

SDL_Keysym get_shortcut(const KeyboardShortcut id) {
	return shortcuts_.at(id).current_shortcut;
}

static const std::map<SDL_Keycode, SDL_Keycode> kNumpadIdentifications = {
   {SDLK_KP_9, SDLK_PAGEUP},         {SDLK_KP_8, SDLK_UP},          {SDLK_KP_7, SDLK_HOME},
   {SDLK_KP_6, SDLK_RIGHT},          {SDLK_KP_5, SDLK_UNKNOWN},     {SDLK_KP_4, SDLK_LEFT},
   {SDLK_KP_3, SDLK_PAGEDOWN},       {SDLK_KP_2, SDLK_DOWN},        {SDLK_KP_1, SDLK_END},
   {SDLK_KP_0, SDLK_INSERT},         {SDLK_KP_PERIOD, SDLK_DELETE}, {SDLK_KP_ENTER, SDLK_RETURN},
   {SDLK_KP_MINUS, SDLK_MINUS},      {SDLK_KP_PLUS, SDLK_PLUS},     {SDLK_KP_DIVIDE, SDLK_SLASH},
   {SDLK_KP_MULTIPLY, SDLK_ASTERISK}};

void normalize_numpad(SDL_Keysym& keysym) {
	auto search = kNumpadIdentifications.find(keysym.sym);
	if (search == kNumpadIdentifications.end()) {
		return;
	}
	if ((keysym.mod & KMOD_NUM) != 0) {
		if (keysym.sym >= SDLK_KP_1 && keysym.sym <= SDLK_KP_9) {
			keysym.sym = keysym.sym - SDLK_KP_1 + SDLK_1;
			return;
		}
		if (keysym.sym == SDLK_KP_0) {
			keysym.sym = SDLK_0;
			return;
		}
		if (keysym.sym == SDLK_KP_PERIOD) {
			keysym.sym = SDLK_PERIOD;
			return;
		}
	}  // Not else, because '/', '*', '-' and '+' are not affected by NumLock state

	if (get_config_bool("numpad_diagonalscrolling", false)) {
		// If this option is enabled, reserve numpad movement keys for map scrolling
		// Numpad 5 becomes go to HQ
		if (keysym.sym >= SDLK_KP_1 && keysym.sym <= SDLK_KP_9) {
			return;
		}
	}  // Not else, because there are 7 more keys which are not affected

	keysym.sym = search->second;
}

uint16_t normalize_keymod(uint16_t keymod) {
	return ((keymod & KMOD_SHIFT) != 0 ? KMOD_SHIFT : KMOD_NONE) |
	       ((keymod & KMOD_CTRL) != 0 ? KMOD_CTRL : KMOD_NONE) |
	       ((keymod & KMOD_ALT) != 0 ? KMOD_ALT : KMOD_NONE) |
	       ((keymod & KMOD_GUI) != 0 ? KMOD_GUI : KMOD_NONE);
}

bool matches_keymod(const uint16_t mod1, const uint16_t mod2) {
	return normalize_keymod(mod1) == normalize_keymod(mod2);
}

bool matches_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	return matches_shortcut(id, code.sym, code.mod);
}
bool matches_shortcut(const KeyboardShortcut id, const SDL_Keycode code, const int mod) {
	const SDL_Keysym key = get_shortcut(id);
	if (key.sym == SDLK_UNKNOWN || code == SDLK_UNKNOWN) {
		return false;
	}

	if (!matches_keymod(key.mod, mod)) {
		return false;
	}

	if (key.sym == code) {
		return true;
	}

	// Some extra checks so we can identify keypad keys with their "normal" equivalents,
	// e.g. pressing '+' or numpad_'+' should always have the same effect

	// This is now only required for config file backward compatibility, as all keyboard
	// events get converted to "normal" keys (except for the numpad keys used for diagonal
	// scrolling if it is enabled)

	if ((mod & KMOD_NUM) != 0) {
		// If numlock is on and a number was pressed, only compare the entered number value.
		// Annoyingly, there seems to be no strict rule whether the SDLK_ constants are
		// ranged 0,1,…,9 or 1,…,9,0 so we have to treat 0 as a special case.
		if (code == SDLK_KP_0) {
			return key.sym == SDLK_0;
		}
		if (code == SDLK_0) {
			return key.sym == SDLK_KP_0;
		}
		if (code >= SDLK_1 && code <= SDLK_9) {
			return key.sym == code + SDLK_KP_1 - SDLK_1;
		}
		if (code >= SDLK_KP_1 && code <= SDLK_KP_9) {
			return key.sym == code + SDLK_1 - SDLK_KP_1;
		}
	}

	if (get_config_bool("numpad_diagonalscrolling", false) &&
	    (code >= SDLK_KP_1 && code <= SDLK_KP_9)) {
		// Reserve numpad movement keys for map scrolling
		return false;
	}

	for (const auto& pair : kNumpadIdentifications) {
		if ((code == pair.first && key.sym == pair.second) ||
		    (code == pair.second && key.sym == pair.first)) {
			return true;
		}
	}

	return false;
}

std::string matching_fastplace_shortcut(const SDL_Keysym key, const std::string& tribename) {
	for (int i = static_cast<int>(KeyboardShortcut::kFastplace_Begin);
	     i < static_cast<int>(KeyboardShortcut::kFastplace_End); ++i) {
		const KeyboardShortcut id = static_cast<KeyboardShortcut>(i);
		if (matches_shortcut(id, key)) {
			const KeyboardShortcutInfo& info = shortcuts_.at(id);
			const auto it = info.fastplace.find(tribename);
			return it == info.fastplace.end() ? "" : it->second;
		}
	}
	return "";
}

KeyboardShortcut shortcut_from_string(const std::string& name) {
	for (const auto& pair : shortcuts_) {
		if (pair.second.internal_name == name) {
			return pair.first;
		}
	}
	throw wexception("Shortcut '%s' does not exist", name.c_str());
}

std::string keymod_string_for(const uint16_t modstate, const bool rt_escape) {
	i18n::Textdomain textdomain("widelands");
	std::vector<std::string> mods;
	if ((modstate & KMOD_SHIFT) != 0) {
		mods.emplace_back(pgettext("hotkey", "Shift"));
	}
	if ((modstate & KMOD_ALT) != 0) {
		mods.emplace_back(pgettext("hotkey", "Alt"));
	}
	if ((modstate & KMOD_GUI) != 0) {
#ifdef __APPLE__
		mods.push_back(pgettext("hotkey", "Cmd"));
#else
		mods.emplace_back(pgettext("hotkey", "GUI"));
#endif
	}
	if ((modstate & KMOD_CTRL) != 0) {
		mods.emplace_back(pgettext("hotkey", "Ctrl"));
	}

	std::string result;

	// Return value will have a trailing "+" if any modifier is matched,
	// because all current uses need it anyway, and extra checks can
	// be avoided both here and in the users this way
	for (const std::string& m : mods) {
		result = format(_("%1$s+%2$s"), m, result);
	}

	return rt_escape ? richtext_escape(result) : result;
}

std::string shortcut_string_for(const KeyboardShortcut id, const bool rt_escape) {
	return shortcut_string_for(get_shortcut(id), rt_escape);
}

static std::string key_name(const SDL_Keycode k) {
	switch (k) {
	case SDLK_SPACE:
		return pgettext("hotkey", "Space");
	case SDLK_RETURN:
		return pgettext("hotkey", "Enter");
	case SDLK_ESCAPE:
		return pgettext("hotkey", "Escape");
	case SDLK_TAB:
		return pgettext("hotkey", "Tab");
	case SDLK_MENU:
		return pgettext("hotkey", "Menu");
	case SDLK_PAUSE:
		return pgettext("hotkey", "Pause");
	case SDLK_PAGEUP:
		return pgettext("hotkey", "Page Up");
	case SDLK_PAGEDOWN:
		return pgettext("hotkey", "Page Down");
	case SDLK_HOME:
		return pgettext("hotkey", "Home");
	case SDLK_END:
		return pgettext("hotkey", "End");
	case SDLK_LEFT:
		return pgettext("hotkey", "Left Arrow Key");
	case SDLK_RIGHT:
		return pgettext("hotkey", "Right Arrow Key");
	case SDLK_UP:
		return pgettext("hotkey", "Up Arrow Key");
	case SDLK_DOWN:
		return pgettext("hotkey", "Down Arrow Key");
	case SDLK_INSERT:
		return pgettext("hotkey", "Insert");
	case SDLK_DELETE:
		return pgettext("hotkey", "Delete");
	case SDLK_BACKSPACE:
		return pgettext("hotkey", "Backspace");
	case SDLK_CAPSLOCK:
		return pgettext("hotkey", "Caps Lock");
	case SDLK_NUMLOCKCLEAR:
		return pgettext("hotkey", "Numpad Lock");
	case SDLK_SCROLLLOCK:
		return pgettext("hotkey", "Scroll Lock");
	case SDLK_KP_1:
		return pgettext("hotkey", "Keypad 1");
	case SDLK_KP_2:
		return pgettext("hotkey", "Keypad 2");
	case SDLK_KP_3:
		return pgettext("hotkey", "Keypad 3");
	case SDLK_KP_4:
		return pgettext("hotkey", "Keypad 4");
	case SDLK_KP_5:
		return pgettext("hotkey", "Keypad 5");
	case SDLK_KP_6:
		return pgettext("hotkey", "Keypad 6");
	case SDLK_KP_7:
		return pgettext("hotkey", "Keypad 7");
	case SDLK_KP_8:
		return pgettext("hotkey", "Keypad 8");
	case SDLK_KP_9:
		return pgettext("hotkey", "Keypad 9");
	case SDLK_KP_0:
		return pgettext("hotkey", "Keypad 0");
	case SDLK_KP_PERIOD:
		return pgettext("hotkey", "Keypad .");
	case SDLK_KP_PLUS:
		return pgettext("hotkey", "Keypad +");
	case SDLK_KP_MINUS:
		return pgettext("hotkey", "Keypad -");
	case SDLK_KP_MULTIPLY:
		return pgettext("hotkey", "Keypad *");
	case SDLK_KP_DIVIDE:
		return pgettext("hotkey", "Keypad /");
	case SDLK_KP_ENTER:
		return pgettext("hotkey", "Keypad Enter");
	default:
		return SDL_GetKeyName(k);
	}
}

std::string shortcut_string_for(const SDL_Keysym sym, const bool rt_escape) {
	i18n::Textdomain textdomain("widelands");
	if (sym.sym == SDLK_UNKNOWN) {
		return _("(disabled)");
	}

	std::string result = format(_("%1$s%2$s"), keymod_string_for(sym.mod, false), key_name(sym.sym));

	return rt_escape ? richtext_escape(result) : result;
}

static void init_fastplace_shortcuts(const bool force_defaults) {
	int counter = 0;
	for (KeyboardShortcut k = KeyboardShortcut::kFastplace_Begin;
	     k <= KeyboardShortcut::kFastplace_End;
	     k = static_cast<KeyboardShortcut>(static_cast<uint16_t>(k) + 1)) {
		if (force_defaults) {
			shortcuts_.erase(k);
		} else if (shortcuts_.count(k) != 0u) {
			continue;
		}

		const unsigned off =
		   static_cast<uint16_t>(k) - static_cast<uint16_t>(KeyboardShortcut::kFastplace_Begin);
		if (off < kFastplaceDefaults.size()) {
			shortcuts_.emplace(k, kFastplaceDefaults[off]);
		} else {
			++counter;
			shortcuts_.emplace(
			   k, KeyboardShortcutInfo({KeyboardShortcutInfo::Scope::kGame}, keysym(SDLK_UNKNOWN),
			                           format("%scustom_%i", kFastplaceGroupPrefix, counter),
			                           [counter]() { return format(_("Fastplace #%i"), counter); }));
		}
	}
}

void init_fastplace_default_shortcuts(
   const std::map<std::string /* key */,
                  std::map<std::string /* tribe */, std::string /* building */>>& fpdefaults) {
#ifndef NDEBUG
	std::set<std::string> used_keys;
#endif

	for (int i = static_cast<int>(KeyboardShortcut::kFastplace_Begin);
	     i < static_cast<int>(KeyboardShortcut::kFastplace_End); ++i) {
		const KeyboardShortcut id = static_cast<KeyboardShortcut>(i);
		KeyboardShortcutInfo& info = shortcuts_.at(id);

#ifndef NDEBUG
		used_keys.insert(info.internal_name);
#endif

		const auto defaults_it = fpdefaults.find(info.internal_name);
		if (defaults_it != fpdefaults.end()) {
			for (const auto& pair : defaults_it->second) {
				if (info.fastplace.count(pair.first) == 0) {
					info.fastplace.emplace(pair);
				}
			}
		}
	}

#ifndef NDEBUG
	for (const auto& pair : fpdefaults) {
		if (used_keys.count(pair.first) == 0u) {
			log_warn("Fastplace defaults: Unused key '%s'", pair.first.c_str());
		}
	}
#endif
}

void init_shortcuts(const bool force_defaults) {
	init_fastplace_shortcuts(force_defaults);
	for (KeyboardShortcut k = KeyboardShortcut::k_Begin; k <= KeyboardShortcut::k_End;
	     k = static_cast<KeyboardShortcut>(static_cast<uint16_t>(k) + 1)) {
		shortcuts_.at(k).current_shortcut = get_default_shortcut(k);
		if (force_defaults) {
			write_shortcut(k, shortcuts_.at(k).current_shortcut);
		}
	}

#ifndef NDEBUG
	// check that the default hotkeys don't conflict with each other
	for (auto& a : shortcuts_) {
		for (auto& b : shortcuts_) {
			if (a.first != b.first && shared_scope(a.second.scopes, b.second) &&
			    matches_shortcut(a.first, b.second.default_shortcut)) {
				log_warn("The default shortcuts for %s and %s (%s, %s) collide",
				         a.second.internal_name.c_str(), b.second.internal_name.c_str(),
				         shortcut_string_for(a.first, false).c_str(),
				         shortcut_string_for(b.first, false).c_str());
			}
		}
	}
#endif

	if (force_defaults) {
		return;
	}

	Section& ss = get_config_section("keyboard_sym");
	while (Section::Value* v = ss.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.sym = v->get_int();
				break;
			}
		}
	}
	Section& sm = get_config_section("keyboard_mod");
	while (Section::Value* v = sm.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.mod = v->get_int();
				break;
			}
		}
	}
	Section& sf = get_config_section("keyboard_fastplace");
	while (Section::Value* v = sf.get_next_val()) {
		const std::string full_key = v->get_name();
		const size_t separator = full_key.find(kFastplaceNameSeparator);
		if (separator == std::string::npos) {
			log_warn("Ignoring malformed fastplace config string: %s", full_key.c_str());
			continue;
		}
		const std::string fp_key = full_key.substr(0, separator);
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == fp_key) {
				pair.second.fastplace[full_key.substr(separator + 1)] = v->get_string();
				break;
			}
		}
	}
}

ChangeType get_keyboard_change(SDL_Keysym keysym, bool enable_big_step) {
	bool to_limit = false;
	if (matches_keymod(keysym.mod, KMOD_CTRL)) {
		to_limit = true;
	} else if (keysym.mod != KMOD_NONE) {
		return ChangeType::kNone;
	}
	switch (keysym.sym) {
	case SDLK_HOME:
		return ChangeType::kSetMin;
	case SDLK_END:
		return ChangeType::kSetMax;
	case SDLK_MINUS:
	case SDLK_DOWN:
	case SDLK_LEFT:
		return to_limit ? ChangeType::kSetMin : ChangeType::kMinus;
	case SDLK_PLUS:
	case SDLK_UP:
	case SDLK_RIGHT:
		return to_limit ? ChangeType::kSetMax : ChangeType::kPlus;
	case SDLK_PAGEDOWN:
		if (enable_big_step) {
			return to_limit ? ChangeType::kSetMin : ChangeType::kBigMinus;
		} else {
			return ChangeType::kNone;
		}
	case SDLK_PAGEUP:
		if (enable_big_step) {
			return to_limit ? ChangeType::kSetMax : ChangeType::kBigPlus;
		} else {
			return ChangeType::kNone;
		}
	default:
		return ChangeType::kNone;
	}
}

void set_config_directory(const std::string& userconfigdir) {
	config_dir.reset(new RealFSImpl(userconfigdir));
	config_dir->ensure_directory_exists(".");

	config_file = userconfigdir;
	config_file += FileSystem::file_separator();
	config_file += kConfigFile;
	log_info("Set configuration file: %s", config_file.c_str());
}

const std::string& get_config_file() {
	return config_file;
}

void read_config() {
	assert(config_dir != nullptr);
	g_options.read(kConfigFile.c_str(), "global", *config_dir);
	get_config_section();
}

void write_config() {
	assert(config_dir != nullptr);
	try {  //  overwrite the old config file
		g_options.write(kConfigFile.c_str(), true, *config_dir);
	} catch (const std::exception& e) {
		log_warn("could not save configuration: %s\n", e.what());
	} catch (...) {
		log_warn("could not save configuration");
	}
}
