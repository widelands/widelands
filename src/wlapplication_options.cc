/*
 * Copyright (C) 2012-2020 by the Widelands Development Team
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

#include <cassert>
#include <cstdlib>
#include <map>
#include <memory>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "io/filesystem/disk_filesystem.h"
#include "logic/filesystem_constants.h"

static Profile g_options(Profile::err_log);

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

static std::map<KeyboardShortcut, SDL_Keysym> shortcuts_;
static void write_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	set_config_int("keyboard", std::to_string(static_cast<int>(id)) + "sym", code.sym);
	set_config_int("keyboard", std::to_string(static_cast<int>(id)) + "mod", code.mod);
}
void set_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	shortcuts_.at(id) = code;
	write_shortcut(id, code);
}

SDL_Keysym get_shortcut(const KeyboardShortcut id) {
	return shortcuts_.at(id);
}

bool matches_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	const SDL_Keysym key = get_shortcut(id);
	if (key.sym != code.sym) {
		return false;
	}

	const bool ctrl1 = code.mod & KMOD_CTRL;
	const bool shift1 = code.mod & KMOD_SHIFT;
	const bool alt1 = code.mod & KMOD_ALT;
	const bool ctrl2 = code.mod & KMOD_CTRL;
	const bool shift2 = code.mod & KMOD_SHIFT;
	const bool alt2 = code.mod & KMOD_ALT;

	return ctrl1 == ctrl2 && shift1 == shift2 && alt1 == alt2;
}

std::string shortcut_string_for(const KeyboardShortcut id) {
	const SDL_Keysym sym = get_shortcut(id);

	std::vector<std::string> mods;
	if (sym.mod & KMOD_CTRL) {
		mods.push_back(pgettext("hotkey", "Ctrl"));
	}
	if (sym.mod & KMOD_SHIFT) {
		mods.push_back(pgettext("hotkey", "Shift"));
	}
	if (sym.mod & KMOD_ALT) {
		mods.push_back(pgettext("hotkey", "Alt"));
	}

	std::string fmt;
	switch (mods.size()) {
	case 0: fmt = _("%1$s"); break;
	case 1: fmt = _("%1$s+%2$s"); break;
	case 2: fmt = _("%1$s+%2$s+%3$s"); break;
	case 3: fmt = _("%1$s+%2$s+%3$s+%4$s"); break;
	default: NEVER_HERE();
	}
	boost::format f(fmt);
	for (const std::string& m : mods) {
		f % m;
	}
	f % SDL_GetKeyName(sym.sym);
	return f.str();
}

std::string to_string(const KeyboardShortcut id) {
	switch (id) {
	case KeyboardShortcut::kMainMenuNew: return _("New Game");
	case KeyboardShortcut::kMainMenuLoad: return _("Load Game");
	case KeyboardShortcut::kMainMenuReplay: return _("Watch Replay");
	case KeyboardShortcut::kMainMenuRandomMatch: return _("New Random Game");
	case KeyboardShortcut::kMainMenuTutorial: return _("Play Tutorial");
	case KeyboardShortcut::kMainMenuCampaign: return _("Campaign");
	case KeyboardShortcut::kMainMenuOptions: return _("Options");
	case KeyboardShortcut::kMainMenuAbout: return _("About");
	case KeyboardShortcut::kMainMenuAddons: return _("Add-Ons");
	case KeyboardShortcut::kMainMenuContinuePlaying: return _("Continue Playing");
	case KeyboardShortcut::kMainMenuContinueEditing: return _("Continue Editing");
	case KeyboardShortcut::kMainMenuEditorNew: return _("Editor – New Map");
	case KeyboardShortcut::kMainMenuEditorLoad: return _("Editor – Load Map");
	case KeyboardShortcut::kMainMenuEditorRandom: return _("Editor – New Random Map");
	case KeyboardShortcut::kMainMenuLogin: return _("Internet Login");
	case KeyboardShortcut::kMainMenuLobby: return _("Online Game Lobby");
	case KeyboardShortcut::kMainMenuLAN: return _("LAN Game");
	case KeyboardShortcut::kMainMenuQuit: return _("Exit Widelands");
	case KeyboardShortcut::kMainMenuSP: return _("Singleplayer");
	case KeyboardShortcut::kMainMenuMP: return _("Multiplayer");
	case KeyboardShortcut::kMainMenuE: return _("Editor");
	case KeyboardShortcut::kGeneralGameBuildhelp: return _("Toggle Buildhelp");
	case KeyboardShortcut::kGeneralGameMinimap: return _("Toggle Minimap");
	}
	NEVER_HERE();
}

#define KEYSYM(x) SDL_Keysym { SDL_GetScancodeFromKey(SDLK_##x), SDLK_##x, 0, 0 }

SDL_Keysym get_default_shortcut(const KeyboardShortcut id) {
	switch (id) {
	case KeyboardShortcut::kMainMenuNew: return KEYSYM(n);
	case KeyboardShortcut::kMainMenuLoad: return KEYSYM(l);
	case KeyboardShortcut::kMainMenuReplay: return KEYSYM(r);
	case KeyboardShortcut::kMainMenuRandomMatch: return KEYSYM(z);
	case KeyboardShortcut::kMainMenuTutorial: return KEYSYM(t);
	case KeyboardShortcut::kMainMenuCampaign: return KEYSYM(h);
	case KeyboardShortcut::kMainMenuOptions: return KEYSYM(o);
	case KeyboardShortcut::kMainMenuAbout: return KEYSYM(F1);
	case KeyboardShortcut::kMainMenuAddons: return KEYSYM(a);
	case KeyboardShortcut::kMainMenuContinuePlaying: return KEYSYM(c);
	case KeyboardShortcut::kMainMenuContinueEditing: return KEYSYM(w);
	case KeyboardShortcut::kMainMenuEditorNew: return KEYSYM(k);
	case KeyboardShortcut::kMainMenuEditorLoad: return KEYSYM(b);
	case KeyboardShortcut::kMainMenuEditorRandom: return KEYSYM(y);
	case KeyboardShortcut::kMainMenuLogin: return KEYSYM(u);
	case KeyboardShortcut::kMainMenuLobby: return KEYSYM(j);
	case KeyboardShortcut::kMainMenuLAN: return KEYSYM(p);
	case KeyboardShortcut::kMainMenuQuit: return KEYSYM(ESCAPE);
	case KeyboardShortcut::kMainMenuSP: return KEYSYM(s);
	case KeyboardShortcut::kMainMenuMP: return KEYSYM(m);
	case KeyboardShortcut::kMainMenuE: return KEYSYM(e);
	case KeyboardShortcut::kGeneralGameBuildhelp: return KEYSYM(SPACE);
	case KeyboardShortcut::kGeneralGameMinimap: return KEYSYM(m);
	}
	NEVER_HERE();
}

#undef KEYSYM

void init_shortcuts(const bool force_defaults) {
	for (KeyboardShortcut k = KeyboardShortcut::k__Begin; k <= KeyboardShortcut::k__End; k = static_cast<KeyboardShortcut>(static_cast<uint16_t>(k) + 1)) {
		shortcuts_[k] = get_default_shortcut(k);
		if (force_defaults) {
			write_shortcut(k, shortcuts_.at(k));
		}
	}
	if (force_defaults) {
		return;
	}

	Section& s = get_config_section("keyboard");
	while (Section::Value* v = s.get_next_val()) {
		char* type;
		const long i = std::strtol(v->get_name(), &type, 10);
		if (strcmp(type, "sym") == 0) {
			shortcuts_.at(static_cast<KeyboardShortcut>(i)).sym = v->get_int();
		} else if (strcmp(type, "mod") == 0) {
			shortcuts_.at(static_cast<KeyboardShortcut>(i)).mod = v->get_int();
		} else {
			log_err("Ignoring malformed hotkey in config: '%s = %s'", v->get_name(), v->get_string());
		}
	}
}

void set_config_directory(const std::string& userconfigdir) {
	config_dir.reset(new RealFSImpl(userconfigdir));
	config_dir->ensure_directory_exists(".");
	log_info("Set configuration file: %s/%s\n", userconfigdir.c_str(), kConfigFile.c_str());
}

void read_config() {
	assert(config_dir != nullptr);
	g_options.read(kConfigFile.c_str(), "global", *config_dir);
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
