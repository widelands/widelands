#ifndef WL_UI_FSMENU_LAUNCH_SPG2_H
#define WL_UI_FSMENU_LAUNCH_SPG2_H

#include <memory>

#include "graphic/playercolor.h"
#include "logic/game_settings.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/mapdetailsbox.h"

struct PlayerDescriptionGroup;
class FullscreenMenuLaunchSPG2 : public FullscreenMenuLaunchGame
{
public:
	FullscreenMenuLaunchSPG2(GameSettingsProvider*, GameController* = nullptr);
	~FullscreenMenuLaunchSPG2() override;

	void start() override;

protected:
	void clicked_ok() override;
	void clicked_back() override;
	UI::Button& ok_button() override;
	UI::Button& back_button() override;

private:
	void layout() override;
	void win_condition_selected() override;

	UI::Box main_box_;
	UI::Box content_box_;
	UI::Box button_box_;
	UI::Box player_box_;
	UI::Box map_box_;
	MapDetailsBox map_details;

	UI::Textarea players_;
	UI::Textarea map_;
	UI::Checkbox peaceful_own_;
	UI::Dropdown<std::string> win_condition_dropdown_own_;
	UI::Textarea win_condition_type;
	UI::Button ok_own_;
	UI::Button back_own_;
	UI::Textarea title_own_;
	UI::Textarea player_name_;

	bool select_map();
	void update(bool map_was_changed);
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG2_H
