#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <memory>
#include <ui_basic/textarea.h>
#include <vector>

#include "logic/game_settings.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_fsmenu/dropdownsupport.h"

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

class SinglePlayerSetupBox : public UI::Box {

public:
	SinglePlayerSetupBox(UI::Panel* const parent,
	                     GameSettingsProvider* const settings,
	                     uint32_t standard_element_height);

	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	SinglePlayerActivePlayerSetupBox active_players_setup;
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
