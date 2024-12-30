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

#include "ui_fsmenu/main.h"

#include <cstdlib>
#include <memory>
#include <optional>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "build_info.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "graphic/graphic_functions.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/single_player_game_settings_provider.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/about.h"
#include "ui_fsmenu/addons/manager.h"
#include "ui_fsmenu/campaign_select.h"
#include "ui_fsmenu/internet_lobby.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/login_box.h"
#include "ui_fsmenu/mapselect.h"
#include "ui_fsmenu/netsetup_lan.h"
#include "ui_fsmenu/options.h"
#include "ui_fsmenu/random_game.h"
#include "ui_fsmenu/scenario_select.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/maptable.h"
#include "wui/savegameloader.h"

namespace FsMenu {

constexpr uint32_t kSplashFadeoutDuration = 2000;
constexpr uint32_t kImageExchangeInterval = 20000;
constexpr uint32_t kImageExchangeDuration = 2500;

constexpr uint32_t kNoSplash = std::numeric_limits<uint32_t>::max();

int16_t MainMenu::calc_desired_window_width(const UI::Window::WindowLayoutID id) {
	switch (id) {
	case UI::Window::WindowLayoutID::kFsMenuDefault:
		return std::max(800, get_w() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuOptions:
	case UI::Window::WindowLayoutID::kFsMenuAbout:
		return std::max(600, get_w() / 2);
	case UI::Window::WindowLayoutID::kFsMenuKeyboardOptions:
		return std::max(720, get_w() * 3 / 4);
	default:
		NEVER_HERE();
	}
}

int16_t MainMenu::calc_desired_window_height(const UI::Window::WindowLayoutID id) {
	switch (id) {
	case UI::Window::WindowLayoutID::kFsMenuDefault:
		return std::max(600, get_h() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuAbout:
		return std::max(500, get_h() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuOptions:
		return std::max(450, get_h() / 2);
	case UI::Window::WindowLayoutID::kFsMenuKeyboardOptions:
		return std::max(400, get_h() / 2);
	default:
		NEVER_HERE();
	}
}

int16_t MainMenu::calc_desired_window_x(const UI::Window::WindowLayoutID id) {
	const UI::WindowStyleInfo& wsi = g_style_manager->window_style(UI::WindowStyle::kFsMenu);
	return (get_w() - (calc_desired_window_width(id) + wsi.left_border_thickness() +
	                   wsi.right_border_thickness())) /
	       2;
}

int16_t MainMenu::calc_desired_window_y(const UI::Window::WindowLayoutID id) {
	const UI::WindowStyleInfo& wsi = g_style_manager->window_style(UI::WindowStyle::kFsMenu);
	return (get_h() - (calc_desired_window_height(id) + wsi.top_border_thickness() +
	                   wsi.bottom_border_thickness())) /
	       2;
}

MainMenu::MainMenu(const bool skip_init)
   : UI::Panel(nullptr,
               UI::PanelStyle::kFsMenu,
               "widelands_main_menu",
               0,
               0,
               g_gr->get_xres(),
               g_gr->get_yres()),
     box_rect_(0, 0, 0, 0),
     butw_(get_w() * 7 / 20),
     buth_(get_h() * 9 / 200),
     padding_(buth_ / 3),
     vbox1_(this, UI::PanelStyle::kFsMenu, "vbox_1", 0, 0, UI::Box::Vertical, 0, 0, padding_),
     vbox2_(this, UI::PanelStyle::kFsMenu, "vbox_2", 0, 0, UI::Box::Vertical, 0, 0, padding_),
     singleplayer_(&vbox1_,
                   "singleplayer",
                   0,
                   0,
                   butw_,
                   6,
                   buth_,
                   "",
                   UI::DropdownType::kTextualMenu,
                   UI::PanelStyle::kFsMenu,
                   UI::ButtonStyle::kFsMenuMenu,
                   [this](MenuTarget t) { action(t); }),
     multiplayer_(&vbox1_,
                  "multiplayer",
                  0,
                  0,
                  butw_,
                  6,
                  buth_,
                  "",
                  UI::DropdownType::kTextualMenu,
                  UI::PanelStyle::kFsMenu,
                  UI::ButtonStyle::kFsMenuMenu,
                  [this](MenuTarget t) { action(t); }),
     replay_(&vbox1_,
             "replay",
             0,
             0,
             butw_,
             6,
             buth_,
             "",
             UI::DropdownType::kTextualMenu,
             UI::PanelStyle::kFsMenu,
             UI::ButtonStyle::kFsMenuMenu,
             [this](MenuTarget t) { action(t); }),
     editor_(&vbox1_,
             "editor",
             0,
             0,
             butw_,
             6,
             buth_,
             "",
             UI::DropdownType::kTextualMenu,
             UI::PanelStyle::kFsMenu,
             UI::ButtonStyle::kFsMenuMenu,
             [this](MenuTarget t) { action(t); }),
     addons_(&vbox2_, "addons", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, ""),
     options_(&vbox2_, "options", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, ""),
     about_(&vbox2_, "about", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, ""),
     exit_(&vbox2_, "exit", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, ""),
     clock_(this,
            UI::PanelStyle::kFsMenu,
            "clock",
            UI::FontStyle::kFsMenuLabel,
            0,
            0,
            0,
            0,
            "",
            UI::Align::kLeft),
     version_(this,
              UI::PanelStyle::kFsMenu,
              "version",
              UI::FontStyle::kFsMenuInfoPanelParagraph,
              0,
              0,
              0,
              0,
              "",
              UI::Align::kCenter),
     copyright_(this,
                UI::PanelStyle::kFsMenu,
                "copyright",
                UI::FontStyle::kFsMenuInfoPanelParagraph,
                0,
                0,
                0,
                0,
                "",
                UI::Align::kCenter),
     init_time_(kNoSplash),

     menu_capsule_(*this) {
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
		   layout();
	   });

	// Load backgrounds, logos etc
	update_template();

	singleplayer_.selected.connect([this]() { action(singleplayer_.get_selected()); });
	multiplayer_.selected.connect([this]() { action(multiplayer_.get_selected()); });
	editor_.selected.connect([this]() { action(editor_.get_selected()); });
	replay_.selected.connect([this]() { action(replay_.get_selected()); });
	addons_.sigclicked.connect([this]() { action(MenuTarget::kAddOns); });
	options_.sigclicked.connect([this]() { action(MenuTarget::kOptions); });
	about_.sigclicked.connect([this]() { action(MenuTarget::kAbout); });
	exit_.sigclicked.connect([this]() { action(MenuTarget::kExit); });

	vbox1_.add(&singleplayer_, UI::Box::Resizing::kFullSize);
	vbox1_.add_inf_space();
	vbox1_.add(&multiplayer_, UI::Box::Resizing::kFullSize);
	vbox1_.add_inf_space();
	vbox1_.add(&editor_, UI::Box::Resizing::kFullSize);
	vbox1_.add_inf_space();
	vbox1_.add(&replay_, UI::Box::Resizing::kFullSize);

	vbox2_.add(&options_, UI::Box::Resizing::kFullSize);
	vbox2_.add_inf_space();
	vbox2_.add(&addons_, UI::Box::Resizing::kFullSize);
	vbox2_.add_inf_space();
	vbox2_.add(&about_, UI::Box::Resizing::kFullSize);
	vbox2_.add_inf_space();
	vbox2_.add(&exit_, UI::Box::Resizing::kFullSize);

	if (!skip_init) {
		init_time_ = SDL_GetTicks();
		splash_state_ = SplashState::kSplash;
		set_button_visibility(false);
	}

	r_login_.open_window = [this]() { new LoginBox(*this, r_login_); };
	r_about_.open_window = [this]() { new About(*this, r_about_); };
	r_addons_.open_window = [this]() { new AddOnsUI::AddOnsCtrl(*this, r_about_); };

	focus();
	set_labels();
	layout();

	initialization_complete();

	reinit_plugins();
}

void MainMenu::main_loop() {
	for (;;) {
		try {
			run<int>();
			return;  // We only get here though normal termination by the user.
		} catch (const WLWarning& e) {
			// WLWarning is reserved for bad circumstances that are (most likely) not a bug.
			show_messagebox(e.title(), e.what());
		} catch (const std::exception& e) {
			// This is the outermost wrapper within the GUI and should only very rarely be reached.
			// Most likely we got here through a bug in Widelands.
			show_messagebox(
			   _("Error!"),
			   format(
			      _("An error has occured. The error message is:\n\n%1$s\n\nPlease report "
			        "this problem to help us improve Widelands. You will find related messages in the "
			        "standard output (stdout.txt on Windows). You are using version %2$s.\n"
			        "Please add this information to your report."),
			      e.what(), build_ver_details()));
		}
	}
}

Widelands::Game* MainMenu::create_safe_game(const bool show_error) {
	try {
		return new Widelands::Game;
	} catch (const std::exception& e) {
		log_err("Error allocating game: %s", e.what());
		if (show_error) {
			UI::WLMessageBox m(
			   this, UI::WindowStyle::kFsMenu, _("Error"),
			   format(_("Unable to create a Game instance!\nError message:\n%s"), e.what()),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		}
	}
	return nullptr;
}

void MainMenu::update_template() {
	UI::Panel::update_template();

	int dropdowns_lineheight =
	   g_style_manager->styled_size(UI::StyledSize::kFsMainMenuDropdownHeight);
	singleplayer_.set_min_lineheight(dropdowns_lineheight);
	multiplayer_.set_min_lineheight(dropdowns_lineheight);
	editor_.set_min_lineheight(dropdowns_lineheight);
	replay_.set_min_lineheight(dropdowns_lineheight);

	title_image_ = g_image_cache->get("loadscreens/logo.png");

	images_.clear();
	for (const std::string& img : g_fs->list_directory(template_dir() + "loadscreens/mainmenu")) {
		images_.push_back(img);
	}
	if (images_.empty() && !is_using_default_theme()) {
		log_warn("No main menu backgrounds found, using fallback images");
		for (const std::string& img :
		     g_fs->list_directory(kDefaultTemplate + "loadscreens/mainmenu")) {
			images_.push_back(img);
		}
	}
	if (images_.empty()) {
		log_warn("No main menu backgrounds found in default theme, using fallback image");
		images_.emplace_back("images/logos/wl-ico-128.png");
	}

	last_image_ = draw_image_ = RNG::static_rand(images_.size());
	last_image_exchange_time_ = 0;
}

void MainMenu::show_messagebox(const std::string& messagetitle, const std::string& errormessage) {
	UI::WLMessageBox mmb(this, UI::WindowStyle::kFsMenu, messagetitle, richtext_escape(errormessage),
	                     UI::WLMessageBox::MBoxType::kOk, UI::Align::kLeft);
	mmb.run<UI::Panel::Returncodes>();
}

void MainMenu::become_modal_again(UI::Panel& prevmodal) {
	if (dynamic_cast<const UI::Window*>(&prevmodal) == nullptr) {
		// Ensure the image is not exchanged directly after returning to the main menu –
		// but only after returning from the game or editor and not from the options window.
		last_image_exchange_time_ = SDL_GetTicks();
		if (last_image_exchange_time_ > kImageExchangeDuration) {
			last_image_exchange_time_ -= kImageExchangeDuration;
		} else {
			last_image_exchange_time_ = 0;
		}
	}
}

void MainMenu::set_labels() {
	{
		// TODO(Nordfriese): Code duplication, the same code is used in InteractiveBase
		Section& global_s = get_config_section();
		set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
		set_panel_snap_distance(global_s.get_int("panel_snap_distance", 10));
		set_dock_windows_to_edges(global_s.get_bool("dock_windows_to_edges", false));
		system_clock_ = get_config_bool("game_clock", true);
	}

	singleplayer_.clear();
	multiplayer_.clear();
	editor_.clear();
	replay_.clear();

	singleplayer_.add(_("New Game"), MenuTarget::kNewGame, nullptr, false, _("Begin a new game"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuNew, false));
	singleplayer_.add(_("New Random Game"), MenuTarget::kRandomGame, nullptr, false,
	                  _("Create a new random match"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuRandomMatch, false));
	singleplayer_.add(_("Campaigns"), MenuTarget::kCampaign, nullptr, false, _("Play a campaign"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuCampaign, false));
	singleplayer_.add(_("Tutorials"), MenuTarget::kTutorial, nullptr, false,
	                  _("Play one of our beginners’ tutorials"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuTutorial, false));
	singleplayer_.add(_("Load Game"), MenuTarget::kLoadGame, nullptr, false,
	                  _("Continue a saved game"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuLoad, false));

	// Refresh the Continue tooltip. The SavegameData must be reloaded after
	// every language switch because it contains localized strings.
	{
		filename_for_continue_playing_ = "";
		std::optional<SavegameData> newest_singleplayer = newest_saved_game_or_replay();
		if (newest_singleplayer.has_value()) {
			filename_for_continue_playing_ = newest_singleplayer->filename;
			singleplayer_.add(
			   _("Continue Playing"), MenuTarget::kContinueLastsave, nullptr, false,
			   format(
			      "%s<br>%s<br>%s<br>%s<br>%s<br>%s",
			      as_font_tag(UI::FontStyle::kFsTooltipHeader,
			                  /* strip leading "save/" and trailing ".wgf" */
			                  filename_for_continue_playing_.substr(
			                     kSaveDir.length() + 1, filename_for_continue_playing_.length() -
			                                               kSaveDir.length() -
			                                               kSavegameExtension.length() - 1)),
			      format(_("Map: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                       newest_singleplayer->mapname)),
			      format(_("Win Condition: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                                 newest_singleplayer->wincondition)),
			      format(_("Players: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                           newest_singleplayer->nrplayers)),
			      format(_("Gametime: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                            newest_singleplayer->gametime)),
			      /** TRANSLATORS: Information about when a game was saved, e.g. 'Saved: Today,
			       * 10:30'
			       */
			      format(_("Saved: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                         newest_singleplayer->savedatestring))),
			   shortcut_string_for(KeyboardShortcut::kMainMenuContinuePlaying, false));
		}
	}

	multiplayer_.add(_("Online Game"), MenuTarget::kMetaserver, nullptr, false,
	                 _("Join the Widelands lobby"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLobby, false));
	multiplayer_.add(_("Online Game Settings"), MenuTarget::kOnlineGameSettings, nullptr, false,
	                 _("Log in as a registered user"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLogin, false));
	multiplayer_.add(_("LAN / Direct IP"), MenuTarget::kLan, nullptr, false,
	                 _("Play a private online game"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLAN, false));

	editor_.add(_("New Map"), MenuTarget::kEditorNew, nullptr, false, _("Create a new empty map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorNew, false));
	editor_.add(_("Random Map"), MenuTarget::kEditorRandom, nullptr, false,
	            _("Create a new random map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorRandom, false));
	editor_.add(_("Load Map"), MenuTarget::kEditorLoad, nullptr, false, _("Edit an existing map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorLoad, false));

	{
		filename_for_continue_editing_ = "";
		std::optional<MapData> last_edited = newest_edited_map();
		if (last_edited.has_value()) {
			filename_for_continue_editing_ = last_edited->filenames.at(0);
			editor_.add(
			   _("Continue Editing"), MenuTarget::kEditorContinue, nullptr, false,
			   format(
			      "%s<br>%s<br>%s<br>%s<br>%s",
			      as_font_tag(UI::FontStyle::kFsTooltipHeader,
			                  /* strip leading "maps/My_Maps/" and trailing ".wmf" */
			                  filename_for_continue_editing_.substr(
			                     kMyMapsDirFull.length() + 1, filename_for_continue_editing_.length() -
			                                                     kMyMapsDirFull.length() -
			                                                     kWidelandsMapExtension.length() - 1)),
			      format(_("Name: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                        last_edited->localized_name)),
			      format(_("Size: %s"),
			             as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                         format(_("%1$u×%2$u"), last_edited->width, last_edited->height))),
			      format(_("Players: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                           std::to_string(last_edited->nrplayers))),
			      format(_("Description: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                               last_edited->description))),
			   shortcut_string_for(KeyboardShortcut::kMainMenuContinueEditing, false));
		}
	}

	replay_.add(_("Load Replay"), MenuTarget::kReplay, nullptr, false,
	            _("Watch the replay of an old game"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuLoadReplay, false));
	{
		filename_for_last_replay_ = "";
		std::optional<SavegameData> newest_replay = newest_saved_game_or_replay(true);
		if (newest_replay.has_value()) {
			filename_for_last_replay_ = newest_replay->filename;
			replay_.add(
			   _("Watch last saved replay"), MenuTarget::kReplayLast, nullptr, false,
			   format(
			      "%s<br>%s<br>%s<br>%s<br>%s<br>%s",
			      as_font_tag(UI::FontStyle::kFsTooltipHeader,
			                  /* strip leading "replays/" and trailing ".wry" */
			                  filename_for_last_replay_.substr(
			                     kReplayDir.length() + 1, filename_for_last_replay_.length() -
			                                                 kReplayDir.length() -
			                                                 kReplayExtension.length() - 1)),
			      format(_("Map: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                       newest_replay->mapname)),
			      format(_("Win Condition: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                                 newest_replay->wincondition)),
			      format(_("Players: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                           newest_replay->nrplayers)),
			      format(_("Gametime: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                            newest_replay->gametime)),
			      /** TRANSLATORS: Information about when a game was saved, e.g. 'Saved: Today,
			       * 10:30'
			       */
			      format(_("Saved: %s"), as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
			                                         newest_replay->savedatestring))),
			   shortcut_string_for(KeyboardShortcut::kMainMenuReplayLast, false));
		}
	}

	singleplayer_.set_label(_("Single Player…"));
	multiplayer_.set_label(_("Multiplayer…"));
	editor_.set_label(_("Editor…"));
	replay_.set_label(_("Watch Replay…"));
	singleplayer_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Begin or load a single-player campaign or free game"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuSP, true), UI::PanelStyle::kFsMenu));
	multiplayer_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Play with your friends over the internet"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuMP, true), UI::PanelStyle::kFsMenu));
	editor_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Launch the map editor"), shortcut_string_for(KeyboardShortcut::kMainMenuE, true),
	   UI::PanelStyle::kFsMenu));
	replay_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Watch again a recorded earlier game"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuReplay, true), UI::PanelStyle::kFsMenu));

	addons_.set_title(_("Add-Ons"));
	addons_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Install and manage add-ons"), shortcut_string_for(KeyboardShortcut::kMainMenuAddons, true),
	   UI::PanelStyle::kFsMenu));
	options_.set_title(_("Options"));
	options_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Technical and game-related settings"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuOptions, true), UI::PanelStyle::kFsMenu));
	about_.set_title(_("About Widelands"));
	about_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Readme, License, and Credits"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuAbout, true), UI::PanelStyle::kFsMenu));
	exit_.set_title(_("Exit Widelands"));
	exit_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Quit the game"), shortcut_string_for(KeyboardShortcut::kMainMenuQuit, true),
	   UI::PanelStyle::kFsMenu));

	version_.set_text(
	   /** TRANSLATORS: %1$s = version string and build type  ("Debug", "Release", or
	      "RelWithDebInfo") */
	   format(_("Version %1$s"), build_ver_details()));
	copyright_.set_text(
	   /** TRANSLATORS: Placeholders are the copyright years */
	   format(_("(C) %1%-%2% by the Widelands Development Team • Licensed under "
	            "the GNU General Public License V2.0"),
	          kWidelandsCopyrightStart, kWidelandsCopyrightEnd));
}

void MainMenu::set_button_visibility(const bool v) {
	if (visible_ == v) {
		return;
	}
	visible_ = v;
	vbox1_.set_visible(v);
	vbox2_.set_visible(v);
	copyright_.set_visible(v);
	version_.set_visible(v);
	clock_.set_visible(v);
}

void MainMenu::end_splashscreen() {
	assert(splash_state_ == SplashState::kSplash);
	verb_log_info("Initiating splash screen fade out");
	if (g_sh->current_songset() != Songset::kMenu) {
		// mix the intro down during the splash fade out phase, then draw() will start the menu music
		// in the menu fade in phase
		g_sh->stop_music(kSplashFadeoutDuration);
	}
	splash_state_ = SplashState::kSplashFadeOut;
	init_time_ = SDL_GetTicks();
}

void MainMenu::abort_splashscreen() {
	verb_log_info("Splash screen ended");
	splash_state_ = SplashState::kDone;
	init_time_ = kNoSplash;
	last_image_exchange_time_ = SDL_GetTicks();
}

void MainMenu::think() {
	UI::Panel::think();
	plugin_actions_->think();
}

void MainMenu::reinit_plugins() {
	lua_.reset(new LuaFsMenuInterface(this));
	plugin_actions_.reset(
	   new PluginActions(this, [this](const std::string& cmd) { lua_->interpret_string(cmd); }));

	for (const auto& pair : AddOns::g_addons) {
		if (pair.second && pair.first->category == AddOns::AddOnCategory::kUIPlugin) {
			lua_->run_script(kAddOnDir + FileSystem::file_separator() + pair.first->internal_name +
			                 FileSystem::file_separator() + "init.lua");
		}
	}
}

bool MainMenu::handle_mousepress(uint8_t /*btn*/, int32_t /*x*/, int32_t /*y*/) {
	if (splash_state_ != SplashState::kDone) {
		abort_splashscreen();
		return true;
	}
	return false;
}

bool MainMenu::handle_key(const bool down, const SDL_Keysym code) {
	// Forward all keys to the open window if there is one
	bool has_open_window = false;
	for (UI::UniqueWindow::Registry* r : {&r_login_, &r_about_, &r_addons_}) {
		if (r->window != nullptr) {
			has_open_window = true;
			if (r->window->handle_key(down, code)) {
				return true;
			}
		}
	}
	if (menu_capsule_.is_visible()) {
		has_open_window = true;
		if (menu_capsule_.handle_key(down, code)) {
			return true;
		}
	}
	if (has_open_window) {
		// If any window is open, block all keypresses to prevent accidentally triggering hotkeys
		return true;
	}

	if (splash_state_ != SplashState::kDone && down) {
		abort_splashscreen();
		if (matches_shortcut(KeyboardShortcut::kMainMenuQuit, code)) {
			// don't initiate quitting in this case
			return true;
		}
	}

	if (plugin_actions_->check_keyboard_shortcut_action(code, down)) {
		return true;
	}

	if (down) {
		auto check_match_shortcut = [this, &code](KeyboardShortcut k, MenuTarget t) {
			if (matches_shortcut(k, code)) {
				action(t);
				return true;
			}
			return false;
		};
		if (check_match_shortcut(KeyboardShortcut::kMainMenuNew, MenuTarget::kNewGame)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLoad, MenuTarget::kLoadGame)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLoadReplay, MenuTarget::kReplay)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuReplayLast, MenuTarget::kReplayLast)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuTutorial, MenuTarget::kTutorial)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuRandomMatch, MenuTarget::kRandomGame)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuCampaign, MenuTarget::kCampaign)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLobby, MenuTarget::kMetaserver)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLAN, MenuTarget::kLan)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuAddons, MenuTarget::kAddOns)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuOptions, MenuTarget::kOptions)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuAbout, MenuTarget::kAbout)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuEditorNew, MenuTarget::kEditorNew)) {
			return true;
		}
		if (check_match_shortcut(
		       KeyboardShortcut::kMainMenuEditorRandom, MenuTarget::kEditorRandom)) {
			return true;
		}
		if (check_match_shortcut(KeyboardShortcut::kMainMenuEditorLoad, MenuTarget::kEditorLoad)) {
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuSP, code)) {
			singleplayer_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuMP, code)) {
			multiplayer_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuE, code)) {
			editor_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuReplay, code)) {
			replay_.toggle();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuQuit, code)) {
			exit();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuContinuePlaying, code)) {
			if (!filename_for_continue_playing_.empty()) {
				action(MenuTarget::kContinueLastsave);
				return true;
			}
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuContinueEditing, code)) {
			if (!filename_for_continue_editing_.empty()) {
				action(MenuTarget::kEditorContinue);
				return true;
			}
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuLogin, code)) {
			show_internet_login();
			return true;
		}
		if (code.sym == SDLK_BACKSPACE && ((code.mod & (KMOD_CTRL | KMOD_SHIFT)) != 0)) {
			// Easter egg: Press Ctrl/Shift+Backspace to exchange the background immediately :-)
			last_image_exchange_time_ -=
			   (last_image_exchange_time_ > kImageExchangeInterval ? kImageExchangeInterval :
			                                                         last_image_exchange_time_);
			return true;
		}
	}
	return UI::Panel::handle_key(down, code);
}

inline float MainMenu::calc_opacity(const uint32_t time) const {
	return last_image_ == draw_image_ ?
	          1.f :
	          std::max(0.f, std::min(1.f, static_cast<float>(time - last_image_exchange_time_) /
	                                         kImageExchangeDuration));
}

/*
 * The four phases of the splash screen are:
 *   1) SplashState::kSplash:
 *        We start with the splash image shown with full opacity on a black background.
 *        We initiate fade out when the intro music ends.
 *   2) SplashState::kSplashFadeOut:
 *        Show the splash image semi-transparent on a black background for `kSplashFadeoutDuration`
 *   3) SplashState::kMenuFadeIn:
 *        Show the background & menu semi-transparent for `kSplashFadeoutDuration`
 *   4) SplashState::kDone:
 *        Show the background & menu with full opacity indefinitely
 *
 * Phases 1 and 2 are handled by draw_overlay().
 * Phase 3 is handled by draw() and draw_overlay() together.
 * Phase 4 is handled by draw() alone.
 * Stepping through the phases is handled by draw_overlay().
 *
 * We skip straight to phase 4 if we are returning from some other FsMenu screen or if a key is
 * pressed or the mouse is clicked.
 */

void MainMenu::draw(RenderTarget& r) {
	if (splash_state_ == SplashState::kSplash || splash_state_ == SplashState::kSplashFadeOut) {
		// Handled by draw_overlay(). The actual menu is not visible in these states.
		return;
	}

	// TODO(tothxa): This shouldn't be in draw(), but we don't have think(), nor a single
	//               entry point.
	// Reset the songset when a game, replay or editing session returns.
	if (g_sh->current_songset() != Songset::kMenu) {
		g_sh->change_music(Songset::kMenu, 500);
	}

	UI::Panel::draw(r);
	r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, 255));
	set_button_visibility(true);

	// Exchange stale background images
	const uint32_t time = SDL_GetTicks();
	assert(time >= last_image_exchange_time_);
	if (time - last_image_exchange_time_ > kImageExchangeInterval) {
		last_image_ = draw_image_;
		if (images_.size() > 1) {
			do {
				draw_image_ = RNG::static_rand(images_.size());
			} while (draw_image_ == last_image_);
		}
		last_image_exchange_time_ = time;
	}

	{  // Draw background images
		float opacity = 1.f;

		if (time - last_image_exchange_time_ < kImageExchangeDuration) {
			const Image* img = g_image_cache->get(images_[last_image_]);
			opacity = calc_opacity(time);
			r.blit_fit(img, true, 1.f - opacity);
		}

		const Image* img = g_image_cache->get(images_[draw_image_]);
		r.blit_fit(img, true, opacity);
	}

	{  // Darken button boxes
		const RGBAColor bg(0, 0, 0, 130);

		r.fill_rect(Recti(box_rect_.x - padding_, box_rect_.y - padding_, box_rect_.w + 2 * padding_,
		                  box_rect_.h + 2 * padding_),
		            bg, BlendMode::Default);

		const int max_w = std::max(copyright_.get_w(), version_.get_w());
		r.fill_rect(Recti((get_w() - max_w - padding_) / 2, version_.get_y() - padding_ / 2,
		                  max_w + padding_, get_h() - version_.get_y() + padding_ / 2),
		            bg, BlendMode::Default);

		if (system_clock_) {
			clock_.set_text(realtimestring());
			r.fill_rect(Recti(0, 0, clock_.get_w() + 2 * padding_, clock_.get_h() + 2 * padding_), bg,
			            BlendMode::Default);
		} else {
			clock_.set_text(std::string());
		}
	}

	// Widelands logo
	const Rectf rect = title_pos();
	const Rectf dest(rect.x + rect.w * 0.2f, rect.y + rect.h * 0.2f, rect.w * 0.6f, rect.h * 0.6f);
	r.blitrect_scale(dest, title_image_, title_image_->rect(), 1.0f, BlendMode::UseAlpha);
}

void MainMenu::draw_overlay(RenderTarget& r) {
	if (splash_state_ == SplashState::kDone) {
		// overlays are needed only during the first three phases
		return;
	}

	const uint32_t time = SDL_GetTicks();

	assert(init_time_ != kNoSplash && time >= init_time_);

	float progress = 0.0f;

	if (splash_state_ == SplashState::kSplash) {
		// When the intro music ends, the event handler in wlapplication.cc starts the main menu
		// music. We use that to detect when it's time to end the splash screen by default.
		// We can't set up a notification, because the main menu may not be created before it ends
		// if the startup is extremely slow for some reason.
		const bool intro_is_playing = (g_sh->current_songset() == Songset::kIntro) &&
		                              (g_sh->is_sound_audible(SoundType::kMusic)) &&
		                              Mix_PlayingMusic() != 0;

		if (!intro_is_playing) {
			end_splashscreen();
		}
	} else if (time - init_time_ > kSplashFadeoutDuration) {
		// The next step is due
		if (splash_state_ == SplashState::kMenuFadeIn ||
		    (time - init_time_ > 2 * kSplashFadeoutDuration)) {
			abort_splashscreen();
			return;
		}

		assert(splash_state_ == SplashState::kSplashFadeOut);
		init_time_ = time;
		last_image_exchange_time_ = time;
		splash_state_ = SplashState::kMenuFadeIn;

	} else {
		// We're in the middle of phase 2 or 3
		progress = static_cast<float>(time - init_time_) / kSplashFadeoutDuration;
	}

	if (splash_state_ != SplashState::kMenuFadeIn) {
		// TODO(tothxa): Some dynamic content would be nice to entertain the user while the
		//               intro music is playing
		const std::string footer =
		   splash_state_ == SplashState::kSplash ? _("Click for the main menu") : "";
		draw_splashscreen(r, footer, 1.0f - progress);
	} else {
		const unsigned alpha = 255 - 255.f * progress;  // fade in of menu = fade out of overlay
		r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, alpha), BlendMode::Default);
	}
}

inline Rectf MainMenu::title_pos() {
	const float imgh = box_rect_.y / 2.5f;
	const float imgw = imgh * title_image_->width() / title_image_->height();
	return Rectf((get_w() - imgw) / 2.f, buth_, imgw, imgh);
}

void MainMenu::layout() {
	butw_ = get_inner_w() / 5;
	buth_ = get_inner_h() / 25;
	padding_ = buth_ / 3;

	copyright_.set_pos(Vector2i(
	   (get_inner_w() - copyright_.get_w()) / 2, get_inner_h() - copyright_.get_h() - padding_ / 2));
	version_.set_pos(Vector2i((get_inner_w() - version_.get_w()) / 2,
	                          copyright_.get_y() - version_.get_h() - padding_ / 2));
	clock_.set_pos(Vector2i(padding_, padding_));

	box_rect_ = Recti((get_inner_w() - padding_) / 2 - butw_,
	                  version_.get_y() - padding_ * 4 - get_inner_h() * 3 / 10, 2 * butw_ + padding_,
	                  get_inner_h() / 4);

	singleplayer_.set_desired_size(butw_, buth_);
	multiplayer_.set_desired_size(butw_, buth_);
	replay_.set_desired_size(butw_, buth_);
	editor_.set_desired_size(butw_, buth_);
	addons_.set_desired_size(butw_, buth_);
	options_.set_desired_size(butw_, buth_);
	about_.set_desired_size(butw_, buth_);
	exit_.set_desired_size(butw_, buth_);

	vbox1_.set_inner_spacing(padding_);
	vbox2_.set_inner_spacing(padding_);
	vbox1_.set_pos(Vector2i(box_rect_.x, box_rect_.y));
	vbox2_.set_pos(Vector2i(box_rect_.x + (box_rect_.w + padding_) / 2, box_rect_.y));
	vbox1_.set_size((box_rect_.w - padding_) / 2, box_rect_.h);
	vbox2_.set_size((box_rect_.w - padding_) / 2, box_rect_.h);

	// Tell child windows to update their size if necessary
	for (UI::Panel* p = get_first_child(); p != nullptr; p = p->get_next_sibling()) {
		if (upcast(UI::Window, w, p)) {
			if (w->window_layout_id() == UI::Window::WindowLayoutID::kNone) {
				continue;
			}

			const bool minimal = w->is_minimal();
			if (minimal) {
				// make sure the new size is set even if the window is minimal…
				w->restore();
			}

			const int16_t desired_w =
			   calc_desired_window_width(w->window_layout_id()) + p->get_lborder() + p->get_rborder();
			const int16_t desired_h =
			   calc_desired_window_height(w->window_layout_id()) + p->get_tborder() + p->get_bborder();
			w->set_size(desired_w, desired_h);
			w->set_pos(Vector2i(calc_desired_window_x(w->window_layout_id()),
			                    calc_desired_window_y(w->window_layout_id())));

			if (minimal) {
				// …and then make it minimal again if it was minimal before
				w->minimize();
			}
		}
	}
}

bool MainMenu::check_desyncing_addon() {
	for (const auto& pair : AddOns::g_addons) {
		if (!pair.first->sync_safe && pair.second) {
			UI::WLMessageBox mmb(
			   this, UI::WindowStyle::kFsMenu, _("Desyncing Add-On Found"),
			   _("An enabled add-on is known to cause desyncs. Proceed at your own risk."),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			return mmb.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk;
		}
	}
	return false;
}

void MainMenu::action(const MenuTarget t) {
	singleplayer_.set_list_visibility(false);
	multiplayer_.set_list_visibility(false);
	editor_.set_list_visibility(false);

	switch (t) {

	case MenuTarget::kExit:
		exit((SDL_GetModState() & KMOD_CTRL) != 0);
		break;

	case MenuTarget::kOptions: {
		OptionsCtrl o(*this, get_config_section());
		break;
	}
	case MenuTarget::kAddOns:
		r_addons_.toggle();
		break;
	case MenuTarget::kAbout:
		r_about_.toggle();
		break;
	case MenuTarget::kOnlineGameSettings:
		r_login_.toggle();
		break;

	case MenuTarget::kReplay:
		if (Widelands::Game* g = create_safe_game(); g != nullptr) {
			menu_capsule_.clear_content();
			new LoadGame(menu_capsule_, *g, *new SinglePlayerGameSettingsProvider(), true, true);
		}
		break;
	case MenuTarget::kReplayLast:
		if (!filename_for_last_replay_.empty()) {
			std::unique_ptr<Widelands::Game> game(create_safe_game(true));
			if (game != nullptr) {
				game->run_replay(filename_for_last_replay_, "");
			}
		}
		break;
	case MenuTarget::kLoadGame:
		if (Widelands::Game* g = create_safe_game()) {
			menu_capsule_.clear_content();
			new LoadGame(menu_capsule_, *g, *new SinglePlayerGameSettingsProvider(), true, false);
		}
		break;

	case MenuTarget::kNewGame: {
		if (Widelands::Game* g = create_safe_game()) {
			menu_capsule_.clear_content();
			new MapSelect(menu_capsule_, nullptr, new SinglePlayerGameSettingsProvider(), nullptr,
			              std::shared_ptr<Widelands::Game>(g));
		}
		break;
	}

	case MenuTarget::kRandomGame:
		menu_capsule_.clear_content();
		new RandomGame(menu_capsule_);
		break;

	case MenuTarget::kContinueLastsave:
		if (!filename_for_continue_playing_.empty()) {
			std::unique_ptr<Widelands::Game> game(create_safe_game(true));
			if (game != nullptr) {
				game->set_ai_training_mode(get_config_bool("ai_training", false));
				SinglePlayerGameSettingsProvider sp;
				try {
					game->run_load_game(filename_for_continue_playing_, "");
				} catch (const std::exception& e) {
					WLApplication::emergency_save(this, *game, e.what());
				}
			}
			// Update the Continue button in case a new savegame was created
			set_labels();
		}
		break;

	case MenuTarget::kTutorial:
		menu_capsule_.clear_content();
		new ScenarioSelect(menu_capsule_, nullptr);
		break;
	case MenuTarget::kCampaign:
		menu_capsule_.clear_content();
		new CampaignSelect(menu_capsule_);
		break;

	case MenuTarget::kLan:
		if (check_desyncing_addon()) {
			break;
		}
		menu_capsule_.clear_content();
		new NetSetupLAN(menu_capsule_);
		break;
	case MenuTarget::kMetaserver: {
		if (check_desyncing_addon()) {
			break;
		}
		menu_capsule_.clear_content();

		std::vector<Widelands::TribeBasicInfo> tribeinfos = Widelands::get_all_tribeinfos(nullptr);
		if (tribeinfos.empty()) {
			show_messagebox(_("No tribes found!"),
			                _("No tribes found in data/tribes/initialization/[tribename]/init.lua."));
			break;
		}

		internet_login(false);

		get_config_string("nickname", nickname_);
		// Only change the password if we use a registered account
		if (register_) {
			get_config_string("password_sha1", password_);
		}

		new InternetLobby(menu_capsule_, nickname_, password_, register_, tribeinfos);
	} break;

	case MenuTarget::kEditorNew:
		EditorInteractive::run_editor(this, EditorInteractive::Init::kNew);
		set_labels();
		break;
	case MenuTarget::kEditorRandom:
		EditorInteractive::run_editor(this, EditorInteractive::Init::kRandom);
		set_labels();
		break;
	case MenuTarget::kEditorLoad:
		EditorInteractive::run_editor(this, EditorInteractive::Init::kLoad);
		set_labels();
		break;
	case MenuTarget::kEditorContinue: {
		if (!filename_for_continue_editing_.empty()) {
			EditorInteractive::run_editor(
			   this, EditorInteractive::Init::kLoadMapDirectly, filename_for_continue_editing_);
			set_labels();
		}
		break;
	}

	default:
		throw wexception("Invalid MenuTarget %d", static_cast<int>(t));
	}
}

void MainMenu::exit(const bool force) {
	if (!force) {
		UI::WLMessageBox confirmbox(this, UI::WindowStyle::kFsMenu, _("Exit Confirmation"),
		                            _("Are you sure you wish to exit Widelands?"),
		                            UI::WLMessageBox::MBoxType::kOkCancel);
		if (confirmbox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
			return;
		}
	}
	end_modal<MenuTarget>(MenuTarget::kBack);
}

/// called if the user is not registered
void MainMenu::show_internet_login(const bool modal) {
	r_login_.create();
	if (modal) {
		r_login_.window->run<MenuTarget>();
		r_login_.destroy();
	}
}
void MainMenu::internet_login_callback() {
	if (auto_log_) {
		auto_log_ = false;
		internet_login(true);
	}
}

/**
 * Called if "Online Game" button was pressed.
 *
 * IF no nickname or a nickname with invalid characters is set, the Online Game Settings
 * are opened.
 *
 * IF at least a name is set, all data is read from the config file
 *
 * This fullscreen menu ends it's modality.
 */
void MainMenu::internet_login(const bool launch_metaserver) {
	nickname_ = get_config_string("nickname", "");
	password_ = get_config_string("password_sha1", "no_password_set");
	register_ = get_config_bool("registered", false);

	// Checks can be done directly in editbox' by using valid_username().
	// This is just to be on the safe side, in case the user changed the password in the config file.
	if (!InternetGaming::ref().valid_username(nickname_)) {
		auto_log_ = true;
		show_internet_login(true);
		return;
	}

	// Try to connect to the metaserver
	const std::string& meta = get_config_string("metaserver", INTERNET_GAMING_METASERVER);
	uint32_t port = get_config_natural("metaserverport", kInternetGamingPort);
	const std::string& auth = register_ ? password_ : get_config_string("uuid", "");
	assert(!auth.empty() || !register_);
	InternetGaming::ref().login(nickname_, auth, register_, meta, port);

	// Check whether metaserver send some data
	if (InternetGaming::ref().logged_in()) {
		if (launch_metaserver) {
			action(MenuTarget::kMetaserver);
		}
	} else {
		// something went wrong -> show the error message
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		show_messagebox(_("Error!"), msg.msg);

		// Reset InternetGaming and passwort and show internet login again
		InternetGaming::ref().reset();
		set_config_string("password_sha1", "no_password_set");
		auto_log_ = true;
		show_internet_login(true);
	}
}

}  //  namespace FsMenu
