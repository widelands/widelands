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
#include <libintl.h>

#include "constants.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "profile/profile.h"
#include "save_handler.h"
#include "sound/sound_handler.h"
#include "wlapplication.h"

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
	filenameset_t files = g_fs->ListDirectory(s->get_string("localedir", INSTALL_LOCALEDIR));
	Profile ln("txts/languages");
	s = &ln.pull_section("languages");
	bool own_selected = "" == language || "en" == language;

	// Add translation directories to the list
	std::vector<LanguageEntry> entries;
	for (const std::string& filename : files) {
		char const* const path = filename.c_str();
		if (!strcmp(FileSystem::FS_Filename(path), ".") ||
		    !strcmp(FileSystem::FS_Filename(path), "..") || !g_fs->IsDirectory(path)) {
			continue;
		}

		char const* const abbreviation = FileSystem::FS_Filename(path);
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

Fullscreen_Menu_Options::Fullscreen_Menu_Options
		(Options_Ctrl::Options_Struct opt)
	:
	Fullscreen_Menu_Base("optionsmenu.jpg"),

// Values for alignment and size
	m_vbutw(get_h() * 333 / 10000),
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),

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

// Spinboxes
	m_sb_maxfps
		(this,
		 (get_w() / 2) - (m_vbutw * 2), get_h() * 3833 / 10000, get_w() / 5, m_vbutw,
		 opt.maxfps, 0, 100, "",
		 g_gr->images().get("pics/but1.png")),
	m_sb_autosave
		(this,
		 get_w() * 6767 / 10000, get_h() * 8167 / 10000, get_w() / 4, m_vbutw,
		 opt.autosave / 60, 0, 100, _("min."),
		 g_gr->images().get("pics/but1.png"), true),

	m_sb_remove_replays
		(this,
		 get_w() * 6767 / 10000, get_h() * 8631 / 10000, get_w() / 4, m_vbutw,
		 /** TRANSLATORS: Options: Remove Replays older than: */
		 /** TRANSLATORS: This will have a number added in front of it */
		 opt.remove_replays, 0, 365, ngettext("day", "days", m_vbutw),
		 g_gr->images().get("pics/but1.png"), true),

// Title
	m_title
		(this,
		 get_w() / 2, get_h() / 40,
		 _("General Options"), UI::Align_HCenter),

// First options block 'general options'
	m_fullscreen (this, Point(get_w() * 3563 / 10000, get_h() * 1667 / 10000)),
	m_label_fullscreen
		(this,
		 get_w() * 1969 / 5000, get_h() * 1833 / 10000,
		 _("Fullscreen"), UI::Align_VCenter),

	m_inputgrab (this, Point(get_w() * 3563 / 10000, get_h() * 2167 / 10000)),
	m_label_inputgrab
		(this,
		 get_w() * 1969 / 5000, get_h() * 2333 / 10000,
		 _("Grab Input"), UI::Align_VCenter),

	m_music (this, Point(get_w() * 3563 / 10000, get_h() * 2667 / 10000)),
	m_label_music
		(this,
		 get_w() * 1969 / 5000, get_h() * 2833 / 10000,
		 _("Enable Music"), UI::Align_VCenter),

	m_fx (this, Point(get_w() * 3563 / 10000, get_h() * 3167 / 10000)),
	m_label_fx
		(this,
		 get_w() * 1969 / 5000, get_h() * 3333 / 10000,
		 _("Enable Sound"), UI::Align_VCenter),

	m_label_maxfps
		(this,
		 get_w() * 3563 / 10000, get_h() * 2 / 5,
		 _("Maximum FPS:"), UI::Align_VCenter),

	m_reslist
		(this,
		 get_w()      / 10, get_h() * 1667 / 10000,
		 get_w() * 19 / 80, get_h() * 2833 / 10000,
		 UI::Align_Left, true),
	m_label_resolution
		(this,
		 get_w() * 1063 / 10000, get_h() * 1417 / 10000,
		 _("In-game resolution"), UI::Align_VCenter),

	m_label_language
		(this,
		 get_w() * 133 / 200, get_h() * 1417 / 10000,
		 _("Language"), UI::Align_VCenter),
	m_language_list
		(this,
		 get_w() * 6563 / 10000, get_h() * 1667 / 10000,
		 get_w() *   21 /    80, get_h() * 2833 / 10000,
		 UI::Align_Left, true),

// Title 2
	m_label_game_options
		(this,
		 get_w() / 2, get_h() / 2,
		 _("In-game Options"), UI::Align_HCenter),

// Second options block 'In-game options'
	m_single_watchwin (this, Point(get_w() * 19 / 200, get_h() * 5833 / 10000)),
	m_label_single_watchwin
		(this,
		 get_w() * 1313 / 10000, get_h() * 3 / 5,
		 _("Use single Watchwindow Mode"), UI::Align_VCenter),

	m_auto_roadbuild_mode (this, Point(get_w() * 19 / 200, get_h() * 63 / 100)),
	m_label_auto_roadbuild_mode
		(this,
		 get_w() * 1313 / 10000, get_h() * 6467 / 10000,
		 _("Start building road after placing a flag"), UI::Align_VCenter),

	m_show_workarea_preview
		(this, Point(get_w() * 19 / 200, get_h() * 6767 / 10000)),
	m_label_show_workarea_preview
		(this,
		 get_w() * 1313 / 10000, get_h() * 6933 / 10000,
		 _("Show buildings area preview"), UI::Align_VCenter),

	m_snap_windows_only_when_overlapping
		(this, Point(get_w() * 19 / 200, get_h() * 7233 / 10000)),
	m_label_snap_windows_only_when_overlapping
		(this,
		 get_w() * 1313 / 10000, get_h() * 37 / 50,
		 _("Snap windows only when overlapping"), UI::Align_VCenter),

	m_dock_windows_to_edges (this, Point(get_w() * 19 / 200, get_h() * 77 / 100)),
	m_label_dock_windows_to_edges
		(this,
		 get_w() * 1313 / 10000, get_h() * 7867 / 10000,
		 _("Dock windows to edges"), UI::Align_VCenter),

	m_label_autosave
		(this,
		 get_w() * 1313 / 10000, get_h() * 8333 / 10000,
		 _("Save game automatically every"), UI::Align_VCenter),
	m_label_remove_replays
		(this,
		 get_w() * 1313 / 10000, get_h() * 8799 / 10000,
		 _("Remove Replays older than:"), UI::Align_VCenter),

	os(opt)
{
	m_advanced_options.sigclicked.connect
		(boost::bind(&Fullscreen_Menu_Options::advanced_options, boost::ref(*this)));
	m_cancel.sigclicked.connect
		(boost::bind(&Fullscreen_Menu_Options::end_modal, this, static_cast<int32_t>(om_cancel)));
	m_apply.sigclicked.connect
		(boost::bind(&Fullscreen_Menu_Options::end_modal, this, static_cast<int32_t>(om_ok)));

	m_advanced_options.set_font(font_small());
	m_apply.set_font(font_small());
	m_cancel.set_font(font_small());

	/** TRANSLATORS Options: Remove Replays older than: */
	m_sb_autosave     .add_replacement(0, _("Off"));
	/** TRANSLATORS Options: Remove Replays older than: */
	m_sb_remove_replays.add_replacement(0, _("Never"));
	/** TRANSLATORS Options: Remove Replays older than: */
	m_sb_remove_replays.add_replacement(1, _("1 day"));

	m_sb_maxfps       .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_sb_autosave     .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_sb_remove_replays.set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	m_title           .set_textstyle(ts_big());
	m_label_fullscreen.set_textstyle(ts_small());
	m_fullscreen      .set_state(opt.fullscreen);
	m_label_inputgrab .set_textstyle(ts_small());
	m_inputgrab       .set_state(opt.inputgrab);
	m_label_music     .set_textstyle(ts_small());
	m_music           .set_state(opt.music);
	m_music           .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_label_fx        .set_textstyle(ts_small());
	m_fx              .set_state(opt.fx);
	m_fx              .set_enabled(not g_sound_handler.m_lock_audio_disabling);
	m_label_maxfps    .set_textstyle(ts_small());
	m_label_resolution.set_textstyle(ts_small());
	m_reslist         .set_font(ui_fn(), fs_small());
	m_label_language  .set_textstyle(ts_small());
	m_language_list   .set_font(ui_fn(), fs_small());

	m_label_game_options             .set_textstyle(ts_big());
	m_label_single_watchwin          .set_textstyle(ts_small());
	m_single_watchwin                .set_state(opt.single_watchwin);
	m_label_auto_roadbuild_mode      .set_textstyle(ts_small());
	m_auto_roadbuild_mode            .set_state(opt.auto_roadbuild_mode);
	m_label_show_workarea_preview    .set_textstyle(ts_small());
	m_show_workarea_preview          .set_state(opt.show_warea);
	m_label_snap_windows_only_when_overlapping.set_textstyle(ts_small());
	m_snap_windows_only_when_overlapping.set_state
		(opt.snap_windows_only_when_overlapping);
	m_label_dock_windows_to_edges    .set_textstyle(ts_small());
	m_dock_windows_to_edges          .set_state(opt.dock_windows_to_edges);
	m_label_autosave                 .set_textstyle(ts_small());
	m_label_remove_replays           .set_textstyle(ts_small());

	//  GRAPHIC_TODO: this shouldn't be here List all resolutions
	// take a copy to not change real video info structure
	SDL_PixelFormat fmt = *SDL_GetVideoInfo()->vfmt;
	fmt.BitsPerPixel = 32;
	for
		(const SDL_Rect * const * modes = SDL_ListModes(&fmt, SDL_SWSURFACE | SDL_FULLSCREEN);
		 modes && *modes;
		 ++modes)
	{
		const SDL_Rect & mode = **modes;
		if (800 <= mode.w and 600 <= mode.h)
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
		char buf[32];
		/** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		sprintf(buf, _("%1$i x %2$i"), m_resolutions[i].xres, m_resolutions[i].yres);
		const bool selected =
			m_resolutions[i].xres  == opt.xres and
			m_resolutions[i].yres  == opt.yres;
		did_select_a_res |= selected;
		m_reslist.add(buf, nullptr, nullptr, selected);
	}
	if (not did_select_a_res) {
		char buf[32];
		/** TRANSLATORS: Screen resolution, e.g. 800 x 600*/
		sprintf(buf, "%1$i x %2$i", opt.xres, opt.yres);
		m_reslist.add(buf, nullptr, nullptr, true);
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

void Fullscreen_Menu_Options::advanced_options() {
	Fullscreen_Menu_Advanced_Options aom(os);
	if (aom.run() == Fullscreen_Menu_Advanced_Options::om_ok) {
		os = aom.get_values();
		end_modal(om_restart);
	}
}

bool Fullscreen_Menu_Options::handle_key(bool down, SDL_keysym code)
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

	return Fullscreen_Menu_Base::handle_key(down, code);
}

Options_Ctrl::Options_Struct Fullscreen_Menu_Options::get_values() {
	const uint32_t res_index = m_reslist.selection_index();

	// Write all data from UI elements
	os.xres                  = m_resolutions[res_index].xres;
	os.yres                  = m_resolutions[res_index].yres;
	os.inputgrab             = m_inputgrab.get_state();
	os.fullscreen            = m_fullscreen.get_state();
	os.single_watchwin       = m_single_watchwin.get_state();
	os.auto_roadbuild_mode   = m_auto_roadbuild_mode.get_state();
	os.show_warea            = m_show_workarea_preview.get_state();
	os.snap_windows_only_when_overlapping
		= m_snap_windows_only_when_overlapping.get_state();
	os.dock_windows_to_edges = m_dock_windows_to_edges.get_state();
	os.music                 = m_music.get_state();
	os.fx                    = m_fx.get_state();
	if (m_language_list.has_selection())
		os.language      = m_language_list.get_selected();
	os.autosave              = m_sb_autosave.getValue();
	os.maxfps                = m_sb_maxfps.getValue();
	os.remove_replays        = m_sb_remove_replays.getValue();

	return os;
}


/**
 * The advanced option menu
 */
Fullscreen_Menu_Advanced_Options::Fullscreen_Menu_Advanced_Options
	(Options_Ctrl::Options_Struct const opt)
	:
	Fullscreen_Menu_Base("optionsmenu.jpg"),

// Values for alignment and size
	m_vbutw (get_h() * 333 / 10000),
	m_butw  (get_w() / 4),
	m_buth  (get_h() * 9 / 200),

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

// Spinboxes
	m_sb_dis_panel
		(this,
		 get_w() * 18 / 25, get_h() * 63 / 100, get_w() / 4, m_vbutw,
		 opt.panel_snap_distance, 0, 100, _("px."),
		 g_gr->images().get("pics/but1.png")),
	m_sb_dis_border
		(this,
		 get_w() * 18 / 25, get_h() * 6768 / 10000, get_w() / 4, m_vbutw,
		 opt.border_snap_distance, 0, 100, _("px."),
		 g_gr->images().get("pics/but1.png")),


// Title
	m_title
		(this,
		 get_w() / 2, get_h() / 40,
		 _("Advanced Options"), UI::Align_HCenter),

// First options block
	m_ui_font_list
		(this,
		 get_w() / 10, get_h() * 1667 / 10000,
		 get_w() /  4, get_h() * 2833 / 10000,
		 UI::Align_Left, true),
	m_label_ui_font
		(this,
		 get_w() * 1063 / 10000, get_h() * 1417 / 10000,
		 _("Main menu font:"), UI::Align_VCenter),
	m_message_sound
		(this, Point(get_w() * 29 / 80, get_h() * 171 / 1000)),
	m_label_message_sound
		(this,
		 get_w() * 4 / 10, get_h() * 1883 / 10000,
		 _("Play a sound at message arrival."),
		 UI::Align_VCenter),

// Second options block
	m_nozip (this, Point(get_w() * 19 / 200, get_h() * 5833 / 10000)),
	m_label_nozip
		(this,
		 get_w() * 1313 / 10000, get_h() * 3 / 5,
		 _("Do not zip widelands data files (maps, replays and savegames)."),
		 UI::Align_VCenter),
	m_label_snap_dis_panel
		(this,
		 get_w() * 1313 / 10000, get_h() * 6467 / 10000,
		 _("Distance for windows to snap to other panels:"), UI::Align_VCenter),
	m_label_snap_dis_border
		(this,
		 get_w() * 1313 / 10000, get_h() * 6933 / 10000,
		 _("Distance for windows to snap to borders:"), UI::Align_VCenter),

	m_remove_syncstreams (this, Point(get_w() * 19 / 200, get_h() * 7220 / 10000)),
	m_label_remove_syncstreams
		(this,
		 get_w() * 1313 / 10000, get_h() * 37 / 50,
		 _("Remove Syncstream dumps on startup"), UI::Align_VCenter),

	m_opengl (this, Point(get_w() * 19 / 200, get_h() * 7715 / 10000)),
	m_label_opengl
		(this,
		 get_w() * 1313 / 10000, get_h() * 7865 / 10000,
		 _("OpenGL rendering"), UI::Align_VCenter),
	m_transparent_chat (this, Point(get_w() * 19 / 200, get_h() * 8180 / 10000)),
	m_label_transparent_chat
		(this,
		 get_w() * 1313 / 10000, get_h() * 8330 / 10000,
		 _("Show in-game chat with transparent background"), UI::Align_VCenter),
	os(opt)
{
	m_cancel.sigclicked.connect
		(boost::bind
			(&Fullscreen_Menu_Advanced_Options::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(om_cancel)));
	m_apply.sigclicked.connect
		(boost::bind
			(&Fullscreen_Menu_Advanced_Options::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(om_ok)));

	m_cancel.set_font(font_small());
	m_apply.set_font(font_small());

	m_title                .set_textstyle(ts_big());
	m_label_message_sound  .set_textstyle(ts_small());
	m_message_sound        .set_state(opt.message_sound);
	m_label_nozip          .set_textstyle(ts_small());
	m_nozip                .set_state(opt.nozip);
	m_label_snap_dis_border.set_textstyle(ts_small());
	m_label_snap_dis_panel .set_textstyle(ts_small());
	m_label_remove_syncstreams.set_textstyle(ts_small());
	m_remove_syncstreams   .set_state(opt.remove_syncstreams);
	m_label_opengl         .set_textstyle(ts_small());
	m_opengl               .set_state(opt.opengl);
	m_label_transparent_chat.set_textstyle(ts_small());
	m_transparent_chat     .set_state(opt.transparent_chat);
	m_sb_dis_border        .set_textstyle(ts_small());
	m_sb_dis_panel         .set_textstyle(ts_small());

	m_label_ui_font.set_textstyle(ts_small());
	m_ui_font_list .set_font(ui_fn(), fs_small());

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
		filenameset_t files =
		   filter(g_fs->ListDirectory("fonts"),
		          [](const std::string& fn) {return boost::ends_with(fn, ".ttf");});

		for
			(filenameset_t::iterator pname = files.begin();
			 pname != files.end();
			 ++pname)
		{
			char const * const path = pname->c_str();
			char const * const name = FileSystem::FS_Filename(path);
			if (!strcmp(name, UI_FONT_NAME_SERIF))
				continue;
			if (!strcmp(name, UI_FONT_NAME_SANS))
				continue;
			if (g_fs->IsDirectory(name))
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

bool Fullscreen_Menu_Advanced_Options::handle_key(bool down, SDL_keysym code)
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

	return Fullscreen_Menu_Base::handle_key(down, code);
}


Options_Ctrl::Options_Struct Fullscreen_Menu_Advanced_Options::get_values() {
	// Write all remaining data from UI elements
	os.message_sound        = m_message_sound.get_state();
	os.nozip                = m_nozip.get_state();
	os.ui_font              = m_ui_font_list.get_selected();
	os.panel_snap_distance  = m_sb_dis_panel.getValue();
	os.border_snap_distance = m_sb_dis_border.getValue();
	os.remove_syncstreams   = m_remove_syncstreams.get_state();
	os.opengl               = m_opengl.get_state();
	os.transparent_chat     = m_transparent_chat.get_state();
	return os;
}


/**
 * Handles communication between window class and options
 */
Options_Ctrl::Options_Ctrl(Section & s)
: m_opt_section(s), m_opt_dialog(new Fullscreen_Menu_Options(options_struct()))
{
	handle_menu();
}

Options_Ctrl::~Options_Ctrl() {
	delete m_opt_dialog;
}

void Options_Ctrl::handle_menu()
{
	int32_t i = m_opt_dialog->run();
	if (i != Fullscreen_Menu_Options::om_cancel)
		save_options();
	if (i == Fullscreen_Menu_Options::om_restart) {
		delete m_opt_dialog;
		m_opt_dialog = new Fullscreen_Menu_Options(options_struct());
		handle_menu(); // Restart general options menu
	}
}

Options_Ctrl::Options_Struct Options_Ctrl::options_struct() {
	Options_Struct opt;
	opt.xres                = m_opt_section.get_int
		("xres",                XRES);
	opt.yres                = m_opt_section.get_int
		("yres",                YRES);
	opt.inputgrab           = m_opt_section.get_bool
		("inputgrab",          false);
	opt.fullscreen          = m_opt_section.get_bool
		("fullscreen",         false);
	opt.single_watchwin     = m_opt_section.get_bool
		("single_watchwin",    false);
	opt.auto_roadbuild_mode = m_opt_section.get_bool
		("auto_roadbuild_mode", true);
	opt.show_warea          = m_opt_section.get_bool
		("workareapreview",    true);
	opt.snap_windows_only_when_overlapping
		= m_opt_section.get_bool
			("snap_windows_only_when_overlapping",      false);
	opt.dock_windows_to_edges
		= m_opt_section.get_bool
			("dock_windows_to_edges",                   false);
	opt.language              =  m_opt_section.get_string
		("language",         "");
	opt.music                 = !m_opt_section.get_bool
		("disable_music",   false);
	opt.fx                    = !m_opt_section.get_bool
		("disable_fx",      false);
	opt.autosave
		= m_opt_section.get_int
			("autosave",        DEFAULT_AUTOSAVE_INTERVAL * 60);
	opt.maxfps                =  m_opt_section.get_int
		("maxfps",              25);

	opt.message_sound         =  m_opt_section.get_bool
		("sound_at_message", true);
	opt.nozip                 =  m_opt_section.get_bool
		("nozip",            false);
	opt.ui_font               =  m_opt_section.get_string
		("ui_font",     "serif");
	opt.border_snap_distance  =  m_opt_section.get_int
		("border_snap_distance", 0);
	opt.panel_snap_distance   =  m_opt_section.get_int
		("panel_snap_distance",  0);
	opt.remove_replays        = m_opt_section.get_int
		("remove_replays", 0);
	opt.remove_syncstreams    = m_opt_section.get_bool
		("remove_syncstreams", true);
	opt.opengl                = m_opt_section.get_bool
		("opengl", true);
	opt.transparent_chat      = m_opt_section.get_bool
		("transparent_chat", true);
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
