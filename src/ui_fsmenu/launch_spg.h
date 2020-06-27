#ifndef WL_UI_FSMENU_LAUNCH_SPG_H
#define WL_UI_FSMENU_LAUNCH_SPG_H

#include <memory>

#include "logic/game_settings.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/singleplayersetupbox.h"

class FullscreenMenuLaunchSPG : public FullscreenMenuLaunchGame {
public:
	FullscreenMenuLaunchSPG(GameSettingsProvider*, GameController* = nullptr);
	~FullscreenMenuLaunchSPG() override;

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

	void update();
	void enforce_player_names_and_tribes(Widelands::Map& map);
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
