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

#ifndef WL_WLAPPLICATION_OPTIONS_H
#define WL_WLAPPLICATION_OPTIONS_H

#include <map>

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

static const std::string kFastplaceGroupPrefix = "fastplace_";

// Keyboard shortcuts. The order in which they are defined here
// defines the order in which they appear in the options menu.
enum class KeyboardShortcut : uint16_t {
	k_Begin = 0,

	kMainMenu_Begin = k_Begin,
	kMainMenuSP = kMainMenu_Begin,
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
	kMainMenu_End = kMainMenuQuit,

	kCommon_Begin = kMainMenu_End + 1,
	kCommonFullscreen = kCommon_Begin,
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
	kCommon_End = kCommonQuicknavNext,

	kEditor_Begin = kCommon_End + 1,
	kEditorMenu = kEditor_Begin,
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
	kEditor_End = kEditorToolsize10,

	kInGame_Begin = kEditor_End + 1,
	kInGameSave = kInGame_Begin,
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
	kInGame_End = kInGameQuicknavGoto9,

	kFastplace_Begin = kInGame_End + 1,
	kFastplace_End = kFastplace_Begin + 127,  // Arbitrary limit of 128 fastplace shortcuts.

	k_End = kFastplace_End
};

/** Check whether a given shortcut is reserved for a fastplace shortcut slot. */
inline bool is_fastplace(const KeyboardShortcut id) {
	return id >= KeyboardShortcut::kFastplace_Begin && id <= KeyboardShortcut::kFastplace_End;
}

/**
 * Change a keyboard shortcut.
 * @param id ID of the shortcut to change.
 * @param code New keysym to use. Ignored when setting a fastplace shortcut to \c "".
 * @param conflict If not \c nullptr and a conflict occurs, this will
 *                 be filled in with the conflicting shortcut's ID.
 * @return The shortcut was changed successfully. If \c false, #conflict will contain the reason.
 */
bool set_shortcut(KeyboardShortcut id, SDL_Keysym code, KeyboardShortcut* conflict);

/** Look up the keysym assigned to a given shortcut ID. */
SDL_Keysym get_shortcut(KeyboardShortcut);

/** Look up the hardcoded default keysym for a given shortcut ID. */
SDL_Keysym get_default_shortcut(KeyboardShortcut);

/** Replace numpad keysyms with their non-numpad equivalents. */
void normalize_numpad(SDL_Keysym&);

/**
 * Filter out all modifiers we are not interested in as well as left/right information.
 * @param keymod Modifier bitset to normalize.
 * @return #KMOD_NONE or a bitset of #KMOD_CTRL, #KMOD_SHIFT, #KMOD_ALT, and #KMOD_GUI.
 */
uint16_t normalize_keymod(uint16_t keymod);

/** Check if the two modifier bitsets match each other. */
bool matches_keymod(uint16_t, uint16_t);

/** Check if the given keysym should trigger the given shortcut. */
bool matches_shortcut(KeyboardShortcut, SDL_Keysym);
bool matches_shortcut(KeyboardShortcut, SDL_Keycode, int modifiers);

/** Look up the fastplace building assigned to a given shortcut. May return \c "". */
std::string matching_fastplace_shortcut(SDL_Keysym, const std::string& tribename);

/** Read all shortcuts from the config file, or replace all mappings with the default values. */
void init_shortcuts(bool force_defaults = false);

/** The human-readable name of a shortcut identifier. */
std::string to_string(KeyboardShortcut);

/** Get the shortcut ID from an internal shortcut name. Throws an exception for invalid names. */
KeyboardShortcut shortcut_from_string(const std::string&);

/**
 * Generate a human-readable description of a keyboard shortcut.
 * Return value will either be an empty string or have a trailing "+".
 */
std::string keymod_string_for(const uint16_t modstate, const bool rt_escape = true);
std::string shortcut_string_for(SDL_Keysym, bool rt_escape = true);
std::string shortcut_string_for(KeyboardShortcut, bool rt_escape = true);

/** Set or get each tribe's fastplace building for a given fastplace group. */
void set_fastplace_shortcuts(KeyboardShortcut, const std::map<std::string, std::string>&);
const std::map<std::string, std::string>& get_fastplace_shortcuts(KeyboardShortcut);
const std::string& get_fastplace_group_name(KeyboardShortcut);

/** Initialize all fastplace group definitions, but do not overwrite existing mappings. */
void init_fastplace_default_shortcuts(
   const std::map<std::string /* key */,
                  std::map<std::string /* tribe */, std::string /* building */>>&);

/** Clear a shortcut. */
void unset_shortcut(KeyboardShortcut);

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
