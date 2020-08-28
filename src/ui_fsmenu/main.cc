/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/i18n.h"
#include "build_info.h"
#include "graphic/image_cache.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "wui/savegameloader.h"

FullscreenMenuMain::FullscreenMenuMain()
   : FullscreenMenuMainMenu(),

     logo_icon_(this, g_image_cache->get("images/ui_fsmenu/main_title.png")),

     // Buttons
     playtutorial(&vbox_,
                  "play_tutorial",
                  0,
                  0,
                  butw_,
                  buth_,
                  UI::ButtonStyle::kFsMenuMenu,
                  _("Play Tutorial")),
     singleplayer(&vbox_,
                  "single_player",
                  0,
                  0,
                  butw_,
                  buth_,
                  UI::ButtonStyle::kFsMenuMenu,
                  _("Single Player")),
     continue_lastsave(&vbox_,
                       "continue_lastsave",
                       0,
                       0,
                       butw_,
                       buth_,
                       UI::ButtonStyle::kFsMenuMenu,
                       _("Continue Playing")),
     multiplayer(
        &vbox_, "multi_player", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Multiplayer")),
     replay(&vbox_, "replay", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Watch Replay")),
     editor(&vbox_, "editor", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Editor")),
     options(&vbox_, "options", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Options")),
     about(&vbox_, "about", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("About Widelands")),
     exit(&vbox_, "exit", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Exit Widelands")),

     // Textlabels
     version(
        this,
        0,
        0,
        0,
        0,
        /** TRANSLATORS: %1$s = version string, %2%s = "Debug" or "Release" */
        (boost::format(_("Version %1$s (%2$s)")) % build_id().c_str() % build_type().c_str()).str(),
        UI::Align::kRight),
     copyright(this,
               0,
               0,
               0,
               0,
               /** TRANSLATORS: Placeholders are the copyright years */
               (boost::format(_("(C) %1%-%2% by the Widelands Development Team")) %
                kWidelandsCopyrightStart % kWidelandsCopyrightEnd)
                  .str()),
     gpl(this, 0, 0, 0, 0, _("Licensed under the GNU General Public License V2.0")) {
	playtutorial.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kTutorial);
	});
	singleplayer.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kSinglePlayer);
	});
	continue_lastsave.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kContinueLastsave);
	});
	multiplayer.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMultiplayer);
	});
	replay.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kReplay);
	});
	editor.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kEditor);
	});
	options.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOptions);
	});
	about.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kAbout);
	});
	exit.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kExit);
	});

	vbox_.add(&playtutorial, UI::Box::Resizing::kFullSize);
	vbox_.add(&singleplayer, UI::Box::Resizing::kFullSize);
	vbox_.add(&multiplayer, UI::Box::Resizing::kFullSize);
	vbox_.add(&continue_lastsave, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&replay, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&editor, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&options, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&about, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&exit, UI::Box::Resizing::kFullSize);

	Widelands::Game game;
	SinglePlayerLoader loader(game);
	std::vector<SavegameData> games = loader.load_files(kSaveDir);
	SavegameData* newest_singleplayer = nullptr;
	for (SavegameData& data : games) {
		if (!data.is_directory() && data.is_singleplayer() &&
		    (newest_singleplayer == nullptr || newest_singleplayer->compare_save_time(data))) {
			newest_singleplayer = &data;
		}
	}
	if (newest_singleplayer) {
		filename_for_continue_ = newest_singleplayer->filename;
		continue_lastsave.set_tooltip(
		   (boost::format("%s<br>%s<br>%s<br>%s<br>%s<br>%s") %
		    g_style_manager->font_style(UI::FontStyle::kTooltipHeader)
		       .as_font_tag(
		          /* strip leading "save/" and trailing ".wgf" */
		          filename_for_continue_.substr(
		             kSaveDir.length() + 1, filename_for_continue_.length() - kSaveDir.length() -
		                                       kSavegameExtension.length() - 1)) %
		    (boost::format(_("Map: %s")) % g_style_manager->font_style(UI::FontStyle::kTooltip)
		                                      .as_font_tag(newest_singleplayer->mapname))
		       .str() %
		    (boost::format(_("Win Condition: %s")) %
		     g_style_manager->font_style(UI::FontStyle::kTooltip)
		        .as_font_tag(newest_singleplayer->wincondition))
		       .str() %
		    (boost::format(_("Players: %s")) % g_style_manager->font_style(UI::FontStyle::kTooltip)
		                                          .as_font_tag(newest_singleplayer->nrplayers))
		       .str() %
		    (boost::format(_("Gametime: %s")) % g_style_manager->font_style(UI::FontStyle::kTooltip)
		                                           .as_font_tag(newest_singleplayer->gametime))
		       .str() %
		    /** TRANSLATORS: Information about when a game was saved, e.g. 'Saved: Today, 10:30' */
		    (boost::format(_("Saved: %s")) % g_style_manager->font_style(UI::FontStyle::kTooltip)
		                                        .as_font_tag(newest_singleplayer->savedatestring))
		       .str())
		      .str());
	} else {
		continue_lastsave.set_enabled(false);
	}

	layout();
}

void FullscreenMenuMain::clicked_ok() {
	// do nothing
}

void FullscreenMenuMain::layout() {
	FullscreenMenuMainMenu::layout();

	logo_icon_.set_pos(
	   Vector2i((get_w() - logo_icon_.get_w()) / 2, title_y_ + logo_icon_.get_h() / 4));

	const int text_height = 0.5 * version.get_h() + padding_;
	version.set_pos(Vector2i(get_w() - version.get_w(), get_h() - text_height));
	copyright.set_pos(Vector2i(0, get_h() - 2 * text_height));
	gpl.set_pos(Vector2i(0, get_h() - text_height));

	playtutorial.set_desired_size(butw_, buth_);
	singleplayer.set_desired_size(butw_, buth_);
	continue_lastsave.set_desired_size(butw_, buth_);
	multiplayer.set_desired_size(butw_, buth_);
	replay.set_desired_size(butw_, buth_);
	editor.set_desired_size(butw_, buth_);
	options.set_desired_size(butw_, buth_);
	about.set_desired_size(butw_, buth_);
	exit.set_desired_size(butw_, buth_);

	vbox_.set_pos(Vector2i(box_x_, box_y_));
	vbox_.set_inner_spacing(padding_);
	vbox_.set_size(butw_, get_h() - vbox_.get_y() - 5 * padding_);
}
