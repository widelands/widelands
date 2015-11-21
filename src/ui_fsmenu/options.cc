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
					  const std::string& init_sortname,
					  const std::string& init_fontname) :
		localename(init_localename),
		descname(init_descname),
		sortname(init_sortname),
		fontname(init_fontname) {}

	bool operator<(const LanguageEntry& other) const {
		return sortname < other.sortname;
	}

	std::string localename; // ISO code for the locale
	std::string descname;   // Native language name
	std::string sortname;   // ASCII Language name used for sorting
	std::string fontname;   // Name of the font with which the language name is displayed.
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
	FullscreenMenuBase("ui_fsmenu.jpg"),

// Values for alignment and size
	m_vbutw   (get_h() * 333 / 10000),
	m_butw    (get_w() / 4),
	m_buth    (get_h() * 9 / 200),
	m_hmargin (get_w() * 19 / 200),
	m_padding (10),
	m_space   (25),
	m_offset_first_group (get_h() * 1417 / 10000),
	m_offset_second_group(get_h() * 5833 / 10000),

	// Title
	m_title
	(this,
	 get_w() / 2, get_h() / 40,
	 _("Options"), UI::Align_HCenter),

	// Buttons
	m_cancel
		(this, "cancel",
		 get_w() * 51 / 80, get_h() * 19 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Cancel"), std::string(), true, false),
	m_apply
		(this, "apply",
		 get_w() * 3 / 8, get_h() * 19 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Apply"), std::string(), true, false),


	tabs_(this, 0, 0, nullptr),
	box_interface_(&tabs_, 0, 0, UI::Box::Horizontal),
	box_interface_column1_(&box_interface_, 0, 0, UI::Box::Vertical),
	box_interface_column2_(&box_interface_, 0, 0, UI::Box::Vertical),
	box_sound_(&tabs_, 0, 0, UI::Box::Vertical),
	box_saving_(&tabs_, 0, 0, UI::Box::Vertical),
	box_gamecontrol_(&tabs_, 0, 0, UI::Box::Vertical),

	// Interface options
	label_resolution_
		(&box_interface_column1_,
		 0, 0,
		 _("In-game resolution"), UI::Align_VCenter),
	resolution_list_
		(&box_interface_column1_,
		 0, 0,
		 (get_w() - 2 * m_hmargin - m_space) / 2, 95,
		 UI::Align_Left, true),
	fullscreen_ (&box_interface_column1_, Point(0, 0), _("Fullscreen")),
	inputgrab_ (&box_interface_column1_, Point(0, 0), _("Grab Input")),

	label_maxfps_(&box_interface_column1_, 0, 0, _("Maximum FPS:"), UI::Align_VCenter),
	sb_maxfps_(&box_interface_column1_, 0, 0, 240, opt.maxfps, 0, 99, ""),

	snap_win_overlap_only_(&box_interface_column1_, Point(0, 0), _("Snap windows only when overlapping")),
	dock_windows_to_edges_(&box_interface_column1_, Point(0, 0), _("Dock windows to edges")),

	label_snap_dis_panel_(&box_interface_column1_, 0, 0, _("Distance for windows to snap to other panels:")),
	sb_dis_panel_
			(&box_interface_column1_, 0, 0, 240,
			 opt.panel_snap_distance, 0, 99, ngettext("pixel", "pixels", opt.panel_snap_distance)),

	label_snap_dis_border_(&box_interface_column1_, 0, 0, _("Distance for windows to snap to borders:")),
	sb_dis_border_
			(&box_interface_column1_, 0, 0, 240,
			 opt.border_snap_distance, 0, 99, ngettext("pixel", "pixels", opt.border_snap_distance)),

	transparent_chat_(&box_interface_column1_, Point(0, 0), _("Show in-game chat with transparent background")),

	label_language_(&box_interface_column2_, 0, 0, _("Language"), UI::Align_VCenter),
	language_list_
		(&box_interface_column2_, 0, 0,
		 (get_w() - 2 * m_hmargin - m_space) / 2, resolution_list_.get_h(),
		 UI::Align_Left, true),


	// Sound options
	music_ (&box_sound_, Point(0, 0), _("Enable Music")),
	fx_ (&box_sound_, Point(0, 0), _("Enable Sound Effects")),
	message_sound_(&box_sound_, Point(0, 0), _("Play a sound at message arrival")),

	// Saving options
	sb_autosave_
		(&box_saving_, 0, 0, 240,
		 /** TRANSLATORS: Options: Save game automatically every: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 opt.autosave / 60, 0, 100, ngettext("minute", "minutes", opt.autosave / 60),
		 g_gr->images().get("pics/but3.png"), true),
	label_autosave_
		(&box_saving_, 0, 0,
		 get_w() - sb_autosave_.get_w() - 2 * m_hmargin,
		 dock_windows_to_edges_.get_h(),
		 _("Save game automatically every"), UI::Align_VCenter),

	sb_remove_replays_
		(&box_saving_, 0, 0, 240,
		 /** TRANSLATORS: Options: Remove Replays older than: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 opt.remove_replays, 0, 365, ngettext("day", "days", opt.remove_replays),
		 g_gr->images().get("pics/but3.png"), true),
	label_remove_replays_
		(&box_saving_, 0, 0,
		 get_w() - sb_remove_replays_.get_w() - 2 * m_hmargin,
		 dock_windows_to_edges_.get_h(),
		 _("Remove replays older than:"), UI::Align_VCenter),
	nozip_(&box_saving_, Point(0, 0), _("Do not zip widelands data files (maps, replays and savegames)")),
	remove_syncstreams_(&box_saving_, Point(0, 0), _("Remove Syncstream dumps on startup")),

	// Game Control options
	/** TRANSLATORS: A watchwindow is a window where you keep watching an object or a map region,*/
	/** TRANSLATORS: and it also lets you jump to it on the map. */
	single_watchwin_(&box_gamecontrol_, Point(0, 0), _("Use single watchwindow mode")),
	auto_roadbuild_mode_(&box_gamecontrol_, Point(0, 0), _("Start building road after placing a flag")),
	show_workarea_preview_(&box_gamecontrol_, Point(0, 0), _("Show buildings area preview")),

	os(opt)
{
	tabs_.add("options_interface",
				 g_gr->images().get("pics/but1.png"), // NOCOM need image. Or define text tabs?
				 &box_interface_,
				 _("Interface"));
	tabs_.add("options_sound",
				 g_gr->images().get("pics/but1.png"), // NOCOM need image. Or define text tabs?
				 &box_sound_,
				 _("Sound"));
	tabs_.add("options_saving",
				 g_gr->images().get("pics/but1.png"), // NOCOM need image. Or define text tabs?
				 &box_saving_,
				 _("Saving"));
	tabs_.add("options_gamecontrol",
				 g_gr->images().get("pics/but1.png"), // NOCOM need image. Or define text tabs?
				 &box_gamecontrol_,
				 _("Game Control"));

	tabs_.set_size(get_inner_w(), get_inner_h() - m_offset_first_group - m_buth);
	tabs_.set_pos(Point(0, m_offset_first_group));

	box_interface_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_sound_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_saving_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	box_gamecontrol_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());

	// Interface
	box_interface_column1_.set_size(box_interface_.get_inner_w() / 2, box_interface_.get_inner_h());
	box_interface_column2_.set_size(box_interface_.get_inner_w() / 2, box_interface_.get_inner_h());

	box_interface_.add(&box_interface_column1_, UI::Align_Left);
	box_interface_.add(&box_interface_column2_, UI::Align_Left);

	box_interface_column1_.add(&label_resolution_, UI::Align_Left);
	box_interface_column1_.add(&resolution_list_, UI::Align_Left);
	box_interface_column1_.add(&fullscreen_, UI::Align_Left);
	box_interface_column1_.add(&inputgrab_, UI::Align_Left);
	box_interface_column1_.add(&label_maxfps_, UI::Align_Left);
	box_interface_column1_.add(&sb_maxfps_, UI::Align_Left);
	box_interface_column1_.add(&snap_win_overlap_only_, UI::Align_Left);
	box_interface_column1_.add(&dock_windows_to_edges_, UI::Align_Left);
	box_interface_column1_.add(&label_snap_dis_panel_, UI::Align_Left);
	box_interface_column1_.add(&sb_dis_panel_, UI::Align_Left);
	box_interface_column1_.add(&label_snap_dis_border_, UI::Align_Left);
	box_interface_column1_.add(&sb_dis_border_, UI::Align_Left);
	box_interface_column1_.add(&transparent_chat_, UI::Align_Left);

	box_interface_column2_.add(&label_language_, UI::Align_Left);
	box_interface_column2_.add(&language_list_, UI::Align_Left);

	// Sound
	box_sound_.add(&music_, UI::Align_Left);
	box_sound_.add(&fx_, UI::Align_Left);
	box_sound_.add(&message_sound_, UI::Align_Left);

	// Saving
	box_saving_.add(&label_autosave_, UI::Align_Left);
	box_saving_.add(&sb_autosave_, UI::Align_Left);
	box_saving_.add(&label_remove_replays_, UI::Align_Left);
	box_saving_.add(&sb_remove_replays_, UI::Align_Left);
	box_saving_.add(&nozip_, UI::Align_Left);
	box_saving_.add(&remove_syncstreams_, UI::Align_Left);

	// Game control
	box_gamecontrol_.add(&single_watchwin_, UI::Align_Left);
	box_gamecontrol_.add(&auto_roadbuild_mode_, UI::Align_Left);
	box_gamecontrol_.add(&show_workarea_preview_, UI::Align_Left);

	m_cancel.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_back, this));
	m_apply.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_ok, this));

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

	m_title           .set_textstyle(UI::TextStyle::ui_big());
	fullscreen_      .set_state(opt.fullscreen);
	inputgrab_       .set_state(opt.inputgrab);
	music_           .set_state(opt.music);
	music_           .set_enabled(!g_sound_handler.lock_audio_disabling_);
	fx_              .set_state(opt.fx);
	fx_              .set_enabled(!g_sound_handler.lock_audio_disabling_);

	single_watchwin_                .set_state(opt.single_watchwin);
	auto_roadbuild_mode_            .set_state(opt.auto_roadbuild_mode);
	show_workarea_preview_          .set_state(opt.show_warea);
	snap_win_overlap_only_          .set_state(opt.snap_win_overlap_only);
	dock_windows_to_edges_          .set_state(opt.dock_windows_to_edges);

	for (int modes = 0; modes < SDL_GetNumDisplayModes(0); ++modes) {
		SDL_DisplayMode  mode;
		SDL_GetDisplayMode(0, modes, & mode);
		if (800 <= mode.w && 600 <= mode.h &&
			 (SDL_BITSPERPIXEL(mode.format) == 32 ||
			  SDL_BITSPERPIXEL(mode.format) == 24)) {
			ScreenResolution this_res = {
			   mode.w, mode.h, static_cast<int32_t>(SDL_BITSPERPIXEL(mode.format))};
			if (this_res.depth == 24) this_res.depth = 32;
			if (m_resolutions.empty()
				 || this_res.xres != m_resolutions.rbegin()->xres
				 || this_res.yres != m_resolutions.rbegin()->yres) {
				m_resolutions.push_back(this_res);
			}
		}
	}

	bool did_select_a_res = false;
	for (uint32_t i = 0; i < m_resolutions.size(); ++i) {
		const bool selected =
			m_resolutions[i].xres  == opt.xres &&
			m_resolutions[i].yres  == opt.yres;
		did_select_a_res |= selected;
		/** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		resolution_list_.add((boost::format(_("%1% x %2%"))
							% m_resolutions[i].xres
							% m_resolutions[i].yres).str(),
						  nullptr, nullptr, selected);
	}
	if (!did_select_a_res) {
		resolution_list_.add((boost::format(_("%1% x %2%"))
							% opt.xres
							% opt.yres).str(),
						  nullptr, nullptr, true);
		uint32_t entry = m_resolutions.size();
		m_resolutions.resize(entry + 1);
		m_resolutions[entry].xres  = opt.xres;
		m_resolutions[entry].yres  = opt.yres;
	}

	add_languages_to_list(opt.language);
	language_list_.focus();

	message_sound_        .set_state(opt.message_sound);
	nozip_                .set_state(opt.nozip);
	remove_syncstreams_   .set_state(opt.remove_syncstreams);
	transparent_chat_     .set_state(opt.transparent_chat);
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

	// We start with the locale directory so we can pick up locales
	// that don't have a configuration file yet.
	FilenameSet files = g_fs->list_directory("locale");

	// Add translation directories to the list
	std::vector<LanguageEntry> entries;
	std::string localename;
	std::string selected_locale;

	try {  // Begin read locales table
		LuaInterface lua;
		std::unique_ptr<LuaTable> all_locales(lua.run_script("i18n/locales.lua"));
		all_locales->do_not_warn_about_unaccessed_keys(); // We are only reading partial information as needed

		for (const std::string& filename : files) {  // Begin scan locales directory
			char const* const path = filename.c_str();
			if (!strcmp(FileSystem::fs_filename(path), ".") ||
				 !strcmp(FileSystem::fs_filename(path), "..") || !g_fs->is_directory(path)) {
				continue;
			}

			try {  // Begin read locale from table
				localename = g_fs->filename_without_ext(path);

				std::unique_ptr<LuaTable> table = all_locales->get_table(localename);
				table->do_not_warn_about_unaccessed_keys();

				std::string name = i18n::make_ligatures(table->get_string("name").c_str());
				const std::string sortname = table->get_string("sort_name");
				std::unique_ptr<UI::FontSet> fontset(new UI::FontSet(localename));

				entries.push_back(LanguageEntry(localename, name, sortname, fontset->serif()));

				if (localename == current_locale) {
					selected_locale = current_locale;
				}

			} catch (const WException&) {
				log("Could not read locale for: %s\n", localename.c_str());
				entries.push_back(LanguageEntry(localename, localename, localename, UI::FontSet::kFallbackFont));
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
									entry.localename == selected_locale, "", entry.fontname);
	}
}


OptionsCtrl::OptionsStruct FullscreenMenuOptions::get_values() {
	const uint32_t res_index = resolution_list_.selection_index();

	// Write all data from UI elements
	os.xres                  = m_resolutions[res_index].xres;
	os.yres                  = m_resolutions[res_index].yres;
	os.inputgrab             = inputgrab_.get_state();
	os.fullscreen            = fullscreen_.get_state();
	os.single_watchwin       = single_watchwin_.get_state();
	os.auto_roadbuild_mode   = auto_roadbuild_mode_.get_state();
	os.show_warea            = show_workarea_preview_.get_state();
	os.snap_win_overlap_only = snap_win_overlap_only_.get_state();
	os.dock_windows_to_edges = dock_windows_to_edges_.get_state();
	os.music                 = music_.get_state();
	os.fx                    = fx_.get_state();
	if (language_list_.has_selection())
		os.language           = language_list_.get_selected();
	os.autosave              = sb_autosave_.get_value();
	os.maxfps                = sb_maxfps_.get_value();
	os.remove_replays        = sb_remove_replays_.get_value();
		os.message_sound        = message_sound_.get_state();
	os.nozip                = nozip_.get_state();
	os.panel_snap_distance  = sb_dis_panel_.get_value();
	os.border_snap_distance = sb_dis_border_.get_value();
	os.remove_syncstreams   = remove_syncstreams_.get_state();
	os.transparent_chat     = transparent_chat_.get_state();

	return os;
}



/**
 * Handles communication between window class and options
 */
OptionsCtrl::OptionsCtrl(Section & s)
: m_opt_section(s), m_opt_dialog(new FullscreenMenuOptions(options_struct()))
{
	handle_menu();
}

OptionsCtrl::~OptionsCtrl() {
	delete m_opt_dialog;
}

void OptionsCtrl::handle_menu()
{
	FullscreenMenuBase::MenuTarget i = m_opt_dialog->run<FullscreenMenuBase::MenuTarget>();
	if (i != FullscreenMenuBase::MenuTarget::kBack)
		save_options();
	if (i == FullscreenMenuBase::MenuTarget::kRestart) {
		delete m_opt_dialog;
		m_opt_dialog = new FullscreenMenuOptions(options_struct());
		handle_menu(); // Restart general options menu
	}
}

OptionsCtrl::OptionsStruct OptionsCtrl::options_struct() {
	OptionsStruct opt;
	opt.xres = m_opt_section.get_int("xres", DEFAULT_RESOLUTION_W);
	opt.yres = m_opt_section.get_int("yres", DEFAULT_RESOLUTION_H);
	opt.inputgrab = m_opt_section.get_bool("inputgrab", false);
	opt.fullscreen = m_opt_section.get_bool("fullscreen", false);
	opt.single_watchwin = m_opt_section.get_bool("single_watchwin", false);
	opt.auto_roadbuild_mode = m_opt_section.get_bool("auto_roadbuild_mode", true);
	opt.show_warea = m_opt_section.get_bool("workareapreview", true);
	opt.snap_win_overlap_only =
	   m_opt_section.get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges = m_opt_section.get_bool("dock_windows_to_edges", false);
	opt.language = m_opt_section.get_string("language", "");
	opt.music = !m_opt_section.get_bool("disable_music", false);
	opt.fx = !m_opt_section.get_bool("disable_fx", false);
	opt.autosave = m_opt_section.get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
	opt.rolling_autosave = m_opt_section.get_int("rolling_autosave", 5);
	opt.maxfps = m_opt_section.get_int("maxfps", 25);

	opt.message_sound = m_opt_section.get_bool("sound_at_message", true);
	opt.nozip = m_opt_section.get_bool("nozip", false);
	opt.border_snap_distance = m_opt_section.get_int("border_snap_distance", 0);
	opt.panel_snap_distance = m_opt_section.get_int("panel_snap_distance", 0);
	opt.remove_replays = m_opt_section.get_int("remove_replays", 0);
	opt.remove_syncstreams = m_opt_section.get_bool("remove_syncstreams", true);
	opt.transparent_chat = m_opt_section.get_bool("transparent_chat", true);
	return opt;
}

void OptionsCtrl::save_options() {
	OptionsCtrl::OptionsStruct opt = m_opt_dialog->get_values();
	m_opt_section.set_int ("xres",                  opt.xres);
	m_opt_section.set_int ("yres",                  opt.yres);
	m_opt_section.set_bool("fullscreen",            opt.fullscreen);
	m_opt_section.set_bool("inputgrab",             opt.inputgrab);
	m_opt_section.set_bool("single_watchwin",       opt.single_watchwin);
	m_opt_section.set_bool("auto_roadbuild_mode",   opt.auto_roadbuild_mode);
	m_opt_section.set_bool("workareapreview",       opt.show_warea);
	m_opt_section.set_bool
		("snap_windows_only_when_overlapping",
		 opt.snap_win_overlap_only);
	m_opt_section.set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	m_opt_section.set_bool("disable_music",        !opt.music);
	m_opt_section.set_bool("disable_fx",           !opt.fx);
	m_opt_section.set_string("language",            opt.language);
	m_opt_section.set_int("autosave",               opt.autosave * 60);
	m_opt_section.set_int("rolling_autosave",       opt.rolling_autosave);
	m_opt_section.set_int("maxfps",                 opt.maxfps);

	m_opt_section.set_bool("sound_at_message",      opt.message_sound);
	m_opt_section.set_bool("nozip",                 opt.nozip);
	m_opt_section.set_int("border_snap_distance",   opt.border_snap_distance);
	m_opt_section.set_int("panel_snap_distance",    opt.panel_snap_distance);

	m_opt_section.set_int("remove_replays",         opt.remove_replays);
	m_opt_section.set_bool("remove_syncstreams",    opt.remove_syncstreams);
	m_opt_section.set_bool("transparent_chat",      opt.transparent_chat);

	WLApplication::get()->set_input_grab(opt.inputgrab);
	i18n::set_locale(opt.language);
	UI::g_fh1->reinitialize_fontset();
	g_sound_handler.set_disable_music(!opt.music);
	g_sound_handler.set_disable_fx(!opt.fx);
}
