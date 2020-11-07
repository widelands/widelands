/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/main.h"

#include <cstdlib>
#include <memory>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/random.h"
#include "build_info.h"
#include "graphic/graphic.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "map_io/widelands_map_loader.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/login_box.h"
#include "wlapplication_options.h"
#include "wui/mapdata.h"
#include "wui/savegameloader.h"

constexpr uint32_t kInitialFadeoutDelay = 2500;
constexpr uint32_t kInitialFadeoutDuration = 4000;
constexpr uint32_t kImageExchangeInterval = 20000;
constexpr uint32_t kImageExchangeDuration = 2500;

constexpr uint32_t kNoSplash = std::numeric_limits<uint32_t>::max();

int16_t FullscreenMenuMain::calc_desired_window_width(const UI::Window::WindowLayoutID id) {
	switch (id) {
	case UI::Window::WindowLayoutID::kFsMenuDefault:
		return std::max(800, get_w() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuOptions:
	case UI::Window::WindowLayoutID::kFsMenuAbout:
		return std::max(600, get_w() / 2);
	default:
		NEVER_HERE();
	}
}

int16_t FullscreenMenuMain::calc_desired_window_height(const UI::Window::WindowLayoutID id) {
	switch (id) {
	case UI::Window::WindowLayoutID::kFsMenuDefault:
		return std::max(600, get_h() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuAbout:
		return std::max(500, get_h() * 4 / 5);
	case UI::Window::WindowLayoutID::kFsMenuOptions:
		return std::max(400, get_h() / 2);
	default:
		NEVER_HERE();
	}
}

int16_t FullscreenMenuMain::calc_desired_window_x(const UI::Window::WindowLayoutID id) {
	return (get_w() - calc_desired_window_width(id)) / 2 - UI::Window::kVerticalBorderThickness;
}

int16_t FullscreenMenuMain::calc_desired_window_y(const UI::Window::WindowLayoutID id) {
	return (get_h() - calc_desired_window_height(id)) / 2 - UI::Window::kTopBorderThickness;
}

FullscreenMenuMain::FullscreenMenuMain(bool first_ever_init)
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
                   UI::ButtonStyle::kFsMenuMenu),
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
                  UI::ButtonStyle::kFsMenuMenu),
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
             UI::ButtonStyle::kFsMenuMenu),
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
     splashscreen_(*g_image_cache->get(std::string(kTemplateDir) + "loadscreens/splash.jpg")),
     title_image_(*g_image_cache->get(std::string(kTemplateDir) + "loadscreens/logo.png")),
     init_time_(kNoSplash),
     last_image_exchange_time_(0),
     draw_image_(0),
     last_image_(0),
     visible_(true),
     auto_log_(false) {
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
		   layout();
	   });

	singleplayer_.selected.connect(
	   [this]() { end_modal<MenuTarget>(singleplayer_.get_selected()); });
	multiplayer_.selected.connect([this]() { end_modal<MenuTarget>(multiplayer_.get_selected()); });
	editor_.selected.connect([this]() { end_modal<MenuTarget>(editor_.get_selected()); });
	replay_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kReplay); });
	/* addons_.sigclicked.connect([this]() {  // Not yet implemented
	   end_modal<MenuTarget>(MenuTarget::kAddOns);
	}); */
	options_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kOptions); });
	about_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kAbout); });
	exit_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kExit); });

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

	addons_.set_enabled(false);  // Not yet implemented

	for (const std::string& img :
	     g_fs->list_directory(std::string(kTemplateDir) + "loadscreens/mainmenu")) {
		images_.push_back(img);
	}
	last_image_ = draw_image_ = std::rand() % images_.size();  // NOLINT

	if (first_ever_init) {
		init_time_ = SDL_GetTicks();
		set_button_visibility(false);
	} else {
		last_image_exchange_time_ = SDL_GetTicks();
	}

	r_login_.open_window = [this]() { new LoginBox(*this, r_login_); };

	focus();
	set_labels();
	layout();
}

using MapEntry = std::pair<MapData, Widelands::MapVersion>;
static void find_maps(const std::string& directory, std::vector<MapEntry>& results) {
	Widelands::Map map;
	for (const std::string& file : g_fs->list_directory(directory)) {
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(file);
		if (ml) {
			try {
				map.set_filename(file);
				ml->preload_map(true);
				if (map.version().map_version_timestamp > 0) {
					results.push_back(MapEntry(
					   MapData(map, file, MapData::MapType::kNormal, MapData::DisplayType::kFilenames),
					   map.version()));
				}
			} catch (...) {
				// invalid file – silently ignore
			}
		} else if (g_fs->is_directory(file)) {
			find_maps(file, results);
		}
	}
}

void FullscreenMenuMain::set_labels() {
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
		Widelands::Game game;
		SinglePlayerLoader loader(game);
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
			   (boost::format("%s<br>%s<br>%s<br>%s<br>%s<br>%s") %
			    g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
			       .as_font_tag(
			          /* strip leading "save/" and trailing ".wgf" */
			          filename_for_continue_playing_.substr(
			             kSaveDir.length() + 1, filename_for_continue_playing_.length() -
			                                       kSaveDir.length() - kSavegameExtension.length() -
			                                       1)) %
			    (boost::format(_("Map: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(newest_singleplayer->mapname))
			       .str() %
			    (boost::format(_("Win Condition: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(newest_singleplayer->wincondition))
			       .str() %
			    (boost::format(_("Players: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(newest_singleplayer->nrplayers))
			       .str() %
			    (boost::format(_("Gametime: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(newest_singleplayer->gametime))
			       .str() %
			    /** TRANSLATORS: Information about when a game was saved, e.g. 'Saved: Today, 10:30' */
			    (boost::format(_("Saved: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(newest_singleplayer->savedatestring))
			       .str())
			      .str(),
			   shortcut_string_for(KeyboardShortcut::kMainMenuContinuePlaying));
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
			editor_.add(_("Continue Editing"), MenuTarget::kEditorContinue, nullptr, false,
			            (boost::format("%s<br>%s<br>%s<br>%s<br>%s") %
			             g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
			                .as_font_tag(
			                   /* strip leading "maps/My_Maps/" and trailing ".wgf" */
			                   filename_for_continue_editing_.substr(
			                      13, filename_for_continue_editing_.length() - 17)) %
			             (boost::format(_("Name: %s")) %
			              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                 .as_font_tag(last_edited->first.localized_name))
			                .str() %
			             (boost::format(_("Size: %s")) %
			              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                 .as_font_tag((boost::format(_("%1$u×%2$u")) % last_edited->first.width %
			                               last_edited->first.height)
			                                 .str()))
			                .str() %
			             (boost::format(_("Players: %s")) %
			              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                 .as_font_tag(std::to_string(last_edited->first.nrplayers)))
			                .str() %
			             (boost::format(_("Description: %s")) %
			              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                 .as_font_tag(last_edited->first.description))
			                .str())
			               .str(),
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
	addons_.set_tooltip(  // TODO(Nordfriese): Replace with purpose text or add _() markup
	   as_tooltip_text_with_hotkey("This feature is still under development",
	                               shortcut_string_for(KeyboardShortcut::kMainMenuAddons),
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
	   (boost::format(_("Version %1$s (%2$s)")) % build_id().c_str() % build_type().c_str()).str());
	copyright_.set_text(
	   /** TRANSLATORS: Placeholders are the copyright years */
	   (boost::format(_("(C) %1%-%2% by the Widelands Development Team · Licensed under "
	                    "the GNU General Public License V2.0")) %
	    kWidelandsCopyrightStart % kWidelandsCopyrightEnd)
	      .str());
}

void FullscreenMenuMain::set_button_visibility(const bool v) {
	if (visible_ == v) {
		return;
	}
	visible_ = v;
	vbox1_.set_visible(v);
	vbox2_.set_visible(v);
	copyright_.set_visible(v);
	version_.set_visible(v);
}

bool FullscreenMenuMain::handle_mousepress(uint8_t, int32_t, int32_t) {
	if (init_time_ != kNoSplash) {
		init_time_ = kNoSplash;
		return true;
	}
	return false;
}

bool FullscreenMenuMain::handle_key(const bool down, const SDL_Keysym code) {
	if (down) {
		bool fell_through = false;
		if (init_time_ != kNoSplash) {
			init_time_ = kNoSplash;
			fell_through = true;
		}

		auto check_match_shortcut = [this, &code](KeyboardShortcut k, MenuTarget t) {
			if (matches_shortcut(k, code)) {
				end_modal<MenuTarget>(t);
				return true;
			}
			return false;
		};
		if (check_match_shortcut(KeyboardShortcut::kMainMenuNew, MenuTarget::kNewGame)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLoad, MenuTarget::kLoadGame)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuReplay, MenuTarget::kReplay)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuTutorial, MenuTarget::kTutorial)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuRandomMatch, MenuTarget::kRandomGame)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuCampaign, MenuTarget::kCampaign)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLobby, MenuTarget::kMetaserver)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuLAN, MenuTarget::kLan)) { return true; }
		// if (check_match_shortcut(KeyboardShortcut::kMainMenuAddons, MenuTarget::kAddOns)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuOptions, MenuTarget::kOptions)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuAbout, MenuTarget::kAbout)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuEditorNew, MenuTarget::kEditorNew)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuEditorRandom, MenuTarget::kEditorRandom)) { return true; }
		if (check_match_shortcut(KeyboardShortcut::kMainMenuEditorLoad, MenuTarget::kEditorLoad)) { return true; }

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
				end_modal<MenuTarget>(MenuTarget::kContinueLastsave);
				return true;
			}
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuContinueEditing, code)) {
			if (!filename_for_continue_editing_.empty()) {
				end_modal<MenuTarget>(MenuTarget::kEditorContinue);
				return true;
			}
		}
		if (matches_shortcut(KeyboardShortcut::kMainMenuLogin, code)) {
			show_internet_login();
			return true;
		}
		if (code.sym == SDLK_F3) {
			// Easter egg: Press F3 to exchange the background immediately :-)
			last_image_exchange_time_ -=
			   (last_image_exchange_time_ > kImageExchangeInterval ? kImageExchangeInterval :
			                                                         last_image_exchange_time_);
			return true;
		}
	}
	return UI::Panel::handle_key(down, code);
}

inline Rectf FullscreenMenuMain::image_pos(const Image& i) {
	return UI::fit_image(i.width(), i.height(), get_w(), get_h());
}

static inline void
do_draw_image(RenderTarget& r, const Rectf& dest, const Image& img, const float opacity) {
	r.blitrect_scale(
	   dest, &img, Recti(0, 0, img.width(), img.height()), opacity, BlendMode::UseAlpha);
}

inline float FullscreenMenuMain::calc_opacity(const uint32_t time) {
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

void FullscreenMenuMain::draw(RenderTarget& r) {
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
		do {
			draw_image_ = std::rand() % images_.size();  // NOLINT
		} while (draw_image_ == last_image_);
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
	   title_image_, 1.f);
}

void FullscreenMenuMain::draw_overlay(RenderTarget& r) {
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
		do_draw_image(r, image_pos(splashscreen_), splashscreen_, opacity);
	} else {
		const unsigned opacity =
		   255 - 255.f * (time - init_time_ - kInitialFadeoutDelay - kInitialFadeoutDuration) /
		            kInitialFadeoutDuration;
		r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, opacity), BlendMode::Default);
	}
}

inline Rectf FullscreenMenuMain::title_pos() {
	const float imgh = box_rect_.y / 3.f;
	const float imgw = imgh * title_image_.width() / title_image_.height();
	return Rectf((get_w() - imgw) / 2.f, buth_, imgw, imgh);
}

void FullscreenMenuMain::layout() {
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

/// called if the user is not registered
void FullscreenMenuMain::show_internet_login(const bool modal) {
	r_login_.create();
	if (modal) {
		r_login_.window->run<int>();
		r_login_.destroy();
	}
}
void FullscreenMenuMain::internet_login_callback() {
	if (auto_log_) {
		auto_log_ = false;
		internet_login();
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
void FullscreenMenuMain::internet_login() {
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
		end_modal<MenuTarget>(MenuTarget::kMetaserver);
	} else {
		// something went wrong -> show the error message
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		UI::WLMessageBox wmb(
		   this, UI::WindowStyle::kFsMenu, _("Error!"), msg.msg, UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();

		// Reset InternetGaming and passwort and show internet login again
		InternetGaming::ref().reset();
		set_config_string("password_sha1", "no_password_set");
		auto_log_ = true;
		show_internet_login(true);
	}
}
