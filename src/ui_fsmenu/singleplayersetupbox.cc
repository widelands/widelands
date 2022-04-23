/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_fsmenu/singleplayersetupbox.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_basic/color_chooser.h"

namespace FsMenu {

SinglePlayerActivePlayerGroup::SinglePlayerActivePlayerGroup(UI::Panel* const parent,
                                                             LaunchGame& lg,
                                                             int32_t const /* w */,
                                                             int32_t const h,
                                                             PlayerSlot id,
                                                             GameSettingsProvider* const settings)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     id_(id),
     settings_(settings),
     player_(this,
             "player",
             0,
             0,
             h,
             h,
             UI::ButtonStyle::kFsMenuSecondary,
             playercolor_image(),
             format(_("Player %u"), static_cast<unsigned>(id_ + 1))),
     player_type_(
        this, lg, format("dropdown_type%d", static_cast<unsigned>(id)), 0, 0, h, h, settings, id),
     tribe_(
        this, lg, format("dropdown_tribe%d", static_cast<unsigned>(id)), 0, 0, h, h, settings, id),
     start_type(this,
                lg,
                format("dropdown_init%d", static_cast<unsigned>(id)),
                0,
                0,
                8 * h,
                h,
                settings,
                id),
     teams_(
        this, lg, format("dropdown_team%d", static_cast<unsigned>(id)), 0, 0, h, h, settings, id) {

	add(&player_);
	add(player_type_.get_dropdown());
	add(tribe_.get_dropdown());
	add(start_type.get_dropdown(), UI::Box::Resizing::kExpandBoth);
	add(teams_.get_dropdown());

	player_.set_disable_style(UI::ButtonDisableStyle::kFlat);
	player_.sigclicked.connect([this]() { choose_color(); });

	update();
}
void SinglePlayerActivePlayerGroup::force_new_dimensions(uint32_t standard_element_height) {
	player_.set_desired_size(standard_element_height, standard_element_height);
	player_type_.set_desired_size(standard_element_height, standard_element_height);
	tribe_.set_desired_size(standard_element_height, standard_element_height);
	start_type.set_desired_size(8 * standard_element_height, standard_element_height);
	teams_.set_desired_size(standard_element_height, standard_element_height);
}

void SinglePlayerActivePlayerGroup::choose_color() {
	Panel* p = this;
	while (p->get_parent() != nullptr) {
		p = p->get_parent();
	}

	UI::ColorChooser c(
	   p, UI::WindowStyle::kFsMenu, settings_->settings().players[id_].color, &kPlayerColors[id_]);
	if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		settings_->set_player_color(id_, c.get_color());
		update();
	}
}

inline const Image* SinglePlayerActivePlayerGroup::playercolor_image() {
	return ::playercolor_image(
	   settings_->settings().players[id_].color, "images/players/player_position_menu.png");
}

void SinglePlayerActivePlayerGroup::update() {
	const GameSettings& settings = settings_->settings();

	player_type_.rebuild();

	const PlayerSettings& player_setting = settings.players[id_];
	player_.set_pic(playercolor_image());
	player_.set_tooltip(player_setting.name);
	player_.set_enabled(player_setting.state != PlayerSettings::State::kClosed &&
	                    !settings.scenario);
	if (player_setting.state == PlayerSettings::State::kClosed ||
	    player_setting.state == PlayerSettings::State::kOpen) {

		teams_.set_visible(false);
		teams_.set_enabled(false);

		tribe_.set_visible(false);
		tribe_.set_enabled(false);

		start_type.set_visible(false);
		start_type.set_enabled(false);
	} else {  // kHuman, kShared, kComputer
		tribe_.rebuild();
		start_type.rebuild();
		teams_.rebuild();
		tribe_.set_enabled(!settings.scenario);
	}
}

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           LaunchGame& lg,
                                           GameSettingsProvider* const settings,
                                           uint32_t standard_element_height)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     launch_game_(lg),
     settings_(settings),
     standard_height_(standard_element_height),
     scrollable_playerbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     title_(this,
            UI::PanelStyle::kFsMenu,
            UI::FontStyle::kFsGameSetupHeadings,
            0,
            0,
            0,
            0,
            _("Players"),
            UI::Align::kRight) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add_space(title_.get_h());
	add(&scrollable_playerbox_, Resizing::kExpandBoth);
	scrollable_playerbox_.set_scrolling(true);
	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& n) {
		if (n.action == NoteGameSettings::Action::kMap) {
			reset();
		}
		update();
	});
}

void SinglePlayerSetupBox::update() {

	const GameSettings& settings = settings_->settings();
	const size_t number_of_players = settings.players.size();

	assert(!settings.tribes.empty());

	for (PlayerSlot i = active_player_groups_.size(); i < number_of_players; ++i) {
		active_player_groups_.push_back(new SinglePlayerActivePlayerGroup(
		   &scrollable_playerbox_, launch_game_, 0, standard_height_, i, settings_));
		scrollable_playerbox_.add(active_player_groups_.at(i), Resizing::kFullSize);
	}

	for (auto& p : active_player_groups_) {
		p->update();
	}

	initialization_complete();
}

void SinglePlayerSetupBox::force_new_dimensions(uint32_t standard_element_height,
                                                int32_t max_size) {
	standard_height_ = standard_element_height;
	for (auto& active_player_group : active_player_groups_) {
		active_player_group->force_new_dimensions(standard_element_height);
	}
	scrollable_playerbox_.set_max_size(get_inner_w(), max_size - 2 * title_.get_h());
}
void SinglePlayerSetupBox::reset() {
	for (auto& p : active_player_groups_) {
		p->die();
	}
	active_player_groups_.clear();
}
}  // namespace FsMenu
