/*
 * Copyright (C) 2012-2024 by the Widelands Development Team
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

bool g_allow_script_console = false;
bool g_write_syncstreams = false;

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

KeyboardShortcut operator+(const KeyboardShortcut& id, const int i) {
	return static_cast<KeyboardShortcut>(static_cast<uint16_t>(id) + i);
}

KeyboardShortcut& operator++(KeyboardShortcut& id) {
	id = id + 1;
	return id;
}

uint16_t operator-(const KeyboardShortcut& a, const KeyboardShortcut& b) {
	assert(a >= b);
	return static_cast<uint16_t>(a) - static_cast<uint16_t>(b);
}

struct KeyboardShortcutInfo {
	std::set<KeyboardShortcutScope> scopes;
	SDL_Keysym default_shortcut;
	SDL_Keysym current_shortcut;
	std::string internal_name;
	std::string descname;
	std::map<std::string /* tribe */, std::string /* building */> fastplace;

	KeyboardShortcutInfo(const std::set<KeyboardShortcutScope>& s,
	                     const SDL_Keysym& sym,
	                     const std::string& n,
	                     const std::string& d)
	   : scopes(s), default_shortcut(sym), current_shortcut(sym), internal_name(n), descname(d) {
	}
};

// Aliases are used for help generation to simplify inserting common shortcuts
struct KeyboardShortcutAlias {
	const KeyboardShortcut real_shortcut;
	const std::string descname_override;  // Use gettext_noop() for the init value!

	explicit KeyboardShortcutAlias(const KeyboardShortcut real,
	                               const std::string& desc_override = std::string())
	   : real_shortcut(real), descname_override(desc_override) {
	}
	[[nodiscard]] std::string translated_descname() const {
		return descname_override.empty() ? to_string(real_shortcut) : _(descname_override);
	}
};

// Use Cmd instead of Ctrl for default shortcuts on MacOS
#ifdef __APPLE__
constexpr uint16_t kDefaultCtrlModifier = KMOD_GUI;
#else
constexpr uint16_t kDefaultCtrlModifier = KMOD_CTRL;
#endif

static const std::vector<KeyboardShortcutInfo> kFastplaceDefaults = {
#define FP(name, descname)                                                                         \
	KeyboardShortcutInfo({KeyboardShortcutScope::kGame}, keysym(SDLK_UNKNOWN),                      \
	                     kFastplaceGroupPrefix + name, descname)
   FP("warehouse", gettext_noop("Warehouse")),
   FP("port", gettext_noop("Port")),
   FP("training_small", gettext_noop("Minor Training Site")),
   FP("training_large", gettext_noop("Major Training Site")),
   FP("military_small_primary", gettext_noop("Primary Small Military Site")),
   FP("military_small_secondary", gettext_noop("Secondary Small Military Site")),
   FP("military_medium_primary", gettext_noop("Primary Medium Military Site")),
   FP("military_medium_secondary", gettext_noop("Secondary Medium Military Site")),
   FP("military_tower", gettext_noop("Tower")),
   FP("military_fortress", gettext_noop("Fortress")),
   FP("woodcutter", gettext_noop("Woodcutter’s House")),
   FP("forester", gettext_noop("Forester’s House")),
   FP("quarry", gettext_noop("Quarry")),
   FP("building_materials_primary", gettext_noop("Primary Building Materials Industry")),
   FP("building_materials_secondary", gettext_noop("Secondary Building Materials Industry")),
   FP("building_materials_tertiary", gettext_noop("Tertiary Building Materials Industry")),
   FP("fisher", gettext_noop("Fishing")),
   FP("hunter", gettext_noop("Hunting")),
   FP("fish_meat_replenisher", gettext_noop("Fish/Meat Replenishing")),
   FP("well", gettext_noop("Well")),
   FP("farm_primary", gettext_noop("Primary Farm")),
   FP("farm_secondary", gettext_noop("Secondary Farm")),
   FP("mill", gettext_noop("Mill")),
   FP("bakery", gettext_noop("Bakery")),
   FP("brewery", gettext_noop("Brewery")),
   FP("smokery", gettext_noop("Smokery")),
   FP("tavern", gettext_noop("Tavern")),
   FP("smelting", gettext_noop("Ore Smelting")),
   FP("tool_smithy", gettext_noop("Tool Smithy")),
   FP("weapon_smithy", gettext_noop("Weapon Smithy")),
   FP("armor_smithy", gettext_noop("Armor Smithy")),
   FP("weaving_mill", gettext_noop("Weaving Mill")),
   FP("shipyard", gettext_noop("Shipyard")),
   FP("ferry_yard", gettext_noop("Ferry Yard")),
   FP("scout", gettext_noop("Scouting")),
   FP("barracks", gettext_noop("Barracks")),
   FP("second_carrier", gettext_noop("Second Carrier")),
   FP("charcoal", gettext_noop("Charcoal")),
   FP("mine_stone", gettext_noop("Stone Mine")),
   FP("mine_coal", gettext_noop("Coal Mine")),
   FP("mine_iron", gettext_noop("Iron Mine")),
   FP("mine_gold", gettext_noop("Gold Mine")),
   FP("agriculture_producer", gettext_noop("Agricultural Producer")),
   FP("agriculture_consumer_primary", gettext_noop("Primary Agricultural Consumer")),
   FP("agriculture_consumer_secondary", gettext_noop("Secondary Agricultural Consumer")),
   FP("industry_alternative", gettext_noop("Alternative Industry")),
   FP("industry_supporter", gettext_noop("Industry Support")),
   FP("terraforming", gettext_noop("Terraforming")),
#undef FP
};

static std::map<KeyboardShortcut, KeyboardShortcutInfo> shortcuts_ = {
   {KeyboardShortcut::kMainMenuNew, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                         keysym(SDLK_n),
                                                         "mainmenu_new",
                                                         gettext_noop("New Game"))},
   {KeyboardShortcut::kMainMenuLoad, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                          keysym(SDLK_l),
                                                          "mainmenu_load",
                                                          gettext_noop("Load Game"))},
   {KeyboardShortcut::kMainMenuReplay, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                            keysym(SDLK_r),
                                                            "mainmenu_replay",
                                                            gettext_noop("Watch Replay"))},
   {KeyboardShortcut::kMainMenuLoadReplay, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                                keysym(SDLK_g),
                                                                "mainmenu_load_replay",
                                                                gettext_noop("Load Replay"))},
   {KeyboardShortcut::kMainMenuReplayLast,
    KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                         keysym(SDLK_d),
                         "mainmenu_replay_last",
                         gettext_noop("Watch Latest Replay"))},
   {KeyboardShortcut::kMainMenuRandomMatch, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                                 keysym(SDLK_z),
                                                                 "mainmenu_random",
                                                                 gettext_noop("New Random Game"))},
   {KeyboardShortcut::kMainMenuTutorial, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                              keysym(SDLK_t),
                                                              "mainmenu_tutorial",
                                                              gettext_noop("Tutorials"))},
   {KeyboardShortcut::kMainMenuCampaign, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                              keysym(SDLK_h),
                                                              "mainmenu_campaign",
                                                              gettext_noop("Campaigns"))},
   {KeyboardShortcut::kMainMenuSP, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                        keysym(SDLK_s),
                                                        "mainmenu_sp",
                                                        gettext_noop("Singleplayer"))},
   {KeyboardShortcut::kMainMenuMP, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                        keysym(SDLK_m),
                                                        "mainmenu_mp",
                                                        gettext_noop("Multiplayer"))},
   {KeyboardShortcut::kMainMenuE,
    KeyboardShortcutInfo(
       {KeyboardShortcutScope::kMainMenu}, keysym(SDLK_e), "mainmenu_e", gettext_noop("Editor"))},
   {KeyboardShortcut::kMainMenuEditorLoad, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                                keysym(SDLK_b),
                                                                "mainmenu_editor_load",
                                                                gettext_noop("Editor – Load Map"))},
   {KeyboardShortcut::kMainMenuEditorNew, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                               keysym(SDLK_k),
                                                               "mainmenu_editor_new",
                                                               gettext_noop("Editor – New Map"))},
   {KeyboardShortcut::kMainMenuEditorRandom,
    KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                         keysym(SDLK_y),
                         "mainmenu_editor_random",
                         gettext_noop("Editor – New Random Map"))},
   {KeyboardShortcut::kMainMenuContinueEditing,
    KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                         keysym(SDLK_w),
                         "mainmenu_editor_continue",
                         gettext_noop("Continue Editing"))},
   {KeyboardShortcut::kMainMenuContinuePlaying,
    KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                         keysym(SDLK_c),
                         "mainmenu_continue",
                         gettext_noop("Continue Playing"))},
   {KeyboardShortcut::kMainMenuQuit, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                          keysym(SDLK_ESCAPE),
                                                          "mainmenu_quit",
                                                          gettext_noop("Exit Widelands"))},
   {KeyboardShortcut::kMainMenuAbout, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                           keysym(SDLK_F1),
                                                           "mainmenu_about",
                                                           gettext_noop("About"))},
   {KeyboardShortcut::kMainMenuAddons, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                            keysym(SDLK_a),
                                                            "mainmenu_addons",
                                                            gettext_noop("Add-Ons"))},
   {KeyboardShortcut::kMainMenuLAN, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                         keysym(SDLK_p),
                                                         "mainmenu_lan",
                                                         gettext_noop("LAN / Direct IP"))},
   {KeyboardShortcut::kMainMenuLobby, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                           keysym(SDLK_j),
                                                           "mainmenu_lobby",
                                                           gettext_noop("Metaserver Lobby"))},
   {KeyboardShortcut::kMainMenuLogin, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                           keysym(SDLK_u),
                                                           "mainmenu_login",
                                                           gettext_noop("Internet Login"))},
   {KeyboardShortcut::kMainMenuOptions, KeyboardShortcutInfo({KeyboardShortcutScope::kMainMenu},
                                                             keysym(SDLK_o),
                                                             "mainmenu_options",
                                                             gettext_noop("Options"))},

   {KeyboardShortcut::kCommonBuildhelp,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_SPACE),
                         "buildhelp",
                         gettext_noop("Toggle Building Spaces"))},
   {KeyboardShortcut::kCommonMinimap,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_m),
                         "minimap",
                         gettext_noop("Toggle Minimap"))},
   {KeyboardShortcut::kCommonEncyclopedia,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_F1),
                         "encyclopedia",
                         gettext_noop("Encyclopedia"))},
   {KeyboardShortcut::kCommonContextMenu, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                               keysym(SDLK_MENU),
                                                               "context_menu",
                                                               gettext_noop("Context Menu"))},
   {KeyboardShortcut::kCommonTextCut, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                           keysym(SDLK_x, kDefaultCtrlModifier),
                                                           "cut",
                                                           gettext_noop("Cut Text"))},
   {KeyboardShortcut::kCommonTextCopy, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                            keysym(SDLK_c, kDefaultCtrlModifier),
                                                            "copy",
                                                            gettext_noop("Copy Text"))},
   {KeyboardShortcut::kCommonTextPaste, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                             keysym(SDLK_v, kDefaultCtrlModifier),
                                                             "paste",
                                                             gettext_noop("Paste Text"))},
   {KeyboardShortcut::kCommonSelectAll, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                             keysym(SDLK_a, kDefaultCtrlModifier),
                                                             "selectall",
                                                             gettext_noop("Select All"))},
   {KeyboardShortcut::kCommonDeleteItem, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                              keysym(SDLK_DELETE),
                                                              "delete",
                                                              gettext_noop("Delete Item"))},
   {KeyboardShortcut::kCommonTooltipAccessibilityMode,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                         keysym(SDLK_F2),
                         "tt_access_mode",
                         gettext_noop("Tooltip Accessibility Mode Key"))},
   {KeyboardShortcut::kCommonFullscreen, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                              keysym(SDLK_f, kDefaultCtrlModifier),
                                                              "fullscreen",
                                                              gettext_noop("Toggle Fullscreen"))},
   {KeyboardShortcut::kCommonScreenshot, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                              keysym(SDLK_F11),
                                                              "screenshot",
                                                              gettext_noop("Take Screenshot"))},
   {KeyboardShortcut::kCommonChangeMusic, KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                                                               keysym(SDLK_F9),
                                                               "change_music",
                                                               gettext_noop("Change Music"))},
   {KeyboardShortcut::kCommonDebugConsole,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                         keysym(SDLK_SPACE, kDefaultCtrlModifier | KMOD_SHIFT),
                         "debugconsole",
                         gettext_noop("Open the Debug Console (developer tool)"))},
   {KeyboardShortcut::kCommonCheatMode,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGlobal},
                         keysym(SDLK_BACKSPACE, kDefaultCtrlModifier | KMOD_SHIFT),
                         "cheatmode",
                         gettext_noop("Toggle Cheat Mode (developer tool)"))},
   {KeyboardShortcut::kCommonSave,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_s, kDefaultCtrlModifier),
                         "save",
                         gettext_noop("Save"))},
   {KeyboardShortcut::kCommonLoad,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_l, kDefaultCtrlModifier),
                         "load",
                         gettext_noop("Load"))},
   {KeyboardShortcut::kCommonExit,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_q, kDefaultCtrlModifier),
                         "exit",
                         gettext_noop("Exit"))},
   {KeyboardShortcut::kCommonZoomIn,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_PLUS, kDefaultCtrlModifier),
                         "zoom_in",
                         gettext_noop("Zoom In"))},
   {KeyboardShortcut::kCommonZoomOut,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_MINUS, kDefaultCtrlModifier),
                         "zoom_out",
                         gettext_noop("Zoom Out"))},
   {KeyboardShortcut::kCommonZoomReset,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_0, kDefaultCtrlModifier),
                         "zoom_reset",
                         gettext_noop("Reset Zoom"))},
   {KeyboardShortcut::kCommonQuicknavNext,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_PERIOD),
                         "quicknav_next",
                         gettext_noop("Jump to Next Location"))},
   {KeyboardShortcut::kCommonQuicknavPrev,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame, KeyboardShortcutScope::kEditor},
                         keysym(SDLK_COMMA),
                         "quicknav_prev",
                         gettext_noop("Jump to Previous Location"))},

   {KeyboardShortcut::kEditorNewMap, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                          keysym(SDLK_n, kDefaultCtrlModifier),
                                                          "editor_new_map",
                                                          gettext_noop("New Map"))},
   {KeyboardShortcut::kEditorNewRandomMap,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_r, kDefaultCtrlModifier),
                         "editor_new_random_map",
                         gettext_noop("New Random Map"))},
   {KeyboardShortcut::kEditorUploadMap, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                             keysym(SDLK_u, kDefaultCtrlModifier),
                                                             "editor_upload_map",
                                                             gettext_noop("Publish Map Online"))},
   {KeyboardShortcut::kEditorMapOptions, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                              keysym(SDLK_o, kDefaultCtrlModifier),
                                                              "editor_mapoptions",
                                                              gettext_noop("Map Options"))},
   {KeyboardShortcut::kEditorUndo, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                        keysym(SDLK_z, kDefaultCtrlModifier),
                                                        "editor_undo",
                                                        gettext_noop("Undo"))},
   {KeyboardShortcut::kEditorRedo, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                        keysym(SDLK_y, kDefaultCtrlModifier),
                                                        "editor_redo",
                                                        gettext_noop("Redo"))},
   {KeyboardShortcut::kEditorTools,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_t),
                         "editor_tools",
                         gettext_noop("Toggle Tools Menu"))},
   {KeyboardShortcut::kEditorChangeHeight,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_h, KMOD_SHIFT),
                         "editor_change_height",
                         gettext_noop("Change Height Tool"))},
   {KeyboardShortcut::kEditorRandomHeight,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_b, KMOD_SHIFT),
                         "editor_random_height",
                         gettext_noop("Random Height Tool"))},
   {KeyboardShortcut::kEditorTerrain, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                           keysym(SDLK_t, KMOD_SHIFT),
                                                           "editor_terrain",
                                                           gettext_noop("Terrain Tool"))},
   {KeyboardShortcut::kEditorImmovables, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                              keysym(SDLK_o, KMOD_SHIFT),
                                                              "editor_immovables",
                                                              gettext_noop("Immovables Tool"))},
   {KeyboardShortcut::kEditorAnimals, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                           keysym(SDLK_a, KMOD_SHIFT),
                                                           "editor_animals",
                                                           gettext_noop("Animals Tool"))},
   {KeyboardShortcut::kEditorResources, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                             keysym(SDLK_r, KMOD_SHIFT),
                                                             "editor_resources",
                                                             gettext_noop("Resources Tool"))},
   {KeyboardShortcut::kEditorPortSpaces, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                              keysym(SDLK_e, KMOD_SHIFT),
                                                              "editor_port_spaces",
                                                              gettext_noop("Port Spaces Tool"))},
   {KeyboardShortcut::kEditorInfo, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                        keysym(SDLK_i, KMOD_SHIFT),
                                                        "editor_info",
                                                        gettext_noop("Info Tool"))},
   {KeyboardShortcut::kEditorMapOrigin, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                             keysym(SDLK_x, KMOD_SHIFT),
                                                             "editor_map_origin",
                                                             gettext_noop("Map Origin Tool"))},
   {KeyboardShortcut::kEditorMapSize, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                           keysym(SDLK_z, KMOD_SHIFT),
                                                           "editor_map_size",
                                                           gettext_noop("Map Size Tool"))},
   {KeyboardShortcut::kEditorPlayers, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                           keysym(SDLK_p, KMOD_SHIFT),
                                                           "editor_players",
                                                           gettext_noop("Players Menu"))},
   {KeyboardShortcut::kEditorToolHistory, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                               keysym(SDLK_h),
                                                               "editor_tool_history",
                                                               gettext_noop("Tool History"))},
   {KeyboardShortcut::kEditorShowhideHeightHeatMap,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_v),
                         "editor_showhide_height_heat_map",
                         gettext_noop("Toggle Height Heat Map"))},
   {KeyboardShortcut::kEditorShowhideGrid, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                                keysym(SDLK_g),
                                                                "editor_showhide_grid",
                                                                gettext_noop("Toggle Grid"))},
   {KeyboardShortcut::kEditorShowhideOceans, KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                                                                  keysym(SDLK_z),
                                                                  "editor_showhide_oceans",
                                                                  gettext_noop("Toggle Oceans"))},
   {KeyboardShortcut::kEditorShowhideImmovables,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_o),
                         "editor_showhide_immovables",
                         gettext_noop("Toggle Immovables"))},
   {KeyboardShortcut::kEditorShowhideCritters,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_a),
                         "editor_showhide_critters",
                         gettext_noop("Toggle Animals"))},
   {KeyboardShortcut::kEditorShowhideResources,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_r),
                         "editor_showhide_resources",
                         gettext_noop("Toggle Resources"))},

   {KeyboardShortcut::kEditorShowhideMaximumBuildhelp,
    KeyboardShortcutInfo({KeyboardShortcutScope::kEditor},
                         keysym(SDLK_SPACE, KMOD_SHIFT),
                         "editor_showhide_maximum_buildhelp",
                         gettext_noop("Toggle Maximum Build Spaces"))},

// toolsize_descr() uses this format string and adds the toolsize
#define EDITOR_TOOLSIZE(radius, key)                                                               \
	{                                                                                               \
		KeyboardShortcut::kEditorToolsize##radius,                                                   \
		   KeyboardShortcutInfo({KeyboardShortcutScope::kEditor}, keysym(SDLK_##key),                \
		                        "editor_toolsize" #radius, gettext_noop("Set Toolsize to %d"))       \
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

#define EDITOR_TOOLGAP(radius, key)                                                                \
	{                                                                                               \
		KeyboardShortcut::kEditorToolgap##radius,                                                    \
		   KeyboardShortcutInfo({KeyboardShortcutScope::kEditor}, keysym(SDLK_##key, KMOD_SHIFT),    \
		                        "editor_toolgap" #radius, gettext_noop("Set Tool Gap to %d%%"))      \
	}
   EDITOR_TOOLGAP(0, 0),
   EDITOR_TOOLGAP(10, 1),
   EDITOR_TOOLGAP(20, 2),
   EDITOR_TOOLGAP(30, 3),
   EDITOR_TOOLGAP(40, 4),
   EDITOR_TOOLGAP(50, 5),
   EDITOR_TOOLGAP(60, 6),
   EDITOR_TOOLGAP(70, 7),
   EDITOR_TOOLGAP(80, 8),
   EDITOR_TOOLGAP(90, 9),
#undef EDITOR_TOOLGAP

   {KeyboardShortcut::kInGameSoundOptions,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_o, kDefaultCtrlModifier),
                         "game_sound_options",
                         gettext_noop("Sound Options"))},
   {KeyboardShortcut::kInGameRestart,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_r, kDefaultCtrlModifier),
                         "game_restart",
                         gettext_noop("Restart Scenario or Replay"))},
   {KeyboardShortcut::kInGameShowhideCensus, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                                  keysym(SDLK_c),
                                                                  "game_showhide_census",
                                                                  gettext_noop("Toggle Census"))},
   {KeyboardShortcut::kInGameShowhideStats,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_s),
                         "game_showhide_stats",
                         gettext_noop("Toggle Status Labels"))},
   {KeyboardShortcut::kInGameShowhideSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_l),
                         "game_showhide_soldiers",
                         gettext_noop("Toggle Soldier Levels"))},
   {KeyboardShortcut::kInGameShowhideBuildings,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_u),
                         "game_showhide_buildings",
                         gettext_noop("Toggle Buildings Visibility"))},
   {KeyboardShortcut::kInGameShowhideWorkareas,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_w),
                         "game_showhide_workareas",
                         gettext_noop("Toggle Overlapping Workareas"))},
   {KeyboardShortcut::kInGameStatsGeneral,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_a),
                         "game_stats_general",
                         gettext_noop("General Statistics"))},
   {KeyboardShortcut::kInGameStatsWares, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                              keysym(SDLK_p),
                                                              "game_stats_wares",
                                                              gettext_noop("Ware Statistics"))},
   {KeyboardShortcut::kInGameStatsBuildings,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_b),
                         "game_stats_buildings",
                         gettext_noop("Building Statistics"))},
   {KeyboardShortcut::kInGameStatsStock, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                              keysym(SDLK_i),
                                                              "game_stats_stock",
                                                              gettext_noop("Stock Inventory"))},
   {KeyboardShortcut::kInGameStatsSoldiers,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_x),
                         "game_stats_soldiers",
                         gettext_noop("Soldier Statistics"))},
   {KeyboardShortcut::kInGameStatsSeafaring,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_e),
                         "game_stats_seafaring",
                         gettext_noop("Seafaring Statistics"))},
   {KeyboardShortcut::kInGameObjectives, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                              keysym(SDLK_t),
                                                              "game_objectives",
                                                              gettext_noop("Objectives"))},
   {KeyboardShortcut::kInGameDiplomacy, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                             keysym(SDLK_d),
                                                             "game_diplomacy",
                                                             gettext_noop("Diplomacy"))},
   {KeyboardShortcut::kInGameMessages,
    KeyboardShortcutInfo(
       {KeyboardShortcutScope::kGame}, keysym(SDLK_n), "game_messages", gettext_noop("Messages"))},
   {KeyboardShortcut::kInGameSpeedDown,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEDOWN),
                         "game_speed_down",
                         gettext_noop("Decrease Game Speed by 1×"))},
   {KeyboardShortcut::kInGameSpeedDownSlow,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEDOWN, KMOD_SHIFT),
                         "game_speed_down_slow",
                         gettext_noop("Decrease Game Speed by 0.25×"))},
   {KeyboardShortcut::kInGameSpeedDownFast,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEDOWN, kDefaultCtrlModifier),
                         "game_speed_down_fast",
                         gettext_noop("Decrease Game Speed by 10×"))},
   {KeyboardShortcut::kInGameSpeedUp,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEUP),
                         "game_speed_up",
                         gettext_noop("Increase Game Speed by 1×"))},
   {KeyboardShortcut::kInGameSpeedUpSlow,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEUP, KMOD_SHIFT),
                         "game_speed_up_slow",
                         gettext_noop("Increase Game Speed by 0.25×"))},
   {KeyboardShortcut::kInGameSpeedUpFast,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_PAGEUP, kDefaultCtrlModifier),
                         "game_speed_up_fast",
                         gettext_noop("Increase Game Speed by 10×"))},
   {KeyboardShortcut::kInGameSpeedReset, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                              keysym(SDLK_PAUSE, KMOD_SHIFT),
                                                              "game_speed_reset",
                                                              gettext_noop("Reset Game Speed"))},
   {KeyboardShortcut::kInGamePause, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                         keysym(SDLK_PAUSE),
                                                         "game_pause",
                                                         gettext_noop("Pause The Game"))},
   {KeyboardShortcut::kInGameScrollToHQ,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_HOME),
                         "game_hq",
                         gettext_noop("Scroll to Starting Field"))},
   {KeyboardShortcut::kInGameChat,
    KeyboardShortcutInfo(
       {KeyboardShortcutScope::kGame}, keysym(SDLK_RETURN), "game_chat", gettext_noop("Chat"))},
   {KeyboardShortcut::kInGamePinnedNote, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                              keysym(SDLK_F8),
                                                              "game_pinned_note",
                                                              gettext_noop("Pinned Note"))},
   {KeyboardShortcut::kInGameMessagesGoto, KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                                                                keysym(SDLK_g),
                                                                "game_msg_goto",
                                                                gettext_noop("Go to Location"))},
   {KeyboardShortcut::kInGameMessagesFilterAll,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_0, KMOD_ALT),
                         "game_msg_filter_all",
                         gettext_noop("Show All Messages"))},
   {KeyboardShortcut::kInGameMessagesFilterGeologists,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_1, KMOD_ALT),
                         "game_msg_filter_geo",
                         gettext_noop("Show Geologists’ Messages Only"))},
   {KeyboardShortcut::kInGameMessagesFilterEconomy,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_2, KMOD_ALT),
                         "game_msg_filter_eco",
                         gettext_noop("Show Economy Messages Only"))},
   {KeyboardShortcut::kInGameMessagesFilterSeafaring,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_3, KMOD_ALT),
                         "game_msg_filter_seafaring",
                         gettext_noop("Show Seafaring Messages Only"))},
   {KeyboardShortcut::kInGameMessagesFilterWarfare,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_4, KMOD_ALT),
                         "game_msg_filter_warfare",
                         gettext_noop("Show Military Messages Only"))},
   {KeyboardShortcut::kInGameMessagesFilterScenario,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_5, KMOD_ALT),
                         "game_msg_filter_scenario",
                         gettext_noop("Show Scenario Messages Only"))},
   {KeyboardShortcut::kInGameSeafaringstatsGotoShip,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_j),
                         "game_sfstats_goto",
                         gettext_noop("Go to Ship"))},
   {KeyboardShortcut::kInGameSeafaringstatsWatchShip,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_f),
                         "game_sfstats_watch",
                         gettext_noop("Watch Ship"))},
   {KeyboardShortcut::kInGameSeafaringstatsOpenShipWindow,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_o),
                         "game_sfstats_open",
                         gettext_noop("Open Ship Window"))},
   {KeyboardShortcut::kInGameSeafaringstatsOpenShipWindowAndGoto,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_o, KMOD_SHIFT),
                         "game_sfstats_open_goto",
                         gettext_noop("Open Ship Window And Go to Ship"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterAll,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_0, KMOD_SHIFT),
                         "game_sfstats_filter_all",
                         gettext_noop("Show All Ships"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterIdle,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_1, KMOD_SHIFT),
                         "game_sfstats_filter_idle",
                         gettext_noop("Show Idle Ships"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterShipping,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_2, KMOD_SHIFT),
                         "game_sfstats_filter_ship",
                         gettext_noop("Show Transport Ships"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpWait,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_3, KMOD_SHIFT),
                         "game_sfstats_filter_wait",
                         gettext_noop("Show Waiting Expeditions"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpScout,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_4, KMOD_SHIFT),
                         "game_sfstats_filter_scout",
                         gettext_noop("Show Scouting Expeditions"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterExpPortspace,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_5, KMOD_SHIFT),
                         "game_sfstats_filter_port",
                         gettext_noop("Show Expeditions with Port Spaces"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterWarship,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_6, KMOD_SHIFT),
                         "game_sfstats_filter_warship",
                         gettext_noop("Show Warships"))},
   {KeyboardShortcut::kInGameSeafaringstatsFilterRefitting,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_7, KMOD_SHIFT),
                         "game_sfstats_filter_refitting",
                         gettext_noop("Show Ships Being Refitted"))},
   {KeyboardShortcut::kInGameQuicknavGUI,
    KeyboardShortcutInfo({KeyboardShortcutScope::kGame},
                         keysym(SDLK_v),
                         "quicknav_gui",
                         gettext_noop("Toggle Quick Navigation"))},

// quicknav_descr() uses these format strings and adds the landmark number
#define QUICKNAV(i)                                                                                \
	{KeyboardShortcut::kInGameQuicknavSet##i,                                                       \
	 KeyboardShortcutInfo({KeyboardShortcutScope::kGame}, keysym(SDLK_##i, kDefaultCtrlModifier),   \
	                      "game_quicknav_set_" #i, gettext_noop("Set Landmark #%d"))},              \
	{                                                                                               \
		KeyboardShortcut::kInGameQuicknavGoto##i,                                                    \
		   KeyboardShortcutInfo({KeyboardShortcutScope::kGame}, keysym(SDLK_##i),                    \
		                        "game_quicknav_goto_" #i, gettext_noop("Go To Landmark #%d"))        \
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

bool is_real(const KeyboardShortcut id) {
	auto it = shortcuts_.find(id);
	return it != shortcuts_.end();
}

bool is_developer_tool(KeyboardShortcut id) {
	return id == KeyboardShortcut::kCommonDebugConsole || id == KeyboardShortcut::kCommonCheatMode;
}

static const std::map<KeyboardShortcut, KeyboardShortcutAlias> shortcut_aliases_ = {
   {KeyboardShortcut::kEditorLoad,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonLoad, gettext_noop("Load Map"))},
   {KeyboardShortcut::kEditorSave,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonSave, gettext_noop("Save Map"))},
   {KeyboardShortcut::kEditorExit,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonExit, gettext_noop("Exit Editor"))},
   {KeyboardShortcut::kEditorHelp,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonEncyclopedia, gettext_noop("Help"))},

   {KeyboardShortcut::kEditorShowhideBuildhelp,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonBuildhelp)},
   {KeyboardShortcut::kEditorMinimap, KeyboardShortcutAlias(KeyboardShortcut::kCommonMinimap)},
   {KeyboardShortcut::kEditorQuicknavPrev,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonQuicknavPrev)},
   {KeyboardShortcut::kEditorQuicknavNext,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonQuicknavNext)},

   {KeyboardShortcut::kInGameLoad,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonLoad, gettext_noop("Load Game"))},
   {KeyboardShortcut::kInGameSave,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonSave, gettext_noop("Save Game"))},
   {KeyboardShortcut::kInGameExit,
    /** TRANSLATORS: This is the helptext for an access key combination. */
    KeyboardShortcutAlias(KeyboardShortcut::kCommonExit, gettext_noop("Exit Game"))},
   {KeyboardShortcut::kInGameEncyclopedia,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonEncyclopedia)},

   {KeyboardShortcut::kInGameShowhideBuildhelp,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonBuildhelp)},
   {KeyboardShortcut::kInGameMinimap, KeyboardShortcutAlias(KeyboardShortcut::kCommonMinimap)},
   {KeyboardShortcut::kInGameQuicknavPrev,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonQuicknavPrev)},
   {KeyboardShortcut::kInGameQuicknavNext,
    KeyboardShortcutAlias(KeyboardShortcut::kCommonQuicknavNext)}};

// Keyboard control additional help texts
namespace {

std::string help_move_map() {
	std::string rv;
	/** TRANSLATORS: This is an access key combination. */
	rv += as_definition_line(pgettext("hotkey", "Arrow keys"),
	                         /** TRANSLATORS: This is the helptext for an access key combination. */
	                         _("Move the map"));
	/** TRANSLATORS: This is an access key combination. */
	rv += as_definition_line(pgettext("hotkey", "Ctrl + Arrow keys"),
	                         /** TRANSLATORS: This is the helptext for an access key combination. */
	                         _("Move the map fast"));
	/** TRANSLATORS: This is an access key combination. */
	rv += as_definition_line(pgettext("hotkey", "Shift + Arrow keys"),
	                         /** TRANSLATORS: This is the helptext for an access key combination. */
	                         _("Move the map slowly"));
	return rv;
}

std::string get_related_hotkeys_help(KeyboardShortcut first,
                                     int step,
                                     int n_keys,
                                     const std::string& description);

std::string help_quicknav() {
	std::string rv = get_related_hotkeys_help(
	   /** TRANSLATORS: This is the helptext for an access key combination. */
	   KeyboardShortcut::kInGameQuicknavSet1, 2, 9, _("Remember current location"));
	rv += get_related_hotkeys_help(
	   /** TRANSLATORS: This is the helptext for an access key combination. */
	   KeyboardShortcut::kInGameQuicknavGoto1, 2, 9, _("Go to previously remembered location"));
	return rv;
}

}  // namespace

static const std::map<KeyboardShortcut, const std::function<std::string()>>
   controls_special_entries_{{KeyboardShortcut::kEditor_Special_MapMove, help_move_map},
                             {KeyboardShortcut::kInGame_Special_MapMove, help_move_map},
                             {KeyboardShortcut::kInGame_Special_Quicknav, help_quicknav}};

// Help formatting functions
namespace {

std::string get_shortcut_help_line(const KeyboardShortcut id) {
	if (auto it = controls_special_entries_.find(id); it != controls_special_entries_.end()) {
		return it->second();
	}

	KeyboardShortcut real_id = id;
	std::string description;

	if (auto it = shortcut_aliases_.find(id); it != shortcut_aliases_.end()) {
		// This is an alias
		real_id = it->second.real_shortcut;
		description = it->second.translated_descname();
	} else {
		assert(is_real(id));
		// Not using to_string(), because we don't want the prefixes for
		// message- and ship window shortcuts.
		description = _(shortcuts_.at(id).descname);
	}

	return as_definition_line(shortcut_string_for(real_id, true), description);
}

std::string get_shortcut_range_help(const KeyboardShortcut start, const KeyboardShortcut end) {
	std::string rv;
	for (KeyboardShortcut id = start; id <= end; ++id) {
		rv += get_shortcut_help_line(id);
	}
	return rv;
}

std::string get_related_hotkeys_help(const KeyboardShortcut first,
                                     const int step,
                                     const int n_keys,
                                     const std::string& description) {
	/** TRANSLATORS: Separator for a list of hotkeys  */
	const std::string separator_format(pgettext("hotkey", "%1$s / %2$s"));

	std::string keys_list;
	std::string current;
	for (int i = 0; i < n_keys; ++i) {
		current = shortcut_string_for(first + i * step, true);
		if (i == 0) {
			keys_list = current;
		} else {
			keys_list = format(separator_format, keys_list, current);
		}
	}
	return as_definition_line(keys_list, description);
}

}  // namespace

std::string get_ingame_shortcut_help() {
	std::string rv(as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Keyboard Shortcuts")));
	rv +=
	   get_shortcut_range_help(KeyboardShortcut::kInGame_Begin, KeyboardShortcut::kInGameMain_End);
	rv += get_shortcut_range_help(
	   KeyboardShortcut::kCommonGeneral_Begin, KeyboardShortcut::kCommonGeneral_End);
	if (g_allow_script_console) {
		rv += get_shortcut_help_line(KeyboardShortcut::kCommonDebugConsole);
		rv += get_shortcut_help_line(KeyboardShortcut::kCommonCheatMode);
	}

	/** TRANSLATORS: Section heading in "Controls" help */
	rv += as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Message Window"));
	rv += get_shortcut_range_help(
	   KeyboardShortcut::kInGameMessages_Begin, KeyboardShortcut::kInGameMessages_End);

	/** TRANSLATORS: Section heading in "Controls" help */
	rv += as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Ship Statistics"));
	rv += get_shortcut_range_help(
	   KeyboardShortcut::kInGameSeafaringstats_Begin, KeyboardShortcut::kInGameSeafaringstats_End);

	rv += as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Add-Ons"));
	rv += get_shortcut_range_help(KeyboardShortcut::k_End + 1, get_highest_used_keyboard_shortcut());

	return rv;
}

std::vector<FastplaceShortcut> get_active_fastplace_shortcuts(const std::string& tribe) {
	std::vector<FastplaceShortcut> rv;
	for (KeyboardShortcut id = KeyboardShortcut::kFastplace_Begin;
	     id <= KeyboardShortcut::kFastplace_End; ++id) {
		if (get_shortcut(id).sym == SDLK_UNKNOWN) {
			continue;
		}
		auto fastplace = get_fastplace_shortcuts(id);
		auto it = fastplace.find(tribe);
		if (it == fastplace.end()) {
			continue;
		}
		rv.emplace_back(FastplaceShortcut{shortcut_string_for(id, true), it->second});
	}
	return rv;
}

std::string get_editor_shortcut_help() {
	std::string rv(as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Keyboard Shortcuts")));
	rv +=
	   get_shortcut_range_help(KeyboardShortcut::kEditor_Begin, KeyboardShortcut::kEditorMain_End);
	rv += get_shortcut_range_help(
	   KeyboardShortcut::kCommonGeneral_Begin, KeyboardShortcut::kCommonGeneral_End);
	if (g_allow_script_console) {
		rv += get_shortcut_help_line(KeyboardShortcut::kCommonDebugConsole);
	}

	/** TRANSLATORS: Heading in the editor keyboard shortcuts help */
	rv += as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, pgettext("editor", "Tools"));
	rv += get_shortcut_range_help(
	   KeyboardShortcut::kEditorTools_Begin, KeyboardShortcut::kEditorTools_End);
	/** TRANSLATORS: This is the helptext for an access key combination. */
	rv += get_related_hotkeys_help(KeyboardShortcut::kEditorToolsize1, 1, 10, _("Change tool size"));
	rv += get_related_hotkeys_help(KeyboardShortcut::kEditorToolgap0, 1, 10, _("Change tool gap"));

	// Mouse controls for tools are included here because they belong to the Tools section
	rv += as_definition_line(
	   /** TRANSLATORS: This is an access key combination. */
	   pgettext("hotkey", "Click"),
	   /** TRANSLATORS: This is the helptext for an access key combination. */
	   _("Place new elements on the map, or increase map elements by the value selected by "
	     "‘Increase/Decrease value’"));
	rv += as_definition_line(
	   /** TRANSLATORS: This is an access key combination. */
	   pgettext("hotkey", "Shift + Click"),
	   /** TRANSLATORS: This is the helptext for an access key combination. */
	   _("Remove elements from the map, or decrease map elements by the value selected by "
	     "‘Increase/Decrease value’"));
	/** TRANSLATORS: This is an access key combination. */
	rv += as_definition_line(pgettext("hotkey", "Ctrl + Click"),
	                         /** TRANSLATORS: This is the helptext for an access key combination. */
	                         _("Set map elements to the value selected by ‘Set Value’"));

	rv += as_paragraph_style(UI::ParagraphStyle::kWuiHeading2, _("Add-Ons"));
	rv += get_shortcut_range_help(KeyboardShortcut::k_End + 1, get_highest_used_keyboard_shortcut());

	return rv;
}

void unset_shortcut(const KeyboardShortcut id) {
	assert(is_real(id));
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

// Keyboard shortcut description special cases
namespace {

std::string toolsize_descr(const KeyboardShortcut id) {
	assert(id >= KeyboardShortcut::kEditorToolsize1 && id <= KeyboardShortcut::kEditorToolsize10);
	const uint16_t i = id - KeyboardShortcut::kEditorToolsize1 + 1;
	return format(_(shortcuts_.at(id).descname), i);
}

std::string toolgap_descr(const KeyboardShortcut id) {
	assert(id >= KeyboardShortcut::kEditorToolgap0 && id <= KeyboardShortcut::kEditorToolgap90);
	const uint16_t i = id - KeyboardShortcut::kEditorToolgap0;
	return format(_(shortcuts_.at(id).descname), 10 * i);
}

std::string quicknav_descr(const KeyboardShortcut id) {
	assert(id >= KeyboardShortcut::kInGameQuicknavSet1 &&
	       id <= KeyboardShortcut::kInGameQuicknavGoto9);
	uint16_t i = id - KeyboardShortcut::kInGameQuicknavSet1;
	if (i % 2 == 0) {
		// id is Set Quicknav
		i = i / 2 + 1;
	} else {
		// id is Goto Quicknav
		i = (i + 1) / 2;
	}
	return format(_(shortcuts_.at(id).descname), i);
}

std::string fastplace_descr(const KeyboardShortcut id) {
	assert(id >= KeyboardShortcut::kFastplace_Begin + kFastplaceDefaults.size() &&
	       id <= KeyboardShortcut::kFastplace_End);
	const uint16_t i = id - KeyboardShortcut::kFastplace_Begin - kFastplaceDefaults.size() + 1;
	return format(_(shortcuts_.at(id).descname), i);
}

}  // namespace

std::string to_string(const KeyboardShortcut id) {
	assert(is_real(id));
	if (id >= KeyboardShortcut::kEditorToolsize1 && id <= KeyboardShortcut::kEditorToolsize10) {
		return toolsize_descr(id);
	}
	if (id >= KeyboardShortcut::kEditorToolgap0 && id <= KeyboardShortcut::kEditorToolgap90) {
		return toolgap_descr(id);
	}
	if (id >= KeyboardShortcut::kInGameQuicknavSet1 &&
	    id <= KeyboardShortcut::kInGameQuicknavGoto9) {
		return quicknav_descr(id);
	}
	if (id >= KeyboardShortcut::kInGameMessages_Begin &&
	    id <= KeyboardShortcut::kInGameMessages_End) {
		/** TRANSLATORS: prefix for message window shortcuts in keyboard options */
		return format(pgettext("hotkey", "Messages: %s"), _(shortcuts_.at(id).descname));
	}
	if (id >= KeyboardShortcut::kInGameSeafaringstats_Begin &&
	    id <= KeyboardShortcut::kInGameSeafaringstats_End) {
		/** TRANSLATORS: prefix for seafaring statistics window shortcuts in keyboard options */
		return format(pgettext("hotkey", "Seafaring: %s"), _(shortcuts_.at(id).descname));
	}
	if (id >= KeyboardShortcut::kFastplace_Begin + kFastplaceDefaults.size() &&
	    id <= KeyboardShortcut::kFastplace_End) {
		return fastplace_descr(id);
	}
	return _(shortcuts_.at(id).descname);
}

SDL_Keysym get_default_shortcut(const KeyboardShortcut id) {
	assert(is_real(id));
	return shortcuts_.at(id).default_shortcut;
}

static void write_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	assert(is_real(id));
	set_config_int("keyboard_sym", shortcuts_.at(id).internal_name, code.sym);
	set_config_int("keyboard_mod", shortcuts_.at(id).internal_name, code.mod);
}

static bool shared_scope(const std::set<KeyboardShortcutScope>& scopes,
                         const KeyboardShortcutInfo& k) {
	if ((scopes.count(KeyboardShortcutScope::kGlobal) != 0u) ||
	    (k.scopes.count(KeyboardShortcutScope::kGlobal) != 0u)) {
		return true;
	}

	return std::any_of(scopes.begin(), scopes.end(),
	                   [&k](KeyboardShortcutScope s) { return k.scopes.count(s) != 0u; });
}

bool set_shortcut(const KeyboardShortcut id, const SDL_Keysym code, KeyboardShortcut* conflict) {
	assert(is_real(id));
	const std::set<KeyboardShortcutScope>& scopes = shortcuts_.at(id).scopes;

	for (auto& pair : shortcuts_) {
		if (pair.first != id && shared_scope(scopes, pair.second) &&
		    matches_shortcut(pair.first, code)) {
			if (!g_allow_script_console && is_developer_tool(pair.first)) {
				// We don't want to advertise these features, so we silently clear their
				// shortcuts on conflict when they are not enabled
				shortcuts_.at(pair.first).current_shortcut = keysym(SDLK_UNKNOWN);
				write_shortcut(pair.first, keysym(SDLK_UNKNOWN));
				continue;
			}

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
	assert(is_real(id));
	return shortcuts_.at(id).current_shortcut;
}

static const std::map<SDL_Keycode, SDL_Keycode> kNumpadIdentifications = {
   {SDLK_KP_9, SDLK_PAGEUP},          {SDLK_KP_8, SDLK_UP},          {SDLK_KP_7, SDLK_HOME},
   {SDLK_KP_6, SDLK_RIGHT},           {SDLK_KP_5, SDLK_UNKNOWN},     {SDLK_KP_4, SDLK_LEFT},
   {SDLK_KP_3, SDLK_PAGEDOWN},        {SDLK_KP_2, SDLK_DOWN},        {SDLK_KP_1, SDLK_END},
   {SDLK_KP_0, SDLK_INSERT},          {SDLK_KP_PERIOD, SDLK_DELETE}, {SDLK_KP_ENTER, SDLK_RETURN},
   {SDLK_KP_MINUS, SDLK_MINUS},       {SDLK_KP_PLUS, SDLK_PLUS},     {SDLK_KP_DIVIDE, SDLK_SLASH},
   {SDLK_KP_MULTIPLY, SDLK_ASTERISK}, {SDLK_APPLICATION, SDLK_MENU}};

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

	return std::any_of(
	   kNumpadIdentifications.begin(), kNumpadIdentifications.end(), [code, key](const auto& pair) {
		   return (code == pair.first && key.sym == pair.second) ||
		          (code == pair.second && key.sym == pair.first);
	   });
}

std::string matching_fastplace_shortcut(const SDL_Keysym key, const std::string& tribename) {
	for (KeyboardShortcut id = KeyboardShortcut::kFastplace_Begin;
	     id <= KeyboardShortcut::kFastplace_End; ++id) {
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

bool shortcut_exists(const std::string& name) {
	return std::any_of(shortcuts_.begin(), shortcuts_.end(),
	                   [&name](const auto& pair) { return pair.second.internal_name == name; });
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
	case SDLK_APPLICATION:
		return pgettext("hotkey", "Application");
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

std::vector<std::string> get_all_keyboard_shortcut_names() {
	std::vector<std::string> result(shortcuts_.size());
	size_t i = 0;
	for (const auto& pair : shortcuts_) {
		result.at(i++) = pair.second.internal_name;
	}
	return result;
}

KeyboardShortcut get_highest_used_keyboard_shortcut() {
	return shortcuts_.empty() ? KeyboardShortcut::k_End : shortcuts_.rbegin()->first;
}

void create_replace_shortcut(const std::string& name,
                             const std::string& descname,
                             const std::set<KeyboardShortcutScope>& scopes,
                             SDL_Keysym default_shortcut) {
	for (auto& pair : shortcuts_) {
		if (pair.second.internal_name == name) {
			pair.second.scopes = scopes;
			pair.second.descname = descname;
			pair.second.default_shortcut = default_shortcut;
			/* Leave the currently set keybinding unchanged though. */
			return;
		}
	}

	shortcuts_.emplace(std::max(KeyboardShortcut::k_End, get_highest_used_keyboard_shortcut()) + 1,
	                   KeyboardShortcutInfo(scopes, default_shortcut, name, descname));
}

static void init_fastplace_shortcuts(const bool force_defaults) {
	int counter = 0;
	for (KeyboardShortcut k = KeyboardShortcut::kFastplace_Begin;
	     k <= KeyboardShortcut::kFastplace_End; ++k) {
		if (force_defaults) {
			shortcuts_.erase(k);
		} else if (shortcuts_.count(k) != 0u) {
			continue;
		}

		const unsigned off = k - KeyboardShortcut::kFastplace_Begin;
		if (off < kFastplaceDefaults.size()) {
			shortcuts_.emplace(k, kFastplaceDefaults[off]);
		} else {
			++counter;
			shortcuts_.emplace(
			   k, KeyboardShortcutInfo({KeyboardShortcutScope::kGame}, keysym(SDLK_UNKNOWN),
			                           format("%scustom_%i", kFastplaceGroupPrefix, counter),
			                           // fastplace_descr() uses this format string and adds the
			                           // fastplace number
			                           gettext_noop("Fastplace #%i")));
		}
	}
}

void init_fastplace_default_shortcuts(
   const std::map<std::string /* key */,
                  std::map<std::string /* tribe */, std::string /* building */>>& fpdefaults) {
#ifndef NDEBUG
	std::set<std::string> used_keys;
#endif

	for (KeyboardShortcut id = KeyboardShortcut::kFastplace_Begin;
	     id <= KeyboardShortcut::kFastplace_End; ++id) {
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
	for (KeyboardShortcut k = KeyboardShortcut::k_Begin; k <= KeyboardShortcut::k_End; ++k) {
		if (!is_real(k)) {
			continue;
		}
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

ChangeType get_keyboard_change(SDL_Keysym keysym) {
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
		return to_limit ? ChangeType::kSetMin : ChangeType::kBigMinus;
	case SDLK_PAGEUP:
		return to_limit ? ChangeType::kSetMax : ChangeType::kBigPlus;
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
