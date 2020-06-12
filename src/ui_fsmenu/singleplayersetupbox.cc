#include "singleplayersetupbox.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/map_loader.h"

SinglePlayerActivePlayerSetupBox::SinglePlayerActivePlayerSetupBox(
   UI::Panel* const parent, GameSettingsProvider* const settings, uint32_t standard_element_height)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     title_(this,
            0,
            0,
            0,
            0,
            _("Players"),
            UI::Align::kRight,
            g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
     settings_(settings) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	active_player_groups.resize(kMaxPlayers);
	for (PlayerSlot i = 0; i < active_player_groups.size(); ++i) {
		active_player_groups.at(i) =
		   new SinglePlayerActivePlayerGroup(this, 0, standard_element_height, i, settings);
		add(active_player_groups.at(i));
	}

	subscriber_ =
	   Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings&) { update(); });
}
void SinglePlayerActivePlayerSetupBox::update() {

	for (auto& active_player_group : active_player_groups) {
		active_player_group->update();
	}
}

void SinglePlayerActivePlayerSetupBox::force_new_dimensions(float scale,
                                                            uint32_t standard_element_height) {
	title_.set_font_scale(scale);
	for (auto& active_player_group : active_player_groups) {
		active_player_group->force_new_dimensions(scale, standard_element_height);
	}
}

SinglePlayerActivePlayerGroup::SinglePlayerActivePlayerGroup(UI::Panel* const parent,
                                                             int32_t const,
                                                             int32_t const h,
                                                             PlayerSlot id,
                                                             GameSettingsProvider* const settings)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal),
     id_(id),
     settings_(settings),
     player_(this,
             "player",
             0,
             0,
             h,
             h,
             UI::ButtonStyle::kFsMenuSecondary,
             playercolor_image(id, "images/players/player_position_menu.png"),
             (boost::format(_("Player %u")) % static_cast<unsigned int>(id_ + 1)).str(),
             UI::Button::VisualState::kFlat),
     player_type_(this,
                  (boost::format("dropdown_type%d") % static_cast<unsigned int>(id)).str(),
                  0,
                  0,
                  h,
                  h,
                  settings,
                  id),
     tribe_(this,
            (boost::format("dropdown_type%d") % static_cast<unsigned int>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id),
     start_type(this,
                (boost::format("dropdown_init%d") % static_cast<unsigned int>(id)).str(),
                0,
                0,
                8 * h,
                h,
                settings,
                id),
     teams_(this,
            (boost::format("dropdown_team%d") % static_cast<unsigned int>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id) {

	add_space(0);
	add(&player_);
	add(player_type_.get_dropdown());
	add(tribe_.get_dropdown());
	add(start_type.get_dropdown());
	add(teams_.get_dropdown());
	add_space(0);

	player_.set_disable_style(UI::ButtonDisableStyle::kFlat);
	player_.set_enabled(false);
}
void SinglePlayerActivePlayerGroup::force_new_dimensions(float, uint32_t standard_element_height) {
	player_.set_desired_size(standard_element_height, standard_element_height);
	player_type_.set_desired_size(standard_element_height, standard_element_height);
	tribe_.set_desired_size(standard_element_height, standard_element_height);
	start_type.set_desired_size(8 * standard_element_height, standard_element_height);
	teams_.set_desired_size(standard_element_height, standard_element_height);
}

void SinglePlayerActivePlayerGroup::update() {
	const GameSettings& settings = settings_->settings();
	if (id_ >= settings.players.size()) {
		set_visible(false);
		return;
	}

	player_type_.rebuild();
	set_visible(true);

	const PlayerSettings& player_setting = settings.players[id_];
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
	}
}

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           GameSettingsProvider* const settings,
                                           uint32_t standard_element_height)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal),
     active_players_setup(this, settings, standard_element_height) {
	add(&active_players_setup);
}

void SinglePlayerSetupBox::force_new_dimensions(float scale, uint32_t standard_element_height) {
	active_players_setup.force_new_dimensions(scale, standard_element_height);
}
