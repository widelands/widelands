/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "logic/map_revision.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "ui_fsmenu/menu.h"
#include "wui/mapdata.h"

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

	using MapEntry = std::pair<MapData, Widelands::MapVersion>;
	static void find_maps(const std::string& directory, std::vector<MapEntry>& results);

	Widelands::Game* create_safe_game(bool show_error = true);

protected:
	void update_template() override;

private:
	void layout() override;

	Recti box_rect_;
	uint32_t butw_, buth_;
	uint32_t padding_;

	UI::Box vbox1_, vbox2_;

	UI::Dropdown<MenuTarget> singleplayer_;
	UI::Dropdown<MenuTarget> multiplayer_;
	UI::Button replay_;
	UI::Dropdown<MenuTarget> editor_;
	UI::Button addons_;
	UI::Button options_;
	UI::Button about_;
	UI::Button exit_;
	UI::Textarea version_;
	UI::Textarea copyright_;

	std::string filename_for_continue_playing_, filename_for_continue_editing_;

	const Image* splashscreen_;
	const Image* title_image_;

	uint32_t init_time_;

	std::vector<std::string> images_;
	uint32_t last_image_exchange_time_{0U};
	size_t draw_image_{0U};
	size_t last_image_{0U};
	Rectf image_pos(const Image&, bool crop = true);
	Rectf title_pos();
	float calc_opacity(uint32_t time) const;

	bool visible_{true};
	void set_button_visibility(bool);

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
