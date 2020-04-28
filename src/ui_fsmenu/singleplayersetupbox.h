#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <vector>

#include "dropdownsupport.h"
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
	//	UI::Dropdown<std::string>
	//	   type_dropdown_;  /// Select who owns the slot (human, AI, open, closed, shared-in).
	TypeDropdownSupport type_;
	//	UI::Dropdown<std::string> tribes_dropdown_;  /// Select the tribe or shared_in player.
	TribeDropdownSupport tribe_;
	//	UI::Dropdown<uintptr_t>
	//	   init_dropdown_;  /// Select the initialization (Headquarters, Fortified Village etc.)
	InitDropdownSupport init_;
	//	UI::Dropdown<uintptr_t> team_dropdown_;  /// Select the team number
	TeamDropdown teams_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
	void on_gamesettings_updated(const NoteGameSettings& note);
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
