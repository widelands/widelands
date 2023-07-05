/*
 * Copyright (C) 2012-2023 by the Widelands Development Team
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
#include <vector>

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
int32_t get_config_int(const std::string& name, int32_t dflt = 0);
int32_t get_config_int(const std::string& section, const std::string& name, int32_t dflt);
uint32_t get_config_natural(const std::string& name, uint32_t dflt);
uint32_t get_config_natural(const std::string& section, const std::string& name, uint32_t dflt);
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

	// These will get different descriptions for the in-game and in-editor help
	kCommonLoad = kCommon_Begin,
	kCommonSave,
	kCommonExit,
	kCommonEncyclopedia,

	// These can be inserted as one block in the in-game and in-editor help
	kCommonGeneral_Begin,
	kCommonZoomIn = kCommonGeneral_Begin,
	kCommonZoomOut,
	kCommonZoomReset,
	kCommonTextCut,
	kCommonTextCopy,
	kCommonTextPaste,
	kCommonSelectAll,
	kCommonDeleteItem,
	kCommonTooltipAccessibilityMode,
	kCommonFullscreen,
	kCommonScreenshot,
	kCommonGeneral_End = kCommonScreenshot,

	// These are only shown in the help in debug builds
	kCommonDebugConsole,
	kCommonCheatMode,

	// These will be moved to related items in the in-game and in-editor help
	kCommonBuildhelp,
	kCommonMinimap,
	kCommonQuicknavPrev,
	kCommonQuicknavNext,
	kCommon_End = kCommonQuicknavNext,

	kEditor_Begin = kCommon_End + 1,
	kEditorNewMap = kEditor_Begin,
	kEditorNewRandomMap,
	kEditorLoad,  // alias of kCommonLoad
	kEditorSave,  // alias of kCommonSave
	kEditorUploadMap,
	kEditorMapOptions,
	kEditorExit,               // alias of kCommonExit
	kEditorHelp,               // alias of kCommonEncyclopedia
	kEditorShowhideBuildhelp,  // alias of kCommonBuildhelp
	kEditorShowhideMaximumBuildhelp,
	kEditorShowhideGrid,
	kEditorShowhideImmovables,
	kEditorShowhideCritters,
	kEditorShowhideResources,
	kEditorMinimap,           // alias of kCommonMinimap
	kEditor_Special_MapMove,  // special entries for map movement with arrow keys
	kEditorQuicknavPrev,      // alias of kCommonQuicknavPrev
	kEditorQuicknavNext,      // alias of kCommonQuicknavNext
	kEditorMain_End = kEditorQuicknavNext,

	// This will be a separate section in the help
	kEditorTools_Begin = kEditorMain_End + 1,
	kEditorTools = kEditorTools_Begin,
	kEditorChangeHeight,
	kEditorRandomHeight,
	kEditorTerrain,
	kEditorImmovables,
	kEditorAnimals,
	kEditorResources,
	kEditorPortSpaces,
	kEditorInfo,
	kEditorMapOrigin,
	kEditorMapSize,
	kEditorPlayers,
	kEditorToolHistory,
	kEditorUndo,
	kEditorRedo,
	kEditorTools_End = kEditorRedo,

	// These will be grouped to one line in the in-editor help
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

	kEditorToolgap0,
	kEditorToolgap10,
	kEditorToolgap20,
	kEditorToolgap30,
	kEditorToolgap40,
	kEditorToolgap50,
	kEditorToolgap60,
	kEditorToolgap70,
	kEditorToolgap80,
	kEditorToolgap90,

	kEditor_End = kEditorToolgap90,

	kInGame_Begin = kEditor_End + 1,
	kInGameSoundOptions = kInGame_Begin,
	kInGameSave,  // alias of kCommonSave
	kInGameLoad,  // alias of kCommonLoad
	kInGameRestart,
	kInGameExit,          // alias of kCommonExit
	kInGameEncyclopedia,  // alias of kCommonEncyclopedia

	kInGameMessages,
	kInGameObjectives,
	kInGameDiplomacy,
	kInGameChat,

	kInGameStatsGeneral,
	kInGameStatsWares,
	kInGameStatsBuildings,
	kInGameStatsStock,
	kInGameStatsSoldiers,
	kInGameStatsSeafaring,

	kInGameSpeedUp,
	kInGameSpeedUpSlow,
	kInGameSpeedUpFast,
	kInGameSpeedDown,
	kInGameSpeedDownSlow,
	kInGameSpeedDownFast,
	kInGamePause,
	kInGameSpeedReset,

	kInGameShowhideBuildhelp,  // alias of kCommonBuildhelp
	kInGameShowhideCensus,
	kInGameShowhideStats,
	kInGameShowhideSoldiers,
	kInGameShowhideBuildings,
	kInGameShowhideWorkareas,

	kInGameMinimap,           // alias of kCommonMinimap
	kInGame_Special_MapMove,  // special entries for map movement with arrow keys
	kInGameScrollToHQ,
	kInGameQuicknavPrev,       // alias of kCommonQuicknavPrev
	kInGameQuicknavNext,       // alias of kCommonQuicknavNext
	kInGame_Special_Quicknav,  // special entries with grouped quicknav set/goto shortcuts
	kInGameQuicknavGUI,
	kInGamePinnedNote,
	kInGameMain_End = kInGamePinnedNote,

	// These will be grouped to two lines in the Encyclopedia
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

	// These have their own sections in the Encyclopedia

	kInGameMessages_Begin,
	kInGameMessagesFilterAll = kInGameMessages_Begin,
	kInGameMessagesFilterGeologists,
	kInGameMessagesFilterEconomy,
	kInGameMessagesFilterSeafaring,
	kInGameMessagesFilterWarfare,
	kInGameMessagesFilterScenario,
	kInGameMessagesGoto,
	kInGameMessages_End = kInGameMessagesGoto,

	kInGameSeafaringstats_Begin = kInGameMessages_End + 1,
	kInGameSeafaringstatsFilterAll = kInGameSeafaringstats_Begin,
	kInGameSeafaringstatsFilterIdle,
	kInGameSeafaringstatsFilterShipping,
	kInGameSeafaringstatsFilterExpWait,
	kInGameSeafaringstatsFilterExpScout,
	kInGameSeafaringstatsFilterExpPortspace,
	kInGameSeafaringstatsGotoShip,
	kInGameSeafaringstatsWatchShip,
	kInGameSeafaringstatsOpenShipWindow,
	kInGameSeafaringstatsOpenShipWindowAndGoto,
	kInGameSeafaringstats_End = kInGameSeafaringstatsOpenShipWindowAndGoto,

	kInGame_End = kInGameSeafaringstats_End,

	kFastplace_Begin = kInGame_End + 1,
	kFastplace_End = kFastplace_Begin + 127,  // Arbitrary limit of 128 fastplace shortcuts.

	k_End = kFastplace_End
};

KeyboardShortcut operator+(const KeyboardShortcut& id, int i);
KeyboardShortcut& operator++(KeyboardShortcut& id);
uint16_t operator-(const KeyboardShortcut& a, const KeyboardShortcut& b);

/**
 * Check whether the given shortcut can be used for setting and retrieving the
 * actual key combination.
 *
 * This function should be used when iterating over KeyboardShortcut ranges.
 *
 * Not "real" shortcuts are aliases and special entries, that are only used for
 * help generation to simplify inserting common shortcuts and other related
 * help entries at the right place.
 */
bool is_real(KeyboardShortcut id);

bool is_debug_only(KeyboardShortcut id);

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

/** Get the formatted help of the current in game keyboard shortcuts including headers. */
std::string get_ingame_shortcut_help();

struct FastplaceShortcut {
	std::string hotkey;
	std::string building;
};

/** Get the current fastplace shortcuts for tribe. */
std::vector<FastplaceShortcut> get_active_fastplace_shortcuts(const std::string& tribe);

/** Get the formatted help of the current editor keyboard shortcuts including headers. */
std::string get_editor_shortcut_help();

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
std::string keymod_string_for(uint16_t modstate, bool rt_escape = true);
std::string shortcut_string_for(SDL_Keysym, bool rt_escape);
std::string shortcut_string_for(KeyboardShortcut, bool rt_escape);

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

// Default step sizes for changing value of spinbox, slider, etc. on PgUp/PgDown or Ctrl+mousewheel
namespace ChangeBigStep {
constexpr int32_t kSmallRange = 3;
constexpr int32_t kMediumRange = 5;
constexpr int32_t kWideRange = 10;
}  // namespace ChangeBigStep

// Return values for changing value of spinbox, slider, etc.
enum class ChangeType : int32_t {
	kSetMin = std::numeric_limits<int32_t>::min(),  // set value to minimum
	                                                //     -- keys: Home, Ctrl + decrease keys
	kBigMinus = -ChangeBigStep::kWideRange,         // decrease by big step -- key: PageDown
	kMinus = -1,                                    // decrease  -- keys: Left, Down, Minus
	kNone = 0,                                      // no change -- all other keys
	kPlus = 1,                                      // increase  -- keys: Right, Up, Plus
	kBigPlus = ChangeBigStep::kWideRange,           // increase by big step -- key: PageUp
	kSetMax = std::numeric_limits<int32_t>::max()   // set value to maximum
	                                                //     -- keys: End, Ctrl + increase keys
};

// Helper function for spinbox, slider, etc. handle_key(...)
ChangeType get_keyboard_change(SDL_Keysym);

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
