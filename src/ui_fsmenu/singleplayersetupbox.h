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

	void update();
	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	PlayerSlot id_;
	GameSettingsProvider* const settings_;
	UI::Button player_;
	PlayerTypeDropdownSupport player_type_;
	TribeDropdownSupport tribe_;
	StartTypeDropdownSupport start_type;
	TeamDropdown teams_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void on_gamesettings_updated(const NoteGameSettings& note);
};

class SinglePlayerActivePlayerSetupBox : public UI::Box {

public:
	SinglePlayerActivePlayerSetupBox(UI::Panel* const parent,
	                                 GameSettingsProvider* const settings,
	                                 uint32_t standard_element_height);

	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	UI::Textarea title_;
	GameSettingsProvider* const settings_;
	std::vector<SinglePlayerActivePlayerGroup*> active_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
	void update();
};

class SinglePlayerPossiblePlayerGroup : public UI::Box {
public:
	SinglePlayerPossiblePlayerGroup(UI::Panel* const parent,
	                                int32_t const w,
	                                int32_t const h,
	                                PlayerSlot id,
	                                GameSettingsProvider* const settings);

	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	PlayerSlot id_;
	RoleDropdownSupport role_;
	UI::Textarea name_;  /// Client nick name
	GameSettingsProvider* const settings_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
};

class SinglePlayerPossiblePlayerSetupBox : public UI::Box {

public:
	SinglePlayerPossiblePlayerSetupBox(UI::Panel* const parent,
	                                   GameSettingsProvider* const settings,
	                                   uint32_t buth);
	void update();
	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	UI::Textarea title_;
	GameSettingsProvider* const settings_;
	std::vector<SinglePlayerPossiblePlayerGroup*> possible_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
};

class SinglePlayerSetupBox : public UI::Box {

public:
	SinglePlayerSetupBox(UI::Panel* const parent,
	                     GameSettingsProvider* const settings,
	                     uint32_t standard_element_height);

	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	SinglePlayerPossiblePlayerSetupBox inactive_players;
	SinglePlayerActivePlayerSetupBox active_players_setup;
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
