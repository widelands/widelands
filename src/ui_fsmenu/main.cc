/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "ui_fsmenu/main.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "build_info.h"
#include "graphic/graphic.h"

FullscreenMenuMain::FullscreenMenuMain()
   : FullscreenMenuMainMenu("images/ui_fsmenu/mainmenu.jpg"),

     // Buttons
     // This box needs to be a bit higher than in the other menus, because we have a lot of buttons
     vbox(this,
          box_x_,
          box_y_ - buth_,
          UI::Box::Vertical,
          butw_,
          get_h() - (box_y_ - buth_),
          padding_),
     playtutorial(&vbox,
                  "play_tutorial",
                  0,
                  0,
                  butw_,
                  buth_,
                  g_gr->images().get(button_background_),
                  _("Play Tutorial")),
     singleplayer(&vbox,
                  "single_player",
                  0,
                  0,
                  butw_,
                  buth_,
                  g_gr->images().get(button_background_),
                  _("Single Player")),
     multiplayer(&vbox,
                 "multi_player",
                 0,
                 0,
                 butw_,
                 buth_,
                 g_gr->images().get(button_background_),
                 _("Multiplayer")),
     replay(&vbox,
            "replay",
            0,
            0,
            butw_,
            buth_,
            g_gr->images().get(button_background_),
            _("Watch Replay")),
     editor(
        &vbox, "editor", 0, 0, butw_, buth_, g_gr->images().get(button_background_), _("Editor")),
     options(
        &vbox, "options", 0, 0, butw_, buth_, g_gr->images().get(button_background_), _("Options")),
     about(&vbox,
           "about",
           0,
           0,
           butw_,
           buth_,
           g_gr->images().get(button_background_),
           _("About Widelands")),
     exit(&vbox,
          "exit",
          0,
          0,
          butw_,
          buth_,
          g_gr->images().get(button_background_),
          _("Exit Widelands")),

     // Textlabels
     version(
        this,
        get_w(),
        get_h(),
        /** TRANSLATORS: %1$s = version string, %2%s = "Debug" or "Release" */
        (boost::format(_("Version %1$s (%2$s)")) % build_id().c_str() % build_type().c_str()).str(),
        UI::Align::kBottomRight),
     copyright(this,
               0,
               get_h() - 0.5 * buth_,
               /** TRANSLATORS: Placeholders are the copyright years */
               (boost::format(_("(C) %1%-%2% by the Widelands Development Team")) %
                kWidelandsCopyrightStart % kWidelandsCopyrightEnd)
                  .str(),
               UI::Align::kBottomLeft),
     gpl(this,
         0,
         get_h(),
         _("Licensed under the GNU General Public License V2.0"),
         UI::Align::kBottomLeft) {
	playtutorial.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kTutorial));
	singleplayer.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kSinglePlayer));
	multiplayer.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kMultiplayer));
	replay.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kReplay));
	editor.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kEditor));
	options.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kOptions));
	about.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kAbout));
	exit.sigclicked.connect(
	   boost::bind(&FullscreenMenuMain::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
	               FullscreenMenuBase::MenuTarget::kExit));

	vbox.add(&playtutorial, UI::Align::kHCenter);
	vbox.add(&singleplayer, UI::Align::kHCenter);
	vbox.add(&multiplayer, UI::Align::kHCenter);

	vbox.add_space(2 * padding_);

	vbox.add(&replay, UI::Align::kHCenter);

	vbox.add_space(2 * padding_);

	vbox.add(&editor, UI::Align::kHCenter);

	vbox.add_space(2 * padding_);

	vbox.add(&options, UI::Align::kHCenter);

	vbox.add_space(2 * padding_);

	vbox.add(&about, UI::Align::kHCenter);

	vbox.add_space(5 * padding_);

	vbox.add(&exit, UI::Align::kHCenter);

	vbox.set_size(butw_, get_h() - vbox.get_y());
}

void FullscreenMenuMain::clicked_ok() {
	;  // do nothing
}
