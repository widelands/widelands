#include "singleplayersetupbox.h"

#include <memory>

#include <boost/algorithm/string.hpp>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

#define AI_NAME_PREFIX "ai" AI_NAME_SEPARATOR

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           int32_t const x,
                                           int32_t const y,
                                           int32_t const w,
                                           int32_t const h,
                                           GameSettingsProvider* const settings,
                                           uint32_t buth)
   : UI::Box(parent, x, y, UI::Box::Vertical, w, h), settings_(settings) {

	single_player_player_groups.resize(kMaxPlayers);
	for (PlayerSlot i = 0; i < single_player_player_groups.size(); ++i) {
		single_player_player_groups.at(i) =
		   new SinglePlayerPlayerGroup(this, 0 /*get_w() - UI::Scrollbar::kSize*/, buth, i, settings);
		add(single_player_player_groups.at(i));
	}
}

SinglePlayerPlayerGroup::SinglePlayerPlayerGroup(UI::Panel* const parent,
                                                 int32_t const w,
                                                 int32_t const h,
                                                 PlayerSlot id,
                                                 GameSettingsProvider* const settings)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
     id_(id),
     settings_(settings),
     player(this,
            "player",
            0,
            0,
            h,
            h,
            UI::ButtonStyle::kFsMenuSecondary,
            playercolor_image(id, "images/players/player_position_menu.png"),
            (boost::format(_("Player %u")) % static_cast<unsigned int>(id_ + 1)).str(),
            UI::Button::VisualState::kFlat),
     type_dropdown_(this,
                    (boost::format("dropdown_type%d") % static_cast<unsigned int>(id)).str(),
                    0,
                    0,
                    50,
                    16,
                    h,
                    _("Type"),
                    UI::DropdownType::kPictorial,
                    UI::PanelStyle::kFsMenu,
                    UI::ButtonStyle::kFsMenuSecondary),
     tribes_dropdown_(this,
                      (boost::format("dropdown_tribes%d") % static_cast<unsigned int>(id)).str(),
                      0,
                      0,
                      50,
                      16,
                      h,
                      _("Tribe"),
                      UI::DropdownType::kPictorial,
                      UI::PanelStyle::kFsMenu,
                      UI::ButtonStyle::kFsMenuSecondary),
     init_dropdown_(this,
                    (boost::format("dropdown_init%d") % static_cast<unsigned int>(id)).str(),
                    0,
                    0,
                    0,
                    16,
                    h,
                    "Barbaren",
                    UI::DropdownType::kTextualNarrow,
                    UI::PanelStyle::kFsMenu,
                    UI::ButtonStyle::kFsMenuSecondary),
     team_dropdown_(this,
                    (boost::format("dropdown_team%d") % static_cast<unsigned int>(id)).str(),
                    0,
                    0,
                    h,
                    16,
                    h,
                    _("Team"),
                    UI::DropdownType::kPictorial,
                    UI::PanelStyle::kFsMenu,
                    UI::ButtonStyle::kFsMenuSecondary) {

	add_space(0);
	add(&player);
	add(&type_dropdown_);
	add(&tribes_dropdown_);
	add(&init_dropdown_, UI::Box::Resizing::kFillSpace);
	add(&team_dropdown_);
	add_space(0);

	subscribe_to_game_settings();
}

void SinglePlayerPlayerGroup::subscribe_to_game_settings() {
	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& note) {
		log("going to update?\n");
		if (settings_->settings().players.empty()) {
			// No map/savegame yet
			return;
		}
		log("yes\n");
		switch (note.action) {
		case NoteGameSettings::Action::kMap:
			// We don't care about map updates, since we receive enough notifications for the
			// slots.
			log("map\n");
			update();
			break;
		case NoteGameSettings::Action::kUser:
			// We might have moved away from a slot, so we need to update the previous slot too.
			// Since we can't track the slots here, we just update everything.
			update();
			break;
		default:
			if (id_ == note.position ||
			    (id_ < settings_->settings().players.size() &&
			     settings_->settings().players.at(id_).state == PlayerSettings::State::kShared)) {
				update();
			}
		}
	});
}

/// Refresh all user interfaces
void SinglePlayerPlayerGroup::update() {
	const GameSettings& settings = settings_->settings();
	if (id_ >= settings.players.size()) {
		set_visible(false);
		return;
	}
	log("players available...\n");
	const PlayerSettings& player_setting = settings.players[id_];
	rebuild_type_dropdown(settings);
	set_visible(true);

	if (player_setting.state == PlayerSettings::State::kClosed ||
	    player_setting.state == PlayerSettings::State::kOpen) {
		team_dropdown_.set_visible(false);
		team_dropdown_.set_enabled(false);
		tribes_dropdown_.set_visible(false);
		tribes_dropdown_.set_enabled(false);
		init_dropdown_.set_visible(false);
		init_dropdown_.set_enabled(false);
	} else {  // kHuman, kShared, kComputer
		       //		rebuild_tribes_dropdown(settings);
		       //		rebuild_init_dropdown(settings);
		       //		rebuild_team_dropdown(settings);
	}

	// Trigger update for the other players for shared_in mode when slots open and close
	//	if (last_state_ != player_setting.state) {
	//		last_state_ = player_setting.state;
	//		for (PlayerSlot slot = 0; slot < settings_->settings().players.size(); ++slot) {
	//			if (slot != id_) {
	//				n->set_player_state(slot, settings.players[slot].state);
	//			}
	//		}
	//	}
}

/// Rebuild the type dropdown from the server settings. This will keep the host and client UIs in
/// sync.
void SinglePlayerPlayerGroup::rebuild_type_dropdown(const GameSettings& settings) {
	//	if (type_selection_locked_) {
	//		return;
	//	}
	fill_type_dropdown(settings);
	type_dropdown_.set_enabled(settings_->can_change_player_state(id_));
	select_entry(settings);
}

void SinglePlayerPlayerGroup::fill_type_dropdown(const GameSettings& settings) {
	type_dropdown_.clear();
	// AIs
	for (const auto* impl : ComputerPlayer::get_implementations()) {
		type_dropdown_.add(_(impl->descname), (boost::format(AI_NAME_PREFIX "%s") % impl->name).str(),
		                   g_gr->images().get(impl->icon_filename), false, _(impl->descname));
	}
	/** TRANSLATORS: This is the name of an AI used in the game setup screens */
	type_dropdown_.add(_("Random AI"), AI_NAME_PREFIX "random",
	                   g_gr->images().get("images/ai/ai_random.png"), false, _("Random AI"));

	// Slot state. Only add shared_in if there are viable slots
	if (settings.is_shared_usable(id_, settings.find_shared(id_))) {
		type_dropdown_.add(_("Shared in"), "shared_in",
		                   g_gr->images().get("images/ui_fsmenu/shared_in.png"), false,
		                   _("Shared in"));
	}

	// Do not close a player in savegames or scenarios
	if (!settings.uncloseable(id_)) {
		type_dropdown_.add(
		   _("Closed"), "closed", g_gr->images().get("images/ui_basic/stop.png"), false, _("Closed"));
	}

	type_dropdown_.add(
	   _("Open"), "open", g_gr->images().get("images/ui_basic/continue.png"), false, _("Open"));
}

void SinglePlayerPlayerGroup::select_entry(const GameSettings& settings) {
	// Now select the entry according to server settings
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kHuman) {
		type_dropdown_.set_image(g_gr->images().get("images/wui/stats/genstats_nrworkers.png"));
		type_dropdown_.set_tooltip((boost::format(_("%1%: %2%")) % _("Type") % _("Human")).str());
	} else if (player_setting.state == PlayerSettings::State::kClosed) {
		type_dropdown_.select("closed");
	} else if (player_setting.state == PlayerSettings::State::kOpen) {
		type_dropdown_.select("open");
	} else if (player_setting.state == PlayerSettings::State::kShared) {
		type_dropdown_.select("shared_in");
	} else {
		if (player_setting.state == PlayerSettings::State::kComputer) {
			if (player_setting.ai.empty()) {
				type_dropdown_.set_errored(_("No AI"));
			} else {
				if (player_setting.random_ai) {
					type_dropdown_.select(AI_NAME_PREFIX "random");
				} else {
					const ComputerPlayer::Implementation* impl =
					   ComputerPlayer::get_implementation(player_setting.ai);
					type_dropdown_.select((boost::format(AI_NAME_PREFIX "%s") % impl->name).str());
				}
			}
		}
	}
}
