#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <ui_basic/textarea.h>
#include <vector>

#include "dropdownsupport.h"
#include "logic/game_settings.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"

class SinglePlayerActivePlayerGroup : public UI::Box {
public:
	SinglePlayerActivePlayerGroup(UI::Panel* const parent,
	                              int32_t const w,
	                              int32_t const h,
	                              PlayerSlot id,
	                              GameSettingsProvider* const settings);

private:
	PlayerSlot id_;
	GameSettingsProvider* const settings_;
	UI::Button player;
	PlayerTypeDropdownSupport player_type;
	TribeDropdownSupport tribe_;
	StartTypeDropdownSupport start_type;
	TeamDropdown teams_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
	void on_gamesettings_updated(const NoteGameSettings& note);
};

class SinglePlayerActivePlayerSetupBox : public UI::Box {

public:
	SinglePlayerActivePlayerSetupBox(UI::Panel* const parent,
	                                 int32_t const x,
	                                 int32_t const y,
	                                 int32_t const w,
	                                 int32_t const h,
	                                 GameSettingsProvider* const settings,
	                                 uint32_t buth);

private:
	UI::Textarea title_;
	GameSettingsProvider* const settings_;
	std::vector<SinglePlayerActivePlayerGroup*> active_player_groups;  // not owned
};

class SinglePlayerPossiblePlayerGroup : public UI::Box {
public:
	SinglePlayerPossiblePlayerGroup(UI::Panel* const parent,
	                                int32_t const w,
	                                int32_t const h,
	                                PlayerSlot id,
	                                GameSettingsProvider* const settings);

private:
	PlayerSlot id_;
	//	UI::Dropdown<uintptr_t> slot_dropdown_;  /// Select the player slot.
	RoleDropdownSupport role_;
	UI::Textarea name_;  /// Client nick name
	GameSettingsProvider* const settings_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
};

class SinglePlayerPossiblePlayerSetupBox : public UI::Box {

public:
	SinglePlayerPossiblePlayerSetupBox(UI::Panel* const parent,
	                                   int32_t const x,
	                                   int32_t const y,
	                                   int32_t const w,
	                                   int32_t const h,
	                                   GameSettingsProvider* const settings,
	                                   uint32_t buth);
	void update();

private:
	UI::Textarea title_;
	GameSettingsProvider* const settings_;
	std::vector<SinglePlayerPossiblePlayerGroup*> possible_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
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
	SinglePlayerPossiblePlayerSetupBox inactive_players;
	SinglePlayerActivePlayerSetupBox active_players_setup;
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
