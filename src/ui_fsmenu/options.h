/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_OPTIONS_H
#define WL_UI_FSMENU_OPTIONS_H

#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"
#include "wui/sound_options.h"

class Section;

namespace FsMenu {

class Options;

class OptionsCtrl {
public:
	struct OptionsStruct {

		// Interface options
		int32_t display;
		int32_t xres;
		int32_t yres;
		bool maximized;
		bool fullscreen;
		bool sdl_cursor;
		bool tooltip_accessibility_mode;

		// Window options
		bool dock_windows_to_edges;
		int32_t panel_snap_distance;
		int32_t border_snap_distance;

		// Saving options
		int32_t autosave;          // autosave interval in minutes
		int32_t rolling_autosave;  // number of file to use for rolling autosave
		int32_t replay_lifetime;   // number of weeks to keep replays around
		bool skip_autosave_on_inactivity;
		bool zip;
		bool save_chat_history;

		// Game options
		int32_t pause_game_on_inactivity;  // inactivity duration in minutes
		bool auto_roadbuild_mode;
		bool transparent_chat;
		bool single_watchwin;
		bool game_clock;
		bool numpad_diagonalscrolling;
		bool edge_scrolling;
		bool invert_movement;
		bool animate_map_panning;
		int32_t display_flags;
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
		bool training_wheels;
#endif

		// Language options
		std::string language;

		// Last tab for reloading the options menu
		uint32_t active_tab;
	};

	explicit OptionsCtrl(MainMenu&, Section&);
	void handle_menu();
	OptionsCtrl::OptionsStruct options_struct(uint32_t active_tab);
	void save_options();

private:
	Section& opt_section_;
	MainMenu& parent_;
	std::unique_ptr<Options> opt_dialog_;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

class Options : public UI::Window {
public:
	explicit Options(MainMenu&, OptionsCtrl::OptionsStruct opt);
	OptionsCtrl::OptionsStruct get_values();

	bool handle_key(bool, SDL_Keysym) override;

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuOptions;
	}

private:
	void layout() override;

	// Fills the language selection list
	void add_languages_to_list(const std::string& current_locale);
	void update_language_stats();

	void add_displays(const OptionsCtrl::OptionsStruct& opt);
	void add_screen_resolutions(const OptionsCtrl::OptionsStruct& opt);

	// Saves the options and closes the window
	void clicked_ok();
	// Saves the options and reloads the active tab
	void clicked_apply();
	// Restores old options when canceled
	void clicked_cancel();

	// Data model for the screen resolution dropdown
	class ScreenResolution {
	public:
		int32_t xres;
		int32_t yres;
		inline bool operator==(const ScreenResolution& x) const {
			return xres == x.xres && yres == x.yres;
		}
		inline bool operator!=(const ScreenResolution& x) const {
			return !(*this == x);
		}
	};

	UI::Box button_box_;
	UI::Button cancel_, apply_, ok_;

	// UI elements
	UI::TabPanel tabs_;
	UI::Box box_interface_, box_interface_hbox_, box_interface_vbox_;
	UI::Box box_sound_;
	UI::Box box_saving_;
	UI::Box box_newgame_;
	UI::Box box_ingame_;

	// Interface options
	UI::Dropdown<std::string> language_dropdown_;
	UI::Dropdown<ScreenResolution> resolution_dropdown_;
	UI::Dropdown<int> display_dropdown_;
	UI::Checkbox sdl_cursor_;
	UI::Checkbox tooltip_accessibility_mode_;
	UI::MultilineTextarea translation_info_;
	// Empty panel for layouting
	// TODO(tothxa): Replace with infinite space if box layouting quirks get fixed
	UI::Panel translation_padding_;

	UI::Checkbox dock_windows_to_edges_;
	UI::SpinBox sb_dis_panel_;
	UI::SpinBox sb_dis_border_;

	UI::Button configure_keyboard_;

	// Sound options
	SoundOptions sound_options_;

	// Saving options
	UI::SpinBox sb_autosave_;
	UI::SpinBox sb_rolling_autosave_;
	UI::SpinBox sb_replay_lifetime_;
	UI::Checkbox skip_autosave_on_inactivity_;
	UI::Checkbox zip_;
	UI::Checkbox save_chat_history_;

	// New Game options
	UI::Checkbox show_buildhelp_;
	UI::Checkbox show_census_;
	UI::Checkbox show_statistics_;
	UI::Checkbox show_soldier_levels_;
	UI::Checkbox show_buildings_;
	UI::Checkbox show_workarea_overlap_;

	// In-Game options
	UI::SpinBox sb_pause_game_on_inactivity_;
	UI::Checkbox auto_roadbuild_mode_;
	UI::Checkbox transparent_chat_;
	UI::Checkbox single_watchwin_;
	UI::Checkbox game_clock_;
	UI::Checkbox numpad_diagonalscrolling_;
	UI::Checkbox edge_scrolling_;
	UI::Checkbox invert_movement_;
	UI::Checkbox animate_map_panning_;

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	UI::Box training_wheels_box_;
	UI::Checkbox training_wheels_;
	UI::Button training_wheels_button_;
#endif

	OptionsCtrl::OptionsStruct os_;

	// Data model for the entries in the language selection list.
	struct LanguageEntry {
		LanguageEntry(const std::string& init_localename, const std::string& init_descname)
		   : localename(init_localename), descname(init_descname) {
		}
		LanguageEntry() : LanguageEntry("", "") {
		}
		std::string localename;  // ISO code for the locale
		std::string descname;    // Native language name
	};
	std::map<std::string, LanguageEntry> language_entries_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_OPTIONS_H
