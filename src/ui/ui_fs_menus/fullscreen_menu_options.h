/*
 * Copyright (C) 2002-2004 by Widelands Development Team
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

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#define NUM_RESOLUTIONS		6

#include "fullscreen_menu_base.h"
#include "system.h"
#include "types.h"
#include "ui_checkbox.h"
#include "ui_radiobutton.h"
#include "ui_listselect.h"
#include "options.h"

class Fullscreen_Menu_Options;

class Options_Ctrl {
	public:
		struct Options_Struct {
			int xres;
			int yres;
			int depth;
			bool inputgrab;
			bool fullscreen;
			bool single_watchwin;
			bool show_warea;
         bool music;
         bool fx;
         std::string language;
		};
		
		Options_Ctrl(Section *s);
		~Options_Ctrl();
		Options_Ctrl::Options_Struct options_struct(Section *s);
		void save_options();
	private:
		Fullscreen_Menu_Options *m_opt_dialog;
		Section *m_opt_section;
};

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */
 
class Fullscreen_Menu_Options : public Fullscreen_Menu_Base {
public:
	Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt);
	Options_Ctrl::Options_Struct Fullscreen_Menu_Options::get_values();
   enum {
      om_cancel = 0,
      om_ok = 1
   };

private:
	UICheckbox*	m_fullscreen;
	UICheckbox*	m_inputgrab;
	UICheckbox*	m_music;
	UICheckbox*	m_fx;
	UICheckbox* m_single_watchwin;
	UICheckbox* m_show_workarea_preview;
	UIListselect* m_reslist;
	UIListselect* m_language_list;

	struct res {
		int xres;
		int yres;
		int depth;
	};
	std::vector<res> m_resolutions;
};

#endif /* OPTIONSMENU_H */
