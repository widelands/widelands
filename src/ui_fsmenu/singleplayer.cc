/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_fsmenu/singleplayer.h"

#include "base/i18n.h"
#include "graphic/text_constants.h"

FullscreenMenuSinglePlayer::FullscreenMenuSinglePlayer()
   : FullscreenMenuMainMenu(),

     // Title
     title(this, 0, 0, _("Single Player"), UI::Align::kCenter),

     // Buttons
     new_game(&vbox_, "new_game", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("New Game")),
     campaign(
        &vbox_, "campaigns", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Campaigns")),
     load_game(
        &vbox_, "load_game", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Load Game")),
     back(&vbox_, "back", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Back")) {
	new_game.sigclicked.connect(
	   boost::bind(&FullscreenMenuSinglePlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kNewGame));
	campaign.sigclicked.connect(
	   boost::bind(&FullscreenMenuSinglePlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kCampaign));
	load_game.sigclicked.connect(
	   boost::bind(&FullscreenMenuSinglePlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kLoadGame));
	back.sigclicked.connect(
	   boost::bind(&FullscreenMenuSinglePlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kBack));

	title.set_fontsize(fs_big());

	vbox_.add(&new_game, UI::Box::Resizing::kFullSize);
	vbox_.add(&campaign, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&load_game, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add_inf_space();
	vbox_.add_inf_space();
	vbox_.add(&back, UI::Box::Resizing::kFullSize);

	layout();
}

void FullscreenMenuSinglePlayer::clicked_ok() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNewGame);
}

void FullscreenMenuSinglePlayer::layout() {
	title.set_size(get_w(), title.get_h());
	FullscreenMenuMainMenu::layout();

	title.set_pos(Vector2i(0, title_y_));

	new_game.set_desired_size(butw_, buth_);
	campaign.set_desired_size(butw_, buth_);
	load_game.set_desired_size(butw_, buth_);
	back.set_desired_size(butw_, buth_);

	vbox_.set_pos(Vector2i(box_x_, box_y_));
	vbox_.set_inner_spacing(padding_);
	vbox_.set_size(butw_, get_h() - vbox_.get_y() - 3 * title_y_);
}
