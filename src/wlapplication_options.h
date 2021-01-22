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

	kGeneralGame__Begin = kMainMenu__End + 1,
	kGeneralGameBuildhelp = kGeneralGame__Begin,
	kGeneralGameMinimap,
	kGeneralGame__End = kGeneralGameMinimap,

	k__End = kGeneralGame__End,
};
bool set_shortcut(KeyboardShortcut, SDL_Keysym, KeyboardShortcut* conflict);
SDL_Keysym get_shortcut(KeyboardShortcut);
SDL_Keysym get_default_shortcut(KeyboardShortcut);
bool matches_shortcut(KeyboardShortcut, SDL_Keysym);
void init_shortcuts(bool force_defaults = false);
std::string to_string(KeyboardShortcut);
std::string shortcut_string_for(SDL_Keysym);
std::string shortcut_string_for(KeyboardShortcut);

/*
 * Sets the directory where to read/write kConfigFile.
 */
void set_config_directory(const std::string& userconfigdir);

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
