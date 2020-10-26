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
#include "graphic/text_layout.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "map_io/widelands_map_loader.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"
#include "wui/login_box.h"
#include "wui/mapdata.h"
#include "wui/savegameloader.h"

constexpr uint32_t kInitialFadeoutDelay = 2500;
constexpr uint32_t kInitialFadeoutDuration = 4000;
constexpr uint32_t kImageExchangeInterval = 20000;
constexpr uint32_t kImageExchangeDuration = 2500;

constexpr uint32_t kNoSplash = std::numeric_limits<uint32_t>::max();

int16_t calc_desired_window_width(const FullscreenMenuMain& parent) {
	return std::max(600, parent.get_w() / 2);
}

int16_t calc_desired_window_height(const FullscreenMenuMain& parent) {
	return std::max(400, parent.get_h() / 2);
}

FullscreenMenuMain::FullscreenMenuMain(bool first_ever_init)
   : FullscreenMenuBase(),
     box_rect_(0, 0, 0, 0),
     butw_(get_w() * 7 / 20),
     buth_(get_h() * 9 / 200),
     padding_(buth_ / 3),
     vbox1_(this, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     vbox2_(this, 0, 0, UI::Box::Vertical, 0, 0, padding_),
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
              0,
              0,
              0,
              0,
              "",
              UI::Align::kCenter,
              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)),
     copyright_(this,
                0,
                0,
                0,
                0,
                "",
                UI::Align::kCenter,
                g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)),
     splashscreen_(*g_image_cache->get(std::string(kTemplateDir) + "loadscreens/splash.jpg")),
     title_image_(*g_image_cache->get(std::string(kTemplateDir) + "loadscreens/logo.png")),
     init_time_(kNoSplash),
     last_image_exchange_time_(0),
     draw_image_(0),
     last_image_(0),
     visible_(true),
     auto_log_(false) {
	singleplayer_.selected.connect(
	   [this]() { end_modal<FullscreenMenuBase::MenuTarget>(singleplayer_.get_selected()); });
	multiplayer_.selected.connect([this]() {
		internet_login();
		end_modal<FullscreenMenuBase::MenuTarget>(multiplayer_.get_selected());
	});
	editor_.selected.connect(
	   [this]() { end_modal<FullscreenMenuBase::MenuTarget>(editor_.get_selected()); });
	replay_.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kReplay);
	});
	addons_.sigclicked.connect([this]() {
	   end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kAddOns);
	});
	options_.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOptions);
	});
	about_.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kAbout);
	});
	exit_.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kExit);
	});

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
				ml->preload_map(true, nullptr);
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

	singleplayer_.add(_("New Game"), FullscreenMenuBase::MenuTarget::kNewGame, nullptr, false,
	                  _("Begin a new game"), "N");
	singleplayer_.add(_("New Random Game"), FullscreenMenuBase::MenuTarget::kRandomGame, nullptr,
	                  false, _("Create a new random match"), "Z");
	singleplayer_.add(_("Campaigns"), FullscreenMenuBase::MenuTarget::kCampaign, nullptr, false,
	                  _("Play a campaign"), "H");
	singleplayer_.add(_("Tutorials"), FullscreenMenuBase::MenuTarget::kTutorial, nullptr, false,
	                  _("Play one of our beginners’ tutorials"), "T");
	singleplayer_.add(_("Load Game"), FullscreenMenuBase::MenuTarget::kLoadGame, nullptr, false,
	                  _("Continue a saved game"), "L");

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
			   _("Continue Playing"), FullscreenMenuBase::MenuTarget::kContinueLastsave, nullptr,
			   false,
			   (boost::format("%s<br>%s<br>%s<br>%s<br>%s<br>%s") %
			    g_style_manager->font_style(UI::FontStyle::kTooltipHeader)
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
			   "C");
		}
	}

	multiplayer_.add(_("Online Game"), FullscreenMenuBase::MenuTarget::kMetaserver, nullptr, false,
	                 _("Join the Widelands lobby"), "J");
	multiplayer_.add(_("Online Game Settings"), FullscreenMenuBase::MenuTarget::kOnlineGameSettings,
	                 nullptr, false, _("Log in as a registered user"), "U");
	multiplayer_.add(_("LAN / Direct IP"), FullscreenMenuBase::MenuTarget::kLan, nullptr, false,
	                 _("Play a private online game"), "P");

	editor_.add(_("New Map"), FullscreenMenuBase::MenuTarget::kEditorNew, nullptr, false,
	            _("Create a new empty map"), "K");
	editor_.add(_("Random Map"), FullscreenMenuBase::MenuTarget::kEditorRandom, nullptr, false,
	            _("Create a new random map"), "Y");
	editor_.add(_("Load Map"), FullscreenMenuBase::MenuTarget::kEditorLoad, nullptr, false,
	            _("Edit an existing map"), "B");

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
			editor_.add(_("Continue Editing"), FullscreenMenuBase::MenuTarget::kEditorContinue,
			            nullptr, false,
			            (boost::format("%s<br>%s<br>%s<br>%s<br>%s") %
			             g_style_manager->font_style(UI::FontStyle::kTooltipHeader)
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
			            "W");
		}
	}

	singleplayer_.set_label(_("Single Player…"));
	multiplayer_.set_label(_("Multiplayer…"));
	editor_.set_label(_("Editor…"));
	singleplayer_.set_tooltip(
	   as_tooltip_text_with_hotkey(_("Begin or load a single-player campaign or free game"), "S"));
	multiplayer_.set_tooltip(
	   as_tooltip_text_with_hotkey(_("Play with your friends over the internet"), "M"));
	editor_.set_tooltip(as_tooltip_text_with_hotkey(_("Launch the map editor"), "E"));

	replay_.set_title(_("Watch Replay"));
	replay_.set_tooltip(as_tooltip_text_with_hotkey(_("Watch the replay of an old game"), "R"));

	addons_.set_title(_("Add-Ons"));
	addons_.set_tooltip(
	   as_tooltip_text_with_hotkey(_("Install and manage add-ons"), "A"));
	options_.set_title(_("Options"));
	options_.set_tooltip(as_tooltip_text_with_hotkey(_("Technical and game-related settings"), "O"));
	about_.set_title(_("About Widelands"));
	about_.set_tooltip(
	   as_tooltip_text_with_hotkey(_("Readme, License, and Credits"), pgettext("hotkey", "F1")));
	exit_.set_title(_("Exit Widelands"));
	exit_.set_tooltip(as_tooltip_text_with_hotkey(_("Quit the game"), pgettext("hotkey", "Esc")));

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
		switch (code.sym) {
		case SDLK_ESCAPE:
			if (!fell_through) {
				end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
				return true;
			}
			break;
		case SDLK_t:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kTutorial);
			return true;
		case SDLK_c:
			if (!filename_for_continue_playing_.empty()) {
				end_modal<FullscreenMenuBase::MenuTarget>(
				   FullscreenMenuBase::MenuTarget::kContinueLastsave);
				return true;
			}
			break;
		case SDLK_w:
			if (!filename_for_continue_editing_.empty()) {
				end_modal<FullscreenMenuBase::MenuTarget>(
				   FullscreenMenuBase::MenuTarget::kEditorContinue);
				return true;
			}
			break;
		case SDLK_n:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNewGame);
			return true;
		case SDLK_z:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kRandomGame);
			return true;
		case SDLK_h:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kCampaign);
			return true;
		case SDLK_l:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kLoadGame);
			return true;
		case SDLK_j:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
			return true;
		case SDLK_u:
			end_modal<FullscreenMenuBase::MenuTarget>(
			   FullscreenMenuBase::MenuTarget::kOnlineGameSettings);
			return true;
		case SDLK_p:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kLan);
			return true;
		case SDLK_a:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kAddOns);
			return true;
		case SDLK_o:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOptions);
			return true;
		case SDLK_r:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kReplay);
			return true;
		case SDLK_F1:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kAbout);
			return true;
		case SDLK_k:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kEditorNew);
			return true;
		case SDLK_y:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kEditorRandom);
			return true;
		case SDLK_b:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kEditorLoad);
			return true;
		case SDLK_s:
			singleplayer_.toggle();
			return true;
		case SDLK_m:
			multiplayer_.toggle();
			return true;
		case SDLK_e:
			editor_.toggle();
			return true;
		default:
			break;
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
	FullscreenMenuBase::draw(r);
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
	butw_ = get_w() / 5;
	buth_ = get_h() / 25;
	padding_ = buth_ / 3;

	copyright_.set_pos(
	   Vector2i((get_w() - copyright_.get_w()) / 2, get_h() - copyright_.get_h() - padding_ / 2));
	version_.set_pos(Vector2i(
	   (get_w() - version_.get_w()) / 2, copyright_.get_y() - version_.get_h() - padding_ / 2));

	box_rect_ =
	   Recti((get_w() - padding_) / 2 - butw_, version_.get_y() - padding_ * 5 / 2 - get_h() / 4,
	         2 * butw_ + padding_, get_h() / 4);

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
}

/// called if the user is not registered
void FullscreenMenuMain::show_internet_login() {
	LoginBox lb(*this);
	if (lb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk && auto_log_) {
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
		show_internet_login();
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
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
	} else {
		// something went wrong -> show the error message
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		UI::WLMessageBox wmb(this, _("Error!"), msg.msg, UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();

		// Reset InternetGaming and passwort and show internet login again
		InternetGaming::ref().reset();
		set_config_string("password_sha1", "no_password_set");
		auto_log_ = true;
		show_internet_login();
	}
}
