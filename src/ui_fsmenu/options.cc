/*
 * Copyright (C) 2002-2004, 2006-2010, 2012 by Widelands Development Team
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

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/save_handler.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"

namespace {

// Data model for the entries in the language selection list.
struct LanguageEntry {
	LanguageEntry(const std::string& init_localename,
					  const std::string& init_descname,
					  const std::string& init_sortname) :
		localename(init_localename),
		descname(init_descname),
		sortname(init_sortname) {}

	bool operator<(const LanguageEntry& other) const {
		return sortname < other.sortname;
	}

	std::string localename; // ISO code for the locale
	std::string descname;   // Native language name
	std::string sortname;   // ASCII Language name used for sorting
};

// Locale identifiers can look like this: ca_ES@valencia.UTF-8
// The contents of 'selected_locale' will be changed to match the 'current_locale'
void find_selected_locale(std::string* selected_locale, const std::string& current_locale) {
	if (selected_locale->empty()) {
		std::vector<std::string> parts;
		boost::split(parts, current_locale, boost::is_any_of("."));
		if (current_locale  == parts[0]) {
			*selected_locale = current_locale;
		} else {
			boost::split(parts, parts[0], boost::is_any_of("@"));
			if (current_locale  == parts[0]) {
				*selected_locale = current_locale;
			} else {
				boost::split(parts, parts[0], boost::is_any_of("_"));
				if (current_locale  == parts[0]) {
					*selected_locale = current_locale;
				}
			}
		}
	}
}

}  // namespace

// TODO(GunChleoc): Arabic: This doesn't fit the window in Arabic.
FullscreenMenuOptions::FullscreenMenuOptions
		(OptionsCtrl::OptionsStruct opt)
	:
	FullscreenMenuBase("images/ui_fsmenu/ui_fsmenu.jpg"),

// Values for alignment and size
	butw_    (get_w() / 5),
	buth_    (get_h() * 9 / 200),
	hmargin_ (get_w() * 19 / 200),
	padding_ (10),
	tab_panel_width_(get_inner_w() - 2 * hmargin_),
	column_width_(tab_panel_width_ - padding_),
	tab_panel_y_(get_h() * 14 / 100),

	// Title
	title_
	(this,
	 get_w() / 2, buth_,
	 _("Options"), UI::Align::kHCenter),

	// Buttons
	cancel_
		(this, "cancel",
		 get_w() * 1 / 4 - butw_ / 2,
		 get_inner_h() - hmargin_,
		 butw_, buth_,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 _("Cancel"), std::string(), true, false),
	apply_
		(this, "apply",
		 get_w() * 2 / 4 - butw_ / 2,
		 get_inner_h() - hmargin_,
		 butw_, buth_,
		 g_gr->images().get("images/ui_basic/but0.png"),
		 _("Apply"), std::string(), true, false),
	ok_
		(this, "ok",
		 get_w() * 3 / 4 - butw_ / 2,
		 get_inner_h() - hmargin_,
		 butw_, buth_,
		 g_gr->images().get("images/ui_basic/but2.png"),
		 _("OK"), std::string(), true, false),

	tabs_(this, hmargin_, 0,
			tab_panel_width_, get_inner_h() - tab_panel_y_ - buth_ - hmargin_,
			g_gr->images().get("images/ui_basic/but1.png"),
			UI::TabPanel::Type::kBorder),

	box_interface_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
	box_windows_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
	box_sound_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
	box_saving_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
	box_game_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
	box_language_(&tabs_, 0, 0, UI::Box::Vertical, 0, 0, padding_),

	// Interface options
	label_resolution_(&box_interface_, _("In-game resolution"), UI::Align::kLeft),
	resolution_list_(&box_interface_, 0, 0, column_width_ / 2, 80, true),

	fullscreen_ (&box_interface_, Point(0, 0), _("Fullscreen"), "", column_width_),
	inputgrab_ (&box_interface_, Point(0, 0), _("Grab Input"), "", column_width_),

	sb_maxfps_(&box_interface_, 0, 0, column_width_ / 2, column_width_ / 4,
				  opt.maxfps, 0, 99,
				  _("Maximum FPS:"), ""),


	// Windows options
	snap_win_overlap_only_(&box_windows_, Point(0, 0), _("Snap windows only when overlapping"),
								  "", column_width_),
	dock_windows_to_edges_(&box_windows_, Point(0, 0), _("Dock windows to edges"),
								  "", column_width_),

	sb_dis_panel_
			(&box_windows_, 0, 0, column_width_, 200,
			 opt.panel_snap_distance, 0, 99, _("Distance for windows to snap to other panels:"),
			 /** TRANSLATORS: Options: Distance for windows to snap to  other panels: */
			 /** TRANSLATORS: This will have a number added in front of it */
			 ngettext("pixel", "pixels", opt.panel_snap_distance)),

	sb_dis_border_
			(&box_windows_, 0, 0, column_width_, 200,
			 opt.border_snap_distance, 0, 99,
			 _("Distance for windows to snap to borders:"),
			 /** TRANSLATORS: Options: Distance for windows to snap to borders: */
			 /** TRANSLATORS: This will have a number added in front of it */
			 ngettext("pixel", "pixels", opt.border_snap_distance)),

	// Sound options
	music_ (&box_sound_, Point(0, 0), _("Enable Music"), "", column_width_),
	fx_ (&box_sound_, Point(0, 0), _("Enable Sound Effects"), "", column_width_),
	message_sound_(&box_sound_, Point(0, 0), _("Play a sound at message arrival"),
						"", column_width_),

	// Saving options
	sb_autosave_
		(&box_saving_, 0, 0, column_width_, 250,
		 opt.autosave / 60, 0, 100, _("Save game automatically every"),
		 /** TRANSLATORS: Options: Save game automatically every: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 ngettext("minute", "minutes", opt.autosave / 60),
		 g_gr->images().get("images/ui_basic/but3.png"), UI::SpinBox::Type::kBig),

	sb_rolling_autosave_
		(&box_saving_, 0, 0, column_width_, 250,
		 opt.rolling_autosave, 1, 20, _("Maximum number of autosave files"),
		 "",
		 g_gr->images().get("images/ui_basic/but3.png"), UI::SpinBox::Type::kBig),

	sb_remove_replays_
		(&box_saving_, 0, 0, column_width_, 250,
		 opt.remove_replays, 0, 365, _("Remove replays older than:"),
		 /** TRANSLATORS: Options: Remove Replays older than: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 ngettext("day", "days", opt.remove_replays),
		 g_gr->images().get("images/ui_basic/but3.png"), UI::SpinBox::Type::kBig),

	nozip_(&box_saving_, Point(0, 0), _("Do not zip widelands data files (maps, replays and savegames)"),
			 "", column_width_),
	remove_syncstreams_(&box_saving_, Point(0, 0), _("Remove Syncstream dumps on startup"),
							  "", column_width_),

	// Game options
	auto_roadbuild_mode_(&box_game_, Point(0, 0), _("Start building road after placing a flag")),
	show_workarea_preview_(&box_game_, Point(0, 0), _("Show buildings area preview")),
	transparent_chat_(&box_game_, Point(0, 0), _("Show in-game chat with transparent background"),
							"", column_width_),

	/** TRANSLATORS: A watchwindow is a window where you keep watching an object or a map region,*/
	/** TRANSLATORS: and it also lets you jump to it on the map. */
	single_watchwin_(&box_game_, Point(0, 0), _("Use single watchwindow mode")),

	// Language options
	label_language_(&box_language_, _("Language"), UI::Align::kLeft),
	language_list_(&box_language_, 0, 0, column_width_ / 2,
						get_inner_h() - tab_panel_y_ - buth_ - hmargin_ - 5 * padding_,
						true),

	os_(opt)
{
	// Set up UI Elements
	title_           .set_textstyle(UI::TextStyle::ui_big());

	tabs_.add("options_interface", _("Interface"), &box_interface_, "");
	tabs_.add("options_windows", _("Windows"), &box_windows_, "");
	tabs_.add("options_sound", _("Sound"), &box_sound_, "");
	tabs_.add("options_saving", _("Saving"), &box_saving_, "");
	tabs_.add("options_game", _("Game"), &box_game_, "");
	tabs_.add("options_language", _("Language"), &box_language_, "");

	// We want the last active tab when "Apply" was clicked.
	if (os_.active_tab < tabs_.tabs().size()) {
		tabs_.activate(os_.active_tab);
	}

	tabs_.set_pos(Point(hmargin_, tab_panel_y_));

	box_interface_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_windows_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_sound_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_saving_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_game_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_language_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());

	// Interface
	box_interface_.add(&label_resolution_, UI::Align::kLeft);
	box_interface_.add(&resolution_list_, UI::Align::kLeft);
	box_interface_.add(&fullscreen_, UI::Align::kLeft);
	box_interface_.add(&inputgrab_, UI::Align::kLeft);
	box_interface_.add(&sb_maxfps_, UI::Align::kLeft);

	// Windows
	box_windows_.add(&snap_win_overlap_only_, UI::Align::kLeft);
	box_windows_.add(&dock_windows_to_edges_, UI::Align::kLeft);
	box_windows_.add(&sb_dis_panel_, UI::Align::kLeft);
	box_windows_.add(&sb_dis_border_, UI::Align::kLeft);

	// Sound
	box_sound_.add(&music_, UI::Align::kLeft);
	box_sound_.add(&fx_, UI::Align::kLeft);
	box_sound_.add(&message_sound_, UI::Align::kLeft);

	// Saving
	box_saving_.add(&sb_autosave_, UI::Align::kLeft);
	box_saving_.add(&sb_rolling_autosave_, UI::Align::kLeft);
	box_saving_.add(&sb_remove_replays_, UI::Align::kLeft);
	box_saving_.add(&nozip_, UI::Align::kLeft);
	box_saving_.add(&remove_syncstreams_, UI::Align::kLeft);

	// Game
	box_game_.add(&auto_roadbuild_mode_, UI::Align::kLeft);
	box_game_.add(&show_workarea_preview_, UI::Align::kLeft);
	box_game_.add(&transparent_chat_, UI::Align::kLeft);
	box_game_.add(&single_watchwin_, UI::Align::kLeft);

	// Language
	box_language_.add(&label_language_, UI::Align::kLeft);
	box_language_.add(&language_list_, UI::Align::kLeft);


	// Bind actions
	cancel_.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_back, this));
	apply_.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_apply, this));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_ok, this));

	/** TRANSLATORS Options: Save game automatically every: */
	sb_autosave_     .add_replacement(0, _("Off"));
	for (UI::Button* temp_button : sb_autosave_.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_autosave_unit,
					 boost::ref(*this)));
	}
	/** TRANSLATORS Options: Remove Replays older than: */
	sb_remove_replays_.add_replacement(0, _("Never"));
	for (UI::Button* temp_button : sb_remove_replays_.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_remove_replays_unit,
					 boost::ref(*this)));
	}
	for (UI::Button* temp_button : sb_dis_panel_.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_dis_panel_unit,
					 boost::ref(*this)));
	}

	for (UI::Button* temp_button : sb_dis_border_.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_dis_border_unit,
					 boost::ref(*this)));
	}

	// Fill in data
	// Interface options
	for (int modes = 0; modes < SDL_GetNumDisplayModes(0); ++modes) {
		SDL_DisplayMode  mode;
		SDL_GetDisplayMode(0, modes, & mode);
		if (800 <= mode.w && 600 <= mode.h &&
			 (SDL_BITSPERPIXEL(mode.format) == 32 ||
			  SDL_BITSPERPIXEL(mode.format) == 24)) {
			ScreenResolution this_res = {
				mode.w, mode.h, static_cast<int32_t>(SDL_BITSPERPIXEL(mode.format))};
			if (this_res.depth == 24) this_res.depth = 32;
			if (resolutions_.empty()
				 || this_res.xres != resolutions_.rbegin()->xres
				 || this_res.yres != resolutions_.rbegin()->yres) {
				resolutions_.push_back(this_res);
			}
		}
	}

	bool did_select_a_res = false;
	for (uint32_t i = 0; i < resolutions_.size(); ++i) {
		const bool selected =
			resolutions_[i].xres  == opt.xres &&
			resolutions_[i].yres  == opt.yres;
		did_select_a_res |= selected;
		/** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		resolution_list_.add((boost::format(_("%1% x %2%"))
							% resolutions_[i].xres
							% resolutions_[i].yres).str(),
						  nullptr, nullptr, selected);
	}
	if (!did_select_a_res) {
		resolution_list_.add((boost::format(_("%1% x %2%"))
							% opt.xres
							% opt.yres).str(),
						  nullptr, nullptr, true);
		uint32_t entry = resolutions_.size();
		resolutions_.resize(entry + 1);
		resolutions_[entry].xres = opt.xres;
		resolutions_[entry].yres = opt.yres;
	}

	fullscreen_           .set_state(opt.fullscreen);
	inputgrab_            .set_state(opt.inputgrab);

	// Windows options
	snap_win_overlap_only_.set_state(opt.snap_win_overlap_only);
	dock_windows_to_edges_.set_state(opt.dock_windows_to_edges);

	// Sound options
	music_                .set_state(opt.music);
	music_                .set_enabled(!g_sound_handler.lock_audio_disabling_);
	fx_                   .set_state(opt.fx);
	fx_                   .set_enabled(!g_sound_handler.lock_audio_disabling_);
	message_sound_        .set_state(opt.message_sound);

	// Saving options
	nozip_                .set_state(opt.nozip);
	remove_syncstreams_   .set_state(opt.remove_syncstreams);

	// Game options
	auto_roadbuild_mode_  .set_state(opt.auto_roadbuild_mode);
	show_workarea_preview_.set_state(opt.show_warea);
	transparent_chat_     .set_state(opt.transparent_chat);
	single_watchwin_      .set_state(opt.single_watchwin);

	// Language options
	add_languages_to_list(opt.language);
	language_list_.focus();
}

void FullscreenMenuOptions::update_sb_autosave_unit() {
	sb_autosave_.set_unit(ngettext("minute", "minutes", sb_autosave_.get_value()));
}

void FullscreenMenuOptions::update_sb_remove_replays_unit() {
	sb_remove_replays_.set_unit(ngettext("day", "days", sb_remove_replays_.get_value()));
}

void FullscreenMenuOptions::update_sb_dis_panel_unit() {
	sb_dis_panel_.set_unit(ngettext("pixel", "pixels", sb_dis_panel_.get_value()));
}

void FullscreenMenuOptions::update_sb_dis_border_unit() {
	sb_dis_border_.set_unit(ngettext("pixel", "pixels", sb_dis_border_.get_value()));
}

void FullscreenMenuOptions::add_languages_to_list(const std::string& current_locale) {

	// We want these two entries on top - the most likely user's choice and the default.
	language_list_.add(_("Try system language"), "", nullptr, current_locale == "");
	language_list_.add("English", "en", nullptr, current_locale == "en");

	// Add translation directories to the list
	std::vector<LanguageEntry> entries;
	std::string selected_locale;

	try {  // Begin read locales table
		LuaInterface lua;
		std::unique_ptr<LuaTable> all_locales(lua.run_script("i18n/locales.lua"));
		all_locales->do_not_warn_about_unaccessed_keys(); // We are only reading partial information as needed

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
				entries.push_back(LanguageEntry(localename, name, sortname));

				if (localename == current_locale) {
					selected_locale = current_locale;
				}

			} catch (const WException&) {
				log("Could not read locale for: %s\n", localename.c_str());
				entries.push_back(LanguageEntry(localename, localename, localename));
			}  // End read locale from table
		}  // End scan locales directory
	} catch (const LuaError& err) {
		log("Could not read locales information from file: %s\n", err.what());
		return;  // Nothing more can be done now.
	}  // End read locales table

	find_selected_locale(&selected_locale, current_locale);
	std::sort(entries.begin(), entries.end());
	for (const LanguageEntry& entry : entries) {
		language_list_.add(entry.descname.c_str(), entry.localename, nullptr,
									entry.localename == selected_locale, "");
	}
}


void FullscreenMenuOptions::clicked_apply() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kApplyOptions);
}


OptionsCtrl::OptionsStruct FullscreenMenuOptions::get_values() {
	// Write all data from UI elements
	// Interface options
	const uint32_t res_index  = resolution_list_.selection_index();
	os_.xres                  = resolutions_[res_index].xres;
	os_.yres                  = resolutions_[res_index].yres;
	os_.fullscreen            = fullscreen_.get_state();
	os_.inputgrab             = inputgrab_.get_state();
	os_.maxfps                = sb_maxfps_.get_value();

	// Windows options
	os_.snap_win_overlap_only = snap_win_overlap_only_.get_state();
	os_.dock_windows_to_edges = dock_windows_to_edges_.get_state();
	os_.panel_snap_distance   = sb_dis_panel_.get_value();
	os_.border_snap_distance  = sb_dis_border_.get_value();

	// Sound options
	os_.music                 = music_.get_state();
	os_.fx                    = fx_.get_state();
	os_.message_sound         = message_sound_.get_state();

	// Saving options
	os_.autosave              = sb_autosave_.get_value();
	os_.rolling_autosave      = sb_rolling_autosave_.get_value();
	os_.remove_replays        = sb_remove_replays_.get_value();
	os_.nozip                 = nozip_.get_state();
	os_.remove_syncstreams    = remove_syncstreams_.get_state();

	// Game options
	os_.auto_roadbuild_mode   = auto_roadbuild_mode_.get_state();
	os_.show_warea            = show_workarea_preview_.get_state();
	os_.transparent_chat      = transparent_chat_.get_state();
	os_.single_watchwin       = single_watchwin_.get_state();

	// Language options
	if (language_list_.has_selection()) {
		os_.language           = language_list_.get_selected();
	}

	// Last tab for reloading the options menu
	os_.active_tab            = tabs_.active();
	return os_;
}


/**
 * Handles communication between window class and options
 */
OptionsCtrl::OptionsCtrl(Section & s)
: opt_section_(s),
  opt_dialog_(std::unique_ptr<FullscreenMenuOptions>(new FullscreenMenuOptions(options_struct(0))))
{
	handle_menu();
}

void OptionsCtrl::handle_menu()
{
	FullscreenMenuBase::MenuTarget i = opt_dialog_->run<FullscreenMenuBase::MenuTarget>();
	if (i != FullscreenMenuBase::MenuTarget::kBack)
		save_options();
	if (i == FullscreenMenuBase::MenuTarget::kApplyOptions) {
		uint32_t active_tab = opt_dialog_->get_values().active_tab;
		g_gr->change_resolution(opt_dialog_->get_values().xres, opt_dialog_->get_values().yres);
		g_gr->set_fullscreen(opt_dialog_->get_values().fullscreen);
		opt_dialog_.reset(new FullscreenMenuOptions(options_struct(active_tab)));
		handle_menu(); // Restart general options menu
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

	// Windows options
	opt.snap_win_overlap_only =
		opt_section_.get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges = opt_section_.get_bool("dock_windows_to_edges", false);
	opt.panel_snap_distance = opt_section_.get_int("panel_snap_distance", 0);
	opt.border_snap_distance = opt_section_.get_int("border_snap_distance", 0);

	// Sound options
	opt.music = !opt_section_.get_bool("disable_music", false);
	opt.fx = !opt_section_.get_bool("disable_fx", false);
	opt.message_sound = opt_section_.get_bool("sound_at_message", true);

	// Saving options
	opt.autosave = opt_section_.get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
	opt.rolling_autosave = opt_section_.get_int("rolling_autosave", 5);
	opt.remove_replays = opt_section_.get_int("remove_replays", 0);
	opt.nozip = opt_section_.get_bool("nozip", false);
	opt.remove_syncstreams = opt_section_.get_bool("remove_syncstreams", true);

	// Game options
	opt.auto_roadbuild_mode = opt_section_.get_bool("auto_roadbuild_mode", true);
	opt.show_warea = opt_section_.get_bool("workareapreview", true);
	opt.transparent_chat = opt_section_.get_bool("transparent_chat", true);
	opt.single_watchwin = opt_section_.get_bool("single_watchwin", false);

	// Language options
	opt.language = opt_section_.get_string("language", "");

	// Last tab for reloading the options menu
	opt.active_tab = active_tab;
	return opt;
}

void OptionsCtrl::save_options() {
	OptionsCtrl::OptionsStruct opt = opt_dialog_->get_values();

	// Interface options
	opt_section_.set_int ("xres",                  opt.xres);
	opt_section_.set_int ("yres",                  opt.yres);
	opt_section_.set_bool("fullscreen",            opt.fullscreen);
	opt_section_.set_bool("inputgrab",             opt.inputgrab);
	opt_section_.set_int("maxfps",                 opt.maxfps);

	// Windows options
	opt_section_.set_bool
		("snap_windows_only_when_overlapping",
		 opt.snap_win_overlap_only);
	opt_section_.set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	opt_section_.set_int("panel_snap_distance",    opt.panel_snap_distance);
	opt_section_.set_int("border_snap_distance",   opt.border_snap_distance);

	// Sound options
	opt_section_.set_bool("disable_music",        !opt.music);
	opt_section_.set_bool("disable_fx",           !opt.fx);
	opt_section_.set_bool("sound_at_message",      opt.message_sound);

	// Saving options
	opt_section_.set_int("autosave",               opt.autosave * 60);
	opt_section_.set_int("rolling_autosave",       opt.rolling_autosave);
	opt_section_.set_int("remove_replays",         opt.remove_replays);
	opt_section_.set_bool("nozip",                 opt.nozip);
	opt_section_.set_bool("remove_syncstreams",    opt.remove_syncstreams);

	// Game options
	opt_section_.set_bool("auto_roadbuild_mode",   opt.auto_roadbuild_mode);
	opt_section_.set_bool("workareapreview",       opt.show_warea);
	opt_section_.set_bool("transparent_chat",      opt.transparent_chat);
	opt_section_.set_bool("single_watchwin",       opt.single_watchwin);

	// Language options
	opt_section_.set_string("language",            opt.language);

	WLApplication::get()->set_input_grab(opt.inputgrab);
	i18n::set_locale(opt.language);
	UI::g_fh1->reinitialize_fontset();
	g_sound_handler.set_disable_music(!opt.music);
	g_sound_handler.set_disable_fx(!opt.fx);
}
