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

#include "wlapplication_options.h"

#include "base/log.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "wlapplication.h"

Profile g_options(Profile::err_log);

void check_config_used() {
    g_options.check_used();
}

Section& get_config_section() {
    return g_options.pull_section("global");
}

Section& get_config_section(const char* section) {
    return g_options.pull_section(section);
}

Section* get_config_section_ptr(const std::string& section) {
    return g_options.get_section(section);
}

bool get_config_bool(const char* name, bool dflt) {
    return g_options.pull_section("global").get_bool(name, dflt);
}

bool get_config_bool(const char* section, const char* name, bool dflt) {
    return g_options.pull_section(section).get_bool(name, dflt);
}

int32_t get_config_int(const char* name, int32_t dflt) {
    return g_options.pull_section("global").get_int(name, dflt);
}

int32_t get_config_int(const char* section, const char* name, int32_t dflt) {
    return g_options.pull_section(section).get_int(name, dflt);
}

uint32_t get_config_natural(const char* name, uint32_t dflt) {
    return g_options.pull_section("global").get_natural(name, dflt);
}

uint32_t get_config_natural(const char* section,
                            const char* name,
                            uint32_t dflt) {
    return g_options.pull_section(section).get_natural(name, dflt);
}

const char* get_config_string(const char* name, const char* dflt) {
    return g_options.pull_section("global").get_string(name, dflt);
}

const char* get_config_string(const char* name, std::string& dflt) {
    return g_options.pull_section("global").get_string(name, dflt.c_str());
}

const char* get_config_string(const char* section,
                              const char* name,
                              const char* dflt) {
    return g_options.pull_section(section).get_string(name, dflt);
}

const char* get_config_string(const char* section,
                              const char* name,
                              std::string& dflt) {
    return g_options.pull_section(section).get_string(name, dflt.c_str());
}

Section& get_config_safe_section() {
    return g_options.get_safe_section("global");
}

Section& get_config_safe_section(const std::string& section) {
    return g_options.get_safe_section(section);
}

void set_config_bool(const char* name, bool value) {
    g_options.pull_section("global").set_bool(name, value);
}

void set_config_bool(const char* section, const char* name, bool value) {
    g_options.pull_section(section).set_bool(name, value);
}

void set_config_int(const char* name, int32_t value) {
    g_options.pull_section("global").set_int(name, value);
}

void set_config_int(const char* section, const char* name, int32_t value) {
    g_options.pull_section(section).set_int(name, value);
}

void set_config_string(const char* name, const char* value) {
    g_options.pull_section("global").set_string(name, value);
}

void set_config_string(const char* section,
                       const char* name,
                       const char* value) {
    g_options.pull_section(section).set_string(name, value);
}

void set_config_string(const char* name, const std::string& value) {
    set_config_string(name, value.c_str());
}

void set_config_string(const char* section,
                       const char* name,
                       const std::string& value) {
    set_config_string(section, name, value.c_str());
}

void read_config(WLApplication* wlapplication) {
#ifdef USE_XDG
    RealFSImpl dir(wlapplication->get_userconfigdir());
    dir.ensure_directory_exists(".");
    log("Set configuration file: %s/%s\n",
        wlapplication->get_userconfigdir().c_str(),
        kConfigFile.c_str());
    g_options.read(kConfigFile.c_str(), "global", dir);
#else
    g_options.read(kConfigFile.c_str(), "global");
#endif
}

void write_config(WLApplication* wlapplication) {
    try {  //  overwrite the old config file
#ifdef USE_XDG
            RealFSImpl dir(wlapplication->get_userconfigdir());
            g_options.write(kConfigFile.c_str(), true, dir);
#else
            g_options.write(kConfigFile.c_str(), true);
#endif
    } catch (const std::exception& e) {
        log("WARNING: could not save configuration: %s\n", e.what());
    } catch (...) {
        log("WARNING: could not save configuration");
    }
}
