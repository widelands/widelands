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

namespace {

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

FullscreenMenuOptions::FullscreenMenuOptions(OptionsCtrl::OptionsStruct opt)
   : FullscreenMenuBase(),

     // Values for alignment and size
     padding_(10),

     // Title
     title_(this,
            0,
            0,
            0,
            0,
            _("Options"),
            UI::Align::kCenter,
            g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),

     // Buttons
     button_box_(this, 0, 0, UI::Box::Horizontal),
     cancel_(&button_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel")),
     apply_(&button_box_, "apply", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Apply")),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")),

     // Tabs
     tabs_(this, UI::TabPanelStyle::kFsMenu),

     box_interface_(&tabs_, 0, 0, UI::Box::Horizontal, 0, 0, padding_),
     box_interface_left_(&box_interface_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     box_windows_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     box_sound_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     box_saving_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     box_game_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),

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

     fullscreen_(&box_interface_left_, Vector2i::zero(), _("Fullscreen"), "", 0),
     inputgrab_(&box_interface_left_, Vector2i::zero(), _("Grab Input"), "", 0),
     sdl_cursor_(&box_interface_left_, Vector2i::zero(), _("Use system mouse cursor"), "", 0),
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
     snap_win_overlap_only_(
        &box_windows_, Vector2i::zero(), _("Snap windows only when overlapping"), "", 0),
     dock_windows_to_edges_(&box_windows_, Vector2i::zero(), _("Dock windows to edges"), "", 0),
     animate_map_panning_(
        &box_windows_, Vector2i::zero(), _("Animate automatic map movements"), "", 0),

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
          Vector2i::zero(),
          _("Compress widelands data files (maps, replays and savegames)"),
          "",
          0),
     write_syncstreams_(&box_saving_,
                        Vector2i::zero(),
                        _("Write syncstreams in network games to debug desyncs"),
                        "",
                        0),

     // Game options
     auto_roadbuild_mode_(
        &box_game_, Vector2i::zero(), _("Start building road after placing a flag")),
     transparent_chat_(
        &box_game_, Vector2i::zero(), _("Show in-game chat with transparent background"), "", 0),

     /** TRANSLATORS: A watchwindow is a window where you keep watching an object or a map region,*/
     /** TRANSLATORS: and it also lets you jump to it on the map. */
     single_watchwin_(&box_game_, Vector2i::zero(), _("Use single watchwindow mode")),
     /** TRANSLATORS: This refers to to zooming with the scrollwheel.*/
     ctrl_zoom_(&box_game_, Vector2i::zero(), _("Zoom only when Ctrl is pressed")),
     game_clock_(&box_game_, Vector2i::zero(), _("Display game time in the top left corner")),
     os_(opt) {

	// Buttons
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_ : &cancel_);
	button_box_.add_inf_space();
	button_box_.add(&apply_);
	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &cancel_ : &ok_);

	// Tabs
	tabs_.add("options_interface", _("Interface"), &box_interface_, "");
	tabs_.add("options_windows", _("Windows"), &box_windows_, "");
	tabs_.add("options_sound", _("Sound"), &box_sound_, "");
	tabs_.add("options_saving", _("Saving"), &box_saving_, "");
	tabs_.add("options_game", _("Game"), &box_game_, "");

	// We want the last active tab when "Apply" was clicked.
	if (os_.active_tab < tabs_.tabs().size()) {
		tabs_.activate(os_.active_tab);
	}

	// Interface
	box_interface_.add(&box_interface_left_);
	box_interface_.add(&translation_info_, UI::Box::Resizing::kExpandBoth);
	box_interface_left_.add(&language_dropdown_);
	box_interface_left_.add(&resolution_dropdown_);
	box_interface_left_.add(&fullscreen_);
	box_interface_left_.add(&inputgrab_);
	box_interface_left_.add(&sdl_cursor_);
	box_interface_left_.add(&sb_maxfps_);

	// Windows
	box_windows_.add(&snap_win_overlap_only_);
	box_windows_.add(&dock_windows_to_edges_);
	box_windows_.add(&animate_map_panning_);
	box_windows_.add(&sb_dis_panel_);
	box_windows_.add(&sb_dis_border_);

	// Sound
	box_sound_.add(&sound_options_);

	// Saving
	box_saving_.add(&sb_autosave_);
	box_saving_.add(&sb_rolling_autosave_);
	box_saving_.add(&zip_);
	box_saving_.add(&write_syncstreams_);

	// Game
	box_game_.add(&auto_roadbuild_mode_);
	box_game_.add(&transparent_chat_);
	box_game_.add(&single_watchwin_);
	box_game_.add(&ctrl_zoom_);
	box_game_.add(&game_clock_);

	// Bind actions
	language_dropdown_.selected.connect([this]() { update_language_stats(false); });
	cancel_.sigclicked.connect([this]() { clicked_cancel(); });
	apply_.sigclicked.connect([this]() { clicked_apply(); });
	ok_.sigclicked.connect([this]() { clicked_ok(); });

	/** TRANSLATORS: Options: Save game automatically every: */
	sb_autosave_.add_replacement(0, _("Off"));

	// Fill in data
	// Interface options
	for (int modes = 0; modes < SDL_GetNumDisplayModes(0); ++modes) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, modes, &mode);
		if (800 <= mode.w && 600 <= mode.h &&
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

	bool did_select_a_res = false;
	for (uint32_t i = 0; i < resolutions_.size(); ++i) {
		const bool selected = resolutions_[i].xres == opt.xres && resolutions_[i].yres == opt.yres;
		did_select_a_res |= selected;
		resolution_dropdown_.add(
		   /** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		   (boost::format(_("%1% x %2%")) % resolutions_[i].xres % resolutions_[i].yres).str(), i,
		   nullptr, selected);
	}
	if (!did_select_a_res) {
		uint32_t entry = resolutions_.size();
		resolutions_.resize(entry + 1);
		resolutions_[entry].xres = opt.xres;
		resolutions_[entry].yres = opt.yres;
		resolution_dropdown_.add(
		   (boost::format(_("%1% x %2%")) % opt.xres % opt.yres).str(), entry, nullptr, true);
	}

	fullscreen_.set_state(opt.fullscreen);
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

	// Language options
	add_languages_to_list(opt.language);
	update_language_stats(true);
	layout();
}

void FullscreenMenuOptions::layout() {

	// Values for alignment and size
	butw_ = get_w() / 5;
	buth_ = get_h() * 9 / 200;
	hmargin_ = get_w() * 19 / 200;
	int tab_panel_width = get_inner_w() - 2 * hmargin_;
	tab_panel_y_ = get_h() * 14 / 100;

	// Title
	title_.set_size(get_w(), title_.get_h());
	title_.set_pos(Vector2i(0, buth_));

	// Buttons
	cancel_.set_desired_size(butw_, buth_);
	apply_.set_desired_size(butw_, buth_);
	ok_.set_desired_size(butw_, buth_);
	button_box_.set_pos(Vector2i(hmargin_ + butw_ / 3, get_inner_h() - hmargin_));
	button_box_.set_size(tab_panel_width - 2 * butw_ / 3, buth_);

	// Tabs
	tabs_.set_pos(Vector2i(hmargin_, tab_panel_y_));
	tabs_.set_size(tab_panel_width, get_inner_h() - tab_panel_y_ - buth_ - hmargin_);

	tab_panel_width -= padding_;
	const int column_width = tab_panel_width / 2;

	// Interface
	box_interface_left_.set_desired_size(column_width + padding_, tabs_.get_inner_h());
	box_interface_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	language_dropdown_.set_desired_size(column_width, language_dropdown_.get_h());
	language_dropdown_.set_height(tabs_.get_h() - language_dropdown_.get_y() - buth_ - 3 * padding_);
	resolution_dropdown_.set_desired_size(column_width, resolution_dropdown_.get_h());
	resolution_dropdown_.set_height(tabs_.get_h() - resolution_dropdown_.get_y() - buth_ -
	                                3 * padding_);

	fullscreen_.set_desired_size(column_width, fullscreen_.get_h());
	inputgrab_.set_desired_size(column_width, inputgrab_.get_h());
	sdl_cursor_.set_desired_size(column_width, sdl_cursor_.get_h());
	sb_maxfps_.set_unit_width(column_width / 2);
	sb_maxfps_.set_desired_size(column_width, sb_maxfps_.get_h());

	// Windows options
	snap_win_overlap_only_.set_desired_size(tab_panel_width, snap_win_overlap_only_.get_h());
	dock_windows_to_edges_.set_desired_size(tab_panel_width, dock_windows_to_edges_.get_h());
	animate_map_panning_.set_desired_size(tab_panel_width, animate_map_panning_.get_h());
	sb_dis_panel_.set_unit_width(200);
	sb_dis_panel_.set_desired_size(tab_panel_width, sb_dis_panel_.get_h());
	sb_dis_border_.set_unit_width(200);
	sb_dis_border_.set_desired_size(tab_panel_width, sb_dis_border_.get_h());

	// Sound options
	sound_options_.set_desired_size(tab_panel_width, tabs_.get_inner_h());

	// Saving options
	sb_autosave_.set_unit_width(250);
	sb_autosave_.set_desired_size(tab_panel_width, sb_autosave_.get_h());
	sb_rolling_autosave_.set_unit_width(250);
	sb_rolling_autosave_.set_desired_size(tab_panel_width, sb_rolling_autosave_.get_h());
	zip_.set_desired_size(tab_panel_width, zip_.get_h());
	write_syncstreams_.set_desired_size(tab_panel_width, write_syncstreams_.get_h());

	// Game options
	auto_roadbuild_mode_.set_desired_size(tab_panel_width, auto_roadbuild_mode_.get_h());
	transparent_chat_.set_desired_size(tab_panel_width, transparent_chat_.get_h());
	single_watchwin_.set_desired_size(tab_panel_width, single_watchwin_.get_h());
	ctrl_zoom_.set_desired_size(tab_panel_width, ctrl_zoom_.get_h());
	game_clock_.set_desired_size(tab_panel_width, game_clock_.get_h());
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
				log("Could not read locale for: %s\n", localename.c_str());
				entries.insert(std::make_pair(localename, LanguageEntry(localename, localename)));
			}  // End read locale from table
		}     // End scan locales directory
	} catch (const LuaError& err) {
		log("Could not read locales information from file: %s\n", err.what());
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
void FullscreenMenuOptions::update_language_stats(bool include_system_lang) {
	int percent = 100;
	std::string message = "";
	if (language_dropdown_.has_selection()) {
		std::string locale = language_dropdown_.get_selected();
		// Empty locale means try system locale
		if (locale.empty() && include_system_lang) {
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
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kApplyOptions);
}

void FullscreenMenuOptions::clicked_cancel() {
	g_sh->load_config();
	clicked_back();
}

OptionsCtrl::OptionsStruct FullscreenMenuOptions::get_values() {
	// Write all data from UI elements
	// Interface options
	if (language_dropdown_.has_selection()) {
		os_.language = language_dropdown_.get_selected();
	}
	if (resolution_dropdown_.has_selection()) {
		const uint32_t res_index = resolution_dropdown_.get_selected();
		os_.xres = resolutions_[res_index].xres;
		os_.yres = resolutions_[res_index].yres;
	}
	os_.fullscreen = fullscreen_.get_state();
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

	// Last tab for reloading the options menu
	os_.active_tab = tabs_.active();
	return os_;
}

/**
 * Handles communication between window class and options
 */
OptionsCtrl::OptionsCtrl(Section& s)
   : opt_section_(s),
     opt_dialog_(
        std::unique_ptr<FullscreenMenuOptions>(new FullscreenMenuOptions(options_struct(0)))) {
	handle_menu();
}

void OptionsCtrl::handle_menu() {
	FullscreenMenuBase::MenuTarget i = opt_dialog_->run<FullscreenMenuBase::MenuTarget>();
	if (i != FullscreenMenuBase::MenuTarget::kBack) {
		save_options();
	}
	if (i == FullscreenMenuBase::MenuTarget::kApplyOptions) {
		uint32_t active_tab = opt_dialog_->get_values().active_tab;
		g_gr->change_resolution(opt_dialog_->get_values().xres, opt_dialog_->get_values().yres);
		g_gr->set_fullscreen(opt_dialog_->get_values().fullscreen);
		opt_dialog_.reset(new FullscreenMenuOptions(options_struct(active_tab)));
		handle_menu();  // Restart general options menu
	}
}

OptionsCtrl::OptionsStruct OptionsCtrl::options_struct(uint32_t active_tab) {
	OptionsStruct opt;
	// Interface options
	opt.xres = opt_section_.get_int("xres", DEFAULT_RESOLUTION_W);
	opt.yres = opt_section_.get_int("yres", DEFAULT_RESOLUTION_H);
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
