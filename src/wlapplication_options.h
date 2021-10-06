/*
 * Copyright (C) 2012-2021 by the Widelands Development Team
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

#ifndef WL_WLAPPLICATION_OPTIONS_H
#define WL_WLAPPLICATION_OPTIONS_H

#include <SDL_keyboard.h>

#include "io/profile.h"

/*
 * Further explanations for all functions and its return values
 * can be found in io/profile.cc
 */

/*
 * Check if all options has been read.
 */
void check_config_used();

/*
 * Get config values from the config settings cache.
 * If the section parameter is omitted the global section will be used.
 * Values are available after read_config() is called.
 */
Section& get_config_section();
Section& get_config_section(const std::string&);
Section* get_config_section_ptr(const std::string&);
bool get_config_bool(const std::string& name, bool dflt);
bool get_config_bool(const std::string& section, const std::string& name, bool dflt);
int32_t get_config_int(const std::string& name, const int32_t dflt = 0);
int32_t get_config_int(const std::string& section, const std::string& name, int32_t dflt);
uint32_t get_config_natural(const std::string& name, uint32_t dflt);
uint32_t
get_config_natural(const std::string& section, const std::string& name, const uint32_t dflt);
std::string get_config_string(const std::string& name, const std::string& dflt);
std::string
get_config_string(const std::string& section, const std::string& name, const std::string& dflt);

/*
 * Get config values from the config settings cache.
 * Throws an exception if the value doesn't exist.
 * If the section parameter is omitted the global section will be used.
 * Values are available after read_config() is called.
 */
Section& get_config_safe_section();
Section& get_config_safe_section(const std::string&);

/*
 * Set config values to the config settings cache.
 * If the section parameter is omitted the global section will be used.
 * Values will be written to the config file once write_config() is called.
 */
void set_config_bool(const std::string& name, bool value);
void set_config_bool(const std::string& section, const std::string& name, bool value);
void set_config_int(const std::string& name, int32_t value);
void set_config_int(const std::string& section, const std::string& name, int32_t value);
void set_config_string(const std::string& name, const std::string& value);
void set_config_string(const std::string& section,
                       const std::string& name,
                       const std::string& value);

// Keyboard shortcuts. The order in which they are defined here
// defines the order in which they appear in the options menu.
enum class KeyboardShortcut : uint16_t {
	k__Begin = 0,

	kMainMenu__Begin = k__Begin,
	kMainMenuSP = kMainMenu__Begin,
	kMainMenuNew,
	kMainMenuRandomMatch,
	kMainMenuCampaign,
	kMainMenuTutorial,
	kMainMenuLoad,
	kMainMenuContinuePlaying,
	kMainMenuMP,
	kMainMenuLobby,
	kMainMenuLogin,
	kMainMenuLAN,
	kMainMenuE,
	kMainMenuEditorNew,
	kMainMenuEditorRandom,
	kMainMenuEditorLoad,
	kMainMenuContinueEditing,
	kMainMenuReplay,
	kMainMenuOptions,
	kMainMenuAddons,
	kMainMenuAbout,
	kMainMenuQuit,
	kMainMenu__End = kMainMenuQuit,

	kCommon__Begin = kMainMenu__End + 1,
	kCommonFullscreen = kCommon__Begin,
	kCommonScreenshot,
	kCommonTextCut,
	kCommonTextCopy,
	kCommonTextPaste,
	kCommonSelectAll,
	kCommonDeleteItem,
	kCommonTooltipAccessibilityMode,
	kCommonEncyclopedia,
	kCommonBuildhelp,
	kCommonMinimap,
	kCommonZoomIn,
	kCommonZoomOut,
	kCommonZoomReset,
	kCommonQuicknavPrev,
	kCommonQuicknavNext,
	kCommon__End = kCommonQuicknavNext,

	kEditor__Begin = kCommon__End + 1,
	kEditorMenu = kEditor__Begin,
	kEditorSave,
	kEditorLoad,
	kEditorUndo,
	kEditorRedo,
	kEditorTools,
	kEditorInfo,
	kEditorPlayers,
	kEditorShowhideGrid,
	kEditorShowhideImmovables,
	kEditorShowhideCritters,
	kEditorShowhideResources,
	kEditorToolsize1,
	kEditorToolsize2,
	kEditorToolsize3,
	kEditorToolsize4,
	kEditorToolsize5,
	kEditorToolsize6,
	kEditorToolsize7,
	kEditorToolsize8,
	kEditorToolsize9,
	kEditorToolsize10,
	kEditor__End = kEditorToolsize10,

	kInGame__Begin = kEditor__End + 1,
	kInGameSave = kInGame__Begin,
	kInGameLoad,
	kInGameChat,
	kInGameMessages,
	kInGameObjectives,
	kInGameShowhideCensus,
	kInGameShowhideStats,
	kInGameShowhideSoldiers,
	kInGameShowhideBuildings,
	kInGameShowhideWorkareas,
	kInGameStatsGeneral,
	kInGameStatsWares,
	kInGameStatsBuildings,
	kInGameStatsStock,
	kInGameStatsSoldiers,
	kInGameStatsSeafaring,
	kInGamePause,
	kInGameSpeedUp,
	kInGameSpeedUpSlow,
	kInGameSpeedUpFast,
	kInGameSpeedDown,
	kInGameSpeedDownSlow,
	kInGameSpeedDownFast,
	kInGameSpeedReset,
	kInGameScrollToHQ,
	kInGameMessagesGoto,
	kInGameMessagesFilterAll,
	kInGameMessagesFilterGeologists,
	kInGameMessagesFilterEconomy,
	kInGameMessagesFilterSeafaring,
	kInGameMessagesFilterWarfare,
	kInGameMessagesFilterScenario,
	kInGameSeafaringstatsGotoShip,
	kInGameSeafaringstatsWatchShip,
	kInGameSeafaringstatsOpenShipWindow,
	kInGameSeafaringstatsOpenShipWindowAndGoto,
	kInGameSeafaringstatsFilterAll,
	kInGameSeafaringstatsFilterIdle,
	kInGameSeafaringstatsFilterShipping,
	kInGameSeafaringstatsFilterExpWait,
	kInGameSeafaringstatsFilterExpScout,
	kInGameSeafaringstatsFilterExpPortspace,
	kInGameQuicknavSet1,
	kInGameQuicknavGoto1,
	kInGameQuicknavSet2,
	kInGameQuicknavGoto2,
	kInGameQuicknavSet3,
	kInGameQuicknavGoto3,
	kInGameQuicknavSet4,
	kInGameQuicknavGoto4,
	kInGameQuicknavSet5,
	kInGameQuicknavGoto5,
	kInGameQuicknavSet6,
	kInGameQuicknavGoto6,
	kInGameQuicknavSet7,
	kInGameQuicknavGoto7,
	kInGameQuicknavSet8,
	kInGameQuicknavGoto8,
	kInGameQuicknavSet9,
	kInGameQuicknavGoto9,
	kInGame__End = kInGameQuicknavGoto9,

	kFastplace__Begin = kInGame__End + 1,
	kFastplace__End = kFastplace__Begin + 127,  // Arbitrary limit of 128 fastplace shortcuts.

	k__End = kFastplace__End
};
bool set_shortcut(KeyboardShortcut, SDL_Keysym, KeyboardShortcut* conflict);
SDL_Keysym get_shortcut(KeyboardShortcut);
SDL_Keysym get_default_shortcut(KeyboardShortcut);
void normalize_numpad(SDL_Keysym&);
uint16_t normalize_keymod(uint16_t keymod);
bool matches_keymod(uint16_t, uint16_t);
bool matches_shortcut(KeyboardShortcut, SDL_Keysym);
bool matches_shortcut(KeyboardShortcut, SDL_Keycode, int modifiers);
std::string matching_fastplace_shortcut(SDL_Keysym);
void init_shortcuts(bool force_defaults = false);
std::string to_string(KeyboardShortcut);
KeyboardShortcut shortcut_from_string(const std::string&);

// Return value will either be an empty string or have a trailing "+"
std::string keymod_string_for(const uint16_t modstate, const bool rt_escape = true);

std::string shortcut_string_for(SDL_Keysym, bool rt_escape = true);
std::string shortcut_string_for(KeyboardShortcut, bool rt_escape = true);
void set_fastplace_shortcut(KeyboardShortcut, const std::string& building);
const std::string& get_fastplace_shortcut(KeyboardShortcut);

// Return values for changing value of spinbox, slider, etc.
enum class ChangeType : int32_t {
	kSetMin = std::numeric_limits<int32_t>::min(),  // set value to minimum
	                                                //     -- keys: Home, Ctrl + decrease keys
	kBigMinus = -10,                                // decrease by big step -- key: PageDown
	kMinus = -1,                                    // decrease  -- keys: Left, Down, Minus
	kNone = 0,                                      // no change -- all other keys
	kPlus = 1,                                      // increase  -- keys: Right, Up, Plus
	kBigPlus = 10,                                  // increase by big step -- key: PageUp
	kSetMax = std::numeric_limits<int32_t>::max()   // set value to maximum
	                                                //     -- keys: End, Ctrl + increase keys
};

// Helper function for spinbox, slider, etc. handle_key(...)
ChangeType get_keyboard_change(SDL_Keysym, bool enable_big_step = false);

/*
 * Sets the directory where to read/write kConfigFile.
 */
void set_config_directory(const std::string& userconfigdir);
const std::string& get_config_file();

/*
 * Reads the configuration from kConfigFile.
 * Assumes that set_config_directory has been called.
 */
void read_config();

/*
 * Writes the configuration to kConfigFile.
 * * Assumes that set_config_directory has been called.
 */
void write_config();

#endif  // end of include guard: WL_WLAPPLICATION_OPTIONS_H
