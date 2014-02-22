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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef FULLSCREEN_MENU_OPTIONS_H
#define FULLSCREEN_MENU_OPTIONS_H

#include <cstring>
#include <string>
#include <vector>

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"

class Fullscreen_Menu_Options;
struct Section;

class Options_Ctrl {
public:
	struct Options_Struct {
		int32_t xres;
		int32_t yres;
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
		uint32_t remove_replays;
		bool remove_syncstreams;
		bool opengl;
		bool transparent_chat;

		// advanced options
		bool message_sound;
		bool nozip;
		std::string ui_font;
		int32_t border_snap_distance;
		int32_t panel_snap_distance;
	};

	Options_Ctrl(Section &);
	~Options_Ctrl();
	void handle_menu();
	Options_Ctrl::Options_Struct options_struct();
	void save_options();
private:
	Section & m_opt_section;
	Fullscreen_Menu_Options * m_opt_dialog;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

class Fullscreen_Menu_Options : public Fullscreen_Menu_Base {
public:
	Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt);
	Options_Ctrl::Options_Struct get_values();
	enum {
		om_cancel  = 0,
		om_ok      = 1,
		om_restart = 2
	};

	/// Handle keypresses
	virtual bool handle_key(bool down, SDL_keysym code) override;

private:
	uint32_t                          m_vbutw;
	uint32_t                          m_butw;
	uint32_t                          m_buth;
	UI::Button           m_advanced_options, m_cancel, m_apply;
	UI::SpinBox                       m_sb_maxfps, m_sb_autosave;
	UI::SpinBox                       m_sb_remove_replays;
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
	UI::Textarea                      m_label_autosave;
	UI::Textarea                      m_label_remove_replays;
	Options_Ctrl::Options_Struct os;

	void advanced_options();

	class ScreenResolution {
	public:
		int32_t xres;
		int32_t yres;
	};

	/// All supported screen resolutions.
	std::vector<ScreenResolution> m_resolutions;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

class Fullscreen_Menu_Advanced_Options : public Fullscreen_Menu_Base {
public:
	Fullscreen_Menu_Advanced_Options(Options_Ctrl::Options_Struct opt);
	Options_Ctrl::Options_Struct get_values();
	enum {
		om_cancel =   0,
		om_ok     =   1
	};

	/// Handle keypresses
	virtual bool handle_key(bool down, SDL_keysym code) override;

private:
	uint32_t                    m_vbutw;
	uint32_t                    m_butw;
	uint32_t                    m_buth;

	UI::Button     m_cancel, m_apply;
	UI::SpinBox                 m_sb_dis_panel, m_sb_dis_border;
	UI::Textarea                m_title;
	UI::Listselect<std::string> m_ui_font_list;
	UI::Textarea                m_label_ui_font;
	UI::Checkbox                m_message_sound;
	UI::Textarea                m_label_message_sound;
	UI::Checkbox                m_nozip;
	UI::Textarea                m_label_nozip;

	UI::Textarea                m_label_snap_dis_panel, m_label_snap_dis_border;

	UI::Checkbox                m_remove_syncstreams;
	UI::Textarea                m_label_remove_syncstreams;
	UI::Checkbox                m_opengl;
	UI::Textarea                m_label_opengl;
	UI::Checkbox                m_transparent_chat;
	UI::Textarea                m_label_transparent_chat;

	Options_Ctrl::Options_Struct os;
};

#endif
