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

#include <stdio.h>
#include "fullscreen_menu_options.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_listselect.h"
#include "constants.h"
#include "graphic.h"

/*
==============================================================================

Fullscreen_Menu_Options

==============================================================================
*/

Fullscreen_Menu_Options::res Fullscreen_Menu_Options::resolutions[NUM_RESOLUTIONS] = {
	{ 640, 480, 16 },
	{ 800, 600, 16 },
	{ 1024, 768, 16 },
	{ 640, 480, 32 },
	{ 800, 600, 32 },
	{ 1024, 768, 32 }
};


Fullscreen_Menu_Options::Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt)
	: Fullscreen_Menu_Base("optionsmenu.jpg") {
	
	// Menu title
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 30, "Grapic Options", Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 330, 420, 174, 24, 0, om_cancel);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title("Cancel");

	b = new UIButton(this, 136, 420, 174, 24, 2, om_ok);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title("Apply");

	// Fullscreen mode
	m_fullscreen = new UICheckbox(this, 260, 70);
	m_fullscreen->set_state(opt.fullscreen);
	new UITextarea(this, 285, 80, "Fullscreen", Align_VCenter);

	// input grab
	m_inputgrab = new UICheckbox(this, 260, 100);
	m_inputgrab->set_state(opt.inputgrab);
	new UITextarea(this, 285, 110, "Grab Input", Align_VCenter);

	// In-game resolution
	new UITextarea(this, 70, 70, "In-game resolution", Align_VCenter);

	m_reslist = new UIListselect(this, 60, 85, 150, 130,Align_Left,true);
	for(int i = 0; i < NUM_RESOLUTIONS; i++) {
		char buf[32];
		sprintf(buf, "%ix%i %i bit", resolutions[i].xres, resolutions[i].yres, resolutions[i].depth);
		bool selected = ((resolutions[i].xres == opt.xres && resolutions[i].depth == opt.depth) ? true : false);
		m_reslist->add_entry(buf,NULL,selected);
	}

	title= new UITextarea(this, MENU_XRES/2, 240, "In-game Options", Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	
	m_single_watchwin = new UICheckbox(this,60,280);
	m_single_watchwin->set_state(opt.single_watchwin);
	new UITextarea(this,85,290,"Use single Watchwindow Mode", Align_VCenter);
}

Options_Ctrl::Options_Struct Fullscreen_Menu_Options::get_values() {
	Options_Ctrl::Options_Struct opt;
	int res_index = m_reslist->get_selection_index();
	opt.xres = resolutions[res_index].xres;
	opt.yres = resolutions[res_index].yres;
	opt.depth = resolutions[res_index].depth;
	opt.fullscreen = m_fullscreen->get_state();
	opt.inputgrab = m_inputgrab->get_state();
	opt.single_watchwin = m_single_watchwin->get_state();
	return opt;
}


//Class Options_Ctrl
//Handles communication between window class and options
Options_Ctrl::Options_Ctrl(Section* s) {
	m_opt_dialog = new Fullscreen_Menu_Options(options_struct(s));
	m_opt_section = s;
	int code = m_opt_dialog->run();
	if (code == Fullscreen_Menu_Options::om_ok)
		save_options();                  
}

Options_Ctrl::~Options_Ctrl() {
	delete m_opt_dialog;
}


Options_Ctrl::Options_Struct Options_Ctrl::options_struct(Section* s) {
	Options_Struct opt;
	opt.xres = s->get_int("xres",640);
	opt.yres = s->get_int("yres",480);
	opt.depth = (strcmp(s->get_string("gfxsys","sw16"),"sw16") == 0 ? 16 : 32);
	opt.inputgrab = s->get_bool("inputgrab", false);
	opt.fullscreen = s->get_bool("fullscreen", false);
	opt.single_watchwin = s->get_bool("single_watchwin",false);
	return opt;
}

void Options_Ctrl::save_options(){
	Options_Ctrl::Options_Struct opt = m_opt_dialog->get_values();
	m_opt_section->set_int("xres", opt.xres);
	m_opt_section->set_int("yres", opt.yres);
	m_opt_section->set_bool("fullscreen", opt.fullscreen);
	m_opt_section->set_bool("inputgrab", opt.inputgrab);
	m_opt_section->set_bool("single_watchwin",opt.single_watchwin);
	((opt.depth == 32) ? m_opt_section->set_string("gfxsys","sw32") : m_opt_section->set_string("gfxsys","sw16"));
	
	Sys_SetInputGrab(opt.inputgrab);
}

