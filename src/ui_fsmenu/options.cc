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

#include "ui_fsmenu/options.h"

#include <algorithm>
#include <memory>

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
#include "ui_fsmenu/keyboard_options.h"
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
#include "ui_fsmenu/training_wheel_options.h"
#endif
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_base.h"

namespace {

constexpr int kDropdownFullscreen = -2;
constexpr int kDropdownMaximized = -1;

constexpr int kDropdownFollowMouse = -1;

// Locale identifiers can look like this: ca_ES@valencia.UTF-8
// The contents of 'selected_locale' will be changed to match the 'current_locale'
void find_selected_locale(std::string* selected_locale, const std::string& current_locale) {
	if (selected_locale->empty()) {
		std::vector<std::string> parts;
		split(parts, current_locale, {'.'});
		if (current_locale == parts[0]) {
			*selected_locale = current_locale;
		} else {
			split(parts, parts[0], {'@'});
			if (current_locale == parts[0]) {
				*selected_locale = current_locale;
			} else {
				split(parts, parts[0], {'_'});
				if (current_locale == parts[0]) {
					*selected_locale = current_locale;
				}
			}
		}
	}
}

}  // namespace

namespace FsMenu {

Options::Options(MainMenu& fsmm, OptionsCtrl::OptionsStruct opt)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "options",
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuOptions),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuOptions),
                _("Options")),

     // Buttons
     button_box_(this, UI::PanelStyle::kFsMenu, "buttons_box", 0, 0, UI::Box::Horizontal),
     cancel_(&button_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel")),
     apply_(&button_box_, "apply", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Apply")),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")),

     // Tabs
     tabs_(this, UI::TabPanelStyle::kFsMenu, "tabs"),

     box_interface_(
        &tabs_, UI::PanelStyle::kFsMenu, "box_interface", 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_interface_hbox_(&box_interface_,
                         UI::PanelStyle::kFsMenu,
                         "hbox_interface",
                         0,
                         0,
                         UI::Box::Horizontal,
                         0,
                         0,
                         kPadding),
     box_interface_vbox_(&box_interface_hbox_,
                         UI::PanelStyle::kFsMenu,
                         "vbox_interface",
                         0,
                         0,
                         UI::Box::Vertical,
                         0,
                         0,
                         kPadding),
     box_sound_(
        &tabs_, UI::PanelStyle::kFsMenu, "box_sound", 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_saving_(
        &tabs_, UI::PanelStyle::kFsMenu, "box_saving", 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_newgame_(
        &tabs_, UI::PanelStyle::kFsMenu, "box_newgame", 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     box_ingame_(
        &tabs_, UI::PanelStyle::kFsMenu, "box_ingame", 0, 0, UI::Box::Vertical, 0, 0, kPadding),

     // Interface options
     language_dropdown_(&box_interface_vbox_,
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
     resolution_dropdown_(&box_interface_vbox_,
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
     display_dropdown_(&box_interface_vbox_,
                       "dropdown_display",
                       0,
                       0,
                       100,  // 100 is arbitrary, will be resized in layout().
                       50,
                       24,
                       pgettext("multiscreen", "Display"),
                       UI::DropdownType::kTextual,
                       UI::PanelStyle::kFsMenu,
                       UI::ButtonStyle::kFsMenuMenu),
     sdl_cursor_(&box_interface_,
                 UI::PanelStyle::kFsMenu,
                 "sdl_cursor",
                 Vector2i::zero(),
                 /** TRANSLATORS: short description for the sdl_cursor option */
                 _("Let the system draw the mouse cursor"),
                 /** TRANSLATORS: tooltip text for the sdl_cursor option */
                 _("If in doubt, leave this enabled. When disabled, cursor updates may be slow "
                   "and the cursor appears frozen during long operations. Disable it only if "
                   "the cursor doesn’t appear right, or if you want it to be visible in "
                   "screenshots or screencasts."),
                 0),
     tooltip_accessibility_mode_(&box_interface_,
                                 UI::PanelStyle::kFsMenu,
                                 "tooltip_accessibility_mode",
                                 Vector2i::zero(),
                                 _("Accessibility mode for tooltips")),
     translation_info_(
        &box_interface_hbox_, "translation_info", 0, 0, 100, 20, UI::PanelStyle::kFsMenu),
     translation_padding_(&box_interface_vbox_, UI::PanelStyle::kFsMenu, "padding", 0, 0, 0, 0),

     // Window options
     dock_windows_to_edges_(&box_interface_,
                            UI::PanelStyle::kFsMenu,
                            "dock_to_edges",
                            Vector2i::zero(),
                            _("Dock windows to edges"),
                            "",
                            0),
     sb_dis_panel_(&box_interface_,
                   "panel_snap_distance",
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

     sb_dis_border_(&box_interface_,
                    "border_snap_distance",
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

     configure_keyboard_(&box_interface_,
                         "configure_keyboard",
                         0,
                         0,
                         0,
                         0,
                         UI::ButtonStyle::kFsMenuSecondary,
                         _("Edit keyboard and mouse actions…")),

     // Sound options
     sound_options_(box_sound_, UI::SliderStyle::kFsMenu),

     // Saving options
     sb_autosave_(&box_saving_,
                  "autosave_interval",
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
                          "rolling_autosave",
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
                          UI::SpinBox::Type::kBig,
                          1,
                          ChangeBigStep::kMediumRange),

     sb_replay_lifetime_(&box_saving_,
                         "replay_lifetime",
                         0,
                         0,
                         0,
                         0,
                         opt.replay_lifetime,
                         0,
                         52,
                         UI::PanelStyle::kFsMenu,
                         _("Delete replays after:"),
                         UI::SpinBox::Units::kWeeks,
                         UI::SpinBox::Type::kBig,
                         1,
                         4),

     skip_autosave_on_inactivity_(&box_saving_,
                                  UI::PanelStyle::kFsMenu,
                                  "skip_autosave_on_inactivity",
                                  Vector2i::zero(),
                                  _("Skip autosaves while inactive"),
                                  "",
                                  0),
     zip_(&box_saving_,
          UI::PanelStyle::kFsMenu,
          "compress",
          Vector2i::zero(),
          _("Compress Widelands data files (maps, replays, and savegames)"),
          "",
          0),

     save_chat_history_(&box_saving_,
                        UI::PanelStyle::kFsMenu,
                        "save_chat_history",
                        Vector2i::zero(),
                        _("Keep history of sent chat messages in a file"),
                        "",
                        0),

     // New Game options
     show_buildhelp_(&box_newgame_,
                     UI::PanelStyle::kFsMenu,
                     "buildhelp",
                     Vector2i::zero(),
                     _("Show Building Spaces")),
     show_census_(
        &box_newgame_, UI::PanelStyle::kFsMenu, "census", Vector2i::zero(), _("Show Census")),
     show_statistics_(
        &box_newgame_, UI::PanelStyle::kFsMenu, "statistics", Vector2i::zero(), _("Show Status")),
     show_soldier_levels_(&box_newgame_,
                          UI::PanelStyle::kFsMenu,
                          "soldier_levels",
                          Vector2i::zero(),
                          _("Show Soldier Levels")),
     show_buildings_(
        &box_newgame_, UI::PanelStyle::kFsMenu, "buildings", Vector2i::zero(), _("Show Buildings")),
     show_workarea_overlap_(&box_newgame_,
                            UI::PanelStyle::kFsMenu,
                            "workarea_overlap",
                            Vector2i::zero(),
                            _("Show Workarea Overlaps")),

     // In-Game options
     sb_pause_game_on_inactivity_(&box_ingame_,
                                  "pause_game_on_inactivity",
                                  0,
                                  0,
                                  0,
                                  0,
                                  opt.pause_game_on_inactivity,
                                  0,
                                  120,
                                  UI::PanelStyle::kFsMenu,
                                  _("Pause game when inactive for:"),
                                  UI::SpinBox::Units::kMinutes,
                                  UI::SpinBox::Type::kBig,
                                  1,
                                  10),

     auto_roadbuild_mode_(&box_ingame_,
                          UI::PanelStyle::kFsMenu,
                          "auto_roadbuild",
                          Vector2i::zero(),
                          _("Start building road after placing a flag")),
     transparent_chat_(&box_ingame_,
                       UI::PanelStyle::kFsMenu,
                       "transparent_chat",
                       Vector2i::zero(),
                       _("Show in-game chat with transparent background"),
                       "",
                       0),

     single_watchwin_(&box_ingame_,
                      UI::PanelStyle::kFsMenu,
                      "single_watchwindow",
                      Vector2i::zero(),
                      /** TRANSLATORS: A watchwindow is a window where you keep watching an object
                         or a map region,*/
                      /** TRANSLATORS: and it also lets you jump to it on the map. */
                      _("Use single watchwindow mode")),
     game_clock_(&box_ingame_,
                 UI::PanelStyle::kFsMenu,
                 "system_clock",
                 Vector2i::zero(),
                 _("Display system time in the info panel")),
     numpad_diagonalscrolling_(&box_ingame_,
                               UI::PanelStyle::kFsMenu,
                               "numpad_diagonal_scrolling",
                               Vector2i::zero(),
                               _("Allow diagonal scrolling with the numeric keypad")),
     edge_scrolling_(&box_ingame_,
                     UI::PanelStyle::kFsMenu,
                     "edge_scrolling",
                     Vector2i::zero(),
                     _("Scroll when the mouse cursor is near the screen edge")),
     invert_movement_(&box_ingame_,
                      UI::PanelStyle::kFsMenu,
                      "invert_movement",
                      Vector2i::zero(),
                      _("Invert click-and-drag map movement direction")),
     animate_map_panning_(&box_ingame_,
                          UI::PanelStyle::kFsMenu,
                          "animate_map_panning",
                          Vector2i::zero(),
                          _("Animate automatic map movements"),
                          "",
                          0),

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
     training_wheels_box_(&box_ingame_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     training_wheels_(&training_wheels_box_,
                      UI::PanelStyle::kFsMenu,
                      Vector2i::zero(),
                      _("Teach me how to play")),
     training_wheels_button_(
        &training_wheels_box_,
        "edit_training_wheels",
        0,
        0,
        0,
        0,
        UI::ButtonStyle::kFsMenuSecondary,
        /** TRANSLATORS: Button to bring up a window to edit teaching progress in the Options */
        _("Progress…")),
#endif
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
	tabs_.add("options_sound", _("Sound"), &box_sound_, "");
	tabs_.add("options_saving", _("Saving"), &box_saving_, "");
	tabs_.add("options_newgame", _("New Games"), &box_newgame_, "");
	tabs_.add("options_ingame", _("In-Game"), &box_ingame_, "");

	// Interface
	box_interface_vbox_.add(&language_dropdown_, UI::Box::Resizing::kFullSize);
	box_interface_vbox_.add(&resolution_dropdown_, UI::Box::Resizing::kFullSize);
	box_interface_vbox_.add(&display_dropdown_, UI::Box::Resizing::kFullSize);
	// TODO(tothxa): Replace with infinite space if box layouting quirks get fixed
	box_interface_vbox_.add(&translation_padding_, UI::Box::Resizing::kFullSize);
	// box_interface_vbox_.add_inf_space();

	box_interface_hbox_.add(&box_interface_vbox_, UI::Box::Resizing::kExpandBoth);
	box_interface_hbox_.add(&translation_info_, UI::Box::Resizing::kExpandBoth);

	box_interface_.add(&box_interface_hbox_, UI::Box::Resizing::kFullSize);

	box_interface_.add(&sdl_cursor_, UI::Box::Resizing::kFullSize);
	box_interface_.add(&tooltip_accessibility_mode_, UI::Box::Resizing::kFullSize);

	box_interface_.add(&dock_windows_to_edges_, UI::Box::Resizing::kFullSize);
	box_interface_.add(&sb_dis_panel_);
	box_interface_.add(&sb_dis_border_);

	box_interface_.add_space(kPadding);
	box_interface_.add(&configure_keyboard_);

	// Sound
	box_sound_.add(&sound_options_, UI::Box::Resizing::kFullSize);

	// Saving
	box_saving_.add(&sb_autosave_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&sb_rolling_autosave_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&sb_replay_lifetime_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&skip_autosave_on_inactivity_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&zip_, UI::Box::Resizing::kFullSize);
	box_saving_.add(&save_chat_history_, UI::Box::Resizing::kFullSize);

	// New Games
	box_newgame_.add(&show_buildhelp_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_census_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_statistics_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_soldier_levels_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_buildings_, UI::Box::Resizing::kFullSize);
	box_newgame_.add(&show_workarea_overlap_, UI::Box::Resizing::kFullSize);

	// In-Game
	box_ingame_.add(&sb_pause_game_on_inactivity_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&auto_roadbuild_mode_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&transparent_chat_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&single_watchwin_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&game_clock_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&numpad_diagonalscrolling_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&edge_scrolling_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&invert_movement_, UI::Box::Resizing::kFullSize);
	box_ingame_.add(&animate_map_panning_, UI::Box::Resizing::kFullSize);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	box_ingame_.add_space(kPadding);
	box_ingame_.add(&training_wheels_box_, UI::Box::Resizing::kFullSize);
	training_wheels_box_.add(&training_wheels_, UI::Box::Resizing::kFullSize);
	training_wheels_box_.add_inf_space();
	training_wheels_box_.add(&training_wheels_button_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	training_wheels_box_.add_space(kPadding);
#endif

	// Bind actions
	language_dropdown_.selected.connect([this]() { update_language_stats(); });

	configure_keyboard_.sigclicked.connect([this]() {
		configure_keyboard_.set_enabled(false);
		cancel_.set_enabled(false);
		apply_.set_enabled(false);
		ok_.set_enabled(false);
		KeyboardOptions o(*this);
		o.run<UI::Panel::Returncodes>();
		configure_keyboard_.set_enabled(true);
		cancel_.set_enabled(true);
		apply_.set_enabled(true);
		ok_.set_enabled(true);
	});

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	training_wheels_.changed.connect(
	   [this]() { training_wheels_button_.set_enabled(training_wheels_.get_state()); });
	training_wheels_button_.set_enabled(training_wheels_.get_state());
	training_wheels_button_.sigclicked.connect([this]() {
		training_wheels_button_.set_enabled(false);
		cancel_.set_enabled(false);
		apply_.set_enabled(false);
		ok_.set_enabled(false);
		TrainingWheelOptions training_wheel_options(get_parent());
		training_wheel_options.run<UI::Panel::Returncodes>();
		training_wheels_button_.set_enabled(true);
		cancel_.set_enabled(true);
		apply_.set_enabled(true);
		ok_.set_enabled(true);
	});
#endif
	cancel_.sigclicked.connect([this]() { clicked_cancel(); });
	apply_.sigclicked.connect([this]() { clicked_apply(); });
	ok_.sigclicked.connect([this]() { clicked_ok(); });

	/** TRANSLATORS: Options: Save game automatically every: */
	sb_autosave_.add_replacement(0, _("Off"));
	sb_pause_game_on_inactivity_.add_replacement(0, _("Off"));
	/** TRANSLATORS: Options: Delete replays after: */
	sb_replay_lifetime_.add_replacement(0, _("Never"));

	// Fill in data
	// Interface options
	add_displays(opt);
	add_screen_resolutions(opt);

	sdl_cursor_.set_state(opt.sdl_cursor);
	tooltip_accessibility_mode_.set_state(opt.tooltip_accessibility_mode);

	// Window options
	dock_windows_to_edges_.set_state(opt.dock_windows_to_edges);

	// Saving options
	skip_autosave_on_inactivity_.set_state(opt.skip_autosave_on_inactivity);
	zip_.set_state(opt.zip);
	save_chat_history_.set_state(opt.save_chat_history);

	// Game options
	auto_roadbuild_mode_.set_state(opt.auto_roadbuild_mode);
	transparent_chat_.set_state(opt.transparent_chat);
	single_watchwin_.set_state(opt.single_watchwin);
	game_clock_.set_state(opt.game_clock);
	numpad_diagonalscrolling_.set_state(opt.numpad_diagonalscrolling);
	edge_scrolling_.set_state(opt.edge_scrolling);
	invert_movement_.set_state(opt.invert_movement);
	animate_map_panning_.set_state(opt.animate_map_panning);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	training_wheels_.set_state(opt.training_wheels);
#endif

	// New Game options
	show_buildhelp_.set_state((opt.display_flags & InteractiveBase::dfShowBuildhelp) != 0);
	show_census_.set_state((opt.display_flags & InteractiveBase::dfShowCensus) != 0);
	show_statistics_.set_state((opt.display_flags & InteractiveBase::dfShowStatistics) != 0);
	show_soldier_levels_.set_state((opt.display_flags & InteractiveBase::dfShowSoldierLevels) != 0);
	show_buildings_.set_state((opt.display_flags & InteractiveBase::dfShowBuildings) != 0);
	show_workarea_overlap_.set_state((opt.display_flags & InteractiveBase::dfShowWorkareaOverlap) !=
	                                 0);

	// Language options
	add_languages_to_list(opt.language);
	update_language_stats();
	layout();

	// We want the last active tab when "Apply" was clicked.
	if (os_.active_tab < tabs_.tabs().size()) {
		tabs_.activate(os_.active_tab);
	}
	initialization_complete();
}

void Options::add_displays(const OptionsCtrl::OptionsStruct& opt) {
	display_dropdown_.add(
	   /** TRANSLATORS: Entry in the display selection dropdown */
	   pgettext("multiscreen", "Follow mouse"), kDropdownFollowMouse, nullptr, opt.display < 0);
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
		SDL_Rect r;
		if (SDL_GetDisplayBounds(i, &r) == 0) {
			display_dropdown_.add(
			   /** TRANSLATORS: Display index and virtual coordinates, e.g, '#0 (0, 0, 1920, 1080)'*/
			   format(_("#%1% (%2%, %3%, %4%, %5%)"), i, r.x, r.y, r.w, r.h), i, nullptr,
			   opt.display == i);
		}
	}
	if (!display_dropdown_.has_selection()) {
		display_dropdown_.select(kDropdownFollowMouse);
	}
	display_dropdown_.set_enabled(SDL_GetNumVideoDisplays() > 1);
}

void Options::add_screen_resolutions(const OptionsCtrl::OptionsStruct& opt) {
	ScreenResolution current_res = {g_gr->get_window_mode_xres(), g_gr->get_window_mode_yres()};

	resolution_dropdown_.add(
	   /** TRANSLATORS: Entry in the window size dropdown */
	   _("Fullscreen"), {kDropdownFullscreen, kDropdownFullscreen}, nullptr, opt.fullscreen);
#ifdef RESIZABLE_WINDOW
	/** TRANSLATORS: Entry in the window size dropdown */
	resolution_dropdown_.add(_("Maximized"), {kDropdownMaximized, kDropdownMaximized}, nullptr,
	                         !resolution_dropdown_.has_selection() && opt.maximized);
#endif

	ScreenResolution previous{0, 0};
	for (int modes = 0; modes < SDL_GetNumDisplayModes(0); ++modes) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, modes, &mode);
		if (kMinimumResolutionW <= mode.w && kMinimumResolutionH <= mode.h &&
		    (SDL_BITSPERPIXEL(mode.format) == 32 || SDL_BITSPERPIXEL(mode.format) == 24)) {
			ScreenResolution this_res = {mode.w, mode.h};
			if (this_res != previous) {
				previous = this_res;
				const bool selected = !resolution_dropdown_.has_selection() && this_res == current_res;
				resolution_dropdown_.add(
				   /** TRANSLATORS: Screen resolution, e.g. 800 × 600 */
				   format(_("%1% × %2%"), this_res.xres, this_res.yres), this_res, nullptr, selected);
			}
		}
	}

	if (!resolution_dropdown_.has_selection()) {
		resolution_dropdown_.add(
		   /** TRANSLATORS: Screen resolution, e.g. 800 × 600 */
		   format(_("%1% × %2%"), current_res.xres, current_res.yres), current_res, nullptr, true);
	}
}

void Options::layout() {
	if (!is_minimal()) {
		const int16_t butw = get_inner_w() / 5;
		const int16_t buth = get_inner_h() / 16;
		constexpr int16_t kSpinboxW = 250;

		// Buttons
		cancel_.set_desired_size(butw, buth);
		apply_.set_desired_size(butw, buth);
		ok_.set_desired_size(butw, buth);
		button_box_.set_size(get_inner_w(), buth);
		button_box_.set_pos(Vector2i(0, get_inner_h() - kPadding - button_box_.get_h()));

		// Tabs
		tabs_.set_size(get_inner_w(), get_inner_h() - buth - 2 * kPadding);

		const int tab_panel_width = get_inner_w() - 3 * kPadding;
		const int unit_w = tab_panel_width / 3;

		// Interface
		language_dropdown_.set_height(tabs_.get_h() - language_dropdown_.get_y() - buth -
		                              3 * kPadding);

		const int min_h = language_dropdown_.get_h() + resolution_dropdown_.get_h() +
		                  display_dropdown_.get_h() + 3 * kPadding;
		const int half_w = (tab_panel_width - 3 * kPadding) / 2;

		// Make initial value big enough to avoid needing a scrollbar
		int translation_h = 3 * min_h;

		// Find out the required height
		translation_info_.set_desired_size(half_w, translation_h);
		translation_info_.set_size(half_w, translation_h);
		int tmp_w = 0;
		translation_info_.get_text_size(&tmp_w, &translation_h);

		// Now set the final height
		translation_h = std::max(translation_h, min_h);
		translation_info_.set_desired_size(half_w, translation_h);
		translation_info_.set_size(half_w, translation_h);
		// TODO(tothxa): Remove if box layouting quirks get fixed
		const int translation_pad_h = translation_h - min_h;
		translation_padding_.set_desired_size(half_w, translation_pad_h);
		translation_padding_.set_size(half_w, translation_pad_h);

		// Interface tab spinboxes
		for (UI::SpinBox* sb : {&sb_dis_panel_, &sb_dis_border_}) {
			sb->set_unit_width(unit_w);
			sb->set_desired_size(tab_panel_width, sb->get_h());
		}

		// Saving options
		for (UI::SpinBox* sb : {&sb_autosave_, &sb_rolling_autosave_, &sb_replay_lifetime_,
		                        &sb_pause_game_on_inactivity_}) {
			sb->set_unit_width(kSpinboxW);
			sb->set_desired_size(tab_panel_width, sb->get_h());
		}
	}
	UI::Window::layout();
}

void Options::add_languages_to_list(const std::string& current_locale) {

	// We want these two entries on top - the most likely user's choice and the default.
	language_dropdown_.add(_("Try system language"), "", nullptr, current_locale.empty());
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
		std::unique_ptr<FileSystem> fs(&FileSystem::create(i18n::get_localedir() + "/widelands"));
		FilenameSet files = fs->list_directory(".");

		for (std::string localename : files) {  // Begin scan locales directory
			const char* path = localename.c_str();
			if ((strcmp(FileSystem::fs_filename(path), ".") == 0) ||
			    (strcmp(FileSystem::fs_filename(path), "..") == 0) ||
			    FileSystem::filename_ext(path) != ".po") {
				continue;
			}
			localename = FileSystem::filename_without_ext(path);

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
		}  // End scan locales directory
	} catch (const LuaError& err) {
		log_err("Could not read locales information from file: %s\n", err.what());
		return;  // Nothing more can be done now.
	}  // End read locales table

	find_selected_locale(&selected_locale, current_locale);
	for (const auto& entry : entries) {
		const LanguageEntry& language_entry = entry.second;
		language_dropdown_.add(language_entry.descname, language_entry.localename, nullptr,
		                       language_entry.localename == selected_locale, "");
	}
}

/**
 * Updates the language statistics message according to the currently selected locale.
 *
 * i18n::get_locale() is parsed,
 *  this can either be "C"
 *  or something like "en_EN.UTF-8"
 *
 *  See https://en.wikipedia.org/wiki/ISO_639
 */
void Options::update_language_stats() {
	int percent = 100;
	std::string message;
	if (language_dropdown_.has_selection()) {
		std::string locale = language_dropdown_.get_selected();
		// Empty locale means try system locale
		if (locale.empty()) {
			std::vector<std::string> parts;
			split(parts, i18n::get_locale(), {'.'});  // split of encoding
			if (language_entries_.count(parts[0]) == 1) {
				locale = parts[0];
			} else {
				std::vector<std::string> parts2;
				split(parts2, parts[0], {'@'});  // no idea where that @ may come from
				if (language_entries_.count(parts2[0]) == 1) {
					locale = parts2[0];
				} else {
					std::vector<std::string> parts3;
					split(parts3, parts2[0], {'_'});  // Split of language from Country
					if (language_entries_.count(parts3[0]) == 1) {
						locale = parts3[0];
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
					   format(_("The translation into %s is complete."), entry.descname);
				} else {
					message = format(
					   /** TRANSLATORS: %1% = language name, %2% = percentage */
					   _("The translation into %1% is %2%%% complete."), entry.descname, percent);
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
		          format(_("If you wish to help us translate, please visit %s"),
		                 "<font underline=1>widelands.org/wiki/TranslatingWidelands</font>");
	}

	translation_info_.set_text(
	   as_richtext_paragraph(message, UI::FontStyle::kFsMenuTranslationInfo));
	layout();
}

void Options::clicked_ok() {
	end_modal<MenuTarget>(MenuTarget::kOk);
}

void Options::clicked_apply() {
	end_modal<MenuTarget>(MenuTarget::kApplyOptions);
}

void Options::clicked_cancel() {
	g_sh->load_config();
	end_modal<MenuTarget>(MenuTarget::kBack);
}

bool Options::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			clicked_cancel();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

OptionsCtrl::OptionsStruct Options::get_values() {
	// Write all data from UI elements
	// Interface options
	if (language_dropdown_.has_selection()) {
		os_.language = language_dropdown_.get_selected();
	}
	if (resolution_dropdown_.has_selection()) {
		const ScreenResolution& res = resolution_dropdown_.get_selected();
		os_.fullscreen = res.xres == kDropdownFullscreen;
		os_.maximized = res.xres == kDropdownMaximized;
		if (!os_.fullscreen && !os_.maximized) {
			os_.xres = res.xres;
			os_.yres = res.yres;
		}
	}
	if (display_dropdown_.has_selection()) {
		os_.display = display_dropdown_.get_selected();
	}
	os_.sdl_cursor = sdl_cursor_.get_state();
	os_.tooltip_accessibility_mode = tooltip_accessibility_mode_.get_state();

	// Window options
	os_.dock_windows_to_edges = dock_windows_to_edges_.get_state();
	os_.panel_snap_distance = sb_dis_panel_.get_value();
	os_.border_snap_distance = sb_dis_border_.get_value();

	// Saving options
	os_.autosave = sb_autosave_.get_value();
	os_.rolling_autosave = sb_rolling_autosave_.get_value();
	os_.replay_lifetime = sb_replay_lifetime_.get_value();
	os_.skip_autosave_on_inactivity = skip_autosave_on_inactivity_.get_state();
	os_.zip = zip_.get_state();
	os_.save_chat_history = save_chat_history_.get_state();

	// Game options
	os_.auto_roadbuild_mode = auto_roadbuild_mode_.get_state();
	os_.transparent_chat = transparent_chat_.get_state();
	os_.single_watchwin = single_watchwin_.get_state();
	os_.game_clock = game_clock_.get_state();
	os_.numpad_diagonalscrolling = numpad_diagonalscrolling_.get_state();
	os_.edge_scrolling = edge_scrolling_.get_state();
	os_.invert_movement = invert_movement_.get_state();
	os_.animate_map_panning = animate_map_panning_.get_state();
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	os_.training_wheels = training_wheels_.get_state();
#endif

	// New Game options
	os_.pause_game_on_inactivity = sb_pause_game_on_inactivity_.get_value();
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
OptionsCtrl::OptionsCtrl(MainMenu& mm, Section& s)
   : opt_section_(s),
     parent_(mm),
     opt_dialog_(std::unique_ptr<Options>(new Options(mm, options_struct(0)))) {
	handle_menu();
}

void OptionsCtrl::handle_menu() {
	MenuTarget i = opt_dialog_->run<MenuTarget>();
	if (i != MenuTarget::kBack) {
		save_options();
		int display = opt_dialog_->get_values().display;
		g_gr->set_fullscreen(opt_dialog_->get_values().fullscreen, display);
		if (opt_dialog_->get_values().maximized) {
			g_gr->set_maximized(true, display);
		} else if (!opt_dialog_->get_values().fullscreen && !opt_dialog_->get_values().maximized) {
			g_gr->change_resolution(
			   opt_dialog_->get_values().xres, opt_dialog_->get_values().yres, true);
		}
		parent_.set_labels();  // update main menu buttons for new language
	}
	if (i == MenuTarget::kApplyOptions) {
		uint32_t active_tab = opt_dialog_->get_values().active_tab;
		opt_dialog_.reset(new Options(parent_, options_struct(active_tab)));
		handle_menu();  // Restart general options menu
	}
}

OptionsCtrl::OptionsStruct OptionsCtrl::options_struct(uint32_t active_tab) {
	OptionsStruct opt;
	// Interface options
	opt.display = opt_section_.get_int("display", kDropdownFollowMouse);
	opt.xres = opt_section_.get_int("xres", kDefaultResolutionW);
	opt.yres = opt_section_.get_int("yres", kDefaultResolutionH);
	opt.maximized = opt_section_.get_bool("maximized", false);
	opt.fullscreen = opt_section_.get_bool("fullscreen", false);
	opt.sdl_cursor = opt_section_.get_bool("sdl_cursor", true);
	opt.tooltip_accessibility_mode = opt_section_.get_bool("tooltip_accessibility_mode", false);

	// Window options
	opt.dock_windows_to_edges = opt_section_.get_bool("dock_windows_to_edges", false);
	opt.panel_snap_distance = opt_section_.get_int("panel_snap_distance", 0);
	opt.border_snap_distance = opt_section_.get_int("border_snap_distance", 0);

	// Saving options
	opt.autosave = opt_section_.get_int("autosave", kDefaultAutosaveInterval * 60);
	opt.rolling_autosave = opt_section_.get_int("rolling_autosave", 5);
	opt.replay_lifetime = opt_section_.get_int("replay_lifetime", 0);
	opt.skip_autosave_on_inactivity = opt_section_.get_bool("skip_autosave_on_inactivity", true);
	opt.zip = !opt_section_.get_bool("nozip", false);
	opt.save_chat_history = opt_section_.get_bool("save_chat_history", false);

	// Game options
	opt.pause_game_on_inactivity = opt_section_.get_int("pause_game_on_inactivity", 0);
	opt.auto_roadbuild_mode = opt_section_.get_bool("auto_roadbuild_mode", true);
	opt.transparent_chat = opt_section_.get_bool("transparent_chat", true);
	opt.single_watchwin = opt_section_.get_bool("single_watchwin", false);
	opt.game_clock = opt_section_.get_bool("game_clock", true);
	opt.numpad_diagonalscrolling = opt_section_.get_bool("numpad_diagonalscrolling", false);
	opt.edge_scrolling = opt_section_.get_bool("edge_scrolling", false);
	opt.invert_movement = opt_section_.get_bool("invert_movement", false);
	opt.animate_map_panning = opt_section_.get_bool("animate_map_panning", true);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	opt.training_wheels = opt_section_.get_bool("training_wheels", true);
#endif

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
	opt_section_.set_int("display", opt.display);
	opt_section_.set_int("xres", opt.xres);
	opt_section_.set_int("yres", opt.yres);
	opt_section_.set_bool("maximized", opt.maximized);
	opt_section_.set_bool("fullscreen", opt.fullscreen);
	opt_section_.set_bool("sdl_cursor", opt.sdl_cursor);
	opt_section_.set_bool("tooltip_accessibility_mode", opt.tooltip_accessibility_mode);

	// Window options
	opt_section_.set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	opt_section_.set_int("panel_snap_distance", opt.panel_snap_distance);
	opt_section_.set_int("border_snap_distance", opt.border_snap_distance);

	// Saving options
	opt_section_.set_int("autosave", opt.autosave * 60);
	opt_section_.set_int("rolling_autosave", opt.rolling_autosave);
	opt_section_.set_int("replay_lifetime", opt.replay_lifetime);
	opt_section_.set_bool("skip_autosave_on_inactivity", opt.skip_autosave_on_inactivity);
	opt_section_.set_bool("nozip", !opt.zip);
	opt_section_.set_bool("save_chat_history", opt.save_chat_history);

	// Game options
	opt_section_.set_int("pause_game_on_inactivity", opt.pause_game_on_inactivity);
	opt_section_.set_bool("auto_roadbuild_mode", opt.auto_roadbuild_mode);
	opt_section_.set_bool("transparent_chat", opt.transparent_chat);
	opt_section_.set_bool("single_watchwin", opt.single_watchwin);
	opt_section_.set_bool("game_clock", opt.game_clock);
	opt_section_.set_bool("numpad_diagonalscrolling", opt.numpad_diagonalscrolling);
	opt_section_.set_bool("edge_scrolling", opt.edge_scrolling);
	opt_section_.set_bool("invert_movement", opt.invert_movement);
	opt_section_.set_bool("animate_map_panning", opt.animate_map_panning);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	opt_section_.set_bool("training_wheels", opt.training_wheels);
#endif

	// New Game options
	opt_section_.set_int("display_flags", opt.display_flags);

	// Language options
	opt_section_.set_string("language", opt.language);

	g_mouse_cursor->set_use_sdl(opt_dialog_->get_values().sdl_cursor);
	i18n::set_locale(opt.language);
	UI::g_fh->reinitialize_fontset(i18n::get_locale());
	WLApplication::get().init_plugin_shortcuts();  // To update the descnames

	// Sound options
	g_sh->save_config();

	// Now write to file
	write_config();
}

}  // namespace FsMenu
