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

#include <cstdio>
#include <iostream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/default_resolution.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/save_handler.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"
#include "wui/text_constants.h"

namespace  {

struct LanguageEntry {
	LanguageEntry(const std::string& init_abbreviation, const std::string& init_descname) :
		abbreviation(init_abbreviation),
		descname(init_descname) {}

	bool operator<(const LanguageEntry& other) const {
		return descname < other.descname;
	}

	std::string abbreviation;
	std::string descname;
};

void add_languages_to_list(UI::Listselect<std::string>* list, const std::string& language) {

	Section* s = &g_options.pull_section("global");
	FilenameSet files = g_fs->list_directory(s->get_string("localedir", INSTALL_LOCALEDIR));
	Profile ln("txts/languages");
	s = &ln.pull_section("languages");
	bool own_selected = "" == language || "en" == language;

	// Add translation directories to the list
	std::vector<LanguageEntry> entries;
	for (const std::string& filename : files) {
		char const* const path = filename.c_str();
		if (!strcmp(FileSystem::fs_filename(path), ".") ||
		    !strcmp(FileSystem::fs_filename(path), "..") || !g_fs->is_directory(path)) {
			continue;
		}

		char const* const abbreviation = FileSystem::fs_filename(path);
		entries.emplace_back(abbreviation, s->get_string(abbreviation, abbreviation));
		own_selected |= abbreviation == language;
	}
	// Add currently used language manually
	if (!own_selected) {
		entries.emplace_back(language, s->get_string(language.c_str(), language.c_str()));
	}
	std::sort(entries.begin(), entries.end());

	for (const LanguageEntry& entry : entries) {
		list->add(entry.descname.c_str(), entry.abbreviation, nullptr, entry.abbreviation == language);
	}
}

}  // namespace

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
		 _("Enable Sound"), UI::Align_VCenter),

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
	m_cancel.sigclicked.connect
		(boost::bind(&FullscreenMenuOptions::end_modal, this, static_cast<int32_t>(om_cancel)));
	m_apply.sigclicked.connect
		(boost::bind(&FullscreenMenuOptions::end_modal, this, static_cast<int32_t>(om_ok)));

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

	m_title           .set_textstyle(ts_big());
	m_fullscreen      .set_state(opt.fullscreen);
	m_inputgrab       .set_state(opt.inputgrab);
	m_music           .set_state(opt.music);
	m_music           .set_enabled(!g_sound_handler.lock_audio_disabling_);
	m_fx              .set_state(opt.fx);
	m_fx              .set_enabled(!g_sound_handler.lock_audio_disabling_);

	m_label_game_options             .set_textstyle(ts_big());
	m_single_watchwin                .set_state(opt.single_watchwin);
	m_auto_roadbuild_mode            .set_state(opt.auto_roadbuild_mode);
	m_show_workarea_preview          .set_state(opt.show_warea);
	m_snap_win_overlap_only          .set_state(opt.snap_win_overlap_only);
	m_dock_windows_to_edges          .set_state(opt.dock_windows_to_edges);

	//  GRAPHIC_TODO(unknown): this shouldn't be here List all resolutions
	// take a copy to not change real video info structure
	SDL_PixelFormat fmt = *SDL_GetVideoInfo()->vfmt;
	fmt.BitsPerPixel = 32;
	for
		(const SDL_Rect * const * modes = SDL_ListModes(&fmt, SDL_SWSURFACE | SDL_FULLSCREEN);
		 modes && *modes;
		 ++modes)
	{
		const SDL_Rect & mode = **modes;
		if (800 <= mode.w && 600 <= mode.h)
		{
			const ScreenResolution this_res = {mode.w, mode.h};
			if
				(m_resolutions.empty()
				 || this_res.xres != m_resolutions.rbegin()->xres
				 || this_res.yres != m_resolutions.rbegin()->yres)
			{
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
							% m_resolutions[i].yres).str().c_str(),
						  nullptr, nullptr, selected);
	}
	if (!did_select_a_res) {
		m_reslist.add((boost::format(_("%1% x %2%"))
							% opt.xres
							% opt.yres).str().c_str(),
						  nullptr, nullptr, true);
		uint32_t entry = m_resolutions.size();
		m_resolutions.resize(entry + 1);
		m_resolutions[entry].xres  = opt.xres;
		m_resolutions[entry].yres  = opt.yres;
	}

	// Fill language list
	m_language_list.add
		(_("Try system language"), "", // "try", as many translations are missing.
		 nullptr, "" == opt.language);

	m_language_list.add
		("English", "en",
		 nullptr, "en" == opt.language);

	add_languages_to_list(&m_language_list, opt.language);
}

void FullscreenMenuOptions::update_sb_autosave_unit() {
	m_sb_autosave.set_unit(ngettext("minute", "minutes", m_sb_autosave.get_value()));
}

void FullscreenMenuOptions::update_sb_remove_replays_unit() {
	m_sb_remove_replays.set_unit(ngettext("day", "days", m_sb_remove_replays.get_value()));
}

void FullscreenMenuOptions::advanced_options() {
	FullscreenMenuAdvancedOptions aom(os);
	if (aom.run() == FullscreenMenuAdvancedOptions::om_ok) {
		os = aom.get_values();
		end_modal(om_restart);
	}
}

bool FullscreenMenuOptions::handle_key(bool down, SDL_keysym code)
{
	if (down) {
		switch (code.sym) {
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				end_modal(static_cast<int32_t>(om_ok));
				return true;
			case SDLK_ESCAPE:
				end_modal(static_cast<int32_t>(om_cancel));
				return true;
			default:
				break; // not handled
		}
	}

	return FullscreenMenuBase::handle_key(down, code);
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
	m_offset_first_group (get_h() * 1417 / 10000),

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
		 get_w() / 2, get_h() / 40,
		 _("Advanced Options"), UI::Align_HCenter),

// First options block
	m_label_ui_font
		(this,
		 m_hmargin, m_offset_first_group,
		 _("Main menu font:"), UI::Align_BottomLeft),
	m_ui_font_list
		(this,
		 m_label_ui_font.get_x(), m_label_ui_font.get_y() + m_label_ui_font.get_h() + m_padding,
		 get_w() - 2 * m_hmargin, 134,
		 UI::Align_Left, true),

	m_label_snap_dis_panel
		(this,
		 m_hmargin, m_ui_font_list.get_y() + m_ui_font_list.get_h() + m_space + m_padding,
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
		 _("Play a sound at message arrival."),
		 UI::Align_VCenter),

	m_nozip (this, Point(m_hmargin,
								m_label_message_sound.get_y() +
								m_label_message_sound.get_h() + m_padding)),
	m_label_nozip
		(this,
		 m_hmargin + m_nozip.get_w() + m_padding, m_nozip.get_y(),
		 get_w() - 2 * m_hmargin - m_nozip.get_w() - m_padding, 40,
		 _("Do not zip widelands data files (maps, replays and savegames)."),
		 UI::Align_VCenter),

	m_remove_syncstreams (this, Point(m_hmargin,
												 m_label_nozip.get_y() +
												 m_label_nozip.get_h() + m_padding)),
	m_label_remove_syncstreams
		(this,
		 m_hmargin + m_remove_syncstreams.get_w() + m_padding, m_remove_syncstreams.get_y(),
		 get_w() - 2 * m_hmargin - m_remove_syncstreams.get_w() - m_padding, 40,
		 _("Remove Syncstream dumps on startup"), UI::Align_VCenter),

	m_opengl (this, Point(m_hmargin,
								 m_label_remove_syncstreams.get_y() +
								 m_label_remove_syncstreams.get_h() + m_padding)),
	m_label_opengl
		(this,
		 m_hmargin + m_opengl.get_w() + m_padding, m_opengl.get_y(),
		 get_w() - 2 * m_hmargin - m_opengl.get_w() - m_padding, 40,
		 _("OpenGL rendering"), UI::Align_VCenter),
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

	m_cancel.sigclicked.connect
		(boost::bind
			(&FullscreenMenuAdvancedOptions::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(om_cancel)));
	m_apply.sigclicked.connect
		(boost::bind
			(&FullscreenMenuAdvancedOptions::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(om_ok)));

	m_title                .set_textstyle(ts_big());
	m_message_sound        .set_state(opt.message_sound);
	m_nozip                .set_state(opt.nozip);
	m_remove_syncstreams   .set_state(opt.remove_syncstreams);
	m_opengl               .set_state(opt.opengl);
	m_transparent_chat     .set_state(opt.transparent_chat);

	// Fill the font list.
	{ // For use of string ui_font take a look at fullscreen_menu_base.cc
		bool cmpbool = !strcmp("serif", opt.ui_font.c_str());
		bool did_select_a_font = cmpbool;
		m_ui_font_list.add
			(_("DejaVuSerif (Default)"), "serif", nullptr, cmpbool);
		cmpbool = !strcmp("sans", opt.ui_font.c_str());
		did_select_a_font |= cmpbool;
		m_ui_font_list.add
			("DejaVuSans", "sans", nullptr, cmpbool);
		cmpbool = !strcmp(UI_FONT_NAME_WIDELANDS, opt.ui_font.c_str());
		did_select_a_font |= cmpbool;
		m_ui_font_list.add
			("Widelands", UI_FONT_NAME_WIDELANDS, nullptr, cmpbool);

		// Fill with all left *.ttf files we find in fonts
		FilenameSet files =
		   filter(g_fs->list_directory("fonts"),
		          [](const std::string& fn) {return boost::ends_with(fn, ".ttf");});

		for
			(FilenameSet::iterator pname = files.begin();
			 pname != files.end();
			 ++pname)
		{
			char const * const path = pname->c_str();
			char const * const name = FileSystem::fs_filename(path);
			if (!strcmp(name, UI_FONT_NAME_SERIF))
				continue;
			if (!strcmp(name, UI_FONT_NAME_SANS))
				continue;
			if (g_fs->is_directory(name))
				continue;
			cmpbool = !strcmp(name, opt.ui_font.c_str());
			did_select_a_font |= cmpbool;
			m_ui_font_list.add
				(name, name, nullptr, cmpbool);
		}

		if (!did_select_a_font)
			m_ui_font_list.select(0);
	}
}

bool FullscreenMenuAdvancedOptions::handle_key(bool down, SDL_keysym code)
{
	if (down) {
		switch (code.sym) {
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				end_modal(static_cast<int32_t>(om_ok));
				return true;
			case SDLK_ESCAPE:
				end_modal(static_cast<int32_t>(om_cancel));
				return true;
			default:
				break; // not handled
		}
	}

	return FullscreenMenuBase::handle_key(down, code);
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
	os.ui_font              = m_ui_font_list.get_selected();
	os.panel_snap_distance  = m_sb_dis_panel.get_value();
	os.border_snap_distance = m_sb_dis_border.get_value();
	os.remove_syncstreams   = m_remove_syncstreams.get_state();
	os.opengl               = m_opengl.get_state();
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
	int32_t i = m_opt_dialog->run();
	if (i != FullscreenMenuOptions::om_cancel)
		save_options();
	if (i == FullscreenMenuOptions::om_restart) {
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
	opt.maxfps = m_opt_section.get_int("maxfps", 25);

	opt.message_sound = m_opt_section.get_bool("sound_at_message", true);
	opt.nozip = m_opt_section.get_bool("nozip", false);
	opt.ui_font = m_opt_section.get_string("ui_font", "serif");
	opt.border_snap_distance = m_opt_section.get_int("border_snap_distance", 0);
	opt.panel_snap_distance = m_opt_section.get_int("panel_snap_distance", 0);
	opt.remove_replays = m_opt_section.get_int("remove_replays", 0);
	opt.remove_syncstreams = m_opt_section.get_bool("remove_syncstreams", true);
	opt.opengl = m_opt_section.get_bool("opengl", true);
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
	m_opt_section.set_int("maxfps",                 opt.maxfps);

	m_opt_section.set_bool("sound_at_message",      opt.message_sound);
	m_opt_section.set_bool("nozip",                 opt.nozip);
	m_opt_section.set_string("ui_font",             opt.ui_font);
	m_opt_section.set_int("border_snap_distance",   opt.border_snap_distance);
	m_opt_section.set_int("panel_snap_distance",    opt.panel_snap_distance);

	m_opt_section.set_int("remove_replays",         opt.remove_replays);
	m_opt_section.set_bool("remove_syncstreams",    opt.remove_syncstreams);
	m_opt_section.set_bool("opengl",                opt.opengl);
	m_opt_section.set_bool("transparent_chat",      opt.transparent_chat);

	WLApplication::get()->set_input_grab(opt.inputgrab);
	i18n::set_locale(opt.language);
	g_sound_handler.set_disable_music(!opt.music);
	g_sound_handler.set_disable_fx(!opt.fx);
}
