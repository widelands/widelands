#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <vector>

#include "logic/game_settings.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"

class SinglePlayerPlayerGroup : public UI::Box {
public:
	SinglePlayerPlayerGroup(UI::Panel* const parent,
	                        int32_t const w,
	                        int32_t const h,
	                        PlayerSlot id,
	                        GameSettingsProvider* const settings);

private:
	PlayerSlot id_;
	GameSettingsProvider* const settings_;
	UI::Button player;
	UI::Dropdown<std::string>
	   type_dropdown_;  /// Select who owns the slot (human, AI, open, closed, shared-in).
	UI::Dropdown<std::string> tribes_dropdown_;  /// Select the tribe or shared_in player.
	UI::Dropdown<uintptr_t>
	   init_dropdown_;  /// Select the initialization (Headquarters, Fortified Village etc.)
	UI::Dropdown<uintptr_t> team_dropdown_;  /// Select the team number
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void subscribe_to_game_settings();
	void update();
	// own class?!
	void rebuild_type_dropdown(const GameSettings& settings);
	void fill_type_dropdown(const GameSettings& settings);
	void select_entry(const GameSettings& settings);
};

class SinglePlayerSetupBox : public UI::Box {

public:
	SinglePlayerSetupBox(UI::Panel* const parent,
	                     int32_t const x,
	                     int32_t const y,
	                     int32_t const w,
	                     int32_t const h,
	                     GameSettingsProvider* const settings,
	                     uint32_t buth);

private:
	GameSettingsProvider* const settings_;
	std::vector<SinglePlayerPlayerGroup*> single_player_player_groups;  // not owned
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
