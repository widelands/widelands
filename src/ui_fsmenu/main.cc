/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "build_info.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/single_player_game_settings_provider.h"
#include "map_io/widelands_map_loader.h"
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
#include "wui/savegameloader.h"

namespace FsMenu {

constexpr uint32_t kInitialFadeoutDelay = 2500;
constexpr uint32_t kInitialFadeoutDuration = 4000;
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
		return std::max(360, get_h() * 3 / 8);
	default:
		NEVER_HERE();
	}
}

int16_t MainMenu::calc_desired_window_x(const UI::Window::WindowLayoutID id) {
	return (get_w() - calc_desired_window_width(id)) / 2 - UI::Window::kVerticalBorderThickness;
}

int16_t MainMenu::calc_desired_window_y(const UI::Window::WindowLayoutID id) {
	return (get_h() - calc_desired_window_height(id)) / 2 - UI::Window::kTopBorderThickness;
}

MainMenu::MainMenu(const bool skip_init)
   : UI::Panel(nullptr, UI::PanelStyle::kFsMenu, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     box_rect_(0, 0, 0, 0),
     butw_(get_w() * 7 / 20),
     buth_(get_h() * 9 / 200),
     padding_(buth_ / 3),
     vbox1_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     vbox2_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, padding_),
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
     replay_(&vbox1_, "replay", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, ""),
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
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelParagraph,
              0,
              0,
              0,
              0,
              "",
              UI::Align::kCenter),
     copyright_(this,
                UI::PanelStyle::kFsMenu,
                UI::FontStyle::kFsMenuInfoPanelParagraph,
                0,
                0,
                0,
                0,
                "",
                UI::Align::kCenter),
     init_time_(kNoSplash),
     last_image_exchange_time_(0),
     draw_image_(0),
     last_image_(0),
     visible_(true),
     menu_capsule_(*this),
     auto_log_(false) {
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
	replay_.sigclicked.connect([this]() { action(MenuTarget::kReplay); });
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
		set_button_visibility(false);
	}

	r_login_.open_window = [this]() { new LoginBox(*this, r_login_); };
	r_about_.open_window = [this]() { new About(*this, r_about_); };
	r_addons_.open_window = [this]() { new AddOnsUI::AddOnsCtrl(*this, r_about_); };

	focus();
	set_labels();
	layout();

	initialization_complete();
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

	splashscreen_ = &load_safe_template_image("loadscreens/splash.jpg");
	title_image_ = &load_safe_template_image("loadscreens/logo.png");

	images_.clear();
	for (const std::string& img : g_fs->list_directory(template_dir() + "loadscreens/mainmenu")) {
		images_.push_back(img);
	}
	if (images_.empty()) {
		log_warn("No main menu backgrounds found, using fallback image");
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

void MainMenu::find_maps(const std::string& directory, std::vector<MapEntry>& results) {
	Widelands::Map map;
	for (const std::string& file : g_fs->list_directory(directory)) {
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(file);
		if (ml) {
			try {
				map.set_filename(file);
				ml->preload_map(true, nullptr);
				if (map.version().map_version_timestamp > 0) {
					MapData::MapType type = map.scenario_types() == Map::SP_SCENARIO ?
                                          MapData::MapType::kScenario :
                                          MapData::MapType::kNormal;
					results.emplace_back(
					   MapData(map, file, type, MapData::DisplayType::kFilenames), map.version());
				}
			} catch (...) {
				// invalid file – silently ignore
			}
		} else if (g_fs->is_directory(file)) {
			find_maps(file, results);
		}
	}
}

void MainMenu::set_labels() {
	{
		// TODO(Nordfriese): Code duplication, the same code is used in InteractiveBase
		Section& global_s = get_config_section();
		set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
		set_panel_snap_distance(global_s.get_int("panel_snap_distance", 10));
		set_snap_windows_only_when_overlapping(
		   global_s.get_bool("snap_windows_only_when_overlapping", false));
		set_dock_windows_to_edges(global_s.get_bool("dock_windows_to_edges", false));
	}

	singleplayer_.clear();
	multiplayer_.clear();
	editor_.clear();

	singleplayer_.add(_("New Game"), MenuTarget::kNewGame, nullptr, false, _("Begin a new game"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuNew));
	singleplayer_.add(_("New Random Game"), MenuTarget::kRandomGame, nullptr, false,
	                  _("Create a new random match"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuRandomMatch));
	singleplayer_.add(_("Campaigns"), MenuTarget::kCampaign, nullptr, false, _("Play a campaign"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuCampaign));
	singleplayer_.add(_("Tutorials"), MenuTarget::kTutorial, nullptr, false,
	                  _("Play one of our beginners’ tutorials"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuTutorial));
	singleplayer_.add(_("Load Game"), MenuTarget::kLoadGame, nullptr, false,
	                  _("Continue a saved game"),
	                  shortcut_string_for(KeyboardShortcut::kMainMenuLoad));

	// Refresh the Continue tooltip. The SavegameData must be reloaded after
	// every language switch because it contains localized strings.
	{
		filename_for_continue_playing_ = "";
		std::unique_ptr<Widelands::Game> game(create_safe_game(false));
		if (game != nullptr) {
			SinglePlayerLoader loader(*game);
			std::vector<SavegameData> games = loader.load_files(kSaveDir);
			SavegameData* newest_singleplayer = nullptr;
			for (SavegameData& data : games) {
				if (!data.is_directory() && data.is_singleplayer() &&
				    (newest_singleplayer == nullptr || newest_singleplayer->compare_save_time(data))) {
					newest_singleplayer = &data;
				}
			}
			if (newest_singleplayer) {
				filename_for_continue_playing_ = newest_singleplayer->filename;
				singleplayer_.add(
				   _("Continue Playing"), MenuTarget::kContinueLastsave, nullptr, false,
				   format("%s<br>%s<br>%s<br>%s<br>%s<br>%s",
				          g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
				             .as_font_tag(
				                /* strip leading "save/" and trailing ".wgf" */
				                filename_for_continue_playing_.substr(
				                   kSaveDir.length() + 1, filename_for_continue_playing_.length() -
				                                             kSaveDir.length() -
				                                             kSavegameExtension.length() - 1)),
				          format(_("Map: %s"),
				                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				                    .as_font_tag(newest_singleplayer->mapname)),
				          format(_("Win Condition: %s"),
				                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				                    .as_font_tag(newest_singleplayer->wincondition)),
				          format(_("Players: %s"),
				                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				                    .as_font_tag(newest_singleplayer->nrplayers)),
				          format(_("Gametime: %s"),
				                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				                    .as_font_tag(newest_singleplayer->gametime)),
				          /** TRANSLATORS: Information about when a game was saved, e.g. 'Saved: Today,
				           * 10:30'
				           */
				          format(_("Saved: %s"),
				                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				                    .as_font_tag(newest_singleplayer->savedatestring))),
				   shortcut_string_for(KeyboardShortcut::kMainMenuContinuePlaying));
			}
		}
	}

	multiplayer_.add(_("Online Game"), MenuTarget::kMetaserver, nullptr, false,
	                 _("Join the Widelands lobby"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLobby));
	multiplayer_.add(_("Online Game Settings"), MenuTarget::kOnlineGameSettings, nullptr, false,
	                 _("Log in as a registered user"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLogin));
	multiplayer_.add(_("LAN / Direct IP"), MenuTarget::kLan, nullptr, false,
	                 _("Play a private online game"),
	                 shortcut_string_for(KeyboardShortcut::kMainMenuLAN));

	editor_.add(_("New Map"), MenuTarget::kEditorNew, nullptr, false, _("Create a new empty map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorNew));
	editor_.add(_("Random Map"), MenuTarget::kEditorRandom, nullptr, false,
	            _("Create a new random map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorRandom));
	editor_.add(_("Load Map"), MenuTarget::kEditorLoad, nullptr, false, _("Edit an existing map"),
	            shortcut_string_for(KeyboardShortcut::kMainMenuEditorLoad));

	{
		filename_for_continue_editing_ = "";
		std::vector<MapEntry> v;
		find_maps("maps/My_Maps", v);
		MapEntry* last_edited = nullptr;
		for (MapEntry& m : v) {
			if (last_edited == nullptr ||
			    m.second.map_version_timestamp > last_edited->second.map_version_timestamp) {
				last_edited = &m;
			}
		}
		if (last_edited) {
			filename_for_continue_editing_ = last_edited->first.filename;
			editor_.add(
			   _("Continue Editing"), MenuTarget::kEditorContinue, nullptr, false,
			   format("%s<br>%s<br>%s<br>%s<br>%s",
			          g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
			             .as_font_tag(
			                /* strip leading "maps/My_Maps/" and trailing ".wgf" */
			                filename_for_continue_editing_.substr(
			                   13, filename_for_continue_editing_.length() - 17)),
			          format(_("Name: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(last_edited->first.localized_name)),
			          format(_("Size: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(format(_("%1$u×%2$u"), last_edited->first.width,
			                                        last_edited->first.height))),
			          format(_("Players: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(std::to_string(last_edited->first.nrplayers))),
			          format(_("Description: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(last_edited->first.description))),
			   shortcut_string_for(KeyboardShortcut::kMainMenuContinueEditing));
		}
	}

	singleplayer_.set_label(_("Single Player…"));
	multiplayer_.set_label(_("Multiplayer…"));
	editor_.set_label(_("Editor…"));
	singleplayer_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Begin or load a single-player campaign or free game"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuSP), UI::PanelStyle::kFsMenu));
	multiplayer_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Play with your friends over the internet"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuMP), UI::PanelStyle::kFsMenu));
	editor_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Launch the map editor"), shortcut_string_for(KeyboardShortcut::kMainMenuE),
	   UI::PanelStyle::kFsMenu));

	replay_.set_title(_("Watch Replay"));
	replay_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Watch the replay of an old game"), shortcut_string_for(KeyboardShortcut::kMainMenuReplay),
	   UI::PanelStyle::kFsMenu));

	addons_.set_title(_("Add-Ons"));
	addons_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Install and manage add-ons"), shortcut_string_for(KeyboardShortcut::kMainMenuAddons),
	   UI::PanelStyle::kFsMenu));
	options_.set_title(_("Options"));
	options_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Technical and game-related settings"),
	   shortcut_string_for(KeyboardShortcut::kMainMenuOptions), UI::PanelStyle::kFsMenu));
	about_.set_title(_("About Widelands"));
	about_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Readme, License, and Credits"), shortcut_string_for(KeyboardShortcut::kMainMenuAbout),
	   UI::PanelStyle::kFsMenu));
	exit_.set_title(_("Exit Widelands"));
	exit_.set_tooltip(as_tooltip_text_with_hotkey(
	   _("Quit the game"), shortcut_string_for(KeyboardShortcut::kMainMenuQuit),
	   UI::PanelStyle::kFsMenu));

	version_.set_text(
	   /** TRANSLATORS: %1$s = version string, %2%s = "Debug" or "Release" */
	   format(_("Version %1$s (%2$s)"), build_id(), build_type()));
	copyright_.set_text(
	   /** TRANSLATORS: Placeholders are the copyright years */
	   format(_("(C) %1%-%2% by the Widelands Development Team · Licensed under "
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
}

bool MainMenu::handle_mousepress(uint8_t /*btn*/, int32_t /*x*/, int32_t /*y*/) {
	if (init_time_ != kNoSplash) {
		init_time_ = kNoSplash;
		return true;
	}
	return false;
}

bool MainMenu::handle_key(const bool down, const SDL_Keysym code) {
	// Forward all keys to the open window if there is one
	bool has_open_window = false;
	for (UI::UniqueWindow::Registry* r : {&r_login_, &r_about_, &r_addons_}) {
		if (r->window) {
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

	if (down) {
		bool fell_through = false;
		if (init_time_ != kNoSplash) {
			init_time_ = kNoSplash;
			fell_through = true;
		}

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
		if (check_match_shortcut(KeyboardShortcut::kMainMenuReplay, MenuTarget::kReplay)) {
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
		if (matches_shortcut(KeyboardShortcut::kMainMenuQuit, code)) {
			if (!fell_through) {
				end_modal<MenuTarget>(MenuTarget::kBack);
				return true;
			}
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
		if (code.sym == SDLK_BACKSPACE && (code.mod & (KMOD_CTRL | KMOD_SHIFT))) {
			// Easter egg: Press Ctrl/Shift+Backspace to exchange the background immediately :-)
			last_image_exchange_time_ -=
			   (last_image_exchange_time_ > kImageExchangeInterval ? kImageExchangeInterval :
                                                                  last_image_exchange_time_);
			return true;
		}
	}
	return UI::Panel::handle_key(down, code);
}

inline Rectf MainMenu::image_pos(const Image& i, const bool crop) {
	return UI::fit_image(i.width(), i.height(), get_w(), get_h(), crop);
}

static inline void
do_draw_image(RenderTarget& r, const Rectf& dest, const Image& img, const float opacity) {
	r.blitrect_scale(
	   dest, &img, Recti(0, 0, img.width(), img.height()), opacity, BlendMode::UseAlpha);
}

inline float MainMenu::calc_opacity(const uint32_t time) const {
	return last_image_ == draw_image_ ?
             1.f :
             std::max(0.f, std::min(1.f, static_cast<float>(time - last_image_exchange_time_) /
	                                         kImageExchangeDuration));
}

/*
 * The four phases of the animation:
 *   1) Show the splash image with full opacity on a black background for `kInitialFadeoutDelay`
 *   2) Show the splash image semi-transparent on a black background for `kInitialFadeoutDuration`
 *   3) Show the background & menu semi-transparent for `kInitialFadeoutDuration`
 *   4) Show the background & menu with full opacity indefinitely
 * We skip straight to the last phase 4 if we are returning from some other FsMenu screen.
 */

void MainMenu::draw(RenderTarget& r) {
	UI::Panel::draw(r);
	r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, 255));

	const uint32_t time = SDL_GetTicks();
	assert(init_time_ == kNoSplash || time >= init_time_);

	if (init_time_ != kNoSplash &&
	    time - init_time_ < kInitialFadeoutDelay + kInitialFadeoutDuration) {
		// still in splash phase
		return;
	}

	// Sanitize phase info and button visibility
	if (init_time_ != kNoSplash &&
	    time - init_time_ > kInitialFadeoutDelay + 2 * kInitialFadeoutDuration) {
		init_time_ = kNoSplash;
	}
	if (init_time_ == kNoSplash ||
	    time - init_time_ > kInitialFadeoutDelay + kInitialFadeoutDuration) {
		set_button_visibility(true);
	}

	// Exchange stale background images
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
			const Image& img = *g_image_cache->get(images_[last_image_]);
			opacity = calc_opacity(time);
			do_draw_image(r, image_pos(img), img, 1.f - opacity);
		}

		const Image& img = *g_image_cache->get(images_[draw_image_]);
		do_draw_image(r, image_pos(img), img, opacity);
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
	}

	// Widelands logo
	const Rectf rect = title_pos();
	do_draw_image(
	   r, Rectf(rect.x + rect.w * 0.2f, rect.y + rect.h * 0.2f, rect.w * 0.6f, rect.h * 0.6f),
	   *title_image_, 1.f);
}

void MainMenu::draw_overlay(RenderTarget& r) {
	if (init_time_ == kNoSplash) {
		// overlays are needed only during the first three phases
		return;
	}
	const uint32_t time = SDL_GetTicks();

	if (time - init_time_ < kInitialFadeoutDelay + kInitialFadeoutDuration) {
		const float opacity = time - init_time_ > kInitialFadeoutDelay ?
                               1.f - static_cast<float>(time - init_time_ - kInitialFadeoutDelay) /
		                                  kInitialFadeoutDuration :
                               1.f;
		do_draw_image(r, image_pos(*splashscreen_, false), *splashscreen_, opacity);
	} else {
		const unsigned opacity =
		   255 - 255.f * (time - init_time_ - kInitialFadeoutDelay - kInitialFadeoutDuration) /
		            kInitialFadeoutDuration;
		r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, opacity), BlendMode::Default);
	}
}

inline Rectf MainMenu::title_pos() {
	const float imgh = box_rect_.y / 3.f;
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

	box_rect_ = Recti((get_inner_w() - padding_) / 2 - butw_,
	                  version_.get_y() - padding_ * 5 / 2 - get_inner_h() / 4, 2 * butw_ + padding_,
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
	for (UI::Panel* p = get_first_child(); p; p = p->get_next_sibling()) {
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
		end_modal<MenuTarget>(MenuTarget::kBack);
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
		if (Widelands::Game* g = create_safe_game()) {
			menu_capsule_.clear_content();
			new LoadGame(menu_capsule_, *g, *new SinglePlayerGameSettingsProvider(), true, true);
		}
		break;
	case MenuTarget::kLoadGame:
		if (Widelands::Game* g = create_safe_game()) {
			menu_capsule_.clear_content();
			new LoadGame(menu_capsule_, *g, *new SinglePlayerGameSettingsProvider(), true, false);
		}
		break;

	case MenuTarget::kNewGame:
		if (Widelands::Game* g = create_safe_game()) {
			menu_capsule_.clear_content();
			new MapSelect(menu_capsule_, nullptr, new SinglePlayerGameSettingsProvider(), nullptr, *g);
		}
		break;

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
		g_sh->change_music(Songset::kIngame, 1000);
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

		g_sh->change_music(Songset::kIngame, 1000);
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
