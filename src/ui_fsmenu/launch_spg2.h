#ifndef WL_UI_FSMENU_LAUNCH_SPG2_H
#define WL_UI_FSMENU_LAUNCH_SPG2_H

#include <memory>

#include "graphic/playercolor.h"
#include "logic/game_settings.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/singleplayersetupbox.h"

class FullscreenMenuLaunchSPG2 : public FullscreenMenuLaunchGame {
public:
	FullscreenMenuLaunchSPG2(GameSettingsProvider*, GameController* = nullptr);
	~FullscreenMenuLaunchSPG2() override;

	void start() override;

protected:
	void clicked_ok() override;
	void clicked_back() override;
	bool clicked_select_map() override;

private:
	void win_condition_selected() override;
	void layout() override;

	SinglePlayerSetupBox player_setup;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	bool is_scenario_;

	void update();
	void set_player_names_and_tribes(Widelands::Map& map);
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG2_H
