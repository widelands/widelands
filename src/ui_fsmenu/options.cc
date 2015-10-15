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
	FullscreenMenuBase("optionsmenu.jpg"),

// Values for alignment and size
	m_vbutw   (get_h() * 333 / 10000),
	m_butw    (get_w() / 4),
	m_buth    (get_h() * 9 / 200),
	m_hmargin (get_w() * 19 / 200),
	m_padding (10),
	m_space   (25),
	m_offset_first_group (get_h() * 1417 / 10000),
	m_offset_second_group(get_h() * 5833 / 10000),

// Buttons
	m_advanced_options
		(this, "advanced_options",
		 get_w() * 9 / 80, get_h() * 19 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Advanced Options"), std::string(), true, false),
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

	// Title
		m_title
			(this,
			 get_w() / 2, get_h() / 40,
			 _("General Options"), UI::Align_HCenter),

	// First options block 'general options', first column
	m_label_resolution
		(this,
		 m_hmargin, m_offset_first_group,
		 _("In-game resolution"), UI::Align_VCenter),
	m_reslist
		(this,
		 m_hmargin, m_label_resolution.get_y() + m_label_resolution.get_h(),
		 (get_w() - 2 * m_hmargin - m_space) / 2, 95,
		 UI::Align_Left, true),

	m_fullscreen (this, Point(m_hmargin,
									  m_reslist.get_y() +
									  m_reslist.get_h() + m_padding)),
	m_label_fullscreen
		(this,
		 m_hmargin + m_fullscreen.get_w() + m_padding,
		 m_reslist.get_y() + m_reslist.get_h() + m_padding,
		 m_reslist.get_w() - m_fullscreen.get_w() - m_padding,
		 m_fullscreen.get_h(),
		 _("Fullscreen"), UI::Align_VCenter),

	m_inputgrab (this, Point(m_hmargin,
									 m_label_fullscreen.get_y() +
									 m_label_fullscreen.get_h() + m_padding)),
	m_label_inputgrab
		(this,
		 m_hmargin + m_inputgrab.get_w() + m_padding,
		 m_label_fullscreen.get_y() + m_label_fullscreen.get_h() + m_padding,
		 m_reslist.get_w() - m_inputgrab.get_w() - m_padding,
		 m_inputgrab.get_h(),
		 _("Grab Input"), UI::Align_VCenter),

	m_label_maxfps
		(this,
		 m_hmargin,
		 m_label_inputgrab.get_y() + m_label_inputgrab.get_h() + m_padding,
		 m_reslist.get_w() - 80, m_inputgrab.get_h(),
		 _("Maximum FPS:"), UI::Align_VCenter),
	m_sb_maxfps
		(this,
		 m_hmargin + m_reslist.get_w() - 80, m_label_maxfps.get_y(),
		 80, m_vbutw,
		 opt.maxfps, 0, 99, "",
		 g_gr->images().get("pics/but1.png")),


	// First options block 'general options', second column
	m_label_language
		(this,
		 get_w() - m_hmargin - (get_w() - 2 * m_hmargin - m_space) / 2, m_offset_first_group,
		 _("Language"), UI::Align_VCenter),
	// same height as m_reslist
	m_language_list
		(this,
		 m_label_language.get_x(), m_label_language.get_y() + m_label_language.get_h(),
		 (get_w() - 2 * m_hmargin - m_space) / 2, m_reslist.get_h(),
		 UI::Align_Left, true),

	m_music (this, Point(m_label_language.get_x(),
								m_language_list.get_y() +
								m_language_list.get_h() + m_padding)),
	m_label_music
		(this,
		 m_label_language.get_x() + m_music.get_w() + m_padding,
		 m_language_list.get_y() + m_language_list.get_h() + m_padding,
		 m_language_list.get_w(), m_music.get_h(),
		 _("Enable Music"), UI::Align_VCenter),

	m_fx (this, Point(m_label_language.get_x(),
							m_label_music.get_y() +
							m_label_music.get_h() + m_padding)),
	m_label_fx
		(this,
		 m_label_language.get_x() + m_fx.get_w() + m_padding,
		 m_label_music.get_y() + m_label_music.get_h() + m_padding,
		 m_language_list.get_w(), m_fx.get_h(),
		 _("Enable Sound Effects"), UI::Align_VCenter),

	// Second options block 'In-game options'
	// Title 2
	m_label_game_options
		(this,
		 get_w() / 2, get_h() / 2,
		 _("In-game Options"), UI::Align_HCenter),

	m_single_watchwin (this, Point(m_hmargin, m_offset_second_group)),
	m_label_single_watchwin
		(this,
		 m_single_watchwin.get_x() + m_single_watchwin.get_w() + m_padding,
		 m_offset_second_group,
		 get_w() - 2 * m_hmargin - m_single_watchwin.get_w(), m_single_watchwin.get_h(),
		 /** TRANSLATORS: A watchwindow is a window where you keep watching an object or a map region,*/
		 /** TRANSLATORS: and it also lets you jump to it on the map. */
		 _("Use single watchwindow mode"), UI::Align_VCenter),

	m_auto_roadbuild_mode (this, Point(m_single_watchwin.get_x(),
												  m_label_single_watchwin.get_y() +
												  m_label_single_watchwin.get_h() + m_padding)),
	m_label_auto_roadbuild_mode
		(this,
		 m_auto_roadbuild_mode.get_x() + m_auto_roadbuild_mode.get_w() + m_padding,
		 m_label_single_watchwin.get_y() + m_label_single_watchwin.get_h() + m_padding,
		 get_w() - 2 * m_hmargin - m_auto_roadbuild_mode.get_w(), m_auto_roadbuild_mode.get_h(),
		 _("Start building road after placing a flag"), UI::Align_VCenter),

	m_show_workarea_preview
		(this, Point(m_auto_roadbuild_mode.get_x(),
						 m_label_auto_roadbuild_mode.get_y() +
						 m_label_auto_roadbuild_mode.get_h() + m_padding)),
	m_label_show_workarea_preview
		(this,
		 m_show_workarea_preview.get_x() + m_show_workarea_preview.get_w() + m_padding,
		 m_label_auto_roadbuild_mode.get_y() + m_label_auto_roadbuild_mode.get_h() + m_padding,
		 get_w() - 2 * m_hmargin - m_show_workarea_preview.get_w(), m_show_workarea_preview.get_h(),
		 _("Show buildings area preview"), UI::Align_VCenter),

	m_snap_win_overlap_only
		(this, Point(m_show_workarea_preview.get_x(),
						 m_label_show_workarea_preview.get_y() +
						 m_label_show_workarea_preview.get_h() + m_padding)),
	m_label_snap_win_overlap_only
		(this,
		 m_snap_win_overlap_only.get_x() + m_snap_win_overlap_only.get_w() + m_padding,
		 m_label_show_workarea_preview.get_y() + m_label_show_workarea_preview.get_h() + m_padding,
		 get_w() - 2 * m_hmargin - m_snap_win_overlap_only.get_w(), m_snap_win_overlap_only.get_h(),
		 _("Snap windows only when overlapping"), UI::Align_VCenter),

	m_dock_windows_to_edges (this, Point(m_snap_win_overlap_only.get_x(),
													 m_label_snap_win_overlap_only.get_y() +
													 m_label_snap_win_overlap_only.get_h() + m_padding)),
	m_label_dock_windows_to_edges
		(this,
		 m_dock_windows_to_edges.get_x() + m_dock_windows_to_edges.get_w() + m_padding,
		 m_label_snap_win_overlap_only.get_y() + m_label_snap_win_overlap_only.get_h() + m_padding,
		 get_w() - 2 * m_hmargin - m_dock_windows_to_edges.get_w(), m_dock_windows_to_edges.get_h(),
		 _("Dock windows to edges"), UI::Align_VCenter),

	m_sb_autosave
		(this,
		 get_w() - m_hmargin - 240,
		 m_dock_windows_to_edges.get_y() + m_dock_windows_to_edges.get_h() + m_padding,
		 240, m_vbutw,
		 /** TRANSLATORS: Options: Save game automatically every: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 opt.autosave / 60, 0, 100, ngettext("minute", "minutes", opt.autosave / 60),
		 g_gr->images().get("pics/but1.png"), true),
	m_label_autosave
		(this,
		 m_dock_windows_to_edges.get_x(),
		 m_sb_autosave.get_y(),
		 get_w() - m_sb_autosave.get_w() - 2 * m_hmargin,
		 m_dock_windows_to_edges.get_h(),
		 _("Save game automatically every"), UI::Align_VCenter),

	m_sb_remove_replays
		(this,
		 get_w() - m_hmargin - 240,
		 m_sb_autosave.get_y() + m_sb_autosave.get_h() + m_padding,
		 240, m_vbutw,
		 /** TRANSLATORS: Options: Remove Replays older than: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 opt.remove_replays, 0, 365, ngettext("day", "days", opt.remove_replays),
		 g_gr->images().get("pics/but1.png"), true),
	m_label_remove_replays
		(this,
		 m_label_autosave.get_x(),
		 m_sb_remove_replays.get_y(),
		 get_w() - m_sb_remove_replays.get_w() - 2 * m_hmargin,
		 m_dock_windows_to_edges.get_h(),
		 _("Remove replays older than:"), UI::Align_VCenter),

	os(opt)
{
	m_advanced_options.sigclicked.connect
		(boost::bind(&FullscreenMenuOptions::advanced_options, boost::ref(*this)));
	m_cancel.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_back, this));
	m_apply.sigclicked.connect(boost::bind(&FullscreenMenuOptions::clicked_ok, this));

	/** TRANSLATORS Options: Save game automatically every: */
	m_sb_autosave     .add_replacement(0, _("Off"));
	for (UI::Button* temp_button : m_sb_autosave.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_autosave_unit,
					 boost::ref(*this)));
	}
	/** TRANSLATORS Options: Remove Replays older than: */
	m_sb_remove_replays.add_replacement(0, _("Never"));
	for (UI::Button* temp_button : m_sb_remove_replays.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuOptions::update_sb_remove_replays_unit,
					 boost::ref(*this)));
	}

	m_title           .set_textstyle(UI::TextStyle::ui_big());
	m_fullscreen      .set_state(opt.fullscreen);
	m_inputgrab       .set_state(opt.inputgrab);
	m_music           .set_state(opt.music);
	m_music           .set_enabled(!g_sound_handler.lock_audio_disabling_);
	m_fx              .set_state(opt.fx);
	m_fx              .set_enabled(!g_sound_handler.lock_audio_disabling_);

	m_label_game_options             .set_textstyle(UI::TextStyle::ui_big());
	m_single_watchwin                .set_state(opt.single_watchwin);
	m_auto_roadbuild_mode            .set_state(opt.auto_roadbuild_mode);
	m_show_workarea_preview          .set_state(opt.show_warea);
	m_snap_win_overlap_only          .set_state(opt.snap_win_overlap_only);
	m_dock_windows_to_edges          .set_state(opt.dock_windows_to_edges);

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
		m_reslist.add((boost::format(_("%1% x %2%"))
							% m_resolutions[i].xres
							% m_resolutions[i].yres).str(),
						  nullptr, nullptr, selected);
	}
	if (!did_select_a_res) {
		m_reslist.add((boost::format(_("%1% x %2%"))
							% opt.xres
							% opt.yres).str(),
						  nullptr, nullptr, true);
		uint32_t entry = m_resolutions.size();
		m_resolutions.resize(entry + 1);
		m_resolutions[entry].xres  = opt.xres;
		m_resolutions[entry].yres  = opt.yres;
	}

	add_languages_to_list(opt.language);
	m_language_list.focus();
}

void FullscreenMenuOptions::update_sb_autosave_unit() {
	m_sb_autosave.set_unit(ngettext("minute", "minutes", m_sb_autosave.get_value()));
}

void FullscreenMenuOptions::update_sb_remove_replays_unit() {
	m_sb_remove_replays.set_unit(ngettext("day", "days", m_sb_remove_replays.get_value()));
}

void FullscreenMenuOptions::advanced_options() {
	FullscreenMenuAdvancedOptions aom(os);
	if (aom.run<FullscreenMenuBase::MenuTarget>() == FullscreenMenuBase::MenuTarget::kOk) {
		os = aom.get_values();
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kRestart);
	}
}

void FullscreenMenuOptions::add_languages_to_list(const std::string& current_locale) {

	// We want these two entries on top - the most likely user's choice and the default.
	m_language_list.add(_("Try system language"), "", nullptr, current_locale == "");
	m_language_list.add("English", "en", nullptr, current_locale == "en");

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
		m_language_list.add(entry.descname.c_str(), entry.localename, nullptr,
									entry.localename == selected_locale, "");
	}
}


OptionsCtrl::OptionsStruct FullscreenMenuOptions::get_values() {
	const uint32_t res_index = m_reslist.selection_index();

	// Write all data from UI elements
	os.xres                  = m_resolutions[res_index].xres;
	os.yres                  = m_resolutions[res_index].yres;
	os.inputgrab             = m_inputgrab.get_state();
	os.fullscreen            = m_fullscreen.get_state();
	os.single_watchwin       = m_single_watchwin.get_state();
	os.auto_roadbuild_mode   = m_auto_roadbuild_mode.get_state();
	os.show_warea            = m_show_workarea_preview.get_state();
	os.snap_win_overlap_only = m_snap_win_overlap_only.get_state();
	os.dock_windows_to_edges = m_dock_windows_to_edges.get_state();
	os.music                 = m_music.get_state();
	os.fx                    = m_fx.get_state();
	if (m_language_list.has_selection())
		os.language           = m_language_list.get_selected();
	os.autosave              = m_sb_autosave.get_value();
	os.maxfps                = m_sb_maxfps.get_value();
	os.remove_replays        = m_sb_remove_replays.get_value();

	return os;
}


/**
 * The advanced option menu
 */
FullscreenMenuAdvancedOptions::FullscreenMenuAdvancedOptions
	(OptionsCtrl::OptionsStruct const opt)
	:
	FullscreenMenuBase("ui_fsmenu.jpg"),

// Values for alignment and size
	m_vbutw   (get_h() * 333 / 10000),
	m_butw    (get_w() / 4),
	m_buth    (get_h() * 9 / 200),
	m_hmargin (get_w() * 19 / 200),
	m_padding (10),
	m_space   (25),

// Buttons
	m_cancel
		(this, "cancel",
		 get_w() * 41 / 80, get_h() * 19 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Cancel"), std::string(), true, false),
	m_apply
		(this, "apply",
		 get_w() / 4,   get_h() * 19 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Apply"), std::string(), true, false),

// Title
	m_title
		(this,
		 get_w() / 2, get_h() * 17 / 150,
		 _("Advanced Options"), UI::Align_HCenter),

// First options block
	m_label_snap_dis_panel
		(this,
		 m_hmargin, get_h() * 9 / 30,
		 _("Distance for windows to snap to other panels:"), UI::Align_VCenter),
	m_label_snap_dis_border
		(this,
		 m_hmargin,  m_label_snap_dis_panel.get_y() + m_label_snap_dis_panel.get_h() + 2 * m_padding,
		 _("Distance for windows to snap to borders:"), UI::Align_VCenter),

	// Spinboxes
	m_sb_dis_panel
			(this,
			 get_w() - m_hmargin - (get_w() / 5), m_label_snap_dis_panel.get_y(),
			 get_w() / 5, m_vbutw,
			 opt.panel_snap_distance, 0, 99, ngettext("pixel", "pixels", opt.panel_snap_distance),
			 g_gr->images().get("pics/but1.png")),

	m_sb_dis_border
			(this,
			 get_w() - m_hmargin - (get_w() / 5), m_label_snap_dis_border.get_y(),
			 get_w() / 5, m_vbutw,
			 opt.border_snap_distance, 0, 99, ngettext("pixel", "pixels", opt.border_snap_distance),
			 g_gr->images().get("pics/but1.png")),

	m_transparent_chat (this, Point(m_hmargin,
											  m_label_snap_dis_border.get_y() +
											  m_label_snap_dis_border.get_h() + m_space)),
	m_label_transparent_chat
		(this,
		 m_hmargin + m_transparent_chat.get_w() + m_padding, m_transparent_chat.get_y(),
		 get_w() - 2 * m_hmargin - m_transparent_chat.get_w() - m_padding, 40,
		 _("Show in-game chat with transparent background"), UI::Align_VCenter),

	m_message_sound
		(this, Point(m_hmargin,
						 m_label_transparent_chat.get_y() +
						 m_label_transparent_chat.get_h() + m_padding)),
	m_label_message_sound
		(this,
		 m_hmargin + m_message_sound.get_w() + m_padding, m_message_sound.get_y(),
		 get_w() - 2 * m_hmargin - m_message_sound.get_w() - m_padding, 40,
		 _("Play a sound at message arrival"),
		 UI::Align_VCenter),

	m_nozip (this, Point(m_hmargin,
								m_label_message_sound.get_y() +
								m_label_message_sound.get_h() + m_padding)),
	m_label_nozip
		(this,
		 m_hmargin + m_nozip.get_w() + m_padding, m_nozip.get_y(),
		 get_w() - 2 * m_hmargin - m_nozip.get_w() - m_padding, 40,
		 _("Do not zip widelands data files (maps, replays and savegames)"),
		 UI::Align_VCenter),

	m_remove_syncstreams (this, Point(m_hmargin,
												 m_label_nozip.get_y() +
												 m_label_nozip.get_h() + m_padding)),
	m_label_remove_syncstreams
		(this,
		 m_hmargin + m_remove_syncstreams.get_w() + m_padding, m_remove_syncstreams.get_y(),
		 get_w() - 2 * m_hmargin - m_remove_syncstreams.get_w() - m_padding, 40,
		 _("Remove Syncstream dumps on startup"), UI::Align_VCenter),

	os(opt)
{
	for (UI::Button* temp_button : m_sb_dis_panel.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuAdvancedOptions::update_sb_dis_panel_unit,
					 boost::ref(*this)));
	}

	for (UI::Button* temp_button : m_sb_dis_border.get_buttons()) {
		temp_button->sigclicked.connect
				(boost::bind
					(&FullscreenMenuAdvancedOptions::update_sb_dis_border_unit,
					 boost::ref(*this)));
	}

	m_cancel.sigclicked.connect(boost::bind(&FullscreenMenuAdvancedOptions::clicked_back, boost::ref(*this)));
	m_apply.sigclicked.connect(boost::bind(&FullscreenMenuAdvancedOptions::clicked_ok, boost::ref(*this)));

	m_title                .set_textstyle(UI::TextStyle::ui_big());
	m_message_sound        .set_state(opt.message_sound);
	m_nozip                .set_state(opt.nozip);
	m_remove_syncstreams   .set_state(opt.remove_syncstreams);
	m_transparent_chat     .set_state(opt.transparent_chat);
}

void FullscreenMenuAdvancedOptions::update_sb_dis_panel_unit() {
	m_sb_dis_panel.set_unit(ngettext("pixel", "pixels", m_sb_dis_panel.get_value()));
}

void FullscreenMenuAdvancedOptions::update_sb_dis_border_unit() {
	m_sb_dis_border.set_unit(ngettext("pixel", "pixels", m_sb_dis_border.get_value()));
}

OptionsCtrl::OptionsStruct FullscreenMenuAdvancedOptions::get_values() {
	// Write all remaining data from UI elements
	os.message_sound        = m_message_sound.get_state();
	os.nozip                = m_nozip.get_state();
	os.panel_snap_distance  = m_sb_dis_panel.get_value();
	os.border_snap_distance = m_sb_dis_border.get_value();
	os.remove_syncstreams   = m_remove_syncstreams.get_state();
	os.transparent_chat     = m_transparent_chat.get_state();
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
