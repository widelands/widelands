/*
 * Copyright (C) 2002-2004, 2006-2008 by Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define DEFINE_LANGUAGES  // So that the language array gets defined

#include "fullscreen_menu_options.h"

#include "constants.h"
#include "graphic.h"
#include "i18n.h"
#include "languages.h"
#include "profile.h"
#include "save_handler.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"

#include <libintl.h>
#include <stdio.h>


Fullscreen_Menu_Options::Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt)
:
Fullscreen_Menu_Base("optionsmenu.jpg"),

// Values for alignment and size
m_vbutw
	(m_yres * 333 / 10000),
m_butw
	(m_xres / 4),
m_buth
	(m_yres * 9 / 200),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Buttons
m_cancel
	(this,
	 m_xres * 41 / 80, m_yres * 19 / 20, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_Options::end_modal, this, om_cancel,
	 _("Cancel"), std::string(), true, false,
	 m_fn, m_fs),
m_apply
	(this,
	 m_xres / 4,   m_yres * 19 / 20, m_butw, m_buth,
	 2,
	 &Fullscreen_Menu_Options::end_modal, this, om_ok,
	 _("Apply"), std::string(), true, false,
	 m_fn, m_fs),
m_fps_plus
	(this,
	 m_xres * 2719 / 5000, m_yres * 3833 / 10000, m_vbutw, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::maxFpsChange, this, plus,
	 "+", _("Increase maximum FPS"), true, false,
	 m_fn, m_fs),
m_fps_minus
	(this,
	 m_xres * 1547 / 2500, m_yres * 3833 / 10000, m_vbutw, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::maxFpsChange, this, minus,
	 "-", _("Decrease maximum FPS"), true, false,
	 m_fn, m_fs),
m_autosave_plus
	(this,
	 m_xres * 2719 / 5000, m_yres * 8167 / 10000, m_vbutw, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::autosaveChange, this, plus,
	 "+", _("Increase autosave interval"), true, false,
	 m_fn, m_fs),
m_autosave_minus
	(this,
	 m_xres * 53 / 80, m_yres * 8167 / 10000, m_vbutw, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::autosaveChange, this, minus,
	 "-", _("Decrease autosave interval"), true, false,
	 m_fn, m_fs),
m_autosave_tenplus
	(this,
	 m_xres * 5063 / 10000, m_yres * 8167 / 10000, m_vbutw * 5 / 4, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::autosaveChange, this, plusTen,
	 "++", _("Increase autosave interval at 10"), true, false,
	 m_fn, m_fs),
m_autosave_tenminus
	(this,
	 m_xres * 3469 / 5000, m_yres * 8167 / 10000, m_vbutw * 5 / 4, m_vbutw,
	 1,
	 &Fullscreen_Menu_Options::autosaveChange, this, minusTen,
	 "--", _("Decrease autosave interval at 10"), true, false,
	 m_fn, m_fs),

// Title
m_title
	(this,
	 m_xres / 2, m_yres / 40,
	 _("General Options"), Align_HCenter),

// First options block 'general options'
m_fullscreen (this, Point(m_xres * 3563 / 10000, m_yres * 1667 / 10000)),
m_label_fullscreen
	(this,
	 m_xres * 1969 / 5000, m_yres * 1833 / 10000,
	 _("Fullscreen"), Align_VCenter),

m_inputgrab (this, Point(m_xres * 3563 / 10000, m_yres * 2167 / 10000)),
m_label_inputgrab
	(this,
	 m_xres * 1969 / 5000, m_yres * 2333 / 10000,
	 _("Grab Input"), Align_VCenter),

m_music (this, Point(m_xres * 3563 / 10000, m_yres * 2667 / 10000)),
m_label_music
	(this,
	 m_xres * 1969 / 5000, m_yres * 2833 / 10000,
	 _("Enable Music"), Align_VCenter),

m_fx (this, Point(m_xres * 3563 / 10000, m_yres * 3167 / 10000)),
m_label_fx
	(this,
	 m_xres * 1969 / 5000, m_yres * 3333 / 10000,
	 _("Enable Sound"), Align_VCenter),

m_label_maxfps
	(this,
	 m_xres * 3563 / 10000, m_yres * 2 / 5,
	 _("Maximum FPS:"), Align_VCenter),
m_value_maxfps
	(this,
	 m_xres * 6063 / 10000, m_yres * 3783 / 10000,
	 "25", Align_Right),

m_reslist
	(this,
	 m_xres      / 10, m_yres * 1667 / 10000,
	 m_xres * 19 / 80, m_yres * 2833 / 10000,
	 Align_Left, true),
m_label_resolution
	(this,
	 m_xres * 1063 / 10000, m_yres * 1417 / 10000,
	 _("In-game resolution"), Align_VCenter),

m_label_language
	(this,
	 m_xres * 133 / 200, m_yres * 1417 / 10000,
	 _("Language"), Align_VCenter),
m_language_list
	(this,
	 m_xres * 6563 / 10000, m_yres * 1667 / 10000,
	 m_xres *   21 /    80, m_yres * 2833 / 10000,
	 Align_Left, true),

// Title 2
m_label_game_options
	(this,
	 m_xres / 2, m_yres / 2,
	 _("In-game Options"), Align_HCenter),

// Second options block 'In-game options'
m_single_watchwin (this, Point(m_xres * 19 / 200, m_yres * 5833 / 10000)),
m_label_single_watchwin
	(this,
	 m_xres * 1313 / 10000, m_yres * 3 / 5,
	 _("Use single Watchwindow Mode"), Align_VCenter),

m_auto_roadbuild_mode (this, Point(m_xres * 19 / 200, m_yres * 63 / 100)),
m_label_auto_roadbuild_mode
	(this,
	 m_xres * 1313 / 10000, m_yres * 6467 / 10000,
	 _("Start roadbuilding after placing flag"), Align_VCenter),

m_show_workarea_preview(this, Point(m_xres * 19 / 200, m_yres * 6767 / 10000)),
m_label_show_workarea_preview
	(this,
	 m_xres * 1313 / 10000, m_yres * 6933 / 10000,
	 _("Show buildings area preview"), Align_VCenter),

m_snap_windows_only_when_overlapping
	(this, Point(m_xres * 19 / 200, m_yres * 7233 / 10000)),
m_label_snap_windows_only_when_overlapping
	(this,
	 m_xres * 1313 / 10000, m_yres * 37 / 50,
	 _("Snap windows only when overlapping"), Align_VCenter),

m_dock_windows_to_edges (this, Point(m_xres * 19 / 200, m_yres * 77 / 100)),
m_label_dock_windows_to_edges
	(this,
	 m_xres * 1313 / 10000, m_yres * 7867 / 10000,
	 _("Dock windows to edges"), Align_VCenter),

m_autosave (this, Point(m_xres * 19 / 200, m_yres * 8167 / 10000)),
m_label_autosave
	(this,
	 m_xres * 1313 / 10000, m_yres * 8333 / 10000,
	 _("Save game automatically every"), Align_VCenter),
m_value_autosave
	(this,
	 m_xres * 761 / 1250, m_yres * 8117 / 10000,
	 "15", Align_Right),
m_label_minute
	(this,
	 m_xres * 49 / 80, m_yres * 8333 / 10000,
	 _("min."), Align_VCenter)
{
	m_title           .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_fullscreen.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_fullscreen      .set_state(opt.fullscreen);
	m_label_inputgrab .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_inputgrab       .set_state(opt.inputgrab);
	m_label_music     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_music           .set_state(opt.music);
	m_music           .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_label_fx        .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_fx              .set_state(opt.fx);
	m_fx              .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_label_maxfps    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_value_maxfps    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_resolution.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_reslist         .set_font(m_fn, m_fs);
	m_label_language  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_language_list   .set_font(m_fn, m_fs);

	m_label_game_options.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_label_single_watchwin      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_single_watchwin                   .set_state(opt.single_watchwin);
	m_label_auto_roadbuild_mode  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_auto_roadbuild_mode               .set_state(opt.auto_roadbuild_mode);
	m_label_show_workarea_preview.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_show_workarea_preview             .set_state(opt.show_warea);
	m_label_snap_windows_only_when_overlapping.set_font
		(m_fn, m_fs, UI_FONT_CLR_FG);
	m_snap_windows_only_when_overlapping.set_state
		(opt.snap_windows_only_when_overlapping);
	m_label_dock_windows_to_edges.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_dock_windows_to_edges             .set_state(opt.dock_windows_to_edges);
	m_label_autosave             .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_value_autosave             .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_label_minute               .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_autosave                          .set_state(opt.autosave > 0);
	m_autosave.changed.set(this, &Fullscreen_Menu_Options::update_autosave);
	m_asvalue = opt.autosave / 60;
	update_autosave();
	m_maxfps = opt.maxfps;
	update_maxfps();

	//  GRAPHIC_TODO: this shouldn't be here List all resolutions
	SDL_PixelFormat* fmt = SDL_GetVideoInfo()->vfmt;
	fmt->BitsPerPixel = 16;
	if
		(SDL_Rect const * const * const modes =
		 SDL_ListModes(fmt, SDL_SWSURFACE | SDL_FULLSCREEN))
		for (uint32_t i = 0; modes[i]; ++i)
			if (640 <= modes[i]->w and 480 <= modes[i]->h) {
				res const this_res = {modes[i]->w, modes[i]->h, 16};
			if
				(not m_resolutions.size()
				 or
				 this_res.xres != m_resolutions[m_resolutions.size()-1].xres
				 or
				 this_res.yres != m_resolutions[m_resolutions.size()-1].yres)
				m_resolutions.push_back(this_res);
			}
	fmt->BitsPerPixel = 32;
	if
		(SDL_Rect const * const * const modes =
		 SDL_ListModes(fmt, SDL_SWSURFACE | SDL_FULLSCREEN))
		for (uint32_t i = 0; modes[i]; ++i)
			if (640 <= modes[i]->w and 480 <= modes[i]->h) {
				res const this_res = {modes[i]->w, modes[i]->h, 32};
				if
					(not m_resolutions.size()
					 or
					 this_res.xres != m_resolutions[m_resolutions.size() - 1].xres
					 or
					 this_res.yres != m_resolutions[m_resolutions.size() - 1].yres)
					m_resolutions.push_back(this_res);
			}

	bool did_select_a_res=false;
	for (uint32_t i = 0; i < m_resolutions.size(); ++i) {
		char buf[32];
		sprintf(buf, "%ix%i %i bit", m_resolutions[i].xres, m_resolutions[i].yres, m_resolutions[i].depth);
		const bool selected =
			m_resolutions[i].xres  == opt.xres and
			m_resolutions[i].yres  == opt.yres and
			m_resolutions[i].depth == opt.depth;
		did_select_a_res|=selected;
		m_reslist.add(buf, 0, -1, selected);
	}
	if (not did_select_a_res) m_reslist.select(m_reslist.size() - 1);

	available_languages[0].name = _("System default language");
	for (uint32_t i = 0; i < NR_LANGUAGES; ++i)
		m_language_list.add
			(available_languages[i].name.c_str(),
			 available_languages[i].abbrev,
			 -1, //  FIXME this should be a flag
			 available_languages[i].abbrev == opt.language);
}

void Fullscreen_Menu_Options::autosaveChange(int32_t const arg) {
	if (arg == plus)
		++m_asvalue;
	if (arg == minus)
		--m_asvalue;
	if (arg == plusTen)
		m_asvalue += 10;
	if (arg == minusTen)
		m_asvalue -= 10;
	update_autosave();
}

void Fullscreen_Menu_Options::update_autosave() {
	m_autosave_plus.set_enabled    (m_autosave.get_state() & (m_asvalue < 300));
	m_autosave_minus.set_enabled   (m_autosave.get_state() & (m_asvalue >   1));
	m_autosave_tenplus.set_enabled (m_autosave.get_state() & (m_asvalue < 291));
	m_autosave_tenminus.set_enabled(m_autosave.get_state() & (m_asvalue >  10));
	char text[32];
	snprintf(text, sizeof(text), "%i", m_asvalue);
	m_value_autosave.set_text(text);
}

void Fullscreen_Menu_Options::maxFpsChange(int32_t const arg) {
	if (arg == plus)
		++m_maxfps;
	if (arg == minus)
		--m_maxfps;
	update_maxfps();
}

void Fullscreen_Menu_Options::update_maxfps() {
	m_fps_plus.set_enabled(m_maxfps < 99);
	m_fps_minus.set_enabled(m_maxfps > 5);
	char text[32];
	snprintf(text, sizeof(text), "%i", m_maxfps);
	m_value_maxfps.set_text(text);
}


Options_Ctrl::Options_Struct Fullscreen_Menu_Options::get_values() {
	const uint32_t res_index = m_reslist.selection_index();
	Options_Ctrl::Options_Struct opt = {
		m_resolutions[res_index].xres,
		m_resolutions[res_index].yres,
		m_resolutions[res_index].depth,
		m_inputgrab                         .get_state   (),
		m_fullscreen                        .get_state   (),
		m_single_watchwin                   .get_state   (),
		m_auto_roadbuild_mode               .get_state   (),
		m_show_workarea_preview             .get_state   (),
		m_snap_windows_only_when_overlapping.get_state   (),
		m_dock_windows_to_edges             .get_state   (),
		m_music                             .get_state   (),
		m_fx                                .get_state   (),
		m_language_list                     .get_selected(),
		m_autosave.get_state() ? m_asvalue : 0,
		m_maxfps
	};
	return opt;
}


/**
 * Handles communication between window class and options
 */
Options_Ctrl::Options_Ctrl(Section & s)
: m_opt_dialog(new Fullscreen_Menu_Options(options_struct(s))), m_opt_section(s)
{
	if (m_opt_dialog->run() == Fullscreen_Menu_Options::om_ok)
		save_options();
}

Options_Ctrl::~Options_Ctrl() {
	delete m_opt_dialog;
}

Options_Ctrl::Options_Struct Options_Ctrl::options_struct(Section & s) {
	Options_Struct opt;
	opt.xres                  =  s.get_int ("xres",                    640);
	opt.yres                  =  s.get_int ("yres",                    480);
	opt.depth                 =  s.get_int ("depth",                    16);
	opt.inputgrab             =  s.get_bool("inputgrab",             false);
	opt.fullscreen            =  s.get_bool("fullscreen",            false);
	opt.single_watchwin       =  s.get_bool("single_watchwin",       false);
	opt.auto_roadbuild_mode   =  s.get_bool("auto_roadbuild_mode",    true);
	opt.show_warea            =  s.get_bool("workareapreview",       false);
	opt.snap_windows_only_when_overlapping
		= s.get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges =  s.get_bool("dock_windows_to_edges", false);
	opt.language              =  s.get_string("language", "");
	opt.music                 = !s.get_bool("disable_music", false);
	opt.fx                    = !s.get_bool("disable_fx", false);
	opt.autosave = s.get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
	opt.maxfps                =  s.get_int("maxfps", 25);
	return opt;
}

void Options_Ctrl::save_options() {
	Options_Ctrl::Options_Struct opt = m_opt_dialog->get_values();
	m_opt_section.set_int ("xres",                  opt.xres);
	m_opt_section.set_int ("yres",                  opt.yres);
	m_opt_section.set_bool("fullscreen",            opt.fullscreen);
	m_opt_section.set_bool("inputgrab",             opt.inputgrab);
	m_opt_section.set_bool("single_watchwin",       opt.single_watchwin);
	m_opt_section.set_bool("auto_roadbuild_mode",   opt.auto_roadbuild_mode);
	m_opt_section.set_bool("workareapreview",       opt.show_warea);
	m_opt_section.set_bool
		("snap_windows_only_when_overlapping",
		 opt.snap_windows_only_when_overlapping);
	m_opt_section.set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	m_opt_section.set_int ("depth",                 opt.depth);
	m_opt_section.set_bool("disable_music",        !opt.music);
	m_opt_section.set_bool("disable_fx",           !opt.fx);
	m_opt_section.set_string("language",            opt.language);
	m_opt_section.set_int("autosave",               opt.autosave * 60);
	m_opt_section.set_int("maxfps",                 opt.maxfps);
	WLApplication::get()->set_input_grab(opt.inputgrab);
	i18n::set_locale(opt.language);
	g_sound_handler.set_disable_music(!opt.music);
	g_sound_handler.set_disable_fx(!opt.fx);
}
