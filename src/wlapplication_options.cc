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

struct KeyboardShortcutInfo {
	enum class Scope {
		kMainMenu,
		kGameAndEditor,
	};

	const Scope scope;
	const SDL_Keysym default_shortcut;
	SDL_Keysym current_shortcut;
	const std::string internal_name;
	const std::function<std::string()> descname;

	KeyboardShortcutInfo(Scope s,
	                     const SDL_Keysym& sym,
	                     const std::string& n,
	                     const std::function<std::string()>& f)
	   : scope(s), default_shortcut(sym), current_shortcut(sym), internal_name(n), descname(f) {
	}
};

static inline SDL_Keysym keysym(const SDL_Keycode c) {
	return SDL_Keysym{SDL_GetScancodeFromKey(c), c, 0, 0};
}

static std::map<KeyboardShortcut, KeyboardShortcutInfo> shortcuts_ = {
   {KeyboardShortcut::kMainMenuNew, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                         keysym(SDLK_n),
                                                         "mainmenu_new",
                                                         []() { return _("New Game"); })},
   {KeyboardShortcut::kMainMenuLoad, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                          keysym(SDLK_l),
                                                          "mainmenu_load",
                                                          []() { return _("Load Game"); })},
   {KeyboardShortcut::kMainMenuReplay, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                            keysym(SDLK_r),
                                                            "mainmenu_replay",
                                                            []() { return _("Watch Replay"); })},
   {KeyboardShortcut::kMainMenuRandomMatch,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_z),
                         "mainmenu_random",
                         []() { return _("New Random Game"); })},
   {KeyboardShortcut::kMainMenuTutorial,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_t),
                         "mainmenu_tutorial",
                         []() { return _("Tutorials"); })},
   {KeyboardShortcut::kMainMenuCampaign,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_c),
                         "mainmenu_campaign",
                         []() { return _("Campaigns"); })},
   {KeyboardShortcut::kMainMenuSP, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                        keysym(SDLK_s),
                                                        "mainmenu_sp",
                                                        []() { return _("Singleplayer"); })},
   {KeyboardShortcut::kMainMenuMP, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                        keysym(SDLK_m),
                                                        "mainmenu_mp",
                                                        []() { return _("Multiplayer"); })},
   {KeyboardShortcut::kMainMenuE, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                       keysym(SDLK_e),
                                                       "mainmenu_e",
                                                       []() { return _("Editor"); })},
   {KeyboardShortcut::kMainMenuEditorLoad,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_b),
                         "mainmenu_editor_load",
                         []() { return _("Editor – Load Map"); })},
   {KeyboardShortcut::kMainMenuEditorNew,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_k),
                         "mainmenu_editor_new",
                         []() { return _("Editor – New Map"); })},
   {KeyboardShortcut::kMainMenuEditorRandom,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_y),
                         "mainmenu_editor_random",
                         []() { return _("Editor – New Random Map"); })},
   {KeyboardShortcut::kMainMenuContinueEditing,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_w),
                         "mainmenu_editor_continue",
                         []() { return _("Continue Editing"); })},
   {KeyboardShortcut::kMainMenuContinuePlaying,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                         keysym(SDLK_c),
                         "mainmenu_continue",
                         []() { return _("Continue Playing"); })},
   {KeyboardShortcut::kMainMenuQuit, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                          keysym(SDLK_ESCAPE),
                                                          "mainmenu_quit",
                                                          []() { return _("Exit Widelands"); })},
   {KeyboardShortcut::kMainMenuAbout, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                           keysym(SDLK_F1),
                                                           "mainmenu_about",
                                                           []() { return _("About"); })},
   {KeyboardShortcut::kMainMenuAddons, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                            keysym(SDLK_a),
                                                            "mainmenu_addons",
                                                            []() { return _("Add-Ons"); })},
   {KeyboardShortcut::kMainMenuLAN, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                         keysym(SDLK_p),
                                                         "mainmenu_lan",
                                                         []() { return _("LAN / Direct IP"); })},
   {KeyboardShortcut::kMainMenuLobby, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                           keysym(SDLK_j),
                                                           "mainmenu_lobby",
                                                           []() { return _("Metaserver Lobby"); })},
   {KeyboardShortcut::kMainMenuLogin, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                           keysym(SDLK_u),
                                                           "mainmenu_login",
                                                           []() { return _("Internet Login"); })},
   {KeyboardShortcut::kMainMenuOptions, KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kMainMenu,
                                                             keysym(SDLK_o),
                                                             "mainmenu_options",
                                                             []() { return _("Options"); })},
   {KeyboardShortcut::kGeneralGameBuildhelp,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kGameAndEditor,
                         keysym(SDLK_SPACE),
                         "buildhelp",
                         []() { return _("Toggle Buildhelp"); })},
   {KeyboardShortcut::kGeneralGameMinimap,
    KeyboardShortcutInfo(KeyboardShortcutInfo::Scope::kGameAndEditor,
                         keysym(SDLK_m),
                         "minimap",
                         []() { return _("Toggle Minimap"); })},
};

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
bool set_shortcut(const KeyboardShortcut id, const SDL_Keysym code, KeyboardShortcut* conflict) {
	const KeyboardShortcutInfo::Scope& scope = shortcuts_.at(id).scope;
	for (auto& pair : shortcuts_) {
		if (pair.first != id && pair.second.scope == scope && matches_shortcut(pair.first, code)) {
			*conflict = pair.first;
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

bool matches_shortcut(const KeyboardShortcut id, const SDL_Keysym code) {
	const SDL_Keysym key = get_shortcut(id);
	if (key.sym != code.sym) {
		return false;
	}

	const bool ctrl1 = key.mod & KMOD_CTRL;
	const bool shift1 = key.mod & KMOD_SHIFT;
	const bool alt1 = key.mod & KMOD_ALT;
	const bool gui1 = key.mod & KMOD_GUI;
	const bool ctrl2 = code.mod & KMOD_CTRL;
	const bool shift2 = code.mod & KMOD_SHIFT;
	const bool alt2 = code.mod & KMOD_ALT;
	const bool gui2 = code.mod & KMOD_GUI;

	return ctrl1 == ctrl2 && shift1 == shift2 && alt1 == alt2 && gui1 == gui2;
}

std::string shortcut_string_for(const KeyboardShortcut id) {
	return shortcut_string_for(get_shortcut(id));
}

std::string shortcut_string_for(const SDL_Keysym sym) {
	std::vector<std::string> mods;
	if (sym.mod & KMOD_SHIFT) {
		mods.push_back(pgettext("hotkey", "Shift"));
	}
	if (sym.mod & KMOD_ALT) {
		mods.push_back(pgettext("hotkey", "Alt"));
	}
	if (sym.mod & KMOD_GUI) {
		mods.push_back(pgettext("hotkey", "GUI"));
	}
	if (sym.mod & KMOD_CTRL) {
		mods.push_back(pgettext("hotkey", "Ctrl"));
	}

	std::string result = SDL_GetKeyName(sym.sym);
	for (const std::string& m : mods) {
		result = (boost::format(_("%1$s+%2$s")) % m % result).str();
	}
	return result;
}

void init_shortcuts(const bool force_defaults) {
	for (KeyboardShortcut k = KeyboardShortcut::k__Begin; k <= KeyboardShortcut::k__End;
	     k = static_cast<KeyboardShortcut>(static_cast<uint16_t>(k) + 1)) {
		shortcuts_.at(k).current_shortcut = get_default_shortcut(k);
		if (force_defaults) {
			write_shortcut(k, shortcuts_.at(k).current_shortcut);
		}
	}
	if (force_defaults) {
		return;
	}

	Section& ss = get_config_section("keyboard_sym");
	Section& sm = get_config_section("keyboard_mod");
	while (Section::Value* v = ss.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.sym = v->get_int();
				break;
			}
		}
	}
	while (Section::Value* v = sm.get_next_val()) {
		for (auto& pair : shortcuts_) {
			if (pair.second.internal_name == v->get_name()) {
				pair.second.current_shortcut.mod = v->get_int();
				break;
			}
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
