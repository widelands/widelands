#include "dropdownsupport.h"

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "logic/game_settings.h"

#define AI_NAME_PREFIX "ai" AI_NAME_SEPARATOR

TribeDropdownSupport::TribeDropdownSupport(UI::Panel* parent,
                                           const std::string& name,
                                           int32_t x,
                                           int32_t y,
                                           uint32_t w,
                                           uint32_t max_list_items,
                                           int button_dimension,
                                           const std::string& label,
                                           GameSettingsProvider* const settings,
                                           PlayerSlot id)
   : DropDownSupport<std::string>(parent,
                                  name,
                                  x,
                                  y,
                                  w,
                                  max_list_items,
                                  button_dimension,
                                  label,
                                  UI::DropdownType::kPictorial,
                                  UI::PanelStyle::kFsMenu,
                                  UI::ButtonStyle::kFsMenuSecondary,
                                  settings,
                                  id) {
}
void TribeDropdownSupport::rebuild() {
}
void TribeDropdownSupport::fill() {
}
void TribeDropdownSupport::select_entry() {
}

TypeDropdownSupport::TypeDropdownSupport(UI::Panel* parent,
                                         const std::string& name,
                                         int32_t x,
                                         int32_t y,
                                         uint32_t w,
                                         uint32_t max_list_items,
                                         int button_dimension,
                                         const std::string& label,
                                         GameSettingsProvider* const settings,
                                         PlayerSlot id)
   : DropDownSupport<std::string>(parent,
                                  name,
                                  x,
                                  y,
                                  w,
                                  max_list_items,
                                  button_dimension,
                                  label,
                                  UI::DropdownType::kPictorial,
                                  UI::PanelStyle::kFsMenu,
                                  UI::ButtonStyle::kFsMenuSecondary,
                                  settings,
                                  id) {
}
void TypeDropdownSupport::rebuild() {
	/// Rebuild the type dropdown from the server settings. This will keep the host and client UIs in
	/// sync.

	//	if (type_selection_locked_) {
	//		return;
	//	}
	fill();
	dropdown_.set_enabled(settings_->can_change_player_state(id_));
	select_entry();
}
void TypeDropdownSupport::fill() {
	const GameSettings& settings = settings_->settings();
	dropdown_.clear();
	// AIs
	for (const auto* impl : ComputerPlayer::get_implementations()) {
		dropdown_.add(_(impl->descname), (boost::format(AI_NAME_PREFIX "%s") % impl->name).str(),
		              g_gr->images().get(impl->icon_filename), false, _(impl->descname));
	}
	/** TRANSLATORS: This is the name of an AI used in the game setup screens */
	dropdown_.add(_("Random AI"), AI_NAME_PREFIX "random",
	              g_gr->images().get("images/ai/ai_random.png"), false, _("Random AI"));

	// Slot state. Only add shared_in if there are viable slots
	if (settings.is_shared_usable(id_, settings.find_shared(id_))) {
		dropdown_.add(_("Shared in"), "shared_in",
		              g_gr->images().get("images/ui_fsmenu/shared_in.png"), false, _("Shared in"));
	}

	// Do not close a player in savegames or scenarios
	if (!settings.uncloseable(id_)) {
		dropdown_.add(
		   _("Closed"), "closed", g_gr->images().get("images/ui_basic/stop.png"), false, _("Closed"));
	}

	dropdown_.add(
	   _("Open"), "open", g_gr->images().get("images/ui_basic/continue.png"), false, _("Open"));
}

void TypeDropdownSupport::select_entry() {
	const GameSettings& settings = settings_->settings();
	// Now select the entry according to server settings
	const PlayerSettings& player_setting = settings.players[id_];
	if (player_setting.state == PlayerSettings::State::kHuman) {
		dropdown_.set_image(g_gr->images().get("images/wui/stats/genstats_nrworkers.png"));
		dropdown_.set_tooltip((boost::format(_("%1%: %2%")) % _("Type") % _("Human")).str());
	} else if (player_setting.state == PlayerSettings::State::kClosed) {
		dropdown_.select("closed");
	} else if (player_setting.state == PlayerSettings::State::kOpen) {
		dropdown_.select("open");
	} else if (player_setting.state == PlayerSettings::State::kShared) {
		dropdown_.select("shared_in");
	} else {
		if (player_setting.state == PlayerSettings::State::kComputer) {
			if (player_setting.ai.empty()) {
				dropdown_.set_errored(_("No AI"));
			} else {
				if (player_setting.random_ai) {
					dropdown_.select(AI_NAME_PREFIX "random");
				} else {
					const ComputerPlayer::Implementation* impl =
					   ComputerPlayer::get_implementation(player_setting.ai);
					dropdown_.select((boost::format(AI_NAME_PREFIX "%s") % impl->name).str());
				}
			}
		}
	}
}
