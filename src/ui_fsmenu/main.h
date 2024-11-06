/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_FSMENU_MAIN_H
#define WL_UI_FSMENU_MAIN_H

#include <memory>

#include "scripting/logic.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "ui_fsmenu/menu.h"
#include "wui/plugins.h"
#include "wui/unique_window_handler.h"

namespace Widelands {
class Game;
}  // namespace Widelands

namespace FsMenu {

enum class MenuTarget {
	kBack = static_cast<int>(UI::Panel::Returncodes::kBack),
	kOk = static_cast<int>(UI::Panel::Returncodes::kOk),

	// Options
	kApplyOptions,

	// Main menu
	kTutorial,
	kContinueLastsave,
	kReplay,
	kReplayLast,
	kOptions,
	kAddOns,
	kAbout,
	kExit,

	// Single player
	kNewGame,
	kRandomGame,
	kCampaign,
	kLoadGame,

	// Multiplayer
	kMetaserver,
	kOnlineGameSettings,
	kLan,

	// Editor
	kEditorNew,
	kEditorRandom,
	kEditorContinue,
	kEditorLoad,
};

/**
 * This runs the main menu. There, you can select
 * between different playmodes, exit and so on.
 */
class MainMenu : public UI::Panel {
public:
	explicit MainMenu(bool skip_init = false);

	/** Exception-safe wrapper around Panel::run */
	void main_loop();

	// Internet login stuff
	void show_internet_login(bool modal = false);
	void internet_login_callback();

	void draw(RenderTarget&) override;
	void draw_overlay(RenderTarget&) override;
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;
	bool handle_key(bool, SDL_Keysym) override;
	void become_modal_again(UI::Panel&) override;

	// Set the labels for all buttons etc. This needs to be called after language switching.
	void set_labels();

	void show_messagebox(const std::string& messagetitle, const std::string& errormessage);

	int16_t calc_desired_window_x(UI::Window::WindowLayoutID);
	int16_t calc_desired_window_y(UI::Window::WindowLayoutID);
	int16_t calc_desired_window_width(UI::Window::WindowLayoutID);
	int16_t calc_desired_window_height(UI::Window::WindowLayoutID);

	Widelands::Game* create_safe_game(bool show_error = true);

	UniqueWindowHandler& unique_windows() {
		return unique_windows_;
	}

	LuaFsMenuInterface& lua() {
		return *lua_;
	}

	void
	set_lua_shortcut(const std::string& name, const std::string& action, bool failsafe, bool down) {
		plugin_actions_->set_keyboard_shortcut(name, action, failsafe, down);
	}

	void reinit_plugins();
	void add_plugin_timer(const std::string& action, uint32_t interval, bool failsafe) {
		plugin_actions_->add_plugin_timer(action, interval, failsafe);
	}

	// Signal ending immediately from any phase
	void abort_splashscreen();

protected:
	void update_template() override;

private:
	void layout() override;
	void think() override;

	UniqueWindowHandler unique_windows_;
	std::unique_ptr<LuaFsMenuInterface> lua_;
	std::unique_ptr<PluginActions> plugin_actions_;

	// Called only from splash screen phase to signal start of fading
	void end_splashscreen();

	Recti box_rect_;
	uint32_t butw_, buth_;
	uint32_t padding_;

	UI::Box vbox1_, vbox2_;

	UI::Dropdown<MenuTarget> singleplayer_;
	UI::Dropdown<MenuTarget> multiplayer_;
	UI::Dropdown<MenuTarget> replay_;
	UI::Dropdown<MenuTarget> editor_;
	UI::Button addons_;
	UI::Button options_;
	UI::Button about_;
	UI::Button exit_;
	UI::Textarea clock_;
	UI::Textarea version_;
	UI::Textarea copyright_;

	std::string filename_for_continue_playing_;
	std::string filename_for_continue_editing_;
	std::string filename_for_last_replay_;

	const Image* title_image_;

	uint32_t init_time_;

	enum class SplashState { kSplash, kSplashFadeOut, kMenuFadeIn, kDone };
	SplashState splash_state_{SplashState::kDone};

	std::vector<std::string> images_;
	uint32_t last_image_exchange_time_{0U};
	size_t draw_image_{0U};
	size_t last_image_{0U};
	Rectf title_pos();
	float calc_opacity(uint32_t time) const;

	bool visible_{true};
	void set_button_visibility(bool);
	bool system_clock_{true};

	void action(MenuTarget);
	void exit(bool force = false);
	bool check_desyncing_addon();

	MenuCapsule menu_capsule_;
	UI::UniqueWindow::Registry r_login_, r_about_, r_addons_;

	void internet_login(bool launch_metaserver);

	// Values from internet login window
	std::string nickname_;
	std::string password_;
	bool auto_log_{false};
	bool register_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_MAIN_H
