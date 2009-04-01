/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_OPTIONS_H
#define FULLSCREEN_MENU_OPTIONS_H

#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_listselect.h"
#include "ui_textarea.h"

#include <string>
#include <cstring>
#include <vector>

class Fullscreen_Menu_Options;
struct Section;

#define NUM_RESOLUTIONS 6

struct Options_Ctrl {
	struct Options_Struct {
		int32_t xres;
		int32_t yres;
		int32_t depth;
		bool inputgrab;
		bool fullscreen;
		bool single_watchwin;
		bool auto_roadbuild_mode;
		bool show_warea;
		bool snap_windows_only_when_overlapping;
		bool dock_windows_to_edges;
		bool music;
		bool fx;
		std::string language;
		int32_t autosave; // autosave interval in minutes
		uint32_t maxfps;

		// advanced options
		bool nozip;
		std::string ui_font;
		int32_t speed_of_new_game;
		int32_t border_snap_distance;
		int32_t panel_snap_distance;
	};

	Options_Ctrl(Section &);
	~Options_Ctrl();
	Options_Ctrl::Options_Struct options_struct(Section &);
	void save_options();
private:
	Fullscreen_Menu_Options *m_opt_dialog;
	Section & m_opt_section;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

struct Fullscreen_Menu_Options : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt);
	Options_Ctrl::Options_Struct get_values();
	enum {
		om_cancel =   0,
		om_ok     =   1,
		plus      =   1,
		minus     =  -1,
		plusTen   =  10,
		minusTen  = -10
	};

private:
	uint32_t                          m_vbutw;
	uint32_t                          m_butw;
	uint32_t                          m_buth;
	uint32_t                          m_fs;
	std::string                       m_fn;
	UI::Button<Fullscreen_Menu_Options> m_advanced_options;
	UI::IDButton<Fullscreen_Menu_Options, int32_t>
		m_cancel, m_apply, m_fps_plus, m_fps_minus, m_autosave_plus,
		m_autosave_minus, m_autosave_tenplus, m_autosave_tenminus;
	UI::Textarea                      m_title;
	UI::Checkbox                      m_fullscreen;
	UI::Textarea                      m_label_fullscreen;
	UI::Checkbox                      m_inputgrab;
	UI::Textarea                      m_label_inputgrab;
	UI::Checkbox                      m_music;
	UI::Textarea                      m_label_music;
	UI::Checkbox                      m_fx;
	UI::Textarea                      m_label_fx;
	UI::Textarea                      m_label_maxfps;
	UI::Textarea                      m_value_maxfps;
	UI::Listselect<void *>            m_reslist;
	UI::Textarea                      m_label_resolution;
	UI::Textarea                      m_label_language;
	UI::Listselect<std::string>       m_language_list;
	UI::Textarea                      m_label_game_options;
	UI::Checkbox                      m_single_watchwin;
	UI::Textarea                      m_label_single_watchwin;
	UI::Checkbox                      m_auto_roadbuild_mode;
	UI::Textarea                      m_label_auto_roadbuild_mode;
	UI::Checkbox                      m_show_workarea_preview;
	UI::Textarea                      m_label_show_workarea_preview;
	UI::Checkbox                      m_snap_windows_only_when_overlapping;
	UI::Textarea                      m_label_snap_windows_only_when_overlapping;
	UI::Checkbox                      m_dock_windows_to_edges;
	UI::Textarea                      m_label_dock_windows_to_edges;
	UI::Checkbox                      m_autosave;
	UI::Textarea                      m_label_autosave;
	UI::Textarea                      m_value_autosave;
	UI::Textarea                      m_label_minute;

	Options_Ctrl::Options_Struct os;

	int32_t m_maxfps;
	int32_t m_asvalue;
	void advanced_options();
	void maxFpsChange(int32_t);
	void update_maxfps();
	void autosaveChange(int32_t);
	void update_autosave();

	struct res {
		int32_t xres;
		int32_t yres;
		int32_t depth;
	};
	std::vector<res> m_resolutions;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

struct Fullscreen_Menu_Advanced_Options : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Advanced_Options(Options_Ctrl::Options_Struct opt);
	Options_Ctrl::Options_Struct get_values();
	enum {
		om_cancel =   0,
		om_ok     =   1
	};

private:
	uint32_t                          m_vbutw;
	uint32_t                          m_butw;
	uint32_t                          m_buth;
	uint32_t                          m_fs;
	std::string                       m_fn;

	UI::IDButton<Fullscreen_Menu_Advanced_Options, int32_t> m_cancel, m_apply;
	UI::Textarea                      m_title;
	UI::Listselect<std::string>       m_ui_font_list;
	UI::Textarea                      m_label_ui_font;
	UI::Checkbox                      m_nozip;
	UI::Textarea                      m_label_nozip;

	Options_Ctrl::Options_Struct os;
};

#endif
