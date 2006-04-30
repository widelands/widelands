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

#define DEFINE_LANGUAGES  // So that the language array gets defined

#include <stdio.h>
#include "fullscreen_menu_options.h"
#include "constants.h"
#include "graphic.h"
#include "i18n.h"
#include "languages.h"
#include "profile.h"
#include "sound_handler.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_listselect.h"
#include "wlapplication.h"

/*
==============================================================================

Fullscreen_Menu_Options

==============================================================================
*/


Fullscreen_Menu_Options::Fullscreen_Menu_Options(Options_Ctrl::Options_Struct opt)
	: Fullscreen_Menu_Base("optionsmenu.jpg")
{

	// Menu title
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 30, _("General Options"), Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 330, 420, 174, 24, 0, om_cancel);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title(_("Cancel").c_str());

	b = new UIButton(this, 136, 420, 174, 24, 2, om_ok);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title(_("Apply").c_str());

	// Fullscreen mode
	m_fullscreen = new UICheckbox(this, 260, 70);
	m_fullscreen->set_state(opt.fullscreen);
	new UITextarea(this, 285, 80, _("Fullscreen"), Align_VCenter);

	// input grab
	m_inputgrab = new UICheckbox(this, 260, 100);
	m_inputgrab->set_state(opt.inputgrab);
	new UITextarea(this, 285, 110, _("Grab Input"), Align_VCenter);

	// Music
	m_music = new UICheckbox(this, 260, 130);
	m_music->set_state(opt.music);
	new UITextarea(this, 285, 140, _("Enable Music"),
	               Align_VCenter);
	if (g_sound_handler.m_lock_audio_disabling) {
		m_music->set_enabled(false);
	}

	// Sound FX
	m_fx = new UICheckbox(this, 260, 160);
	m_fx->set_state(opt.fx);
	new UITextarea(this, 285, 170, _("Enable Sound"),
	               Align_VCenter);
	if (g_sound_handler.m_lock_audio_disabling) {
		m_fx->set_enabled(false);
	}


	// In-game resolution
	new UITextarea(this, 70, 70, _("In-game resolution"), Align_VCenter);

   // GRAPHIC_TODO: this shouldn't be here List all resolutions
   SDL_PixelFormat* fmt = SDL_GetVideoInfo()->vfmt;
   fmt->BitsPerPixel = 16;
   SDL_Rect** modes = SDL_ListModes( fmt, SDL_SWSURFACE | SDL_FULLSCREEN );
   if( modes )
      for( uint i = 0; modes[i]; i++ ) {
         if(modes[i]->w < 640) continue;
	 res this_res = {
            modes[i]->w,
            modes[i]->h,
            16
         };
         if( !m_resolutions.size() ||
               this_res.xres != m_resolutions[m_resolutions.size()-1].xres ||
               this_res.yres != m_resolutions[m_resolutions.size()-1].yres)
            m_resolutions.push_back(this_res);
      }
   fmt->BitsPerPixel = 32;
   modes = SDL_ListModes( fmt, SDL_SWSURFACE | SDL_FULLSCREEN );
   if( modes )
      for( uint i = 0; modes[i]; i++ ) {
         if(modes[i]->w < 640) continue;
         res this_res = {
            modes[i]->w,
            modes[i]->h,
            32
         };
         if( !m_resolutions.size() ||
               this_res.xres != m_resolutions[m_resolutions.size()-1].xres ||
               this_res.yres != m_resolutions[m_resolutions.size()-1].yres)
            m_resolutions.push_back(this_res);
      }

	m_reslist = new UIListselect(this, 60, 85, 150, 130,Align_Left,true);
	bool did_select_a_res=false;
	for(uint i = 0; i < m_resolutions.size(); i++) {
		char buf[32];
		sprintf(buf, "%ix%i %i bit", m_resolutions[i].xres, m_resolutions[i].yres, m_resolutions[i].depth);
		bool selected = ((m_resolutions[i].xres == opt.xres
      && m_resolutions[i].yres == opt.yres
      && m_resolutions[i].depth == opt.depth) ? true : false);
		did_select_a_res|=selected;
		m_reslist->add_entry(buf,NULL,selected);
	}
	if (!did_select_a_res)
		m_reslist->select(m_reslist->get_nr_entries()-1);

   // Available locales
  	// In-game resolution
	new UITextarea(this, MENU_XRES/2+70, 70, _("Language"), Align_VCenter);
	m_language_list = new UIListselect(this, MENU_XRES/2 + 60, 85, 170, 130,Align_Left,true);
   available_languages[0].name = _( "System default language" );
   for(uint i = 0; i < NR_LANGUAGES; i++) {
		bool selected = false;
	   if(  available_languages[i].abbrev == opt.language )
         selected = true;
      m_language_list->add_entry( available_languages[i].name.c_str(),
            &available_languages[i].abbrev, selected);
	}


	title= new UITextarea(this, MENU_XRES/2, 240, _("In-game Options"), Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

   // Toggle Options
	m_single_watchwin = new UICheckbox(this,60,280);
	m_single_watchwin->set_state(opt.single_watchwin);
	new UITextarea(this,85,290,_("Use single Watchwindow Mode"), Align_VCenter);
	m_show_workarea_preview= new UICheckbox(this,60,305);
	m_show_workarea_preview->set_state(opt.show_warea);
	new UITextarea(this,85,315,_("Show buildings area preview"), Align_VCenter);
	m_snap_windows_only_when_overlapping = new UICheckbox(this,60,330);
	m_snap_windows_only_when_overlapping->set_state(opt.snap_windows_only_when_overlapping);
	new UITextarea(this,85,340,_("Snap windows only when overlapping"), Align_VCenter);
	m_dock_windows_to_edges = new UICheckbox(this,60,355);
	m_dock_windows_to_edges->set_state(opt.dock_windows_to_edges);
	new UITextarea(this,85,365,_("Dock windows to edges"), Align_VCenter);
}

Options_Ctrl::Options_Struct Fullscreen_Menu_Options::get_values() {
	Options_Ctrl::Options_Struct opt;
	int res_index = m_reslist->get_selection_index();
	opt.xres = m_resolutions[res_index].xres;
	opt.yres = m_resolutions[res_index].yres;
	opt.depth = m_resolutions[res_index].depth;
	opt.fullscreen = m_fullscreen->get_state();
	opt.inputgrab = m_inputgrab->get_state();
	opt.single_watchwin = m_single_watchwin->get_state();
	opt.show_warea = m_show_workarea_preview->get_state();
	opt.snap_windows_only_when_overlapping = m_snap_windows_only_when_overlapping->get_state();
	opt.dock_windows_to_edges = m_dock_windows_to_edges->get_state();
	opt.language = ((std::string*)(m_language_list->get_selection()))->c_str();
   opt.music = m_music->get_state();
   opt.fx = m_fx->get_state();
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
	opt.depth = s->get_int("depth",16);
	opt.inputgrab = s->get_bool("inputgrab", false);
	opt.fullscreen = s->get_bool("fullscreen", false);
	opt.single_watchwin = s->get_bool("single_watchwin",false);
	opt.show_warea= s->get_bool("workareapreview",false);
	opt.snap_windows_only_when_overlapping= s->get_bool("snap_windows_only_when_overlapping", false);
	opt.dock_windows_to_edges= s->get_bool("dock_windows_to_edges", false);
	opt.language = s->get_string("language", "");
   opt.music = !s->get_bool("disable_music", false);
   opt.fx = !s->get_bool("disable_fx", false );
   return opt;
}

void Options_Ctrl::save_options(){
	Options_Ctrl::Options_Struct opt = m_opt_dialog->get_values();
	m_opt_section->set_int("xres", opt.xres);
	m_opt_section->set_int("yres", opt.yres);
	m_opt_section->set_bool("fullscreen", opt.fullscreen);
	m_opt_section->set_bool("inputgrab", opt.inputgrab);
	m_opt_section->set_bool("single_watchwin",opt.single_watchwin);
	m_opt_section->set_bool("workareapreview",opt.show_warea);
	m_opt_section->set_bool("snap_windows_only_when_overlapping", opt.snap_windows_only_when_overlapping);
	m_opt_section->set_bool("dock_windows_to_edges", opt.dock_windows_to_edges);
	m_opt_section->set_int("depth", opt.depth);
   m_opt_section->set_bool("disable_music", !opt.music);
   m_opt_section->set_bool("disable_fx", !opt.fx);
   m_opt_section->set_string("language", opt.language.c_str());
   WLApplication::get()->set_input_grab(opt.inputgrab);
   i18n::set_locale( opt.language.c_str() );
   g_sound_handler.set_disable_music( !opt.music );
   g_sound_handler.set_disable_fx( !opt.fx );
}

