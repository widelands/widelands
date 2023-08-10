/*
 * Copyright (C) 2007-2023 by the Widelands Development Team
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

#include "wui/interactive_gamebase.h"

#include <cstdlib>
#include <memory>

#include "base/macros.h"
#include "base/multithreading.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "network/gamehost.h"
#include "ui_basic/toolbar_setup.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_client_disconnected.h"
#include "wui/game_diplomacy_menu.h"
#include "wui/game_exit_confirm_box.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_sound_menu.h"
#include "wui/game_summary.h"
#include "wui/info_panel.h"
#include "wui/interactive_player.h"
#include "wui/toolbar.h"
#include "wui/watchwindow.h"

namespace {

std::string speed_string(int const speed) {
	if (speed != 0) {
		/** TRANSLATORS: This is a game speed value */
		return format(_("%1$u.%2$u×"), (speed / 1000), (speed / 100 % 10));
	}
	return _("PAUSE");
}

constexpr uint8_t kSpeedSlow = 250;
constexpr uint16_t kSpeedDefault = 1000;
constexpr uint16_t kSpeedFast = 10000;
}  // namespace

InteractiveGameBase::InteractiveGameBase(Widelands::Game& g,
                                         Section& global_s,
                                         bool const multiplayer,
                                         ChatProvider* chat_provider)
   : InteractiveBase(g, global_s, chat_provider),
     multiplayer_(multiplayer),
     showhidemenu_(toolbar(),
                   "dropdown_menu_showhide",
                   0,
                   0,
                   UI::main_toolbar_button_size(),
                   10,
                   UI::main_toolbar_button_size(),
                   /** TRANSLATORS: Title for a menu button in the game. This menu will show/hide
                      building spaces, census, status, etc. */
                   _("Show / Hide"),
                   UI::DropdownType::kPictorialMenu,
                   UI::PanelStyle::kWui,
                   UI::ButtonStyle::kWuiPrimary,
                   [this](ShowHideEntry t) { showhide_menu_selected(t); }),
     grid_marker_pic_(g_image_cache->get("images/wui/overlays/grid_marker.png")),
     special_coords_marker_pic_(g_image_cache->get("images/wui/overlays/special.png")),
     pause_on_inactivity_(get_config_int("pause_game_on_inactivity", 0) * 60 * 1000),
     can_restart_(g.is_replay() || !g.list_of_scenarios().empty()),
     mainmenu_(toolbar(),
               "dropdown_menu_main",
               0,
               0,
               UI::main_toolbar_button_size(),
               10,
               UI::main_toolbar_button_size(),
               as_tooltip_text_with_hotkey(
                  /** TRANSLATORS: Title for the main menu button in the game */
                  _("Main Menu"),
                  pgettext("hotkey", "Esc"),
                  UI::PanelStyle::kWui),
               UI::DropdownType::kPictorialMenu,
               UI::PanelStyle::kWui,
               UI::ButtonStyle::kWuiPrimary,
               [this](MainMenuEntry t) { main_menu_selected(t); }),
     gamespeedmenu_(toolbar(),
                    "dropdown_menu_gamespeed",
                    0,
                    0,
                    UI::main_toolbar_button_size(),
                    10,
                    UI::main_toolbar_button_size(),
                    /** TRANSLATORS: Title for a menu button in the game. This menu will show
                       options o increase/decrease the gamespeed, and to pause the game */
                    _("Game Speed"),
                    UI::DropdownType::kPictorialMenu,
                    UI::PanelStyle::kWui,
                    UI::ButtonStyle::kWuiPrimary,
                    [this](GameSpeedEntry t) { gamespeed_menu_selected(t); }) {
	if (chat_provider_ != nullptr) {
		chat_overlay()->set_chat_provider(*chat_provider_);
	}
}

void InteractiveGameBase::add_main_menu() {
	mainmenu_.set_image(g_image_cache->get("images/wui/menus/main_menu.png"));
	toolbar()->add(&mainmenu_);
	mainmenu_.selected.connect([this] { main_menu_selected(mainmenu_.get_selected()); });
	rebuild_main_menu();
}

void InteractiveGameBase::rebuild_main_menu() {
	mainmenu_.clear();
#ifndef NDEBUG  //  only in debug builds
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Script Console"), MainMenuEntry::kScriptConsole,
	              g_image_cache->get("images/wui/menus/lua.png"), false,
	              /** TRANSLATORS: Tooltip for Script Console in the game's main menu */
	              "", pgettext("hotkey", "Ctrl+Shift+Space"));
#endif

	menu_windows_.sound_options.open_window = [this] {
		new GameOptionsSoundMenu(*this, menu_windows_.sound_options);
	};
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Sound Options"), MainMenuEntry::kOptions,
	              g_image_cache->get("images/wui/menus/options.png"), false,
	              /** TRANSLATORS: Tooltip for Sound Options in the game's main menu */
	              _("Set sound effect and music options"),
	              shortcut_string_for(KeyboardShortcut::kInGameSoundOptions, false));

	menu_windows_.savegame.open_window = [this] {
		new GameMainMenuSaveGame(*this, menu_windows_.savegame, GameMainMenuSaveGame::Type::kSave);
	};
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Save Game"), MainMenuEntry::kSaveMap,
	              g_image_cache->get("images/wui/menus/save_game.png"), false, "",
	              shortcut_string_for(KeyboardShortcut::kCommonSave, false));

	if (game().is_replay()) {
		menu_windows_.loadgame.open_window = [this] {
			new GameMainMenuSaveGame(
			   *this, menu_windows_.loadgame, GameMainMenuSaveGame::Type::kLoadReplay);
		};
		/** TRANSLATORS: An entry in the game's main menu */
		mainmenu_.add(_("Load Replay"), MainMenuEntry::kLoadMap,
		              g_image_cache->get("images/wui/menus/load_game.png"), false, "",
		              shortcut_string_for(KeyboardShortcut::kCommonLoad, false));

		/** TRANSLATORS: An entry in the game's main menu */
		mainmenu_.add(_("Restart Replay"), MainMenuEntry::kRestartScenario,
		              g_image_cache->get("images/wui/menus/restart_scenario.png"), false, "",
		              shortcut_string_for(KeyboardShortcut::kInGameRestart, false));
	} else if (!is_multiplayer()) {
		menu_windows_.loadgame.open_window = [this] {
			new GameMainMenuSaveGame(
			   *this, menu_windows_.loadgame, GameMainMenuSaveGame::Type::kLoadSavegame);
		};
		/** TRANSLATORS: An entry in the game's main menu */
		mainmenu_.add(_("Load Game"), MainMenuEntry::kLoadMap,
		              g_image_cache->get("images/wui/menus/load_game.png"), false, "",
		              shortcut_string_for(KeyboardShortcut::kCommonLoad, false));
	}

	if (!game().list_of_scenarios().empty()) {
		/** TRANSLATORS: An entry in the game's main menu */
		mainmenu_.add(_("Restart Scenario"), MainMenuEntry::kRestartScenario,
		              g_image_cache->get("images/wui/menus/restart_scenario.png"), false, "",
		              shortcut_string_for(KeyboardShortcut::kInGameRestart, false));
	}

	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Exit Game"), MainMenuEntry::kExitGame,
	              g_image_cache->get("images/wui/menus/exit.png"), false, "",
	              shortcut_string_for(KeyboardShortcut::kCommonExit, false));
}

void InteractiveGameBase::main_menu_selected(MainMenuEntry entry) {
	switch (entry) {
#ifndef NDEBUG  //  only in debug builds
	case MainMenuEntry::kScriptConsole: {
		GameChatMenu::create_script_console(
		   this, color_functor(), debugconsole_, *DebugConsole::get_chat_provider());
	} break;
#endif
	case MainMenuEntry::kOptions: {
		menu_windows_.sound_options.toggle();
	} break;
	case MainMenuEntry::kSaveMap: {
		menu_windows_.savegame.toggle();
	} break;
	case MainMenuEntry::kRestartScenario: {
		handle_restart((SDL_GetModState() & KMOD_CTRL) != 0);
	} break;
	case MainMenuEntry::kLoadMap:
		if (!is_multiplayer()) {
			menu_windows_.loadgame.toggle();
		}
		break;
	case MainMenuEntry::kExitGame: {
		if ((SDL_GetModState() & KMOD_CTRL) != 0) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
		} else {
			new GameExitConfirmBox(*this, *this);
		}
	} break;
	}
}

void InteractiveGameBase::handle_restart(const bool force) {
	const bool r = game().is_replay();
	const std::string& next = r ? game().replay_filename() : game().list_of_scenarios().front();
	if (force) {
		game().set_next_game_to_load(next);
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		GameExitConfirmBox* gecb =
		   new GameExitConfirmBox(*this, *this, r ? _("Restart Replay") : _("Restart Scenario"),
		                          r ? _("Are you sure you wish to restart this replay?") :
                                    _("Are you sure you wish to restart this scenario?"));
		gecb->ok.connect([this, next] { game().set_next_game_to_load(next); });
	}
}

void InteractiveGameBase::add_diplomacy_menu() {
	add_toolbar_button(
	   "wui/menus/diplomacy", "diplomacy",
	   as_tooltip_text_with_hotkey(_("Diplomacy"),
	                               shortcut_string_for(KeyboardShortcut::kInGameDiplomacy, true),
	                               UI::PanelStyle::kWui),
	   &diplomacy_, true);
	diplomacy_.open_window = [this] { new GameDiplomacyMenu(*this, diplomacy_); };
}

void InteractiveGameBase::add_showhide_menu() {
	showhidemenu_.set_image(g_image_cache->get("images/wui/menus/showhide.png"));
	toolbar()->add(&showhidemenu_);

	rebuild_showhide_menu();
	showhidemenu_.selected.connect([this] { showhide_menu_selected(showhidemenu_.get_selected()); });
}

void InteractiveGameBase::rebuild_showhide_menu() {
	const ShowHideEntry last_selection =
	   showhidemenu_.has_selection() ? showhidemenu_.get_selected() : ShowHideEntry::kBuildingSpaces;
	showhidemenu_.clear();
	/** TRANSLATORS: An entry in the game's show/hide menu to toggle whether building spaces are
	 * shown */
	showhidemenu_.add(buildhelp() ? _("Hide Building Spaces") : _("Show Building Spaces"),
	                  ShowHideEntry::kBuildingSpaces,
	                  g_image_cache->get("images/wui/menus/toggle_buildhelp.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kCommonBuildhelp, false));

	/** TRANSLATORS: An entry in the game's show/hide menu to toggle whether building names are shown
	 */
	showhidemenu_.add(get_display_flag(dfShowCensus) ? _("Hide Census") : _("Show Census"),
	                  ShowHideEntry::kCensus,
	                  g_image_cache->get("images/wui/menus/toggle_census.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kInGameShowhideCensus, false));

	showhidemenu_.add(get_display_flag(dfShowStatistics) ?
                         /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
                          * building status labels are shown */
                         _("Hide Status") :
                         _("Show Status"),
	                  ShowHideEntry::kStatistics,
	                  g_image_cache->get("images/wui/menus/toggle_statistics.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kInGameShowhideStats, false));

	showhidemenu_.add(get_display_flag(dfShowSoldierLevels) ?
                         /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
                          * level information is shown above soldiers' heads */
                         _("Hide Soldier Levels") :
                         _("Show Soldier Levels"),
	                  ShowHideEntry::kSoldierLevels,
	                  g_image_cache->get("images/wui/menus/toggle_soldier_levels.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kInGameShowhideSoldiers, false));

	showhidemenu_.add(get_display_flag(dfShowBuildings) ?
                         /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
                          * buildings are greyed out */
                         _("Hide Buildings") :
                         _("Show Buildings"),
	                  ShowHideEntry::kBuildings,
	                  g_image_cache->get("images/wui/stats/genstats_nrbuildings.png"), false, "",
	                  shortcut_string_for(KeyboardShortcut::kInGameShowhideBuildings, false));

	showhidemenu_.select(last_selection);
}

void InteractiveGameBase::showhide_menu_selected(ShowHideEntry entry) {
	switch (entry) {
	case ShowHideEntry::kBuildingSpaces: {
		toggle_buildhelp();
	} break;
	case ShowHideEntry::kCensus: {
		set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
	} break;
	case ShowHideEntry::kStatistics: {
		set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
	} break;
	case ShowHideEntry::kSoldierLevels: {
		set_display_flag(dfShowSoldierLevels, !get_display_flag(dfShowSoldierLevels));
	} break;
	case ShowHideEntry::kBuildings: {
		set_display_flag(dfShowBuildings, !get_display_flag(dfShowBuildings));
	} break;
	case ShowHideEntry::kWorkareaOverlap: {
		set_display_flag(dfShowWorkareaOverlap, !get_display_flag(dfShowWorkareaOverlap));
	} break;
	}
	showhidemenu_.toggle();
}

void InteractiveGameBase::add_gamespeed_menu() {
	gamespeedmenu_.set_image(g_image_cache->get("images/wui/menus/gamespeed.png"));
	toolbar()->add(&gamespeedmenu_);
	rebuild_gamespeed_menu();
	gamespeedmenu_.selected.connect(
	   [this] { gamespeed_menu_selected(gamespeedmenu_.get_selected()); });
}

void InteractiveGameBase::rebuild_gamespeed_menu() {
	const GameSpeedEntry last_selection =
	   gamespeedmenu_.has_selection() ? gamespeedmenu_.get_selected() : GameSpeedEntry::kIncrease;

	gamespeedmenu_.clear();

	gamespeedmenu_.add(_("Speed +"), GameSpeedEntry::kIncrease,
	                   g_image_cache->get("images/wui/menus/gamespeed_increase.png"), false,
	                   /** TRANSLATORS: Tooltip for Speed + in the game's game speed menu */
	                   _("Increase the game speed"),
	                   shortcut_string_for(KeyboardShortcut::kInGameSpeedUp, false));

	gamespeedmenu_.add(_("Speed -"), GameSpeedEntry::kDecrease,
	                   g_image_cache->get("images/wui/menus/gamespeed_decrease.png"), false,
	                   /** TRANSLATORS: Tooltip for Speed - in the game's game speed menu */
	                   _("Decrease the game speed"),
	                   shortcut_string_for(KeyboardShortcut::kInGameSpeedDown, false));

	if (!is_multiplayer()) {
		if ((get_game()->game_controller() != nullptr) &&
		    get_game()->game_controller()->is_paused()) {
			gamespeedmenu_.add(_("Resume"), GameSpeedEntry::kPause,
			                   g_image_cache->get("images/wui/menus/gamespeed_resume.png"), false,
			                   /** TRANSLATORS: Tooltip for Pause in the game's game speed menu */
			                   _("Resume the Game"),
			                   shortcut_string_for(KeyboardShortcut::kInGamePause, false));
		} else {
			gamespeedmenu_.add(_("Pause"), GameSpeedEntry::kPause,
			                   g_image_cache->get("images/wui/menus/gamespeed_pause.png"), false,
			                   /** TRANSLATORS: Tooltip for Pause in the game's game speed menu */
			                   _("Pause the Game"),
			                   shortcut_string_for(KeyboardShortcut::kInGamePause, false));
		}
	}

	gamespeedmenu_.select(last_selection);
}

void InteractiveGameBase::gamespeed_menu_selected(GameSpeedEntry entry) {
	switch (entry) {
	case GameSpeedEntry::kIncrease: {
		increase_gamespeed((SDL_GetModState() & KMOD_SHIFT) != 0 ? kSpeedSlow :
		                   (SDL_GetModState() & KMOD_CTRL) != 0  ? kSpeedFast :
                                                                 kSpeedDefault);
		// Keep the window open so that the player can click this multiple times
		gamespeedmenu_.toggle();
	} break;
	case GameSpeedEntry::kDecrease: {
		decrease_gamespeed((SDL_GetModState() & KMOD_SHIFT) != 0 ? kSpeedSlow :
		                   (SDL_GetModState() & KMOD_CTRL) != 0  ? kSpeedFast :
                                                                 kSpeedDefault);
		// Keep the window open so that the player can click this multiple times
		gamespeedmenu_.toggle();
	} break;
	case GameSpeedEntry::kPause: {
		if (!is_multiplayer()) {
			toggle_game_paused();
			gamespeedmenu_.toggle();
		}
	} break;
	}
}

void InteractiveGameBase::add_chat_ui() {
	add_toolbar_button(
	   "wui/menus/chat", "chat",
	   as_tooltip_text_with_hotkey(
	      _("Chat"), shortcut_string_for(KeyboardShortcut::kInGameChat, true), UI::PanelStyle::kWui),
	   &chat_, true);
	chat_.open_window = [this] {
		if (chat_provider_ != nullptr) {
			GameChatMenu::create_chat_console(this, color_functor(), chat_, *chat_provider_);
		}
	};
}

void InteractiveGameBase::increase_gamespeed(uint16_t speed) const {
	if (GameController* const ctrl = get_game()->game_controller()) {
		uint32_t const current_speed = ctrl->desired_speed();
		ctrl->set_desired_speed(current_speed + speed);
	}
}

void InteractiveGameBase::decrease_gamespeed(uint16_t speed) const {
	if (GameController* const ctrl = get_game()->game_controller()) {
		uint32_t const current_speed = ctrl->desired_speed();
		ctrl->set_desired_speed(current_speed > speed ? current_speed - speed : 0);
	}
}

void InteractiveGameBase::reset_gamespeed() const {
	if (GameController* const ctrl = get_game()->game_controller()) {
		ctrl->set_desired_speed(kSpeedDefault);
	}
}

void InteractiveGameBase::toggle_game_paused() {
	if (GameController* const ctrl = get_game()->game_controller()) {
		ctrl->toggle_paused();
		// Toggle Pause / Resume in the menu
		rebuild_gamespeed_menu();
	}
}

bool InteractiveGameBase::handle_key(bool down, SDL_Keysym code) {
	if (InteractiveBase::handle_key(down, code)) {
		return true;
	}

	if (!down) {
		return false;
	}

	if (matches_shortcut(KeyboardShortcut::kInGameSpeedReset, code)) {
		reset_gamespeed();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGamePause, code)) {
		toggle_game_paused();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedDown, code)) {
		decrease_gamespeed(kSpeedDefault);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedDownSlow, code)) {
		decrease_gamespeed(kSpeedSlow);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedDownFast, code)) {
		decrease_gamespeed(kSpeedFast);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedUp, code)) {
		increase_gamespeed(kSpeedDefault);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedUpSlow, code)) {
		increase_gamespeed(kSpeedSlow);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSpeedUpFast, code)) {
		increase_gamespeed(kSpeedFast);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameShowhideCensus, code)) {
		set_display_flag(
		   InteractiveBase::dfShowCensus, !get_display_flag(InteractiveBase::dfShowCensus));
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameShowhideStats, code)) {
		set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameShowhideSoldiers, code)) {
		set_display_flag(dfShowSoldierLevels, !get_display_flag(dfShowSoldierLevels));
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameShowhideBuildings, code)) {
		set_display_flag(dfShowBuildings, !get_display_flag(dfShowBuildings));
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameStatsGeneral, code)) {
		menu_windows_.stats_general.toggle();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameDiplomacy, code)) {
		diplomacy_.toggle();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kCommonSave, code)) {
		new GameMainMenuSaveGame(*this, menu_windows_.savegame, GameMainMenuSaveGame::Type::kSave);
		return true;
	}
	if (!is_multiplayer()) {
		if (matches_shortcut(KeyboardShortcut::kCommonLoad, code)) {
			new GameMainMenuSaveGame(*this, menu_windows_.loadgame,
			                         game().is_replay() ? GameMainMenuSaveGame::Type::kLoadReplay :
                                                       GameMainMenuSaveGame::Type::kLoadSavegame);
			return true;
		}
		if (can_restart_ && matches_shortcut(KeyboardShortcut::kInGameRestart, code)) {
			handle_restart();
			return true;
		}
	}
	if (matches_shortcut(KeyboardShortcut::kCommonExit, code)) {
		new GameExitConfirmBox(*this, *this);
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kInGameSoundOptions, code)) {
		menu_windows_.sound_options.toggle();
		return true;
	}
	if ((chat_provider_ != nullptr) && matches_shortcut(KeyboardShortcut::kInGameChat, code)) {
		if (chat_.window == nullptr) {
			GameChatMenu::create_chat_console(this, color_functor(), chat_, *chat_provider_);
		}
		return dynamic_cast<GameChatMenu*>(chat_.window)->enter_chat_message();
	}

	if (code.sym == SDLK_ESCAPE) {
		mainmenu_.toggle();
		return true;
	}

	return false;
}

bool InteractiveGameBase::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kGameSpeed, x, y, modstate);
	if (change == 0) {
		return false;
	}
	if (change < 0) {
		decrease_gamespeed(-kSpeedSlow * change);
	} else {
		increase_gamespeed(kSpeedSlow * change);
	}
	return true;
}

void InteractiveGameBase::think() {
	InteractiveBase::think();

	if (pause_on_inactivity_ != 0 &&
	    static_cast<int>(SDL_GetTicks() - UI::Panel::time_of_last_user_activity()) >
	       pause_on_inactivity_) {
		Widelands::Game& g = game();
		if (g.game_controller() != nullptr && !g.game_controller()->is_paused()) {
			toggle_game_paused();
		}
	}
}

/// \return a pointer to the running \ref Game instance.
Widelands::Game* InteractiveGameBase::get_game() const {
	return dynamic_cast<Widelands::Game*>(&egbase());
}

Widelands::Game& InteractiveGameBase::game() const {
	return dynamic_cast<Widelands::Game&>(egbase());
}

void InteractiveGameBase::show_watch_window(Widelands::Bob& b) {
	WatchWindow* window = ::show_watch_window(*this, b.get_position());
	window->follow(&b);
}

void InteractiveGameBase::draw_overlay(RenderTarget& dst) {
	InteractiveBase::draw_overlay(dst);

	GameController* game_controller = game().game_controller();
	// Display the gamespeed.
	if (game_controller != nullptr) {
		std::string game_speed;
		const int64_t computed_target = game_controller->real_speed();
		const int64_t desired = game_controller->desired_speed();
		int64_t actual = average_real_gamespeed();
		constexpr int64_t kFluctuationTolerance = 1000;  // Arbitrary value.
		if (abs(actual - computed_target) < kFluctuationTolerance) {
			actual = computed_target;  // Ignore minor fluctuations.
		}

		if (desired == computed_target && actual == computed_target) {
			if (actual != 1000) {
				game_speed = speed_string(actual);
			}
		} else if (desired == computed_target || actual == computed_target) {
			game_speed = format
			   /** TRANSLATORS: actual_speed (desired_speed) */
			   (_("%1$s (%2$s)"), speed_string(actual), speed_string(desired));
		} else {
			game_speed = format
			   /** TRANSLATORS: actual_speed (target_speed) (desired_speed) */
			   (_("%1$s (%2$s) (%3$s)"), speed_string(actual), speed_string(computed_target),
			    speed_string(desired));
		}

		info_panel_.set_speed_string(game_speed);
	}
}

void InteractiveGameBase::set_sel_pos(Widelands::NodeAndTriangle<> const center) {
	InteractiveBase::set_sel_pos(center);

	const Widelands::Map& map = egbase().map();

	// If we have an immovable, we might want to show a tooltip
	Widelands::BaseImmovable* imm = map[center.node].get_immovable();
	if (imm == nullptr) {
		return set_tooltip("");
	}

	// If we have a player, only show tooltips if he sees the field
	const Widelands::Player* player = nullptr;
	if (upcast(InteractivePlayer, iplayer, this)) {
		player = iplayer->get_player();
		if (player != nullptr && !player->see_all() &&
		    (!player->is_seeing(Widelands::Map::get_index(center.node, map.get_width())))) {
			return set_tooltip("");
		}
	}

	if (imm->descr().type() == Widelands::MapObjectType::IMMOVABLE) {
		// Trees, Resource Indicators, fields ...
		return set_tooltip(imm->descr().descname());
	}
	if (upcast(Widelands::ProductionSite, productionsite, imm)) {
		// No productionsite tips for hostile players
		if (player == nullptr || !player->is_hostile(*productionsite->get_owner())) {
			return set_tooltip(
			   productionsite->info_string(Widelands::Building::InfoStringFormat::kTooltip));
		}
	}
	set_tooltip("");
}

/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void InteractiveGameBase::postload() {
	// Recalc whole map for changed owner stuff
	egbase().mutable_map()->recalc_whole_map(egbase());
}

void InteractiveGameBase::start() {
	// Multiplayer games don't save the view position, so we go to the starting position instead
	if (is_multiplayer()) {
		Widelands::PlayerNumber pln = player_number();
		const Widelands::PlayerNumber max = game().map().get_nrplayers();
		if (pln == 0) {
			// Spectator, use the view of the first viable player
			for (pln = 1; pln <= max; ++pln) {
				if (game().get_player(pln) != nullptr) {
					break;
				}
			}
		}
		// Adding a check, just in case there was no viable player found for spectator
		if (game().get_player(pln) != nullptr) {
			map_view()->scroll_to_field(game().map().get_starting_pos(pln), MapView::Transition::Jump);
		}
	}
}

/**
 * See if we can reasonably open a ship window at the current selection position.
 * If so, do it and return true; otherwise, return false.
 */
bool InteractiveGameBase::try_show_ship_windows() {
	const Widelands::Map& map = game().map();
	Widelands::Area<Widelands::FCoords> area(map.get_fcoords(get_sel_pos().node), 1);

	std::vector<Widelands::Bob*> all_ships;
	std::vector<Widelands::Ship*> manageable;
	std::vector<Widelands::Ship*> attackable;
	if (map.find_bobs(egbase(), area, &all_ships, Widelands::FindBobShip()) != 0u) {
		for (Widelands::Bob* bob : all_ships) {
			upcast(Widelands::Ship, ship, bob);
			assert(ship != nullptr);  // FindBobShip should have returned only ships
			if (can_see(ship->owner().player_number())) {
				manageable.push_back(ship);
			} else if (get_player() != nullptr && get_player()->is_hostile(ship->owner()) &&
			           ship->can_be_attacked()) {
				attackable.push_back(ship);
			}
		}
	}

	if (manageable.empty() && attackable.empty()) {
		return false;  // No ships nearby.
	}

	if (manageable.size() + attackable.size() == 1) {
		// Exactly one ship nearby, open its appropriate window.
		if (attackable.empty()) {
			show_ship_window(manageable.front());
		} else {
			show_attack_window(get_sel_pos().node, attackable.front(), true);
		}
		return true;
	}

	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		// Open all applicable windows if Ctrl is held.
		for (Widelands::Ship* ship : manageable) {
			show_ship_window(ship);
		}
		for (Widelands::Ship* ship : attackable) {
			show_attack_window(get_sel_pos().node, ship, true);
		}
		return true;
	}

	// Show a selection dialog.
	show_ship_selection_window(this, &fieldaction_, get_sel_pos().node, manageable, attackable);
	return true;
}

void InteractiveGameBase::show_game_summary() {
	NoteThreadSafeFunction::instantiate(
	   [this]() {
		   if (game_summary_.window != nullptr) {
			   game_summary_.window->set_visible(true);
			   game_summary_.window->think();
			   return;
		   }
		   new GameSummaryScreen(this, &game_summary_);
	   },
	   true);
}

bool InteractiveGameBase::show_game_client_disconnected() {
	assert(dynamic_cast<GameHost*>(get_game()->game_controller()) != nullptr);
	if (client_disconnected_.window == nullptr) {
		if (upcast(GameHost, host, get_game()->game_controller())) {
			new GameClientDisconnected(this, client_disconnected_, host);
			return true;
		}
	}
	return false;
}
