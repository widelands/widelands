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
m_cancel
	(this,
	 410, 570, 190, 24,
	 0,
	 &Fullscreen_Menu_Options::end_modal, this, om_cancel,
	 _("Cancel")),
m_apply
	(this,
	 200, 570, 190, 24,
	 2,
	 &Fullscreen_Menu_Options::end_modal, this, om_ok,
	 _("Apply")),
//ToDo: Implement working Interface for setting up maxfps
/*m_fps_plus
	(this,
	 MENU_XRES / 2 + 35, 230, 20, 20,
	 1,
	 &Fullscreen_Menu_Options::end_modal, this, om_ok,
	 "+"),
m_fps_minus
	(this,
	 MENU_XRES / 2 + 95, 230, 20, 20,
	 1,
	 &Fullscreen_Menu_Options::end_modal, this, om_ok,
	 "-"),*/
m_title(this, MENU_XRES / 2, 20, _("General Options"), Align_HCenter),
m_fullscreen                        (this, Point(285, 100)),
m_label_fullscreen(this, 315, 110, _("Fullscreen"), Align_VCenter),
m_inputgrab                         (this, Point(285, 160)),
m_label_inputgrab(this, 315, 140, _("Grab Input"), Align_VCenter),
m_music                             (this, Point(285, 160)),
m_label_music(this, 315, 170, _("Enable Music"), Align_VCenter),
m_fx                                (this, Point(285, 190)),
m_label_fx(this, 315, 200, _("Enable Sound"), Align_VCenter),
m_label_maxfps(this, 285, 240, _("Maximum FPS:"), Align_VCenter),
m_value_maxfps(this, MENU_XRES / 2 + 65, 240, "25", Align_VCenter),
m_reslist(this, 80, 100, 190, 170, Align_Left, true),
m_label_resolution(this, 85, 85, _("In-game resolution"), Align_VCenter),
m_label_language(this, MENU_XRES / 2 + 135, 85, _("Language"), Align_VCenter),
m_language_list(this, MENU_XRES / 2 + 125, 100, 210, 170, Align_Left, true),
m_label_game_options
	(this, MENU_XRES / 2, 300, _("In-game Options"), Align_HCenter),
m_single_watchwin                   (this, Point(76, 350)),
m_label_single_watchwin
	(this, 105, 360, _("Use single Watchwindow Mode"), Align_VCenter),
m_auto_roadbuild_mode               (this, Point(76, 378)),
m_label_auto_roadbuild_mode
	(this, 105, 388, _("Start roadbuilding after placing flag"), Align_VCenter),
m_show_workarea_preview             (this, Point(76, 406)),
m_label_show_workarea_preview
	(this, 105, 416, _("Show buildings area preview"), Align_VCenter),
m_snap_windows_only_when_overlapping(this, Point(76, 434)),
m_label_snap_windows_only_when_overlapping
	(this, 105, 444, _("Snap windows only when overlapping"), Align_VCenter),
m_dock_windows_to_edges             (this, Point(76, 462)),
m_label_dock_windows_to_edges
	(this, 105, 472, _("Dock windows to edges"), Align_VCenter),
m_autosave                          (this, Point(76, 490)),
m_label_autosave
	(this, 105, 500, "Autosave game every XXX minutes", Align_VCenter)
{
	m_title     .set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	m_fullscreen.set_state(opt.fullscreen);
	m_inputgrab .set_state(opt.inputgrab);
	m_music     .set_state(opt.music);
	m_music     .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_fx        .set_state(opt.fx);
	m_fx        .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_single_watchwin                   .set_state(opt.single_watchwin);
	m_auto_roadbuild_mode               .set_state(opt.auto_roadbuild_mode);
	m_show_workarea_preview             .set_state(opt.show_warea);
	m_snap_windows_only_when_overlapping.set_state
		(opt.snap_windows_only_when_overlapping);
	m_dock_windows_to_edges             .set_state(opt.dock_windows_to_edges);
	m_autosave                          .set_state(opt.autosave > 0);

	char textmaxfps[2];
	sprintf(textmaxfps, "%i", opt.maxfps);
	m_value_maxfps                      .set_text(textmaxfps);

	char buffer[255];
	snprintf
		(buffer, sizeof(buffer),
		 ngettext
		 	("Autosave game every %d minute", "Autosave game every %d minutes",
		 	 DEFAULT_AUTOSAVE_INTERVAL),
		 DEFAULT_AUTOSAVE_INTERVAL);
	m_label_autosave.set_text(buffer);


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


	m_label_game_options.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
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
		m_autosave.get_state() ? DEFAULT_AUTOSAVE_INTERVAL : 0,
	};
	return opt;
}


/**
 * Handles communication between window class and options
 */
Options_Ctrl::Options_Ctrl(Section* s) {
	m_opt_dialog = new Fullscreen_Menu_Options(options_struct(s));
	m_opt_section = s;
	int32_t code = m_opt_dialog->run();
	if (code == Fullscreen_Menu_Options::om_ok)
		save_options();
}

Options_Ctrl::~Options_Ctrl() {
	delete m_opt_dialog;
}

Options_Ctrl::Options_Struct Options_Ctrl::options_struct(Section* s) {
	Options_Struct opt;
	opt.xres                  =  s->get_int ("xres",                    640);
	opt.yres                  =  s->get_int ("yres",                    480);
	opt.depth                 =  s->get_int ("depth",                    16);
	opt.inputgrab             =  s->get_bool("inputgrab",             false);
	opt.fullscreen            =  s->get_bool("fullscreen",            false);
	opt.single_watchwin       =  s->get_bool("single_watchwin",       false);
	opt.auto_roadbuild_mode   =  s->get_bool("auto_roadbuild_mode",    true);
	opt.show_warea            =  s->get_bool("workareapreview",       false);
	opt.snap_windows_only_when_overlapping
		= s->get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges =  s->get_bool("dock_windows_to_edges", false);
	opt.language              =  s->get_string("language", "");
	opt.music                 = !s->get_bool("disable_music", false);
	opt.fx                    = !s->get_bool("disable_fx", false);
	opt.autosave = s->get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
	opt.maxfps                =  s->get_int("maxfps", 25);
	return opt;
}

void Options_Ctrl::save_options() {
	Options_Ctrl::Options_Struct opt = m_opt_dialog->get_values();
	m_opt_section->set_int ("xres",                  opt.xres);
	m_opt_section->set_int ("yres",                  opt.yres);
	m_opt_section->set_bool("fullscreen",            opt.fullscreen);
	m_opt_section->set_bool("inputgrab",             opt.inputgrab);
	m_opt_section->set_bool("single_watchwin",       opt.single_watchwin);
	m_opt_section->set_bool("auto_roadbuild_mode",   opt.auto_roadbuild_mode);
	m_opt_section->set_bool("workareapreview",       opt.show_warea);
	m_opt_section->set_bool
		("snap_windows_only_when_overlapping",
		 opt.snap_windows_only_when_overlapping);
	m_opt_section->set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	m_opt_section->set_int ("depth",                 opt.depth);
	m_opt_section->set_bool("disable_music",        !opt.music);
	m_opt_section->set_bool("disable_fx",           !opt.fx);
	m_opt_section->set_string("language",            opt.language);
	m_opt_section->set_int("autosave",               opt.autosave * 60);
	//m_opt_section->set_int("maxfps",                 opt.maxfps);
	WLApplication::get()->set_input_grab(opt.inputgrab);
	i18n::set_locale(opt.language.c_str());
	g_sound_handler.set_disable_music(!opt.music);
	g_sound_handler.set_disable_fx(!opt.fx);
}
