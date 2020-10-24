/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_fsmenu/options.h"

#include <memory>

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_base.h"

namespace {

constexpr int kDropdownFullscreen = -2;
constexpr int kDropdownMaximized = -1;

// Locale identifiers can look like this: ca_ES@valencia.UTF-8
// The contents of 'selected_locale' will be changed to match the 'current_locale'
void find_selected_locale(std::string* selected_locale, const std::string& current_locale) {
	if (selected_locale->empty()) {
		std::vector<std::string> parts;
		boost::split(parts, current_locale, boost::is_any_of("."));
		if (current_locale == parts[0]) {
			*selected_locale = current_locale;
		} else {
			boost::split(parts, parts[0], boost::is_any_of("@"));
			if (current_locale == parts[0]) {
				*selected_locale = current_locale;
			} else {
				boost::split(parts, parts[0], boost::is_any_of("_"));
				if (current_locale == parts[0]) {
					*selected_locale = current_locale;
				}
			}
		}
	}
}

}  // namespace

constexpr int16_t kPadding = 4;
FullscreenMenuOptions::FullscreenMenuOptions(FullscreenMenuMain& fsmm,
                                             OptionsCtrl::OptionsStruct opt)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "options",
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuOptions),
                _("Options")),

     // Buttons
     button_box_(this, 0, 0, UI::Box::Horizontal),
     cancel_(&button_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel")),
     apply_(&button_box_, "apply", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Apply")),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")),

     // Tabs
     tabs_(this, UI::TabPanelStyle::kFsMenu),

     box_interface_(&tabs_, 0, 0, UI::Box::Horizontal, 0, 0, kPadding),
     box_interface_left_(&box_interface_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_windows_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_sound_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_saving_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_newgame_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_ingame_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, kPadding),

     // Interface options
     language_dropdown_(&box_interface_left_,
                        "dropdown_language",
                        0,
                        0,
                        100,  // 100 is arbitrary, will be resized in layout().
                        50,
                        24,
                        _("Language"),
                        UI::DropdownType::kTextual,
                        UI::PanelStyle::kFsMenu,
                        UI::ButtonStyle::kFsMenuMenu),
     resolution_dropdown_(&box_interface_left_,
                          "dropdown_resolution",
                          0,
                          0,
                          100,  // 100 is arbitrary, will be resized in layout().
                          50,
                          24,
                          _("Window Size"),
                          UI::DropdownType::kTextual,
                          UI::PanelStyle::kFsMenu,
                          UI::ButtonStyle::kFsMenuMenu),

     inputgrab_(
        &box_interface_left_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Grab Input"), "", 0),
     sdl_cursor_(&box_interface_left_,
                 UI::PanelStyle::kFsMenu,
                 Vector2i::zero(),
                 _("Use system mouse cursor"),
                 "",
                 0),
     sb_maxfps_(&box_interface_left_,
                0,
                0,
                0,
                0,
                opt.maxfps,
                0,
                99,
                UI::PanelStyle::kFsMenu,
                _("Maximum FPS:")),
     translation_info_(&box_interface_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu),

     // Windows options
     snap_win_overlap_only_(&box_windows_,
                            UI::PanelStyle::kFsMenu,
                            Vector2i::zero(),
                            _("Snap windows only when overlapping"),
                            "",
                            0),
     dock_windows_to_edges_(&box_windows_,
                            UI::PanelStyle::kFsMenu,
                            Vector2i::zero(),
                            _("Dock windows to edges"),
                            "",
                            0),
     animate_map_panning_(&box_windows_,
                          UI::PanelStyle::kFsMenu,
                          Vector2i::zero(),
                          _("Animate automatic map movements"),
                          "",
                          0),

     sb_dis_panel_(&box_windows_,
                   0,
                   0,
                   0,
                   0,
                   opt.panel_snap_distance,
                   0,
                   99,
                   UI::PanelStyle::kFsMenu,
                   _("Distance for windows to snap to other panels:"),
                   UI::SpinBox::Units::kPixels),

     sb_dis_border_(&box_windows_,
                    0,
                    0,
                    0,
                    0,
                    opt.border_snap_distance,
                    0,
                    99,
                    UI::PanelStyle::kFsMenu,
                    _("Distance for windows to snap to borders:"),
                    UI::SpinBox::Units::kPixels),

     // Sound options
     sound_options_(box_sound_, UI::SliderStyle::kFsMenu),

     // Saving options
     sb_autosave_(&box_saving_,
                  0,
                  0,
                  0,
                  0,
                  opt.autosave / 60,
                  0,
                  100,
                  UI::PanelStyle::kFsMenu,
                  _("Save game automatically every:"),
                  UI::SpinBox::Units::kMinutes,
                  UI::SpinBox::Type::kBig),

     sb_rolling_autosave_(&box_saving_,
                          0,
                          0,
                          0,
                          0,
                          opt.rolling_autosave,
                          1,
                          20,
                          UI::PanelStyle::kFsMenu,
                          _("Maximum number of autosave files:"),
                          UI::SpinBox::Units::kNone,
                          UI::SpinBox::Type::kBig),

     zip_(&box_saving_,
          UI::PanelStyle::kFsMenu,
          Vector2i::zero(),
          _("Compress widelands data files (maps, replays and savegames)"),
          "",
          0),
     write_syncstreams_(&box_saving_,
                        UI::PanelStyle::kFsMenu,
                        Vector2i::zero(),
                        _("Write syncstreams in network games to debug desyncs"),
                        "",
                        0),
     // New Game options
     show_buildhelp_(
        &box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Building Spaces")),
     show_census_(&box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Census")),
     show_statistics_(
        &box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Statistics")),
     show_soldier_levels_(
        &box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Soldier Levels")),
     show_buildings_(&box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Buildings")),
     show_workarea_overlap_(
        &box_newgame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Workarea Overlaps")),

     // In-Game options
     auto_roadbuild_mode_(&box_ingame_,
                          UI::PanelStyle::kFsMenu,
                          Vector2i::zero(),
                          _("Start building road after placing a flag")),
     transparent_chat_(&box_ingame_,
                       UI::PanelStyle::kFsMenu,
                       Vector2i::zero(),
                       _("Show in-game chat with transparent background"),
                       "",
                       0),

     /** TRANSLATORS: A watchwindow is a window where you keep watching an object or a map region,*/
     /** TRANSLATORS: and it also lets you jump to it on the map. */
     single_watchwin_(
        &box_ingame_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Use single watchwindow mode")),
     /** TRANSLATORS: This refers to to zooming with the scrollwheel.*/
     ctrl_zoom_(&box_ingame_,
                UI::PanelStyle::kFsMenu,
                Vector2i::zero(),
                _("Zoom only when Ctrl is pressed")),
     game_clock_(&box_ingame_,
                 UI::PanelStyle::kFsMenu,
                 Vector2i::zero(),
                 _("Display game time in the top left corner")),
     numpad_diagonalscrolling_(&box_ingame_,
                               UI::PanelStyle::kFsMenu,
                               Vector2i::zero(),
                               _("Allow diagonal scrolling with the numeric keypad")),
     training_wheels_box_(&box_ingame_, 0, 0, UI::Box::Horizontal),
     training_wheels_(&training_wheels_box_,
                      UI::PanelStyle::kFsMenu,
                      Vector2i::zero(),
                      _("Teach me how to play")),
     training_wheels_reset_(&training_wheels_box_,
                            "reset_training_wheels",
                            0,
                            0,
                            0,
                            0,
                            UI::ButtonStyle::kFsMenuSecondary,
                            _("Reset progress")),
     os_(opt) {

	do_not_layout_on_resolution_change();

	// Buttons
	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_ : &cancel_);
	button_box_.add_inf_space();
	button_box_.add(&apply_);
	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &cancel_ : &ok_);
	button_box_.add_inf_space();

	// Tabs
	tabs_.add("options_interface", _("Interface"), &box_interface_, "");
	tabs_.add("options_windows", _("Windows"), &box_windows_, "");
	tabs_.add("options_sound", _("Sound"), &box_sound_, "");
	tabs_.add("options_saving", _("Saving"), &box_saving_, "");
	tabs_.add("options_newgame", _("New Games"), &box_newgame_, "");
	tabs_.add("options_ingame", _("In-Game"), &box_ingame_, "");

	// We want the last active tab when "Apply" was clicked.
	if (os_.active_tab < tabs_.tabs().size()) {
		tabs_.activate(os_.active_tab);
	}

	// Interface
	box_interface_.add(&box_interface_left_);
	box_interface_.add(&translation_info_, UI::Box::Resizing::kExpandBoth);
	box_interface_left_.add(&language_dropdown_, UI::Box::Resizing::kFullSize);
	box_interface_left_.add(&resolution_dropdown_, UI::Box::Resizing::kFullSize);
	box_interface_left_.add(&inputgrab_, UI::Box::Resizing::kFullSize);
	box_interface_left_.add(&sdl_cursor_, UI::Box::Resizing::kFullSize);
	box_interface_left_.add(&sb_maxfps_);

	// Windows
	box_windows_.add(&snap_win_overlap_only_, UI::Box::Resizing::kFullSize);
	box_windows_.add(&dock_windows_to_edges_, UI::Box::Resizing::kFullSize);
	box_windows_.add(&animate_map_panning_, UI::Box::Resizing::kFullSize);
	box_windows_.add(&sb_dis_panel_);
	box_windows_.add(&sb_dis_border_);

	// Sound
	box_sound_.add(&sound_options_, UI::Box::Resizing::kFullSize);

	// Saving
	box_saving_.add(&sb_autosave_);
	box_saving_.add(&sb_rolling_autosave_);
	box_saving_.add(&zip_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&write_syncstreams_, UI::Box::Resizing::kFullSize);

	// New Games
	box_newgame_.add(&show_buildhelp_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_census_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_statistics_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_soldier_levels_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_buildings_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_workarea_overlap_, UI::Box::Resizing::kFullSize);

	// In-Game
	box_ingame_.add(&auto_roadbuild_mode_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&transparent_chat_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&single_watchwin_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&ctrl_zoom_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&game_clock_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&numpad_diagonalscrolling_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&training_wheels_box_, UI::Box::Resizing::kFullSize);
	training_wheels_box_.add(&training_wheels_, UI::Box::Resizing::kFullSize);
	training_wheels_box_.add_inf_space();
	training_wheels_box_.add(&training_wheels_reset_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	training_wheels_box_.add_space(kPadding);

	// Bind actions
	language_dropdown_.selected.connect([this]() { update_language_stats(); });
	training_wheels_reset_.sigclicked.connect([this]() {
		training_wheels_reset_.set_enabled(false);
		Profile training_wheels_profile;
		training_wheels_profile.write(kTrainingWheelsFile);
	});
	cancel_.sigclicked.connect([this]() { clicked_cancel(); });
	apply_.sigclicked.connect([this]() { clicked_apply(); });
	ok_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kOk); });

	/** TRANSLATORS: Options: Save game automatically every: */
	sb_autosave_.add_replacement(0, _("Off"));

	// Fill in data
	// Interface options
	for (int modes = 0; modes < SDL_GetNumDisplayModes(0); ++modes) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, modes, &mode);
		if (kMinimumResolutionW <= mode.w && kMinimumResolutionH <= mode.h &&
		    (SDL_BITSPERPIXEL(mode.format) == 32 || SDL_BITSPERPIXEL(mode.format) == 24)) {
			ScreenResolution this_res = {
			   mode.w, mode.h, static_cast<int32_t>(SDL_BITSPERPIXEL(mode.format))};
			if (this_res.depth == 24) {
				this_res.depth = 32;
			}
			if (resolutions_.empty() || this_res.xres != resolutions_.rbegin()->xres ||
			    this_res.yres != resolutions_.rbegin()->yres) {
				resolutions_.push_back(this_res);
			}
		}
	}

	int cur_win_res_x = g_gr->get_window_mode_xres();
	int cur_win_res_y = g_gr->get_window_mode_yres();

	/** TRANSLATORS: Entry in the window size dropdown*/
	resolution_dropdown_.add(_("Fullscreen"), kDropdownFullscreen, nullptr, opt.fullscreen);
#ifdef RESIZABLE_WINDOW
	/** TRANSLATORS: Entry in the window size dropdown*/
	resolution_dropdown_.add(_("Maximized"), kDropdownMaximized, nullptr,
	                         !resolution_dropdown_.has_selection() && opt.maximized);
#endif

	for (uint32_t i = 0; i < resolutions_.size(); ++i) {
		const bool selected = !resolution_dropdown_.has_selection() &&
		                      resolutions_[i].xres == cur_win_res_x &&
		                      resolutions_[i].yres == cur_win_res_y;
		resolution_dropdown_.add(
		   /** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		   (boost::format(_("%1% x %2%")) % resolutions_[i].xres % resolutions_[i].yres).str(), i,
		   nullptr, selected);
	}

	if (!resolution_dropdown_.has_selection()) {
		int entry = resolutions_.size();
		resolutions_.resize(entry + 1);
		resolutions_[entry].xres = cur_win_res_x;
		resolutions_[entry].yres = cur_win_res_y;
		resolution_dropdown_.add(
		   /** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		   (boost::format(_("%1% x %2%")) % cur_win_res_x % cur_win_res_y).str(), entry, nullptr,
		   true);
	}

	inputgrab_.set_state(opt.inputgrab);
	sdl_cursor_.set_state(opt.sdl_cursor);

	// Windows options
	snap_win_overlap_only_.set_state(opt.snap_win_overlap_only);
	dock_windows_to_edges_.set_state(opt.dock_windows_to_edges);
	animate_map_panning_.set_state(opt.animate_map_panning);

	// Saving options
	zip_.set_state(opt.zip);
	write_syncstreams_.set_state(opt.write_syncstreams);

	// Game options
	auto_roadbuild_mode_.set_state(opt.auto_roadbuild_mode);
	transparent_chat_.set_state(opt.transparent_chat);
	single_watchwin_.set_state(opt.single_watchwin);
	ctrl_zoom_.set_state(opt.ctrl_zoom);
	game_clock_.set_state(opt.game_clock);
	numpad_diagonalscrolling_.set_state(opt.numpad_diagonalscrolling);
	training_wheels_.set_state(opt.training_wheels);

	// New Game options
	show_buildhelp_.set_state(opt.display_flags & InteractiveBase::dfShowBuildhelp);
	show_census_.set_state(opt.display_flags & InteractiveBase::dfShowCensus);
	show_statistics_.set_state(opt.display_flags & InteractiveBase::dfShowStatistics);
	show_soldier_levels_.set_state(opt.display_flags & InteractiveBase::dfShowSoldierLevels);
	show_buildings_.set_state(opt.display_flags & InteractiveBase::dfShowBuildings);
	show_workarea_overlap_.set_state(opt.display_flags & InteractiveBase::dfShowWorkareaOverlap);

	// Language options
	add_languages_to_list(opt.language);
	update_language_stats();
	layout();
}

void FullscreenMenuOptions::layout() {
	if (!is_minimal()) {
		const int16_t butw = get_inner_w() / 5;
		const int16_t buth = get_inner_h() / 16;

		// Buttons
		cancel_.set_desired_size(butw, buth);
		apply_.set_desired_size(butw, buth);
		ok_.set_desired_size(butw, buth);
		button_box_.set_size(get_inner_w(), buth);
		button_box_.set_pos(Vector2i(0, get_inner_h() - kPadding - button_box_.get_h()));

		// Tabs
		tabs_.set_size(get_inner_w(), get_inner_h() - buth - 2 * kPadding);

		const int tab_panel_width = get_inner_w() - 2 * kPadding;
		const int column_width = tab_panel_width / 2;

		// Interface
		box_interface_left_.set_desired_size(column_width + kPadding, tabs_.get_inner_h());
		language_dropdown_.set_height(tabs_.get_h() - language_dropdown_.get_y() - buth -
		                              3 * kPadding);
		sb_maxfps_.set_unit_width(column_width / 2);
		sb_maxfps_.set_desired_size(column_width, sb_maxfps_.get_h());

		// Windows options
		sb_dis_panel_.set_unit_width(200);
		sb_dis_panel_.set_desired_size(tab_panel_width, sb_dis_panel_.get_h());
		sb_dis_border_.set_unit_width(200);
		sb_dis_border_.set_desired_size(tab_panel_width, sb_dis_border_.get_h());

		// Saving options
		sb_autosave_.set_unit_width(250);
		sb_autosave_.set_desired_size(tab_panel_width, sb_autosave_.get_h());
		sb_rolling_autosave_.set_unit_width(250);
		sb_rolling_autosave_.set_desired_size(tab_panel_width, sb_rolling_autosave_.get_h());
	}
	UI::Window::layout();
}

void FullscreenMenuOptions::add_languages_to_list(const std::string& current_locale) {

	// We want these two entries on top - the most likely user's choice and the default.
	language_dropdown_.add(_("Try system language"), "", nullptr, current_locale == "");
	language_dropdown_.add("English", "en", nullptr, current_locale == "en");

	// Handle non-standard setups where the locale directory might be missing
	if (!g_fs->is_directory(i18n::get_localedir())) {
		return;
	}

	// Add translation directories to the list. Using the LanguageEntries' sortnames as a key for
	// getting a sorted result.
	std::map<std::string, LanguageEntry> entries;
	std::string selected_locale;

	try {  // Begin read locales table
		LuaInterface lua;
		std::unique_ptr<LuaTable> all_locales(lua.run_script("i18n/locales.lua"));
		all_locales->do_not_warn_about_unaccessed_keys();  // We are only reading partial information
		                                                   // as needed

		// We start with the locale directory so we can pick up locales
		// that don't have a configuration file yet.
		std::unique_ptr<FileSystem> fs(&FileSystem::create(i18n::get_localedir()));
		FilenameSet files = fs->list_directory(".");

		for (const std::string& localename : files) {  // Begin scan locales directory
			const char* path = localename.c_str();
			if (!strcmp(FileSystem::fs_filename(path), ".") ||
			    !strcmp(FileSystem::fs_filename(path), "..") || !fs->is_directory(path)) {
				continue;
			}

			try {  // Begin read locale from table
				std::unique_ptr<LuaTable> table = all_locales->get_table(localename);
				table->do_not_warn_about_unaccessed_keys();

				std::string name = i18n::make_ligatures(table->get_string("name").c_str());
				const std::string sortname = table->get_string("sort_name");
				std::unique_ptr<LanguageEntry> entry(new LanguageEntry(localename, name));
				entries.insert(std::make_pair(sortname, *entry));
				language_entries_.insert(std::make_pair(localename, *entry));

				if (localename == current_locale) {
					selected_locale = current_locale;
				}

			} catch (const WException&) {
				log_err("Could not read locale for: %s\n", localename.c_str());
				entries.insert(std::make_pair(localename, LanguageEntry(localename, localename)));
			}  // End read locale from table
		}     // End scan locales directory
	} catch (const LuaError& err) {
		log_err("Could not read locales information from file: %s\n", err.what());
		return;  // Nothing more can be done now.
	}           // End read locales table

	find_selected_locale(&selected_locale, current_locale);
	for (const auto& entry : entries) {
		const LanguageEntry& language_entry = entry.second;
		language_dropdown_.add(language_entry.descname.c_str(), language_entry.localename, nullptr,
		                       language_entry.localename == selected_locale, "");
	}
}

/**
 * Updates the language statistics message according to the currently selected locale.
 * @param include_system_lang We only want to include the system lang if it matches the Widelands
 * locale.
 */
void FullscreenMenuOptions::update_language_stats() {
	int percent = 100;
	std::string message = "";
	if (language_dropdown_.has_selection()) {
		std::string locale = language_dropdown_.get_selected();
		// Empty locale means try system locale
		if (locale.empty()) {
			std::vector<std::string> parts;
			boost::split(parts, i18n::get_locale(), boost::is_any_of("."));
			if (language_entries_.count(parts[0]) == 1) {
				locale = parts[0];
			} else {
				boost::split(parts, parts[0], boost::is_any_of("@"));
				if (language_entries_.count(parts[0]) == 1) {
					locale = parts[0];
				} else {
					boost::split(parts, parts[0], boost::is_any_of("_"));
					if (language_entries_.count(parts[0]) == 1) {
						locale = parts[0];
					}
				}
			}
		}

		// If we have the locale, grab the stats and set the message
		if (language_entries_.count(locale) == 1) {
			try {
				const LanguageEntry& entry = language_entries_[locale];
				Profile prof("i18n/translation_stats.conf");
				Section& s = prof.get_safe_section("global");
				const int total = s.get_int("total");
				s = prof.get_safe_section(locale);
				percent = static_cast<int>(floor(100 * s.get_int("translated") / total));
				if (percent == 100) {
					message =
					   /** TRANSLATORS: %s = language name */
					   (boost::format(_("The translation into %s is complete.")) % entry.descname).str();
				} else {
					/** TRANSLATORS: %1% = language name, %2% = percentage */
					message = (boost::format(_("The translation into %1% is %2%%% complete.")) %
					           entry.descname % percent)
					             .str();
				}
			} catch (...) {
			}
		}
	}

	// We will want some help with incomplete translations. We set this lower than 100%,
	// because some translators let things drop a bit sometimes because they're busy and
	// will catch up with the work later.
	if (percent <= 90) {
		message = message + " " +
		          (boost::format(_("If you wish to help us translate, please visit %s")) %
		           "<font underline=1>widelands.org/wiki/TranslatingWidelands</font>")
		             .str();
	}
	// Make font a bit smaller so the link will fit at 800x600 resolution.
	translation_info_.set_text(
	   as_richtext_paragraph(message, UI::FontStyle::kFsMenuTranslationInfo));
}

void FullscreenMenuOptions::clicked_apply() {
	end_modal<MenuTarget>(MenuTarget::kApplyOptions);
}

void FullscreenMenuOptions::clicked_cancel() {
	g_sh->load_config();
	end_modal<MenuTarget>(MenuTarget::kBack);
}

bool FullscreenMenuOptions::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			end_modal<MenuTarget>(MenuTarget::kOk);
			return true;
		case SDLK_ESCAPE:
			end_modal<MenuTarget>(MenuTarget::kBack);
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

OptionsCtrl::OptionsStruct FullscreenMenuOptions::get_values() {
	// Write all data from UI elements
	// Interface options
	if (language_dropdown_.has_selection()) {
		os_.language = language_dropdown_.get_selected();
	}
	if (resolution_dropdown_.has_selection()) {
		const int res_index = resolution_dropdown_.get_selected();
		os_.fullscreen = res_index == kDropdownFullscreen;
		os_.maximized = res_index == kDropdownMaximized;
		if (res_index != kDropdownFullscreen && res_index != kDropdownMaximized) {
			os_.xres = resolutions_[res_index].xres;
			os_.yres = resolutions_[res_index].yres;
		}
	}
	os_.inputgrab = inputgrab_.get_state();
	os_.sdl_cursor = sdl_cursor_.get_state();
	os_.maxfps = sb_maxfps_.get_value();

	// Windows options
	os_.snap_win_overlap_only = snap_win_overlap_only_.get_state();
	os_.dock_windows_to_edges = dock_windows_to_edges_.get_state();
	os_.animate_map_panning = animate_map_panning_.get_state();
	os_.panel_snap_distance = sb_dis_panel_.get_value();
	os_.border_snap_distance = sb_dis_border_.get_value();

	// Saving options
	os_.autosave = sb_autosave_.get_value();
	os_.rolling_autosave = sb_rolling_autosave_.get_value();
	os_.zip = zip_.get_state();
	os_.write_syncstreams = write_syncstreams_.get_state();

	// Game options
	os_.auto_roadbuild_mode = auto_roadbuild_mode_.get_state();
	os_.transparent_chat = transparent_chat_.get_state();
	os_.single_watchwin = single_watchwin_.get_state();
	os_.ctrl_zoom = ctrl_zoom_.get_state();
	os_.game_clock = game_clock_.get_state();
	os_.numpad_diagonalscrolling = numpad_diagonalscrolling_.get_state();
	os_.training_wheels = training_wheels_.get_state();

	// New Game options
	int32_t flags = show_buildhelp_.get_state() ? InteractiveBase::dfShowBuildhelp : 0;
	flags |= show_census_.get_state() ? InteractiveBase::dfShowCensus : 0;
	flags |= show_statistics_.get_state() ? InteractiveBase::dfShowStatistics : 0;
	flags |= show_soldier_levels_.get_state() ? InteractiveBase::dfShowSoldierLevels : 0;
	flags |= show_buildings_.get_state() ? InteractiveBase::dfShowBuildings : 0;
	flags |= show_workarea_overlap_.get_state() ? InteractiveBase::dfShowWorkareaOverlap : 0;
	os_.display_flags = flags;

	// Last tab for reloading the options menu
	os_.active_tab = tabs_.active();
	return os_;
}

/**
 * Handles communication between window class and options
 */
OptionsCtrl::OptionsCtrl(FullscreenMenuMain& mm, Section& s)
   : opt_section_(s),
     parent_(mm),
     opt_dialog_(
        std::unique_ptr<FullscreenMenuOptions>(new FullscreenMenuOptions(mm, options_struct(0)))) {
	handle_menu();
}

void OptionsCtrl::handle_menu() {
	MenuTarget i = opt_dialog_->run<MenuTarget>();
	if (i != MenuTarget::kBack) {
		save_options();
		g_gr->set_fullscreen(opt_dialog_->get_values().fullscreen);
		if (opt_dialog_->get_values().maximized) {
			g_gr->set_maximized(true);
		} else if (!opt_dialog_->get_values().fullscreen && !opt_dialog_->get_values().maximized) {
			g_gr->change_resolution(
			   opt_dialog_->get_values().xres, opt_dialog_->get_values().yres, true);
		}
	}
	if (i == MenuTarget::kApplyOptions) {
		uint32_t active_tab = opt_dialog_->get_values().active_tab;
		opt_dialog_.reset(new FullscreenMenuOptions(parent_, options_struct(active_tab)));
		handle_menu();  // Restart general options menu
	}
}

OptionsCtrl::OptionsStruct OptionsCtrl::options_struct(uint32_t active_tab) {
	OptionsStruct opt;
	// Interface options
	opt.xres = opt_section_.get_int("xres", kDefaultResolutionW);
	opt.yres = opt_section_.get_int("yres", kDefaultResolutionH);
	opt.maximized = opt_section_.get_bool("maximized", false);
	opt.fullscreen = opt_section_.get_bool("fullscreen", false);
	opt.inputgrab = opt_section_.get_bool("inputgrab", false);
	opt.maxfps = opt_section_.get_int("maxfps", 25);
	opt.sdl_cursor = opt_section_.get_bool("sdl_cursor", true);

	// Windows options
	opt.snap_win_overlap_only = opt_section_.get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges = opt_section_.get_bool("dock_windows_to_edges", false);
	opt.animate_map_panning = opt_section_.get_bool("animate_map_panning", true);
	opt.panel_snap_distance = opt_section_.get_int("panel_snap_distance", 0);
	opt.border_snap_distance = opt_section_.get_int("border_snap_distance", 0);

	// Saving options
	opt.autosave = opt_section_.get_int("autosave", kDefaultAutosaveInterval * 60);
	opt.rolling_autosave = opt_section_.get_int("rolling_autosave", 5);
	opt.zip = !opt_section_.get_bool("nozip", false);
	opt.write_syncstreams = opt_section_.get_bool("write_syncstreams", true);

	// Game options
	opt.auto_roadbuild_mode = opt_section_.get_bool("auto_roadbuild_mode", true);
	opt.transparent_chat = opt_section_.get_bool("transparent_chat", true);
	opt.single_watchwin = opt_section_.get_bool("single_watchwin", false);
	opt.ctrl_zoom = opt_section_.get_bool("ctrl_zoom", false);
	opt.game_clock = opt_section_.get_bool("game_clock", true);
	opt.numpad_diagonalscrolling = opt_section_.get_bool("numpad_diagonalscrolling", false);
	opt.training_wheels = opt_section_.get_bool("training_wheels", true);

	// New Game options
	opt.display_flags = opt_section_.get_int("display_flags", InteractiveBase::kDefaultDisplayFlags);

	// Language options
	opt.language = opt_section_.get_string("language", "");

	// Last tab for reloading the options menu
	opt.active_tab = active_tab;
	return opt;
}

void OptionsCtrl::save_options() {
	OptionsCtrl::OptionsStruct opt = opt_dialog_->get_values();

	// Interface options
	opt_section_.set_int("xres", opt.xres);
	opt_section_.set_int("yres", opt.yres);
	opt_section_.set_bool("maximized", opt.maximized);
	opt_section_.set_bool("fullscreen", opt.fullscreen);
	opt_section_.set_bool("inputgrab", opt.inputgrab);
	opt_section_.set_int("maxfps", opt.maxfps);
	opt_section_.set_bool("sdl_cursor", opt.sdl_cursor);

	// Windows options
	opt_section_.set_bool("snap_windows_only_when_overlapping", opt.snap_win_overlap_only);
	opt_section_.set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	opt_section_.set_bool("animate_map_panning", opt.animate_map_panning);
	opt_section_.set_int("panel_snap_distance", opt.panel_snap_distance);
	opt_section_.set_int("border_snap_distance", opt.border_snap_distance);

	// Saving options
	opt_section_.set_int("autosave", opt.autosave * 60);
	opt_section_.set_int("rolling_autosave", opt.rolling_autosave);
	opt_section_.set_bool("nozip", !opt.zip);
	opt_section_.set_bool("write_syncstreams", opt.write_syncstreams);

	// Game options
	opt_section_.set_bool("auto_roadbuild_mode", opt.auto_roadbuild_mode);
	opt_section_.set_bool("transparent_chat", opt.transparent_chat);
	opt_section_.set_bool("single_watchwin", opt.single_watchwin);
	opt_section_.set_bool("ctrl_zoom", opt.ctrl_zoom);
	opt_section_.set_bool("game_clock", opt.game_clock);
	opt_section_.set_bool("numpad_diagonalscrolling", opt.numpad_diagonalscrolling);
	opt_section_.set_bool("training_wheels", opt.training_wheels);

	// New Game options
	opt_section_.set_int("display_flags", opt.display_flags);

	// Language options
	opt_section_.set_string("language", opt.language);

	WLApplication::get()->set_input_grab(opt.inputgrab);
	g_mouse_cursor->set_use_sdl(opt_dialog_->get_values().sdl_cursor);
	i18n::set_locale(opt.language);
	UI::g_fh->reinitialize_fontset(i18n::get_locale());

	// Sound options
	g_sh->save_config();

	// Now write to file
	write_config();
}
