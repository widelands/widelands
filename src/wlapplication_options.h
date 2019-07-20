/*
 * Copyright (C) 2012-2019 by the Widelands Development Team
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

#include "io/profile.h"
#include "wlapplication.h"

/*
 * Further explanations for all functions and its return values
 * can be found in io/profile.cc
 */
class Section;

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
Section& get_config_section(const char*);
Section* get_config_section_ptr(const std::string&);
bool get_config_bool(const char* name, bool dflt);
bool get_config_bool(const char* section, const char* name, bool dflt);
int32_t get_config_int(const char* name, int32_t dflt = 0);
int32_t get_config_int(const char* section, const char* name, int32_t dflt);
uint32_t get_config_natural(const char* name, uint32_t dflt);
uint32_t get_config_natural(const char* section, const char* name, uint32_t dflt);
const char* get_config_string(const char* name, const char* dflt);
const char* get_config_string(const char* name, std::string& dflt);
const char* get_config_string(const char* section,
                              const char* name,
                              const char* dflt);
const char* get_config_string(const char* section,
                              const char* name,
                              std::string& dflt);

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
void set_config_bool(const char* name, bool value);
void set_config_bool(const char* section, const char* name, bool value);
void set_config_int(const char* name, int32_t value);
void set_config_int(const char* section, const char* name, int32_t value);
void set_config_string(const char* name, const char* value);
void set_config_string(const char* name, const std::string& value);
void set_config_string(const char* section, const char* name, const char* value);
void set_config_string(const char* section,
                       const char* name,
                       const std::string& value);

/*
 * Reads the configuration from kConfigFile.
 * Defaults to $XDG_CONFIG_HOME/widelands/config on Unix.
 * Defaults to homedir/config everywhere else, if homedir is set manually or if
 * built without XDG-support.
 *
 * This function needs access to the WLApplication object to distinguish
 * between multiple possible states.
 * While we could use WLApplication::get() this would get us in an infinite loop
 * because this function will be called in the constructor of said object and
 * WLApplication::get() spawns another object if there isn't one yet.
 */
void read_config(WLApplication*);

/*
 * Writes the configuration to kConfigFile.
 * Defaults to $XDG_CONFIG_HOME/widelands/config on Unix.
 * Defaults to homedir/config everywhere else, if homedir is set manually or if
 * built without XDG-support.
 *
 * This function needs access to the WLApplication object to distinguish
 * between multiple possible states.
 * While we could use WLApplication::get() this would get us in an infinite loop
 * because this function will be called in the constructor of said object and
 * WLApplication::get() spawns another object if there isn't one yet.
 */
void write_config(WLApplication*);

#endif // end of include guard: WL_WLAPPLICATION_OPTIONS_H
