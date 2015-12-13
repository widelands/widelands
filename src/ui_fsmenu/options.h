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

#ifndef WL_UI_FSMENU_OPTIONS_H
#define WL_UI_FSMENU_OPTIONS_H

#include <cstring>
#include <string>
#include <vector>

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"

class FullscreenMenuOptions;
class Section;


class OptionsCtrl {
public:
	struct OptionsStruct {
		int32_t xres;
		int32_t yres;
		bool inputgrab;
		bool fullscreen;
		bool single_watchwin;
		bool auto_roadbuild_mode;
		bool show_warea;
		bool snap_win_overlap_only;
		bool dock_windows_to_edges;
		bool music;
		bool fx;
		std::string language;
		int32_t autosave; // autosave interval in minutes
		int32_t rolling_autosave; //number of file to use for rolling autosave
		uint32_t maxfps;
		uint32_t remove_replays;
		bool remove_syncstreams;
		bool transparent_chat;

		// advanced options
		bool message_sound;
		bool nozip;
		std::string ui_font;
		int32_t border_snap_distance;
		int32_t panel_snap_distance;
	};

	OptionsCtrl(Section &);
	~OptionsCtrl();
	void handle_menu();
	OptionsCtrl::OptionsStruct options_struct();
	void save_options();
private:
	Section & m_opt_section;
	FullscreenMenuOptions * m_opt_dialog;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

class FullscreenMenuOptions : public FullscreenMenuBase {
public:
	FullscreenMenuOptions(OptionsCtrl::OptionsStruct opt);
	OptionsCtrl::OptionsStruct get_values();

private:
	uint32_t const              m_vbutw;
	uint32_t const              m_butw;
	uint32_t const              m_buth;
	uint32_t const              m_hmargin;
	uint32_t const              m_padding;
	uint32_t const              m_space;
	uint32_t const              m_offset_first_group;
	uint32_t const              m_offset_second_group;

	UI::Button                  m_advanced_options, m_cancel, m_apply;

	UI::Textarea                m_title;
	UI::Textarea                m_label_resolution;
	UI::Listselect<void *>      m_reslist;
	UI::Checkbox                m_fullscreen;
	UI::Checkbox                m_inputgrab;
	UI::SpinBox                 m_sb_maxfps;

	UI::Textarea                m_label_language;
	UI::Listselect<std::string> m_language_list;
	UI::Checkbox                m_music;
	UI::Checkbox                m_fx;

	UI::Textarea                m_label_game_options;
	UI::Checkbox                m_single_watchwin;
	UI::Checkbox                m_auto_roadbuild_mode;
	UI::Checkbox                m_show_workarea_preview;
	UI::Checkbox                m_snap_win_overlap_only;
	UI::Checkbox                m_dock_windows_to_edges;
	UI::SpinBox                 m_sb_autosave;
	UI::SpinBox                 m_sb_remove_replays;

	OptionsCtrl::OptionsStruct  os;

	void update_sb_autosave_unit();
	void update_sb_remove_replays_unit();
	void advanced_options();

	// Fills the language selection list
	void add_languages_to_list(const std::string& current_locale);

	class ScreenResolution {
	public:
		int32_t xres;
		int32_t yres;
		int32_t depth;
	};

	/// All supported screen resolutions.
	std::vector<ScreenResolution> m_resolutions;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */

class FullscreenMenuAdvancedOptions : public FullscreenMenuBase {
public:
	FullscreenMenuAdvancedOptions(OptionsCtrl::OptionsStruct opt);
	OptionsCtrl::OptionsStruct get_values();

private:
	void update_sb_dis_panel_unit();
	void update_sb_dis_border_unit();

	uint32_t const              m_vbutw;
	uint32_t const              m_butw;
	uint32_t const              m_buth;
	uint32_t const              m_hmargin;
	uint32_t const              m_padding;
	uint32_t const              m_space;

	UI::Button                  m_cancel, m_apply;
	UI::Textarea                m_title;

	UI::SpinBox                 m_sb_dis_panel, m_sb_dis_border;
	UI::Checkbox                m_transparent_chat;
	UI::Checkbox                m_message_sound;
	UI::Checkbox                m_nozip;
	UI::Checkbox                m_remove_syncstreams;

	OptionsCtrl::OptionsStruct  os;
};

#endif  // end of include guard: WL_UI_FSMENU_OPTIONS_H
